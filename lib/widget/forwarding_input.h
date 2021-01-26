#ifndef MC__FORWARDING_INPUT_H
#define MC__FORWARDING_INPUT_H

/*** typedefs(not structures) and defined constants **********************************************/

/* Casting macros. */
#define FORW_INPUT(x) ((WForwardingInput *)(x))
#define CONST_FORW_INPUT(x) ((const WForwardingInput *)(x))

typedef struct
{
    WInput base;

    /* Fields for new logic. */
    Widget *forward_to_widget;  /* The paired widget to receive unhandled keys */
} WForwardingInput;

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

WForwardingInput *forwarding_input_new (int y, int x, const int *colors,
                                        int len, const char *text, const char *histname,
                                        input_complete_t completion_flags,
                                        Widget * forward_to_widget);

cb_ret_t forw_input_callback (Widget * w, Widget * sender, widget_msg_t msg, int parm, void *data);
cb_ret_t forw_input_handle_char (WForwardingInput * in, int key);

/*** inline functions ****************************************************************************/

#endif /* MC__FORWARDING_INPUT_H */
