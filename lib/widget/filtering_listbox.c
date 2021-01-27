/*
   A class extending WListbox with dynamic filtering (i.e.: removal) of entries.

   Copyright (C) <2021>
   Free Software Foundation, Inc.

   Written by:
   Sebastian Gniazdowski <sgniazdowski@gmail.com>, 2021.

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

/** \file filtering_listbox.c
 *  \brief A WListbox inheriting class that adds dynamic filtering of entries.
 *  \author Sebastian Gniazdowski
 *  \date 2021
 *
 * In order to enable/disable (i.e.: toggle) MultiSearch on an listbox you can send a message:
 *      send_message (listbox, NULL, MSG_ACTION, CK_MultiSearch, NULL);
 *
 * It'll extend owning dialog with a new MultiSearch input and pair it up with the listbox, so that
 * some of keys (basically all that aren't in input's keymap) are forwarded to listbox.
 */

#include <config.h>

#include "lib/global.h"
#include "src/setup.h"
#include "lib/widget.h"
#include "lib/tty/tty.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

static WLEntry *
filt_listbox_shallow_copy_entry (WLEntry * src, gboolean take_ownership)
{
    WLEntry *copy;
    copy = g_new (WLEntry, 1);
    *copy = *src;

    /* Who has the ownership of the data? */
    src->free_text = src->free_text && !take_ownership;
    src->free_data = src->free_data && !take_ownership;
    copy->free_text = copy->free_text && take_ownership;
    copy->free_data = copy->free_data && take_ownership;

    return copy;
}

/* --------------------------------------------------------------------------------------------- */

static void
filt_listbox_make_one_line_room (WFilteringListbox * sl, gboolean should_add_free_room)
{
    WListbox *l = LISTBOX (sl);
    Widget *w = WIDGET (l), *owner = WIDGET (WIDGET (w)->owner);
    WRect r_dialog, r_listbox;
    int new_dialog_height, new_dialog_ypos, new_listbox_height, take_give_from_to_owner = 1;

    /*
     * IF the enlarged dialog won't fit the screen, don't resize it but the listbox instead.
     * Do it also when requested if the listbox is large (for small listboxes always try to
     * enlarge the dialog).
     */
    if ((sl->resize_strategy == FILT_LIST_DIALOG_AUTO_RESIZE && LINES <= owner->lines + 2) ||
        (sl->resize_strategy == FILT_LIST_KEEP_DIALOG_SIZE && owner->lines > 7))
        take_give_from_to_owner = 0;

    /* Increase the height of the dialog by 1, so that the new input fits. */
    if (should_add_free_room)
    {
        new_dialog_height = owner->lines + take_give_from_to_owner;
        new_listbox_height = w->lines + (-1 + take_give_from_to_owner);
        new_dialog_ypos = owner->y - take_give_from_to_owner;
    }
    else
    {
        new_dialog_height = owner->lines - take_give_from_to_owner;
        new_listbox_height = w->lines - (-1 + take_give_from_to_owner);
        new_dialog_ypos = owner->y + take_give_from_to_owner;
    }
    rect_init (&r_dialog, new_dialog_ypos, owner->x, new_dialog_height, owner->cols);
    rect_init (&r_listbox, w->y, w->x, new_listbox_height, w->cols);

    /*
     * Doing widget_set_size_rect(w, &r_listbox) causes problems as it invokes
     * drawing of the widget owner.
     */
    send_message (w, NULL, MSG_RESIZE, 0, &r_listbox);
    send_message (owner, NULL, MSG_RESIZE, 0, &r_dialog);
}

/* --------------------------------------------------------------------------------------------- */

static void
filt_listbox_show_multi_search_widget (WFilteringListbox * sl)
{
    WListbox *l = LISTBOX (sl);
    Widget *w = WIDGET (l), *owner = WIDGET (WIDGET (l)->owner);
    WForwardingInput *multi_search_in;
    int distance_y = owner->y + owner->lines - (w->y + w->lines) + 1;
    int distance_x = w->cols > 40 ? 5 : 1, small = w->cols <= 15 ? 1 : 0;

    filt_listbox_make_one_line_room (sl, 1);

    /* Adjust active display. */
    if (l->pos < 0)
        l->pos = 0;
    if (l->pos < l->top || l->pos >= l->top + w->lines)
        l->top = l->pos - w->lines / 2 + 1;
    if (l->top < 0)
        l->top = 0;

    multi_search_in = forwarding_input_new (owner->lines - distance_y, distance_x,
                                            input_colors, w->cols - 2 - distance_x + small, "",
                                            "multi_search", INPUT_COMPLETE_NONE, w);
    group_add_widget_autopos (GROUP (owner), multi_search_in, WPOS_KEEP_TOP | WPOS_CENTER_HORZ,
                              NULL);
    /* Initialize input widget. */
    send_message (WIDGET (multi_search_in), w, MSG_INIT, 0, NULL);
    dlg_read_history (DIALOG (w->owner));

    /* Draw dialog and listbox, and then input. */
    widget_draw (WIDGET (w->owner));
    widget_draw (w);
    widget_draw (WIDGET (multi_search_in));
}

