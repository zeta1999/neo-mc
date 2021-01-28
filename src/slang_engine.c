/*
   Implements S-Lang scripting for MC.

   Copyright (C) 2021
   Free Software Foundation, Inc.

   Written by:
   Sebastian Gniazdowski, 2021

   This file is part of the Midnight Commander.

   The Midnight Commander is free software: you can redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the License,
   or (at your option) any later version.

   The Midnight Commander is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \file slang_engine.c
 *  \brief Internals of the S-Lang scripting for MC
 *  \author Sebastian Gniazdowski
 *  \date 2021
 *
 *  Functions that initialize S-Lang interpreter, load init.sl initialization script, load all
 *  plugins from the plugin directory, catch errors and display them in dialogs, etc.
 */

#include <config.h>

#include <slang.h>
#include <ctype.h>

#include "lib/global.h"

#include "lib/fileloc.h"
#include "lib/widget.h"
#include "lib/widget/wtools.h"
#include "lib/sub-util.h"
#include "lib/logging.h"
#include "src/slang_engine.h"
#include "src/filemanager/fman-dir.h"
#include "src/util.h"

/*** global variables ****************************************************************************/

int num_action_hook_functions = 0;
GHashTable *action_hook_functions = NULL;

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

static GString *last_error = NULL;
static char *last_fname = NULL;

/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

static void
slang_error_handler (char *error_msg)
{
    if (!last_error)
        last_error = g_string_sized_new (255);

    /* Truncate previous message. */
    g_string_truncate (last_error, 0);
    /* Append  error message (copying it) and a two new lines. */
    g_string_append (last_error, "Error in script:");

    /* Does message contain a file path? If so, append a newline, else a space. */
    if (strchr (error_msg, '/'))
        g_string_append_c (last_error, '\n');
    else
        g_string_append_c (last_error, ' ');

    /* Append message. */
    g_string_append (last_error, error_msg);
    g_string_append (last_error, "\n\n");

    /* Append a traceback header. */
    g_string_append (last_error, "Traceback:\n");
}

/* --------------------------------------------------------------------------------------------- */

static void
slang_dump_handler (char *traceback)
{
    char *last_fragment_search_str, **tb_bits;

    if (!last_error)
        last_error = g_string_sized_new (255);

    /* Split to check for `/path:<number>:other` pattern. If detected, add preceding new line. */
    tb_bits = g_strsplit (traceback, ":", 3);
    if (strchr (tb_bits[0], '/') != NULL && tb_bits[1] && isdigit (tb_bits[1][0]) && tb_bits[2])
        g_string_append (last_error, "\n");
    g_strfreev (tb_bits);

    /* Append traceback into error message buffer. */
    if (traceback)
        g_string_append (last_error, traceback);

    /*  Search string to detect last part of traceback (i.e.: if it's top function traceback). */
    last_fragment_search_str = g_strdup_printf (":%s:", last_fname ? last_fname : "");

    /* If it's final message, then schedule a message display with complete error buffer. */
    if (g_strstr_len (traceback, -1, last_fragment_search_str) != NULL ||
        g_strstr_len (traceback, -1, ":<top-level>:") != NULL)
        postponed_message (D_ERROR, " S-Lang script error ", "%s", last_error->str);

    g_free (last_fragment_search_str);
}

/* --------------------------------------------------------------------------------------------- */

