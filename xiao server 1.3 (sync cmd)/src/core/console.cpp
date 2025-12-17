#include "prototypes.h"

namespace console
{
      // Command buffer using FreeRTOS queue
      static constexpr auto COMMAND_QUEUE_LENGTH = 10;
      static constexpr auto COMMAND_BUFFER_SIZE = 256;
      static QueueHandle_t commandQueue = nullptr;
      String cmdArgBuff = "";

      static bool execute_imp(String &cmdBuff)
      {
            String cmd = "";
            cmdBuff.trim();
            cmdBuff.toLowerCase();

            if (cmdBuff.indexOf("(") == -1)
                  cmd = cmdBuff;
            else
            {
                  cmdArgBuff = cmdBuff.substring(cmdBuff.indexOf("(") + 1, cmdBuff.indexOf(")"));
                  cmd = cmdBuff.substring(0, cmdBuff.indexOf("("));
            }

            for_x(cmd_list_len)
            {
                  if (cmd_list[x].name.equalsIgnoreCase(cmd))
                  {
                        if (!cmd_list[x].codePtr)
                        {
                              printer("nullptr for", qt(cmd));
                              cmdArgBuff = "";
                              return false;
                        }

                        printer("new cmd:", qt(cmd + "(" + peekArgs() + ")"));

                        cmd_list[x].codePtr();
                        cmdArgBuff.clear();
                        return true;
                  }
            }

            printer("cmd_t", qt(cmd), "not found. See 'help' for more");

            if (printInfo)
                  printCmds();

            cmdArgBuff.clear();
            return false;
      }

      static void readSerial()
      {
            static String buff;
            buff.reserve(1024);

            while (Serial.available())
            {
                  char inChar = Serial.read();

                  if (inChar != '\n' && inChar != ';')
                        buff += inChar;
                  else
                  {
                        printerInfo("raw Serial =", qt(buff));
                        execute(buff); // Queue instead of immediate execution
                        buff = "";
                  }
            }
      }

      String peekArgs()
      {
            String copy = cmdArgBuff;
            String buff;

            while (copy != "")
            {
                  buff += extractString(copy);

                  if (copy != "")
                        buff += protocol::objSeparator;
            }
            return buff;
      }

      String extractString(String &buff)
      {
            String result = "";

            buff.trim();
            const int separatorIndex = buff.indexOf(protocol::argSeparator);

            if (separatorIndex == -1)
            {
                  result = buff;
                  buff = ""; // empty buffer to prevent infinite loops
                  goto end;
            }

            result = buff.substring(0, separatorIndex);
            buff = buff.substring(separatorIndex + 1); // substring doesn't modify og obj

            if (result == "")
                  printer("console::extractString err");

      end:
            result.trim();
            return result;
      }
      bool extractBool(String &buff)
      {
            int extractedArg = extractInt(buff);
            if (extractedArg != 1 && extractedArg != 0)
            {
                  printer("extractBool err outOfScope:", extractedArg);
                  return 0;
            }

            return static_cast<bool>(extractedArg);
      }
      int extractInt(String &buff)
      {
            return extractString(buff).toInt();
      }
      float extractFloat(String &buff)
      {
            return extractString(buff).toFloat();
      }

      bool init()
      {
            if (!commandQueue)
                  commandQueue = xQueueCreate(COMMAND_QUEUE_LENGTH, sizeof(char[COMMAND_BUFFER_SIZE]));

            printToSerial = storage::get("printToSerial", true);
            printToServer = storage::get("printToServer", true);
            printInfo = storage::get("printInfo", true);

            esp_log_set_vprintf([](const char *format, va_list args)
                                {
                                    // calculate needed size
                                    va_list args_copy;
                                    va_copy(args_copy, args);
                                    int len = vsnprintf(nullptr, 0, format, args_copy);
                                    va_end(args_copy);
                                    
                                    if (len < 0) return 0;
                                    
                                    // Allocate exact buffer (+1 for null terminator)
                                    char* buffer = (char*)malloc(len + 1);
                                    if (!buffer) return 0;
                                    
                                    // Format into buffer
                                    vsnprintf(buffer, len + 1, format, args);

                                    if (printToSerial) 
                                          Serial.print(buffer);
                                
                                    if (printToServer) 
                                          WebSerial.print(buffer);
                                       
                                    
                                    free(buffer);
                                    
                                    return len; // it must return what og was returning. It's not important how many bytes were printer but how much oryginally should be 
                                });

            Serial.begin(MONITOR_SPEED);

            if (storage::get("w8forInput", true))
            {
                  Timer timer(1000);
                  while (!Serial.available() && millis() < 10'000)
                  {
                        delay(10);
                        if (timer.timePassed())
                              Serial.println("click any button to start...");
                  }

                  while (Serial.available()) // discard garbage
                        Serial.read();
            }
           
            printer("---- START ----");
            printer("uploaded: " __DATE__ " " __TIME__);

            return commandQueue != nullptr;
      }

      bool execute(const String &cmd)
      {
            // Convert String to fixed buffer for queue
            char commandBuffer[COMMAND_BUFFER_SIZE];
            cmd.toCharArray(commandBuffer, COMMAND_BUFFER_SIZE);

            // Try to send to queue with 0 timeout (non-blocking)
            if (xQueueSend(commandQueue, commandBuffer, 200) == pdPASS)
            {
                  printerInfo("Command queued:", qt(cmd));
                  return true;
            }
            else
            {
                  printer("Command queue full, dropped:", qt(cmd));
                  return false;
            }
      }

      void loop()
      {
            readSerial();

            // recv stored commands and send them to be executed
            static char commandBuffer[COMMAND_BUFFER_SIZE];
            if (xQueueReceive(commandQueue, commandBuffer, 0) == pdTRUE)
            {
                  String cmdBuff = String(commandBuffer);
                  printerInfo("queue cmd recv:", qt(cmdBuff));
                  execute_imp(cmdBuff);
            }
      }

      void printCmds()
      {
            printer("available cmds [total: " + (String)cmd_list_len + "]");

            for_x(cmd_list_len)
                printer(cmd_list[x].name + "(" + cmd_list[x].argsTypesList + ")");
      }

}

#include "commands.h"