/* --------------------------------------------------------------------------------------------- */

static void
filt_listbox_hide_multi_search_widget (WFilteringListbox * sl)
{
    WListbox *l = LISTBOX (sl);
    Widget *w = WIDGET (sl);
    Widget *in;
    in = widget_find_by_type (WIDGET (WIDGET (l)->owner), forw_input_callback);
    if (in != NULL)
    {
        group_remove_widget (in);
        filt_listbox_make_one_line_room (sl, 0);
        group_select_next_widget (WIDGET (l)->owner);

        /*
         * Repainting is needed because some part of the resized dialog can be left on the
         * background.
         */
        if (sl->resize_strategy != FILT_LIST_KEEP_DIALOG_SIZE || w->lines <= 7)
            repaint_screen ();

        widget_draw (WIDGET (w->owner));
        widget_draw (w);
        widget_destroy (in);
    }
}

/* --------------------------------------------------------------------------------------------- */

/* Return TRUE if given listbox is in WST_FILTER state. */
static gboolean
filt_listbox_is_filter_state (WFilteringListbox * sl)
{
    return widget_get_state (WIDGET (sl), WST_FILTER);
}

/* --------------------------------------------------------------------------------------------- */

static void
filt_listbox_filter_list (WFilteringListbox * sl, const char *text)
{
    WListbox *l = LISTBOX (sl);
    int i, size;
    GList *le;
    char **query_terms;
    gboolean has_c_files = FALSE, has_header_files = FALSE;
    /*
     * Remove the list and allocate a new one. The elements are only shallowly freed because the
     * internal data is still used (and kept in list_keep field).
     */
    if (l->list != NULL)
        g_queue_free_full (l->list, g_free);
    l->list = g_queue_new ();

    /* Split the query into space delimeted strings. */
    query_terms = g_strsplit (text, " ", 10);


    size = g_queue_get_length (sl->list_keep);
    le = g_queue_peek_head_link (sl->list_keep);
    for (i = 0; i < size; i++, le = g_list_next (le))
    {
        WLEntry *e = LENTRY (le->data);
        size_t e_len;

        /* Compute lenght once, then use it. */
        e_len = e->text ? strlen (e->text) : 0;

        if (g_strrstr (e->text, ".c") == e->text + e_len - 2 ||
            g_strrstr (e->text, ".c~") == e->text + e_len - 3)
            has_c_files = TRUE;
        if (g_strrstr (e->text, ".cpp") == e->text + e_len - 4 ||
            g_strrstr (e->text, ".cpp~") == e->text + e_len - 5)
            has_c_files = TRUE;
        if (g_strrstr (e->text, ".h") == e->text + e_len - 2 ||
            g_strrstr (e->text, ".h~") == e->text + e_len - 3)
            has_header_files = TRUE;
        if (has_c_files && has_header_files)
            break;
    }

    /*
     * Get the size of the listbox and iterate over it testing each element against «all» words in
     * query_terms.
     */
    le = g_queue_peek_head_link (sl->list_keep);
    for (i = 0; i < size; i++, le = g_list_next (le))
    {
        WLEntry *e = LENTRY (le->data);
        size_t e_len;
        gboolean match = TRUE;

        e_len = e->text ? strlen (e->text) : 0;

        /* Test the query against the list entry. */
        for (gchar ** p = query_terms; *p != NULL; p++)
        {

            gboolean is_p_special;

            /* Is it a special, shorthand query ("c" or "h") ? */
            is_p_special = g_strcmp0 (*p, "h") == 0 || g_strcmp0 (*p, "c") == 0;

            /* Use special meaning of c/h queries only, when files seem to include such files. */
            if (is_p_special && (has_c_files || has_header_files))
            {
                if (has_header_files && **p == 'h' && (g_strrstr (e->text, ".h") ==
                                                       e->text + e_len - 2
                                                       || g_strrstr (e->text,
                                                                     ".h~") == e->text + e_len - 3))
                {
                    /* The file name (...apparently) matches -> do nothing. */
                }
                else if (has_c_files && **p == 'c' && (g_strrstr (e->text, ".c") ==
                                                       e->text + e_len - 2
                                                       || g_strrstr (e->text,
                                                                     ".c~") == e->text + e_len - 3
                                                       || g_strrstr (e->text,
                                                                     ".cpp") == e->text + e_len - 4
                                                       || g_strrstr (e->text,
                                                                     ".cpp~") ==
                                                       e->text + e_len - 5))
                {
                    /* The file name (...apparently) matches -> do nothing. */
                }
                else
                {
                    /* No matching of a special query -> filter element out. */
                    match = FALSE;
                    break;
                }
            }
            else if (**p != '\0' && !strcasestr (e->text, *p))
            {
                /*
                 * No match -> filter out this entry. */
                match = FALSE;
                break;
            }
        }

        /* If all the terms matched, then add the element to the list. */
        if (match)
            g_queue_push_tail (l->list, filt_listbox_shallow_copy_entry (e, FALSE));
    }
    if (listbox_is_empty (l))
    {
        listbox_add_item (l, LISTBOX_APPEND_AT_END, 0, "<no search results>", NULL, FALSE);
        LENTRY (g_queue_peek_head_link (l->list)->data)->index = -2;
    }
    size = g_queue_get_length (l->list);
    if (l->pos >= size)
        listbox_select_entry (l, size - 1);
    else
        listbox_select_entry (l, l->pos);

    g_strfreev (query_terms);
}

