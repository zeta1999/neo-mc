/*
   Implementation of functions supplied to S-Lang interpreter.

   Copyright (C) 2021
   Free Software Foundation, Inc.

   Written by:
   Sebastian Gniazdowski <sgniazdowski@gmail.com>, 2021

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

/** \file slang_api_functions.c
 *  \brief Implementation of functions exported/added to S-Lang interpreter.
 *  \author Sebastian Gniazdowski
 *  \date 2021
 *
 *  Here are functions implemented that are automatically exported to S-Lang interpreter by Slirp
 *  utility. They connect the interpreter with mc process and allow to direct it to perform some
 *  tasks like getting a word from current buffer, moving a cursor, etc.
 */


#include <config.h>

#include "lib/global.h"
#include "lib/widget.h"
#include "lib/sub-util.h"

#include "src/slang_engine.h"
#include "src/slang_api_functions.h"
#include "src/keybind-defaults.h"
#include "src/editor/editwidget.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* Gets  current editor object. */
static WEdit *
get_cure (void)
{
    GList *dialog = top_dlg;

    /* Search first fullscreen dialog */
    for (; dialog != NULL; dialog = g_list_next (dialog))
        if ((WIDGET (dialog->data)->pos_flags & WPOS_FULLSCREEN) != 0)
            break;
    if (dialog)
        return (WEdit *) (GROUP (dialog->data)->current->data);
    else
        return NULL;
}

/* --------------------------------------------------------------------------------------------- */

/*
 * Updates  possibly reallocated keymap pointer in all open editors.
 */

