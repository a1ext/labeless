#ifndef _SCRIPTAPI_GUI_H
#define _SCRIPTAPI_GUI_H

#include "_scriptapi.h"

namespace Script
{
    namespace Gui
    {
        namespace Disassembly
        {
            extern bool SelectionGet(duint* start, duint* end);
            extern bool SelectionSet(duint start, duint end);
            extern duint SelectionGetStart();
            extern duint SelectionGetEnd();
        }; //Disassembly

        namespace Dump
        {
            extern bool SelectionGet(duint* start, duint* end);
            extern bool SelectionSet(duint start, duint end);
            extern duint SelectionGetStart();
            extern duint SelectionGetEnd();
        }; //Dump

        namespace Stack
        {
            extern bool SelectionGet(duint* start, duint* end);
            extern bool SelectionSet(duint start, duint end);
            extern duint SelectionGetStart();
            extern duint SelectionGetEnd();
        }; //Stack
    }; //Gui

    namespace Gui
    {
        enum Window
        {
            DisassemblyWindow,
            DumpWindow,
            StackWindow
        };

        extern bool SelectionGet(Window window, duint* start, duint* end);
        extern bool SelectionSet(Window window, duint start, duint end);
        extern duint SelectionGetStart(Window window);
        extern duint SelectionGetEnd(Window window);
        extern void Message(const char* message);
        extern bool MessageYesNo(const char* message);
        extern bool InputLine(const char* title, char* text); //text[GUI_MAX_LINE_SIZE]
        extern bool InputValue(const char* title, duint* value);
        extern void Refresh();
        extern void AddQWidgetTab(void* qWidget);
        extern void ShowQWidgetTab(void* qWidget);
        extern void CloseQWidgetTab(void* qWidget);

    }; //Gui
}; //Script

#endif //_SCRIPTAPI_GUI_H