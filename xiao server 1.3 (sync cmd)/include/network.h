#pragma once
#include "Arduino.h"

namespace network
{
      constexpr auto port = 80;
      extern AsyncWebServer server;
      extern AsyncWebServerRequest *savedClientAsRequest;


      bool checkUserAuth(AsyncWebServerRequest *request);
      String templateProcessor(const String &var);
      void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
      String getHostName();
      void begin();

      namespace wifi
      {
            constexpr int max_credentials = 5;
            constexpr int max_wait_connection = 10000; // in ms

            void captivePortal();
            bool connect(String login, String password);
            void begin();
            String getCreditenstialsList();
            String getIP();
            String getSSID();
            String getScan();
            String getCreditenstialsList();
            String getRSSI();
      };

      namespace webFS {
            
            #include "fileSystemPages.h"
            void begin();
            void handleUpload(AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final);
      }

   
}