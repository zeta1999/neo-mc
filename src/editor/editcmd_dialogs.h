#ifndef MC__EDITCMD_DIALOGS_H
#define MC__EDITCMD_DIALOGS_H

#include "src/editor/edit.h"

/*** typedefs(not structures) and defined constants **********************************************/

typedef struct etags_hash_struct etags_hash_t;

#define B_REPLACE_ALL (B_USER+1)
#define B_REPLACE_ONE (B_USER+2)
#define B_SKIP_REPLACE (B_USER+3)

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

void editcmd_dialog_replace_show (WEdit *, const char *, const char *, char **, char **);

gboolean editcmd_dialog_search_show (WEdit * edit);

int editcmd_dialog_raw_key_query (const char *heading, const char *query, gboolean cancel);

char *editcmd_dialog_completion_show (const WEdit * edit, GQueue * compl, int max_width);

etags_hash_t *editcmd_dialog_select_tags_object_show (WEdit *, char *, int, etags_hash_t *,
                                                      etags_jump_type_t, int);

int editcmd_dialog_replace_prompt_show (WEdit *, char *, char *, int, int);
/*** inline functions ****************************************************************************/
#endif /* MC__EDITCMD_DIALOGS_H */
