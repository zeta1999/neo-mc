#ifndef MC__EDIT_ETAGS_H
#define MC__EDIT_ETAGS_H 1

#include <sys/types.h>          /* size_t */
#include "lib/global.h"         /* include <glib.h> */

/*** typedefs(not structures) and defined constants **********************************************/

#define MAX_WIDTH_DEF_DIALOG 60 /* max width of the dialog */
#define MAX_TAG_OBJECTS   350
#define SHORT_DEF_LEN   70
#define LONG_DEF_LEN    70
#define LINE_DEF_LEN    16

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

typedef struct etags_hash_struct
{
    size_t filename_len;
    char *fullpath;
    char *filename;
    char *short_define;
    long line;
} etags_hash_t;


typedef enum
{
    TAG_JUMP_KIND_FUNCTION_LIST,   /* List of functions in current file. */
    TAG_JUMP_KIND_TYPE_LIST,       /* List of type definitions in current file. */
    TAG_JUMP_KIND_VAR_LIST,        /* List of variables in current file. */
    TAG_JUMP_KIND_OTHER_LIST,      /* List of other tag object types for the current file. */
    TAG_JUMP_KIND_ANY_LIST,        /* List of all tags for current file. */
    TAG_JUMP_KIND_MATCH_WORD,      /* A list of tag objects matching left word. */
    TAG_JUMP_KIND_QUICK_WHOLE_WORD /* Future – instantly jump to the id under cursor, same file */
} etags_jump_type_t;

typedef enum
{
    TAG_RANK_FUNCTIONS,     /* Function definitions */
    TAG_RANK_TYPES,         /* Types (structs, typedefs, etc.)  */
    TAG_RANK_VARIABLES,     /* Variables */
    TAG_RANK_OTHER,         /* Other kind (not of the above) */
    TAG_RANK_ANY            /* All kinds */
} etags_rank_t;

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/


int etags_set_definition_hash (const char *tagfile, const char *start_path,
                               const char *match_func, etags_hash_t * def_hash);

int etags_get_objects_for_file (etags_rank_t type, const char *tagfile,
                            const char *start_path, const char *match_filename,
                            etags_hash_t * functions_hash,
                            int *max_len_return, int size_limit);

int etags_locate_tags_file(char **tagfile_return, char **path_return);

/*** inline functions ****************************************************************************/
#endif /* MC__EDIT_ETAGS_H */