/* --------------------------------------------------------------------------------------------- */

/* Restores original elements of the list (from sl->list_keep) and turns off the WST_FILTER state. */
static gboolean
filt_listbox_set_to_normal_state (WFilteringListbox * sl)
{
    WListbox *l = LISTBOX (sl);
    /* The listbox is already in non-filter state? */
    if (!widget_get_state (WIDGET (l), WST_FILTER))
    {
        /* Return doing no change, just signal the error. */
        return FALSE;
    }

    /* The keep-list must be allocated (even if it's empty). */
    g_assert (sl->list_keep != NULL);

    /* Mark the new state. */
    widget_set_state (WIDGET (l), WST_FILTER, FALSE);

    /* Release the filtered list and replace it with the original, complete list (it owns the
     * internal data, hence the release is a shallow one). */
    g_queue_free_full (l->list, g_free);
    l->list = sl->list_keep;
    sl->list_keep = NULL;
    return TRUE;
}

/* --------------------------------------------------------------------------------------------- */

/*
 * Sets the listbox into «filter» state. In this state, there's a separate copy of all list
 * entries, while the original (and displayed) field ->list is being a filtered version of the
 * full copy.
 */
static gboolean
filt_listbox_set_to_filter_state (WFilteringListbox * sl)
{
    WListbox *l = LISTBOX (sl);
    GList *le;

    /* The listbox is already in filter state? */
    if (widget_get_state (WIDGET (l), WST_FILTER))
    {
        /* Return doing no change, just signal the error. */
        return FALSE;
    }

    /* Mark the new state. */
    widget_set_state (WIDGET (l), WST_FILTER, TRUE);

    /* No list copy when entering filter mode. */
    g_assert (sl->list_keep == NULL);
    sl->list_keep = g_queue_new ();

    /* Skip empty lists. */
    if (listbox_is_empty (l))
        return TRUE;

    /*
     * Remember the original position in the list in the field. It'll be used to determine the
     * virtual_pos field.
     */
    listbox_init_indices (l);

    /* Perform a shallow copy of the original list. */
    for (le = g_queue_peek_head_link (l->list); le != NULL; le = g_list_next (le))
    {
        WLEntry *copy;
        copy = filt_listbox_shallow_copy_entry (LENTRY (le->data), TRUE);
        g_queue_push_tail (sl->list_keep, copy);
    }

    return TRUE;
}

/* --------------------------------------------------------------------------------------------- */

/*
 * If the list is filtered it replaces from the back list (list_keep). It returns whether such
 * change occurred – FALSE means that the list was already unfiltered.
 */
