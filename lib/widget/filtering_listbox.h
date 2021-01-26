#ifndef MC__FILTERING_LISTBOX_H
#define MC__FILTERING_LISTBOX_H

/*** typedefs(not structures) and defined constants **********************************************/

/* Casting macros. */
#define FILT_LISTBOX(x) ((WFilteringListbox *)(x))
#define CONST_FILT_LISTBOX(x) ((const WFilteringListbox *)(x))

/*** enums ***************************************************************************************/

typedef enum filt_listbox_resize_strategy_e
{
    FILT_LIST_EXTEND_DIALOG = 0,
    FILT_LIST_KEEP_DIALOG_SIZE,
    FILT_LIST_DIALOG_AUTO_RESIZE
} filt_listbox_resize_strategy_t;

/*** structures declarations (and typedefs of structures)*****************************************/

typedef struct WFilteringListbox_s
{
    WListbox base;
    gboolean initialized;       /* Whether MSG_INIT has been received. */

    /* Fields for new logic. */
    GQueue *list_keep;          /* Unfiltered list (used in  WST_FILTER state). */
    filt_listbox_resize_strategy_t resize_strategy;
} WFilteringListbox;

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

WFilteringListbox *filtering_listbox_new (int y, int x, int height, int width,
                                          gboolean deletable, lcback_fn callback,
                                          filt_listbox_resize_strategy_t resize);
gboolean filt_listbox_ensure_unfiltered_state (WFilteringListbox * l);
gboolean filt_listbox_conditionally_enable_multi_search_init (WFilteringListbox * l);
void filt_listbox_select_entry (WFilteringListbox * sl, int dest);
cb_ret_t filt_listbox_callback (Widget * w, Widget * sender, widget_msg_t msg, int parm,
                                void *data);

/*** inline functions ****************************************************************************/

#endif /* MC__FILTERING_LISTBOX_H */
