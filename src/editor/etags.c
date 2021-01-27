/*
   Editor C-code navigation via tags.
   make TAGS file via command:
   $ find . -type f -name "*.[ch]" | etags -l c --declarations -

   or, if etags utility not installed:
   $ find . -type f -name "*.[ch]" | ctags --c-kinds=+p --fields=+iaS --extra=+q -e -L-

   Copyright (C) 2009-2020
   Free Software Foundation, Inc.

   Written by:
   Ilia Maslakov <il.smind@gmail.com>, 2009
   Slava Zanko <slavazanko@gmail.com>, 2009

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

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <ctype.h>

#include "lib/global.h"
#include "lib/sub-util.h"       /* canonicalize_pathname() */
#include "lib/fileloc.h"
#include "lib/strutil.h"

#include "etags.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

int
etags_locate_tags_file (char **tagfile_return, char **path_return)
{
    char *tagfile = *tagfile_return, *path = *path_return, *ptr = NULL;
    int search_result = 0;

    /* Recursive search file 'TAGS' in parent dirs */
    do
    {
        ptr = g_path_get_dirname (path);
        g_free (path);
        path = ptr;
        g_free (tagfile);
        tagfile = mc_build_filename (path, TAGS_NAME, (char *) NULL);
        if (exist_file (tagfile))
        {
            search_result = 1;
            break;
        }
    }
    while (strcmp (path, PATH_SEP_STR) != 0);

    *tagfile_return = tagfile;
    *path_return = path;

    return search_result;
}

