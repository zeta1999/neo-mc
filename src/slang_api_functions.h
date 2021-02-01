#ifndef MC__SLANG_API_FUNCTIONS_H
#define MC__SLANG_API_FUNCTIONS_H

#include "lib/global.h"

/*** typedefs(not structures) and defined constants **********************************************/

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/


/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

/* Control functions */
int slang_api__action(void *ARRAY_2_VOIDP, char *action_name, long action_code, Multi_Type_Action_Data *data);

/* Movement functions */
void slang_api__cure_cursor_move (int offset);

/* Getting offsets */
int slang_api__cure_cursor_offset (void);
int slang_api__cure_get_eol (void);
int slang_api__cure_get_bol (void);

/* Getting data from  buffer */
char *slang_api__cure_get_left_whole_word (int skip_space);
int slang_api__cure_get_byte (int byte_idx);

/* Editing functions */
int slang_api__cure_delete (void);
int slang_api__cure_backspace (void);
void slang_api__cure_insert_ahead (int c);

/* Dialog functions */
int slang_api__listbox (int h, int w, char *title, char **items, unsigned long size);
char *slang_api__listbox_with_data (int h, int w, char *title, char **items, unsigned long size,
                                    char **data, unsigned long size2);
int slang_api__listbox_auto (char *title, char **items, unsigned long size);
void slang_api__message (const char *title, const char *body);

/* Action hooks */
int slang_api__set_action_hook (const char *command, const char *function_name,
                                const char *user_data);
#ifndef MC__KEYBIND_H
int keybind_add_new_action (const char *new_command_name, int new_ck_id);
#endif

/* Key bindings */
int slang_api__editor_map_key_to_action (const char *key_combination, const char *command_name);
int slang_api__editor_map_key_to_func (const char *new_command_name,
                                       const char *key_combination, const char *function_name);

/*** inline functions ****************************************************************************/

#endif
