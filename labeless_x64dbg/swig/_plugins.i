%module _plugins
%{
#include <windows.h>
#include "_plugins.h"
%}

/* Implemented In Python As _plugin_logprintf("%s\n" %s)
 * extern void _plugin_logputs(const char* text);
 */
extern void _plugin_logprintf(const char* format, ...);