gboolean
filt_listbox_ensure_unfiltered_state (WFilteringListbox * sl)
{
    gboolean ret = FALSE;
    if (filt_listbox_is_filter_state (sl))
        ret = filt_listbox_set_to_normal_state (sl);
    return ret;
}

/* --------------------------------------------------------------------------------------------- */

gboolean
filt_listbox_conditionally_enable_multi_search_init (WFilteringListbox * sl)
{
    gboolean start_with_multi_search_active;

    /* Option of starting the listbox with MultiSearch pre-activated. */
    start_with_multi_search_active = option_multisearch;

    /* CK_MultiSearch toggles the state. */
    if (start_with_multi_search_active)
        send_message (WIDGET (sl), NULL, MSG_ACTION, CK_MultiSearch, NULL);
    else
        /* Only init embedded position indices. */
        listbox_init_indices (LISTBOX (sl));

    /* Return if did enable MultiSearch. */
    return start_with_multi_search_active;
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

WFilteringListbox *
filtering_listbox_new (int y, int x, int height, int width,
                       gboolean deletable, lcback_fn callback,
                       filt_listbox_resize_strategy_t resize)
{
    WFilteringListbox *object;
    Widget *w_ref;

    /* Allocate memory for the object body. */
    object = g_new (WFilteringListbox, 1);

    /* Forward the call to construct the inherited object. */
    listbox_init (&object->base, y, x, height, width, deletable, callback);

    /* Alter fields of base class. */
    w_ref = WIDGET (object);
    w_ref->callback = filt_listbox_callback;    /* Set custom callback handler */

    /* Set extending fields of this class. */
    object->list_keep = NULL;   /* No back buffer at startup */
    object->resize_strategy = resize;   /* Save resize strategy */
    object->initialized = FALSE;

    return object;
}

/* --------------------------------------------------------------------------------------------- */

cb_ret_t
filt_listbox_callback (Widget * w, Widget * sender, widget_msg_t msg, int parm, void *data)
{
    WFilteringListbox *sl = FILT_LISTBOX (w);   /* s* - from `screen`, a "screened" listbox */
    cb_ret_t ret = MSG_NOT_HANDLED;
    long activity;

    switch (msg)
    {
    case MSG_INIT:
        if (!sl->initialized)
        {
            filt_listbox_conditionally_enable_multi_search_init (sl);
            sl->initialized = TRUE;
        }
        /* WListbox doesn't have MSG_INIT, so don't forward. */
        ret = MSG_HANDLED;
        break;
    case MSG_ACTION:
        if (parm == CK_MultiSearch)
        {
            gboolean retval;
            /* Toggle the multi term searching of any listbox. */
            if (filt_listbox_is_filter_state (sl))
            {
                /* Remove the input widget from the dialog. */
                filt_listbox_hide_multi_search_widget (sl);
                /* Restore original (unfiltered) listbox contents. */
                retval = filt_listbox_set_to_normal_state (sl);
            }
            else
            {
                /* Add input widget for the filter query at the bottom of the dialog window. */
                filt_listbox_show_multi_search_widget (sl);
                /* ... and then turn on the filter state. */
                retval = filt_listbox_set_to_filter_state (sl);
            }
            if (!retval)
                message (D_ERROR | D_CENTER, MSG_ERROR,
                         "An internal error #3 occurred (filtered listbox support).");

            ret = MSG_HANDLED;
        }
        break;

    case MSG_KEY:
        activity = widget_lookup_key (WIDGET (sl), parm);
        if (activity == CK_MultiSearch)
            ret = send_message (w, NULL, MSG_ACTION, CK_MultiSearch, NULL);
        break;

    case MSG_NOTIFY:
        if (widget_get_state (w, WST_FILTER))
        {
            filt_listbox_filter_list (sl, (char *) data);
            ret = MSG_HANDLED;
        }
        widget_draw (w);
        /* Protect against normal, non ForwardingInput messages, which might not have sender set. */
        if (sender)
            widget_draw (sender);
        break;

    case MSG_DESTROY:
        filt_listbox_ensure_unfiltered_state (sl);
        /* ret is unhandled -> the message will be forwarded to base class. */
        break;
    default:
        break;
    }

    /* Forward action to base class in case it's not yet handled. */
    if (ret == MSG_NOT_HANDLED)
        ret = listbox_callback (w, sender, msg, parm, data);

    return ret;
}

/* --------------------------------------------------------------------------------------------- */
