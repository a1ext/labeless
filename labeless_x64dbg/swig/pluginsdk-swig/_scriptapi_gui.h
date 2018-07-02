#ifndef _SCRIPTAPI_GUI_H
#define _SCRIPTAPI_GUI_H

#include "_scriptapi.h"

namespace Script
{
    namespace Gui
    {
        namespace Disassembly
        {
            %rename(Gui_Disassembly_SelectionGet) SelectionGet;
            extern bool SelectionGet(duint* start, duint* end);

            %rename(Gui_Disassembly_SelectionSet) SelectionSet;
            extern bool SelectionSet(duint start, duint end);

            %rename(Gui_Disassembly_SelectionGetStart) SelectionGetStart;
            extern duint SelectionGetStart();

            %rename(Gui_Disassembly_SelectionGetEnd) SelectionGetEnd;
            extern duint SelectionGetEnd();
        }; //Disassembly

        namespace Dump
        {
            %rename(Gui_Dump_SelectionGet) SelectionGet;
            extern bool SelectionGet(duint* start, duint* end);

            %rename(Gui_Dump_SelectionSet) SelectionSet;
            extern bool SelectionSet(duint start, duint end);

            %rename(Gui_Dump_SelectionGetStart) SelectionGetStart;
            extern duint SelectionGetStart();

            %rename(Gui_Dump_SelectionGetEnd) SelectionGetEnd;
            extern duint SelectionGetEnd();
        }; //Dump

        namespace Stack
        {
            %rename(Gui_Stack_SelectionGet) SelectionGet;
            extern bool SelectionGet(duint* start, duint* end);
            %rename(Gui_Stack_SelectionSet) SelectionSet;
            extern bool SelectionSet(duint start, duint end);
            %rename(Gui_Stack_SelectionGetStart) SelectionGetStart;
            extern duint SelectionGetStart();
            %rename(Gui_Stack_SelectionGetEnd) SelectionGetEnd;
            extern duint SelectionGetEnd();
        }; //Stack

        namespace Graph
        {
            %rename(Gui_Graph_SelectionGetStart) SelectionGetStart;
            extern duint SelectionGetStart();
        }; //Graph

        namespace MemMap
        {
            %rename(Gui_MemMap_SelectionGetStart) SelectionGetStart;
            extern duint SelectionGetStart();
        }; //MemoryMap

        namespace SymMod
        {
            %rename(Gui_SymMod_SelectionGetStart) SelectionGetStart;
            extern duint SelectionGetStart();
        }; //SymMod
    }; //Gui

    namespace Gui
    {
        enum Window
        {
            DisassemblyWindow,
            DumpWindow,
            StackWindow,
            GraphWindow,
            MemMapWindow,
            SymModWindow
        };

        %rename(Gui_SelectionGet) SelectionGet;
        extern bool SelectionGet(Window window, duint* start, duint* end);
        
        %rename(Gui_SelectionSet) SelectionSet;
        extern bool SelectionSet(Window window, duint start, duint end);
        
        %rename(Gui_SelectionGetStart) SelectionGetStart;
        extern duint SelectionGetStart(Window window);
        
        %rename(Gui_SelectionGetEnd) SelectionGetEnd;
        extern duint SelectionGetEnd(Window window);
        
        %rename(Gui_Message) Message;
        extern void Message(const char* message);
        
        %rename(Gui_MessageYesNo) MessageYesNo;
        extern bool MessageYesNo(const char* message);

        %rename(Gui_InputLine) InputLine;
        %pybuffer_string(char* text)
        extern bool InputLine(const char* title, char* text); //text[GUI_MAX_LINE_SIZE]
        
        %rename(Gui_InputValue) InputValue;
        extern bool InputValue(const char* title, duint* value);
        
        %rename(Gui_Refresh) Refresh;
        extern void Refresh();
        
        %rename(Gui_AddQWidgetTab) AddQWidgetTab;
        extern void AddQWidgetTab(void* qWidget);
        
        %rename(Gui_ShowQWidgetTab) ShowQWidgetTab;
        extern void ShowQWidgetTab(void* qWidget);
        
        %rename(Gui_CloseQWidgetTab) CloseQWidgetTab;
        extern void CloseQWidgetTab(void* qWidget);
    }; //Gui
}; //Script

#endif //_SCRIPTAPI_GUI_H