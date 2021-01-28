/** \file logging.h
 *  \brief Header: provides a log file to ease tracing the program
 */

#ifndef MC_LOGGING_H
#define MC_LOGGING_H

#include <config.h>

/*
   This file provides an easy-to-use function for writing all kinds of
   events into a central log file that can be used for debugging.
 */

/*** typedefs(not structures) and defined constants **********************************************/

/* *INDENT-OFF* */
#ifdef USE_MAINTAINER_MODE
void mc_log (const char *fmt, ...) G_GNUC_PRINTF (1, 2);
void mc_always_log (const char *fmt, ...) G_GNUC_PRINTF (1, 2);
#else
/* Use printf() to silence the unused variable warnings. */
#define mc_log(fmt,...) do { if(0) printf(fmt, ##__VA_ARGS__);  } while(0)
#define mc_always_log(fmt,...) do { if(0) printf(fmt, ##__VA_ARGS__); } while(0)
#endif
/* *INDENT-ON* */

#define mc_log_mark() mc_log("%s:%d",__FILE__,__LINE__)

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

/*** inline functions ****************************************************************************/

#endif
