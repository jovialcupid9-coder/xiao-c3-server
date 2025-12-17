#pragma once
#include "prototypes.h"

// manual wrapper that works as semi reflection system
// enables calling functions by name run time for using CLI/WEB_CLI/REST
namespace console
{
      static int cmd_list_len = -1; // count amount of cmds to prevent accessing out of bounds pointers

      struct Cmd
      {
            String name;
            String argsTypesList;
            funcPtr codePtr;

            inline Cmd(String name, String argsTypesList, funcPtr codePtr) : name(name), argsTypesList(argsTypesList), codePtr(codePtr)
            {
                  // todo:: make it work - 'cmd_list' was not declared in this scope
                  //  for_x(cmd_list_len){
                  // if(cmd_list[x].name == this->name){
                  //      printer("cmd name already exists!");
                  //      this->name += "(copy)";
                  // }
                  // }
                  cmd_list_len++;
            }
            inline Cmd(String name, funcPtr codePtr) : Cmd(name, "", codePtr) {}// inline constructor cannot have default param it's not the last one


      };

      extern Cmd cmd_list[];
      extern String cmdArgBuff; // cannot be static cuz extract default value

      void printCmds();
      bool execute(const String &cmd); // put data into buffer to be executed in sync
      void loop();
      String peekArgs();
      bool init();

      String extractString(String &buff = cmdArgBuff);
      int extractInt(String &buff = cmdArgBuff);
      bool extractBool(String &buff = cmdArgBuff);
      float extractFloat(String &buff = cmdArgBuff);
};