static void
slang_exit_hook_handler (char *fname)
{
    /* g_free is NULL safe, no need to check. */
    g_free (last_fname);
    last_fname = g_strdup (fname);
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* Slirp doesn't provide a header so the declaration is placed here to mute warning. */
int init_slang_api_functions_module_ns (char *ns_name);

int
slang_init_engine (void)
{
    vfs_path_t *slang_init_vpath, *slang_init_vpath_system;
    long mc_loglevel = mc_global.log_level;

    /* Handler for the (generated first) basic error message. */
    SLang_Error_Hook = slang_error_handler;
    /* Handler for the (generated second) detailed traceback message. */
    SLang_Dump_Routine = slang_dump_handler;
    /* Used to detect the last fragment of a traceback. */
    SLang_Exit_Function = slang_exit_hook_handler;

    /* Set traceback level - do a full traceback via the dump handler. */
    SLang_Traceback = SL_TB_FULL;

    /* Create the hash table for hooks. */
    action_hook_functions = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, NULL);

    /* Init S-Lang subsystems. */
    if (-1 == SLang_init_all ())
        return FALSE;

    /* Add `mc_loglevel` variable that controls execution information messages. */
    if (-1 == SLadd_intrinsic_variable ((char *) "mc_loglevel",
                                        (VOID_STAR) & mc_global.log_level, SLANG_LONG_TYPE, 0))
    {
        postponed_message (D_ERROR, "MC Plugin Subsystem Error", "%s\n%s",
                           "Couldn't add mc_loglevel to S-Lang interpreter.",
                           "Something is very wrong.");
    }

    /* Init the `mc` namespace. */
    init_slang_api_functions_module_ns ((char *) "mc");

    /* Establish path to the init.sl file. */
    slang_init_vpath_system =
        vfs_path_build_filename (mc_global.sysconfig_dir, MC_SLANG_INIT_FILE, (char *) NULL);
    slang_init_vpath =
        mc_config_get_full_vpath(MC_SLANG_INIT_FILE);
    check_for_default(slang_init_vpath_system, slang_init_vpath);

    /*
     * The loglevel can be set most early via environment (MC_LOGLEVEL), and later
     * but still quite early in init.sl.
     */
    if (mc_loglevel >= 2)
        mc_always_log("Loading slang init script from: %s", vfs_path_as_str(slang_init_vpath));

    /* Source `init.sl` into the S-Lang interpreter. */
    if (-1 == SLang_load_file ((char *) vfs_path_as_str(slang_init_vpath)))
    {
        /* Clear the error and reset the interpreter */
        SLang_restart (1);
        SLang_set_error (0);
        /* Refresh the helper var in case its origin changed. */
        mc_loglevel = mc_global.log_level;
        if (mc_loglevel >= 2)
            mc_always_log("Couldn't load init.sl startup script.");
        if (mc_loglevel >= 3)
            postponed_message (D_ERROR, "MC Plugin Subsystem Error",
                               "Couldn't load init.sl startup script.");
    }
    else
    {
        mc_loglevel = mc_global.log_level;
        if (mc_loglevel >= 2)
            mc_always_log("Correctly loaded init.sl startup script.");
        if (mc_loglevel >= 3)
            postponed_message (D_NORMAL, "MC Plugin Subsystem",
                               "Correctly loaded init.sl startup script.");
    }
    return 1;
}

/* --------------------------------------------------------------------------------------------- */

int
slang_plugins_init (void)
{
    long mc_loglevel = mc_global.log_level;
    int ret = TRUE,             /* Default is OK return value */
        i;
    const char *plugin_dir_path;
    dir_list dir;
    vfs_path_t *path;
    file_entry_t *ptr;
    dir_sort_options_t sort_options = { 0, 1, 0 };      /* A case-sensitive sort */

    dir.size = DIR_LIST_MIN_SIZE;
    dir.list = g_new (file_entry_t, DIR_LIST_MIN_SIZE);
    dir.len = 0;
    dir.callback = NULL;

    /* Get the VFS path for the plugins directory. */
    plugin_dir_path = mc_config_get_full_path (MC_PLUGIN_DIR);
    path = vfs_path_from_str (plugin_dir_path);

    if (!dir_list_load (&dir, path, (GCompareFunc) strcmp, &sort_options, "*.sl"))
    {
        postponed_message (D_ERROR, "MC Plugin Subsystem", "Couldn't read the plugin folder.");
        return FALSE;
    }

    for (i = 0, ptr = dir.list; i < dir.len; i++, ptr++)
    {
        char *pl_path;

        if (DIR_IS_DOT (ptr->fname) || DIR_IS_DOTDOT (ptr->fname))
            continue;
        /* Construct full path to the found plugin (a text file with an .sl extension) script. */
        pl_path = mc_build_filename (plugin_dir_path, ptr->fname, (char *) NULL);

        if (!pl_path)
            continue;

        /* Load the script with the interpreter. */
        if (-1 == SLang_load_file ((char *) pl_path) || SLang_get_error ())
        {
            /* Error occurred: clear the error and reset the interpreter. */
            SLang_restart (1);
            SLang_set_error (0);
            /* Refresh local loglevel in case its origin was changed by  script. */
            mc_loglevel = mc_global.log_level;
            if (mc_loglevel >= 3)
                postponed_message (D_ERROR, "MC Plugin Subsystem", "Couldn't load the plugin: %s",
                                   pl_path);
            if (mc_loglevel >= 2)
                mc_always_log("Warning: Couldn't load plugin: %s", pl_path);
            ret = FALSE;
        }
        else
        {
            mc_loglevel = mc_global.log_level;
            /* Report if the loglevel is 2 or more. */
            if (mc_loglevel >= 2)
                postponed_message (D_NORMAL, "MC Plugin Subsystem", "Plugin: %s loaded correctly",
                                   pl_path);
            if (mc_loglevel >= 2)
                mc_always_log("Plugin %s loaded correctly", pl_path);
        }
        g_free (pl_path);
    }

    return ret;
}

/* --------------------------------------------------------------------------------------------- */

/* A function that looks up the command associated S-Lang code callback. */
GSList *
get_command_callback (int ck_id)
{
    GSList *value = g_hash_table_lookup (action_hook_functions, GINT_TO_POINTER (ck_id));
    return value;
}

/* --------------------------------------------------------------------------------------------- */