static gboolean
parse_define (const char *buf, char **long_name, char **short_name, long *line)
{
    /* *INDENT-OFF* */
    enum
    {
        in_longname,
        in_shortname,
        in_shortname_first_char,
        in_line, finish
    } def_state = in_longname;
    /* *INDENT-ON* */

    static char longdef[LONG_DEF_LEN];
    static char shortdef[SHORT_DEF_LEN];
    static char linedef[LINE_DEF_LEN];
    int nlong = 0;
    int nshort = 0;
    int nline = 0;
    char c = *buf;

    while (!(c == '\0' || c == '\n'))
    {
        switch (def_state)
        {
        case in_longname:
            if (c == 0x01)
            {
                def_state = in_line;
            }
            else if (c == 0x7F)
            {
                def_state = in_shortname;
            }
            else
            {
                if (nlong < LONG_DEF_LEN - 1)
                {
                    longdef[nlong++] = c;
                }
            }
            break;
        case in_shortname_first_char:
            if (isdigit (c))
            {
                nshort = 0;
                buf--;
                def_state = in_line;
            }
            else if (c == 0x01)
            {
                def_state = in_line;
            }
            else
            {
                if (nshort < SHORT_DEF_LEN - 1)
                {
                    shortdef[nshort++] = c;
                    def_state = in_shortname;
                }
            }
            break;
        case in_shortname:
            if (c == 0x01)
            {
                def_state = in_line;
            }
            else if (c == '\n')
            {
                def_state = finish;
            }
            else
            {
                if (nshort < SHORT_DEF_LEN - 1)
                {
                    shortdef[nshort++] = c;
                }
            }
            break;
        case in_line:
            if (c == ',' || c == '\n')
            {
                def_state = finish;
            }
            else if (isdigit (c))
            {
                if (nline < LINE_DEF_LEN - 1)
                {
                    linedef[nline++] = c;
                }
            }
            break;
        case finish:
            longdef[nlong] = '\0';
            shortdef[nshort] = '\0';
            linedef[nline] = '\0';
            *long_name = longdef;
            *short_name = shortdef;
            *line = atol (linedef);
            return TRUE;
        default:
            break;
        }
        buf++;
        c = *buf;
    }
    *long_name = NULL;
    *short_name = NULL;
    *line = 0;
    return FALSE;
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* Fills the etags info array with ·all· objects of given ·type· (functions, etc.) */
int
etags_get_objects_for_file (etags_rank_t type, const char *tagfile,
                            const char *start_path, const char *match_filename,
                            etags_hash_t * functions_hash, int *max_len_return, int size_limit)
{
    /* *INDENT-OFF* */
    enum
    {
        start,
        in_filename,
        in_define
    } state = start;
    /* *INDENT-ON* */

    FILE *f;
    char buf[BUF_LARGE];

    int num = 0;                /* returned value */
    char *filename = NULL;

    if (!match_filename || !tagfile)
        return 0;

    *max_len_return = 0;

    /* open file with positions */
    f = fopen (tagfile, "r");
    if (f == NULL)
        return 0;

    while (fgets (buf, sizeof (buf), f))
    {
        switch (state)
        {
        case start:
            if (buf[0] == 0x0C)
            {
                state = in_filename;
            }
            break;
        case in_filename:
            {
                size_t pos;

                pos = strcspn (buf, ",");
                g_free (filename);
                filename = g_strndup (buf, pos);
                state = in_define;
                break;
            }
        case in_define:
            if (buf[0] == 0x0C)
            {
                state = in_filename;
                break;
            }
            /* check if the filename matches the requested one */
            if (strcmp (filename, match_filename) == 0)
            {
                char *longname = NULL;
                char *shortname = NULL;
                long line = 0;

                parse_define (buf, &longname, &shortname, &line);
                if (num < size_limit - 1)
                {
                    gboolean can_be_func, can_be_var, can_be_type, is_other;

                    /* Prepare the work variable. */
                    char *longname_wr;
                    longname_wr = g_strdup (longname);

                    /* Function – if there's '(' in the declaration. */
                    can_be_func = strstr (longname, "(") != NULL;
                    /* Variable – if there's no parens and no # in the declaration. */
                    can_be_var = strstr (g_strdelimit (longname_wr, "}{()#", ''), "") == NULL;
                    /* Type – if there's a 'struct', 'typedef', 'enum' or '}' in the declaration. */
                    can_be_type = (strstr (longname, "struct ") ||
                                   strstr (longname, "typedef ") ||
                                   strstr (longname, "enum ")) ||
                        (strstr (longname, "}") &&
                         (g_str_has_suffix (shortname, "_t") ||
                          g_str_has_suffix (shortname, "_type")));
                    /* Other kind – nor any of the above. */
                    is_other = !can_be_func && !can_be_var && !can_be_type;

                    /* Renew the work variable. */
                    g_free (longname_wr);
                    longname_wr = g_strdup (longname);

                    /* A closer examination of type tags. */
                    if (type == TAG_RANK_TYPES && can_be_type && !can_be_func)
                    {
                        /*
                         * Verify if it's not a struct variable or an enum.
                         * It filters out occurrences such as:
                         * – struct type SHORTNAME … – i.e.: the shortname at 3rd position, because
                         *   this means that a struct variable, not a struct type is being defined.
                         */
                        gchar **words =
                            g_strsplit (str_collapse_whitespace (longname_wr, ' '), " ", -1);
                        if (words[2] && strcmp (words[2], shortname) == 0)
                            can_be_type = FALSE;
                        g_strfreev (words);
                    }

                    /* A closer examination of variable tags. */
                    if (type == TAG_RANK_VARIABLES && can_be_var)
                    {
                        /* Verify if it's not a struct typedef or an enum. */
                        gchar **words =
                            g_strsplit (str_collapse_whitespace (longname_wr, ' '), " ", -1);
                        if (strcmp (words[0], "typedef") == 0 || !words[0] || !words[1])
                            can_be_var = FALSE;
                        /* Most probably an enum ENUM = 0|1|… assignment. */
                        if (!words[0] || strstr (words[0], "=") || (words[1] && words[1][0] == '='))
                            can_be_var = FALSE;
                        g_strfreev (words);
                    }

                    /* Free the work variable. */
                    g_free (longname_wr);

                    /* Is the object of the requested type? */
                    if (type == TAG_RANK_ANY ||
                        ((type == TAG_RANK_FUNCTIONS && can_be_func) ||
                         (type == TAG_RANK_VARIABLES && can_be_var) ||
                         (type == TAG_RANK_TYPES && can_be_type) ||
                         (type == TAG_RANK_OTHER && is_other)))
                    {
                        /* Update the max. length return variable */
                        int max_len_candidate;
                        max_len_candidate = strlen (shortname);
                        if (*max_len_return < max_len_candidate)
                            *max_len_return = max_len_candidate;

                        /* Save the filename. */
                        functions_hash[num].filename = g_strdup (filename);
                        functions_hash[num].filename_len = strlen (filename);

                        /* Save and canonicalize the path to the file. */
                        functions_hash[num].fullpath =
                            mc_build_filename (start_path, filename, (char *) NULL);
                        canonicalize_pathname (functions_hash[num].fullpath);

                        /* Save the short define. */
                        if (shortname)
                            functions_hash[num].short_define = g_strdup (shortname);
                        else
                            functions_hash[num].short_define = g_strdup (longname);

                        /* Save the line number. */
                        functions_hash[num].line = line;

                        /* Increase the count of the matched objects. */
                        num++;
                    }
                }
            }
            break;
        default:
            break;
        }
    }

    g_free (filename);
    fclose (f);
    return num;
}

/* --------------------------------------------------------------------------------------------- */

int
etags_set_definition_hash (const char *tagfile, const char *start_path,
                           const char *match_func, etags_hash_t * def_hash)
{
    /* *INDENT-OFF* */
    enum
    {
        start,
        in_filename,
        in_define
    } state = start;
    /* *INDENT-ON* */

    FILE *f;
    char buf[BUF_LARGE];

    char *chekedstr = NULL;

    int num = 0;                /* returned value */
    char *filename = NULL;

    if (!match_func || !tagfile)
        return 0;

    /* open file with positions */
    f = fopen (tagfile, "r");
    if (f == NULL)
        return 0;

    while (fgets (buf, sizeof (buf), f))
    {
        switch (state)
        {
        case start:
            if (buf[0] == 0x0C)
            {
                state = in_filename;
            }
            break;
        case in_filename:
            {
                size_t pos;

                pos = strcspn (buf, ",");
                g_free (filename);
                filename = g_strndup (buf, pos);
                state = in_define;
                break;
            }
        case in_define:
            if (buf[0] == 0x0C)
            {
                state = in_filename;
                break;
            }
            /* check if the filename matches the define pos */
            chekedstr = strstr (buf, match_func);
            if (chekedstr)
            {
                char *longname = NULL;
                char *shortname = NULL;
                long line = 0;

                parse_define (chekedstr, &longname, &shortname, &line);
                if (num < MAX_TAG_OBJECTS - 1)
                {
                    def_hash[num].filename_len = strlen (filename);
                    def_hash[num].fullpath =
                        mc_build_filename (start_path, filename, (char *) NULL);

                    canonicalize_pathname (def_hash[num].fullpath);
                    def_hash[num].filename = g_strdup (filename);
                    def_hash[num].short_define = g_strdup (shortname ? shortname : longname);
                    def_hash[num].line = line;
                    num++;
                }
            }
            break;
        default:
            break;
        }
    }

    g_free (filename);
    fclose (f);
    return num;
}

/* --------------------------------------------------------------------------------------------- */