static gboolean
update_editor_keymaps (global_keymap_t * keymap)
{
    WEdit *e;
    WGroup *g;
    gboolean ret = FALSE;

    e = get_cure ();
    if (e == NULL)
        return ret;
    g = GROUP (WIDGET (e)->owner);
    if (g == NULL)
        return ret;

    editor_map = keymap;
    for (GList * it = g->widgets; it != NULL; it = g_list_next (it))
    {
        if (edit_widget_is_editor (WIDGET (it->data)))
        {
            WIDGET (it->data)->keymap = keymap;
            ret = TRUE;
        }
    }

    return ret;
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/*
 * S-LANG FUNCTION: _cure_cursor_move(offset)
 */

void
slang_api__cure_cursor_move (int offset)
{
    edit_cursor_move (get_cure (), offset);
}

/* --------------------------------------------------------------------------------------------- */

/*
 * S-LANG FUNCTION: cure_cursor_offset()
 * RETURN VALUE: The offset in bytes of the current cursor position
 */

int
slang_api__cure_cursor_offset (void)
{
    return get_cure ()->buffer.curs1;
}

/* --------------------------------------------------------------------------------------------- */

/*
 * S-LANG FUNCTION: cure_get_eol(byte_idx)
 * RETURN VALUE: the byte offset of the end of the current line
 */

int
slang_api__cure_get_eol (void)
{
    return edit_buffer_get_current_eol (&get_cure ()->buffer);
}

/* --------------------------------------------------------------------------------------------- */

/*
 * S-LANG FUNCTION: cure_get_bol(byte_idx)
 * RETURN VALUE: the byte offset of the beginning of the current line
 */

int
slang_api__cure_get_bol (void)
{
    return edit_buffer_get_current_bol (&get_cure ()->buffer);
}

/* --------------------------------------------------------------------------------------------- */

/*
 * S-LANG FUNCTION: cure_get_byte(byte_idx)
 * RETURN VALUE: the byte at the given index in the currently open file
 */

int
slang_api__cure_get_byte (int byte_idx)
{
    return edit_buffer_get_byte (&get_cure ()->buffer, byte_idx);
}

/* --------------------------------------------------------------------------------------------- */

/*
 * S-LANG FUNCTION: cure_get_left_whole_word(byte_idx)
 * RETURN VALUE: word the byte at the given index in the currently open file
 */

char *
slang_api__cure_get_left_whole_word (int skip_space)
{
    GString *res_gstr;
    char *res;
    res_gstr = edit_buffer_get_left_whole_word (&get_cure ()->buffer, skip_space, NULL, FALSE);

    /* Ensure a non-null result. */
    if (!res_gstr)
        res_gstr = g_string_new ("");

    /* Return inner buffer of GString. */
    res = res_gstr->str;
    g_string_free (res_gstr, FALSE);

    return res;
}

/* --------------------------------------------------------------------------------------------- */

/*
 * S-LANG FUNCTION: cure_delete()
 * RETURN VALUE: The code of the deleted character
 */

int
slang_api__cure_delete (void)
{
    return edit_buffer_delete (&get_cure ()->buffer);
}

/* --------------------------------------------------------------------------------------------- */

/*
 * S-LANG FUNCTION: cure_backspace()
 * RETURN VALUE: The code of the deleted character
 */

int
slang_api__cure_backspace (void)
{
    return edit_buffer_backspace (&get_cure ()->buffer);
}

/* --------------------------------------------------------------------------------------------- */

/*
 * S-LANG FUNCTION: cure_insert_ahead()
 */

void
slang_api__cure_insert_ahead (int c)
{
    edit_buffer_insert_ahead (&get_cure ()->buffer, c);
}

/* --------------------------------------------------------------------------------------------- */

/*
 * S-LANG FUNCTION: listbox( width, height, [ items…, NULL ], help_anchor )
 * RETURN VALUE: the index of the selected element or -1 if cancelled.
 */

/* A S-Lang script function which allows to display a centered dialog with a listbox
   in it and then get the index of the selected item, if any. */
int
slang_api__listbox (int h, int w, char *title, char **items, unsigned long size)
{
    Listbox *listb;
    char **cur_item;
    int selected;
    unsigned long i = 0;

    /* Use an utility function to create the dialog with the listbox in it. */
    listb = create_listbox_window (h, w, title, "");

    /* Add the requested elements to the listbox. */
    for (cur_item = items; i < size && *cur_item != NULL; i++, cur_item++)
    {
        listbox_add_item (listb->list, LISTBOX_APPEND_AT_END, 's', *cur_item, NULL, FALSE);
    }

    /* Run the dialog and get and then return the index of the selected item. */
    selected = run_listbox (listb);
    return selected;
}

/* --------------------------------------------------------------------------------------------- */

/*
 * S-LANG FUNCTION: listbox_with_data( width, height, [ items…, NULL ], [ data…, NULL ], help_anchor )
 * RETURN VALUE: the associated data of the selected element or NULL if cancelled.
 */

/* A S-Lang script function which allows to display a centered dialog with a listbox
   in it and then get the associated data of the selected item, if any (otherwise,
   i.e.: if canceled, it returns "". The data has to be an array of char* strings
   (ended with NULL). */
char *
slang_api__listbox_with_data (int h, int w, char *title, char **items, unsigned long size,
                              char **data, unsigned long size2)
{
    Listbox *listb;
    char **cur_item, **cur_data = data;
    char *selected;
    unsigned long p, q = 0;

    /* Use an utility function to create the dialog with the listbox in it. */
    listb = create_listbox_window (h, w, title, "");

    /* Add the requested elements to the listbox, also passing the associated data. */
    for (p = 0, cur_item = items; p < size && *cur_item != NULL; p++, cur_item++)
    {
        listbox_add_item (listb->list, LISTBOX_APPEND_AT_END, 's', *cur_item, *cur_data, FALSE);

        /* Advance the item data pointer, respecting its size. */
        if (q < size2)
        {
            q++;
            cur_data++;
        }
    }

    /* Run the dialog and get and then return the associated data of the selected
       item, duplicating its allocation (for S-Lang scripting host memory
       management). The data has to be a char* string. */
    selected = run_listbox_with_data (listb, NULL);
    return selected ? g_strdup (selected) : NULL;       //g_strdup("");
}

/* --------------------------------------------------------------------------------------------- */

/*
 * S-LANG FUNCTION: listbox_auto(title, [items…, NULL])
 * RETURN VALUE: the index of the selected item or -1 if cancelled.
 */

/*
 * Shows a listbox with the height and width automatically adapted for the
 * contents and the title.
 */
int
slang_api__listbox_auto (char *title, char **items, unsigned long size)
{
    return slang_api__listbox (5, 5, title, items, size);
}

/* --------------------------------------------------------------------------------------------- */

/*
 * S-LANG FUNCTION: message(title, body)
 */

/* Shows a message popup with the given title and body. */
void
slang_api__message (const char *title, const char *body)
{
    message (D_NORMAL, title, "%s", body);
}

/* --------------------------------------------------------------------------------------------- */

/*
 * S-LANG FUNCTION: set_action_hook(command, function_name, user_data)
 * RETURN VALUE: The number of the registered callbacks after the addition
 */

int
slang_api__set_action_hook (const char *command, const char *function_name, const char *user_data)
{
    int value, ins_ret;
    GSList *new_element = NULL;

    /* Get the integer value code for the command name `command`. */
    value = keybind_lookup_action (command);

    new_element = g_slist_append (new_element, (char *) function_name);
    new_element = g_slist_append (new_element, (char *) user_data);

    /* Insert the `function_name` into the hash table. */
    ins_ret = g_hash_table_insert (action_hook_functions, GINT_TO_POINTER (value), new_element);

    /* Increase the element count accordingly. */
    num_action_hook_functions += ins_ret;
    return num_action_hook_functions;
}

/* --------------------------------------------------------------------------------------------- */

/*
 * S-LANG FUNCTION: editor_map_key(key_combination, command_name)
 * RETURN VALUE: 1 or 0 indicating if the binding was properly added
 */
int
slang_api__editor_map_key_to_action (const char *command_name, const char *key_combination)
{
    int value;
    value = keybind_lookup_action (command_name);
    if (value == CK_IgnoreKey)
        /* No such command found - return 0. */
        return FALSE;

    /* Overriding the bindings coming from a keymap *file* is not allowed for S-Lang plugins. */
    if (keybind_lookup_keymap_origin (editor_map, value) != ORIGIN_FILE)
    {
        keybind_cmd_bind (editor_keymap, key_combination, value, ORIGIN_SLANG_SCRIPT);
        /* Update editors' keymap pointers  to the GArray's data in case it is reallocated. */
        update_editor_keymaps ((global_keymap_t *) editor_keymap->data);
    }
    else
    {
        /* Returning -1 will mean that action exists, but its key mapping cannot be altered. */
        return -TRUE;
    }
    return TRUE;
}

/* --------------------------------------------------------------------------------------------- */

/*
 * S-LANG FUNCTION: editor_map_key_to_func(new_command_name, key_combination, function_name)
 * RETURN VALUE: 1
 *
 * This is a convenience function that equals to calling the following 3 function
 *   mc->add_new_action("CustomCommand", <some-int-ID>);
 *   mc->set_action_hook("CustomCommand", "a_slang_function_name");
 *   mc->editor_map_key("CustomCommand", "alt-y");
 *
 * with the only difference that the ID of the command is chosen automatically.
 */
int
slang_api__editor_map_key_to_func (const char *new_command_name,
                                   const char *key_combination, const char *function_name)
{
    int ret;
    keybind_add_new_action (new_command_name, new_dynamic_command_id++);
    slang_api__set_action_hook (new_command_name, function_name, NULL);
    ret = slang_api__editor_map_key_to_action (new_command_name, key_combination);

    return ret;
}

/* --------------------------------------------------------------------------------------------- */
