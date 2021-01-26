/*
   A key forwarding extended input class.

   Copyright (C) 2021
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

/** \file forwarding-input.c
 *  \brief This input class has a feature of forwarding unrecognized keys to a widget passed at
 *         creation.
 *  \author Sebastian Gniazdowski
 *  \date 2021
 *
 *  It's being used by MultiSearch to allow moving across listbox while typing to the input.
 */

#include <config.h>

#include "lib/global.h"
#include "lib/widget.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

WForwardingInput *
forwarding_input_new (int y, int x, const int *colors,
                      int len, const char *text, const char *histname,
                      input_complete_t completion_flags, Widget * forward_to_widget)
{
    WForwardingInput *object;
    Widget *w_ref;

    /* Allocate memory for the object body. */
    object = g_new (WForwardingInput, 1);

    /* Call upper constructor to initialize the inherited object. */
    input_init (&object->base, y, x, colors, len, text, histname, completion_flags);

    /* Alter fields of base class. */
    w_ref = WIDGET (object);
    w_ref->callback = forw_input_callback;      /* Set custom callback handler */

    /* Set  extending fields of this class. */
    object->forward_to_widget = forward_to_widget;
    return object;
}

/* --------------------------------------------------------------------------------------------- */

cb_ret_t
forw_input_callback (Widget * w, Widget * sender, widget_msg_t msg, int parm, void *data)
{
    WForwardingInput *in = FORW_INPUT (w);
    gboolean key_message = FALSE;
    cb_ret_t ret;

    switch (msg)
    {
    case MSG_KEY:
        ret = forw_input_handle_char (in, parm);
        key_message = TRUE;
        break;
    default:
        break;
    }

    /*
     * Simply pass on all messages to the base class (except for MSG_KEY, which might have
     * been possibly already sent from forw_input_handle_char() function).
     */

    if (!key_message)
        ret = input_callback (WIDGET (in), sender, msg, parm, data);

    return ret;
}

/* --------------------------------------------------------------------------------------------- */

cb_ret_t
forw_input_handle_char (WForwardingInput * in, int key)
{
    cb_ret_t ret = MSG_NOT_HANDLED;
    gboolean sent_to_base = FALSE;
    long activity;
    char *str_cp;

    /* Save to detect if a change happened. */
    str_cp = g_strdup (in->base.buffer);

    /* Is this key recognized by the base object? */
    activity = widget_lookup_key (WIDGET (in), key);
    if (activity != CK_IgnoreKey && activity != CK_Complete)
    {
        /* Yes -> send the key to the upper class. */
        ret = input_callback (WIDGET (in), WIDGET (in), MSG_KEY, key, NULL);
        sent_to_base = TRUE;
    }
    /* Should we try to forward the key to any paired widget? */
    if (in->forward_to_widget != NULL && ret == MSG_NOT_HANDLED)
    {
        /* Is it maybe recognized by forward_to_widget paired object? */
        activity = widget_lookup_key (WIDGET (in->forward_to_widget), key);
        if (activity != CK_IgnoreKey)
        {
            /* Yes - forward the key to the paired widget (most probably WListbox). */
            ret = send_message (WIDGET (in->forward_to_widget), NULL, MSG_KEY, key, NULL);
        }
    }

    /*
     * If not handled yet, then send the key to the base object for general recognition (if
     * not already done that).
     */

    if (!sent_to_base && ret == MSG_NOT_HANDLED)
    {
        ret = input_callback (WIDGET (in), WIDGET (in), MSG_KEY, key, NULL);
        sent_to_base = TRUE;    /* currently unused */
    }

    /* Send update signal to paired widget «iff» input's text has changed. */
    if (in->forward_to_widget != NULL && g_strcmp0 (str_cp, in->base.buffer) != 0)
        send_message (WIDGET (in->forward_to_widget), NULL, MSG_NOTIFY, key, in->base.buffer);
    g_free (str_cp);

    return ret;
}

/* --------------------------------------------------------------------------------------------- */
