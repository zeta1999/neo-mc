#ifndef MC__SLANG_ENGINE_H
#define MC__SLANG_ENGINE_H

/*** typedefs(not structures) and defined constants **********************************************/

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

extern int num_action_hook_functions;
extern GHashTable *action_hook_functions;

/*** declarations of public functions ************************************************************/

int slang_init_engine (void);
int slang_plugins_init (void);
GSList *get_action_hook (int ck_id);

/*** inline functions ****************************************************************************/

#endif
