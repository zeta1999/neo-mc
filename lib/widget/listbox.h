
/** \file listbox.h
 *  \brief Header: WListbox widget
 */

#ifndef MC__WIDGET_LISTBOX_H
#define MC__WIDGET_LISTBOX_H

/*** typedefs(not structures) and defined constants **********************************************/

#define LISTBOX(x) ((WListbox *)(x))
#define LENTRY(x) ((WLEntry *)(x))

/*** enums ***************************************************************************************/

/* callback should return one of the following values */
typedef enum
{
    LISTBOX_CONT,               /* continue */
    LISTBOX_DONE                /* finish dialog */
} lcback_ret_t;

typedef enum
{
    LISTBOX_APPEND_AT_END = 0,  /* append at the end */
    LISTBOX_APPEND_BEFORE,      /* insert before current */
    LISTBOX_APPEND_AFTER,       /* insert after current */
    LISTBOX_APPEND_SORTED       /* insert alphabetically */
} listbox_append_t;

/*** structures declarations (and typedefs of structures)*****************************************/

struct WListbox;
typedef lcback_ret_t (*lcback_fn) (struct WListbox * l);

typedef struct WLEntry
{
    int index;                  /* The location in the list (used when it's filtered) */
    char *text;                 /* Text to display */
    gboolean free_text;         /* Whether to free the text on entry's removal */
    int hotkey;
    void *data;                 /* Client information */
    gboolean free_data;         /* Whether to free the data on entry's removal */
} WLEntry;

typedef struct WListbox
{
    Widget widget;
    GQueue *list;               /* Pointer to the list of WLEntry */
    int pos;                    /* The current element displayed */
    int virtual_pos;            /* The initial index of the current element, works also for filtered listbox */
    int top;                    /* The first element displayed */
    gboolean allow_duplicates;  /* Do we allow duplicates on the list? */
    gboolean scrollbar;         /* Draw a scrollbar? */
    gboolean deletable;         /* Can list entries be deleted? */
    lcback_fn callback;         /* The callback function */
    int cursor_x, cursor_y;     /* Cache the values */
} WListbox;

/*** global variables defined in .c file *********************************************************/

extern const global_keymap_t *listbox_map;

/*** declarations of public functions ************************************************************/

void listbox_init (WListbox * l, int y, int x, int height, int width, gboolean deletable,
                   lcback_fn callback);
WListbox *listbox_new (int y, int x, int height, int width, gboolean deletable, lcback_fn callback);
cb_ret_t listbox_callback (Widget * w, Widget * sender, widget_msg_t msg, int parm, void *data);

int listbox_search_text (WListbox * l, const char *text);
int listbox_search_data (WListbox * l, const void *data);
void listbox_select_first (WListbox * l);
void listbox_select_last (WListbox * l);
void listbox_select_entry (WListbox * l, int dest);
int listbox_get_length (const WListbox * l);
void listbox_get_current (WListbox * l, char **string, void **extra);
WLEntry *listbox_get_nth_item (const WListbox * l, int pos);
GList *listbox_get_first_link (const WListbox * l);
void listbox_remove_current (WListbox * l);
gboolean listbox_is_empty (const WListbox * l);
void listbox_set_list (WListbox * l, GQueue * list);
void listbox_remove_list (WListbox * l);
void listbox_init_indices (WListbox * l);
char *listbox_add_item (WListbox * l, listbox_append_t pos, int hotkey, const char *text,
                        void *data, gboolean free_data);

/*** inline functions ****************************************************************************/

#endif /* MC__WIDGET_LISTBOX_H */
