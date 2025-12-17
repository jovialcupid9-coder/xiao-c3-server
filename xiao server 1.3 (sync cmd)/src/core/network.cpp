#include "prototypes.h"

namespace network
{
  AsyncWebServer server(network::port);
  AsyncWebServerRequest *savedClientAsRequest = NULL;

  // Standalone getContentType i cannot acces one from lib
  static String getContentType(String filename)
  {
    if (filename.endsWith(".htm"))
      return "text/html";
    else if (filename.endsWith(".html"))
      return "text/html";
    else if (filename.endsWith(".css"))
      return "text/css";
    else if (filename.endsWith(".js"))
      return "application/javascript";
    else if (filename.endsWith(".json"))
      return "application/json";
    else if (filename.endsWith(".png"))
      return "image/png";
    else if (filename.endsWith(".jpg"))
      return "image/jpeg";
    else if (filename.endsWith(".jpeg"))
      return "image/jpeg";
    else if (filename.endsWith(".gif"))
      return "image/gif";
    else if (filename.endsWith(".svg"))
      return "image/svg+xml";
    else if (filename.endsWith(".ico"))
      return "image/x-icon";
    else if (filename.endsWith(".xml"))
      return "text/xml";
    else if (filename.endsWith(".pdf"))
      return "application/pdf";
    else if (filename.endsWith(".zip"))
      return "application/zip";
    else if (filename.endsWith(".gz"))
      return "application/x-gzip";
    else if (filename.endsWith(".txt"))
      return "text/plain";
    return "application/octet-stream";
  }

  bool checkUserAuth(AsyncWebServerRequest *request)
  {
    if (wifi::getSSID().equalsIgnoreCase(storage::get("trusted_ssid", "avi")))
    {
      printerInfo("trusted env, bypass");
      return true;
    }
    const String login = storage::get("server_login", "admin");
    const String password = storage::get("server_password", "admin");
    bool res = request->authenticate(login.c_str(), password.c_str());

    printerInfo("untrusty env, pass needed");

    if (!res)
      request->requestAuthentication();

    return res;
  }

  void sRequestSend_HELPER(AsyncWebServerRequest *req, int statusCode,
                           const String &contentType, const String &response)
  {
    if (req != NULL)
    {
      AsyncWebServerResponse *resp = req->beginResponse(statusCode, contentType, response);
      resp->addHeader("Access-Control-Allow-Origin", "*");
      resp->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
      resp->addHeader("Access-Control-Allow-Headers", "Content-Type");
      req->send(resp);
    }
  }

  String templateProcessor(const String &var)
  {
    auto validate = [](const String &text)
    { return "\"" + text + "\""; };

    if (var == "PLACEHOLDER_FILE_LIST")
      return validate(storage::getFiles());

    if (var == "PLACEHOLDER_ARG_SEPARATOR")
      return validate(protocol::argSeparator);

    if (var == "PLACEHOLDER_OBJ_SEPARATOR")
      return validate(protocol::objSeparator);

    printer("templateProcessor::err no", qt(var), "found");
    return String();
  }

  String getHostName()
  {
    return (String)WiFi.getHostname();
  }

  void begin()
  {
    wifi::begin();

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Override Cross-Origin Resource Sharing (CORS)
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO:: rethink if it should be begun automatically? maybe start it when not found to prevent clocks taking? what about loop then
    WebSerial.begin(&server, "/console");
    WebSerial.onMessage([&](uint8_t *data, size_t len)
                        {
    String msg = String((char *)data, len);

    if (printToSerial)
      Serial.println("WebSerial =" + qt(msg));

    console::execute(msg); });

    server.on("/webSerial", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->redirect("/console"); });

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    webFS::begin();

    server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request)
              { request->redirect("/dashboard"); });

    server.on("/dashBoard_data", HTTP_GET, [](AsyncWebServerRequest *request)
              {
      JsonDocument doc;       

      doc["host"] = WiFi.getHostname();
      doc["date"] = __DATE__;
      doc["time"] = __TIME__;
      doc["model"] = ESP.getChipModel();
      doc["ip"] = WiFi.localIP().toString();
      doc["ssid"] = WiFi.SSID();
      doc["rssi"] = WiFi.RSSI();
      doc["frag"] = String(performance::getFragmentation()) + "%";
      doc["uptime"] = formatMs(millis());
      doc["heap"] = formatBytes(ESP.getFreeHeap(), ESP.getHeapSize());
      doc["sketch"] = formatBytes(ESP.getSketchSize(), ESP.getFreeSketchSpace() + ESP.getSketchSize());
      doc["psram"] = formatBytes(ESP.getMaxAllocPsram(), ESP.getPsramSize());
      doc["spiffs"]= formatBytes(SPIFFS.usedBytes(), SPIFFS.totalBytes()); 
      doc["interval"] = cache(storage::get("dashBoard_interval", 10000));

      String out;
      serializeJson(doc, out);
      request->send(200, "application/json", out); });

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    server.on("/pattern", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              {
      static String accumulatedData = "";

      // If this is the first chunk, reset the buffer
      if (index == 0)
      {
        accumulatedData = "";
        printer("Starting new pattern upload, total size:", total, "bytes");
      }

      char chunk[len + 1];
      memcpy(chunk, data, len);
      chunk[len] = '\0';
      accumulatedData += chunk;

      if (index + len != total)
        request->send(200, "text/plain", "chunk_ack");
      else
      {
        printer("Final chunk received, processing complete data");

        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, accumulatedData);

        if (err)
        {
          printer("JSON parse error:", err.c_str());
          request->send(400, "text/plain", "JSON parse error");
          return;
        }

        if (!doc.containsKey("patternName") || !doc.containsKey("powerFactor") || !doc.containsKey("hzRange") || !doc.containsKey("stepInterval") || !doc.containsKey("pattern"))
        {
          printer("ill formed");
          request->send(400, "text/plain", "ill formed");
          return;
        }

        printer("\npatternName:", doc["patternName"].as<String>(),
                "\npowerFactor:", doc["powerFactor"].as<int>(),
                "\nhzRange:", doc["hzRange"].as<int>(),
                "\nstepInterval:", doc["stepInterval"].as<int>(),
                "\nstartRightAway:", doc["startRightAway"].as<int>());

          const auto obj = patternMenager::dyspatch(doc["patternName"]);
          if (!obj)
            return;

          obj->stop();
          obj->powerFactor = doc["powerFactor"];
          obj->pwmHz = doc["hzRange"];
          obj->timer.setInterval(doc["stepInterval"]);
          obj->step = 0;
          obj->running = true;

          String in = doc["pattern"];
          char buf[in.length() + 1];
          in.toCharArray(buf, sizeof(buf));
          char *tok = strtok(buf, " ");
          int idx = 0;

          while(tok && idx < Pattern_t::len - 1 /* for termination */)
          {
              obj->pattern[idx] = atoi(tok);
              printer("step[", idx, "]", obj->pattern[idx]);
              tok = strtok(nullptr, " ");
              idx++;
          }
          obj->pattern[idx] = Pattern_t::terminator;
          obj->start();
          
          request->send(200, "application/json", "{\"status\":\"received\"}");

          accumulatedData = "";
        } });

    server.on("/config.json", HTTP_GET, [](AsyncWebServerRequest *request)
              {
      String out = storage::get("config.json");
      request->send(200, "application/json", out); });

    server.on("/config.json", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              {
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, data, len);
        
        if (err) {
          constexpr static auto text = "JSON parse error";
          request->send(400, "text/plain", text);
          printer(text, err.c_str());
          return;
        }
        
        if (!doc.containsKey("data")) {
          constexpr static auto text = "ill formated = doesn't contain key 'data'";
          printer(text);
          request->send(400, "text/plain", text);
          return;       
        }

        JsonArray dataArray = doc["data"];
        for (JsonObject item : dataArray) 
          printer("datename:", item["dataname"].as<String>(), "=", item["current_value"].as<String>());

        storage::put("config.json", doc);
        printer("Config updated via web interface");
        
        request->send(200, "application/json", "{\"status\":\"success\"}"); });

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // serve all files stored automatically
    server.onNotFound([](AsyncWebServerRequest *request)
                      {
                        String path = request->url();
                        path.toLowerCase();
                        printer("not found url", path);

                        if (path.indexOf(".") == -1) // default type
                          path += ".html";

                        if (SPIFFS.exists(path))
                        { 
                          request->send(SPIFFS, path, getContentType(path));
                          printer("file found, sending data");
                        } 
                        else
                          request->send(404, "text/html", "<style>body{background:#333;color:#eee;font-family:Arial;text-align:center}</style><br/><br/><h1>404 Not Found</h1><br/><p>Page doesn't exist.</p>"); });

    // preload all files to prevent need to search thru all files each time
    File root = SPIFFS.open("/");

    for (File file = root.openNextFile(); file; file = root.openNextFile())
    {
      const String filePath = "/" + String(file.name());
      const String contentType = getContentType(filePath);

      if (contentType == "text/plain")
        continue;

      printer("catched", qt(filePath), "as", contentType);

      server.on(filePath.c_str(), HTTP_GET, [filePath, contentType](AsyncWebServerRequest *request)
                { request->send(SPIFFS, filePath, contentType); });

      if (filePath.endsWith(".html")) // default type
      {
        String noExts = filePath;
        noExts.replace(".html", "");

        server.on(noExts.c_str(), HTTP_GET, [filePath, contentType](AsyncWebServerRequest *request)
                  { request->send(SPIFFS, filePath, contentType); });
      }
    }
    root.close();

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    server.begin();

    const String hostName = storage::get("hostName", "esp32"); // must be called after Wifi is initialized completly
    printer("hostName", qt(hostName) + ":",
            "wifi =", WiFi.setHostname(hostName.c_str()), "|",
            "mdns =", (MDNS.begin(hostName) && MDNS.addService("_http", "_tcp", port)));
  }

  namespace wifi
  {
    static IPAddress subnet(255, 255, 0, 0);
    static IPAddress gateway(192, 168, 1, 1);

    void captivePortal()
    {
      const String SSID = storage::get("hostName", "ESP32");
      const String password = storage::get("cp_password", "admin");

      printer("all ssid/password failed, starting captive portal", qt(SSID),
              qt(password));

      WiFi.disconnect();
      WiFi.softAP(SSID, password);

      server.on("/wifi_data", HTTP_POST, [](AsyncWebServerRequest *request)
                {
        if (!request->hasParam("SSID", true) ||
            request->hasParam("password", true)) {
          request->send(400, "text/plain", "missing org");
          printer("ap invalid new SSID data");
          return;
        }
        const String SSID = request->getParam("SSID", true)->value();
        const String password = request->getParam("password", true)->value();

        if (SSID.length() > 32 || password.length() > 32) {
          request->send(400, "text/plain", "data too long");
          printer("ap too long new data");
          return;
        }

        storage::put("wifi_SSID0", SSID);
        storage::put("wifi_passwword0", password);
        printer("new creditensials:", qt(SSID), qt(password));
        request->send(200, "text/plain", "ok, going to sta mode");

        WiFi.softAPdisconnect(true);
        wifi::begin(); });

      server.begin();
    }

    bool connect(String ssid, String password)
    {
      if (ssid == "" || password == "")
        return false;

      printer("trying", qt(ssid), "with", qt(password), "...");

      WiFi.disconnect();
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);

      Timer timer(max_wait_connection, true);
      while (!timer.timePassed())
      {
        delay(5);

        if (WiFi.status() == WL_CONNECTED)
        {
          printer("connected to ssid", qt(ssid) + ":", "IP:", getIP());
          return true;
        }
      }
      printer("connection to ssid", qt(ssid), "failed");
      return false;
    }

    bool findSpecificSSID(String SSID)
    {
      int n = WiFi.scanNetworks();

      for (int i = 0; i < n; ++i)
        if (WiFi.SSID(i) == SSID)
        {
          return true;
          break;
        }

      return false;
    }

    void begin()
    {
      String lastSSID = storage::get("wifi_lastSSID", "avi");
      String lastPassword = storage::get("wifi_lastPassword", "emka1968");

      if (connect(lastSSID, lastPassword))
        return;

      printerInfo("begin last used cred failed, searching thru list");

      for_x(max_credentials)
      {
        String SSID = storage::get("wifi_SSID" + (String)x);
        String password = storage::get("wifi_password" + (String)x);

        if (SSID == lastSSID) // prevent repetition
          continue;

        if (SSID.isEmpty() || password.isEmpty())
          break;

        if (connect(SSID, password))
        {
          storage::put("wifi_lastSSID", SSID);
          storage::put("wifi_lastPassword", password);
          printerInfo("begin last used creditentials updated to:", qt(SSID), qt(password));

          return;
        }
      }
      captivePortal();
    }

    String getCreditenstialsList()
    {
      String buff = "";
      for (int i = 0; i < max_credentials; i++)
      {
        String SSID = storage::get("wifi_SSID" + (String)i);
        String password = storage::get("wifi_password" + (String)i);

        if (SSID.isEmpty() || password.isEmpty())
          break;

        buff += qt(SSID) + " " + qt(password) +
                (i == max_credentials - 1 ? "" : ",\n");
      }
      return buff;
    }

    String getIP() { return WiFi.localIP().toString(); }

    String getRSSI() { return (String)WiFi.RSSI(); }

    String getSSID() { return (String)WiFi.SSID(); }

    String getScan()
    {
      int n = WiFi.scanNetworks(); // WiFi.scanNetworks returns the
                                   // number of networks found
      String buff = "wifi::scan found " + String(n) + " networks\n";

      for_x(n) buff += String(x + 1) + ": " + (String)WiFi.SSID(x) +
                       " (RRSI: " + (String)WiFi.RSSI(x) + "), ";

      return buff;
    }
  }

  namespace webFS
  {
    const char fs_minimal[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">
</head>
<style>
   body {
            background: #121212;
            color: #e0e0e0;
        }
</style>
<body>

  <button onclick="listFilesButton()">List Files</button>
  <button onclick="showUploadButtonFancy()">Upload File</button>
  </p>
  <p id="status"></p>
  <p id="detailsheader"></p>
  <p id="details"></p>
<script>

function listFilesButton() {
  xmlhttp=new XMLHttpRequest();
  xmlhttp.open("GET", "/listfiles", false);
  xmlhttp.send();
  document.getElementById("detailsheader").innerHTML = "<h3>Files<h3>";
  document.getElementById("details").innerHTML = xmlhttp.responseText;
}
function downloadDeleteButton(filename, action) {
  var urltocall = "/file?name=" + filename + "&action=" + action;
  xmlhttp=new XMLHttpRequest();
  if (action == "delete") {
    xmlhttp.open("GET", urltocall, false);
    xmlhttp.send();
    document.getElementById("status").innerHTML = xmlhttp.responseText;
    xmlhttp.open("GET", "/listfiles", false);
    xmlhttp.send();
    document.getElementById("details").innerHTML = xmlhttp.responseText;
  }
  if (action == "download") {
    document.getElementById("status").innerHTML = "";
    window.open(urltocall,"_blank");
  }
}
function showUploadButtonFancy() {
  document.getElementById("detailsheader").innerHTML = "<h3>Upload File<h3>"
  document.getElementById("status").innerHTML = "";
  var uploadform = "<form method = \"POST\" action = \"/\" enctype=\"multipart/form-data\"><input type=\"file\" name=\"data\"/><input type=\"submit\" name=\"upload\" value=\"Upload\" title = \"Upload File\"></form>"
  document.getElementById("details").innerHTML = uploadform;
  var uploadform =
  "<form id=\"upload_form\" enctype=\"multipart/form-data\" method=\"post\">" +
  "<input type=\"file\" name=\"file1\" id=\"file1\" onchange=\"uploadFile()\"><br>" +
  "<progress id=\"progressBar\" value=\"0\" max=\"100\" style=\"width:300px;\"></progress>" +
  "<h3 id=\"status\"></h3>" +
  "<p id=\"loaded_n_total\"></p>" +
  "</form>";
  document.getElementById("details").innerHTML = uploadform;
}
function _(el) {
  return document.getElementById(el);
}
function uploadFile() {
  var file = _("file1").files[0];
  // alert(file.name+" | "+file.size+" | "+file.type);
  var formdata = new FormData();
  formdata.append("file1", file);
  var ajax = new XMLHttpRequest();
  ajax.upload.addEventListener("progress", progressHandler, false);
  ajax.addEventListener("load", completeHandler, false); // doesnt appear to ever get called even upon success
  ajax.addEventListener("error", errorHandler, false);
  ajax.addEventListener("abort", abortHandler, false);
  ajax.open("POST", "/upload");
  ajax.send(formdata);
}
function progressHandler(event) {
  //_("loaded_n_total").innerHTML = "Uploaded " + event.loaded + " bytes of " + event.total; // event.total doesnt show accurate total file size
  _("loaded_n_total").innerHTML = "Uploaded " + event.loaded + " bytes";
  var percent = (event.loaded / event.total) * 100;
  _("progressBar").value = Math.round(percent);
  _("status").innerHTML = Math.round(percent) + "% uploaded... please wait";
  if (percent >= 100) {
    _("status").innerHTML = "Please wait, writing file to filesystem";
  }
}
function completeHandler(event) {
  _("status").innerHTML = "Upload Complete";
  _("progressBar").value = 0;
  xmlhttp=new XMLHttpRequest();
  xmlhttp.open("GET", "/listfiles", false);
  xmlhttp.send();
  document.getElementById("status").innerHTML = "File Uploaded";
  document.getElementById("detailsheader").innerHTML = "<h3>Files<h3>";
  document.getElementById("details").innerHTML = xmlhttp.responseText;
}
function errorHandler(event) {
  _("status").innerHTML = "Upload Failed";
}
function abortHandler(event) {
  _("status").innerHTML = "inUpload Aborted";
}
</script>
</body>
</html>
)rawliteral";

    static void fsUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
    {
      if (!checkUserAuth(request))
        return;

      if (!index)
        request->_tempFile = SPIFFS.open("/" + filename, FILE_WRITE);

      if (len)
        request->_tempFile.write(data, len);

      if (final)
      {
        request->_tempFile.close();
        request->redirect("/listFiles");
        printer("done", filename, formatBytes(len));
      }
    }
    void begin()
    {
      server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request)
                { request->send(200); }, fsUpload);

      server.on("/listfiles", HTTP_GET, [](AsyncWebServerRequest *request)
                {
                  DynamicJsonDocument doc(1024);
                  JsonArray arr = doc.to<JsonArray>();

                  File root = SPIFFS.open("/");
                  File f = root.openNextFile();
                  while (f) {
                    JsonObject obj = arr.createNestedObject();
                    obj["name"] = f.name();
                    obj["size"] = formatBytes(f.size());
                    f = root.openNextFile();
                  }
                  root.close();

                  AsyncResponseStream *response = request->beginResponseStream("application/json");
                  serializeJson(doc, *response);
                  request->send(response); 
                });

      server.on("/file", HTTP_ANY, [](AsyncWebServerRequest *request)
                {
                  if (!checkUserAuth(request)) return;

                  if (!request->hasParam("name") || !request->hasParam("action")) {
                    request->send(400, "text/plain", "ERROR: name and action required");
                    return;
                  }

                  const String path = "/" + request->getParam("name")->value();
                  const String fileAction = request->getParam("action")->value();

                  printer("incoming:", fileAction, path);

                  if (fileAction != "edit" && !storage::exists(path)) 
                  {
                    request->send(400, "text/plain", "err: " + qt(path) + " not found");
                    return;
                  }

                  if (fileAction == "download") 
                  {
                    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, path, "application/octet-stream");
                    response->addHeader("Content-Disposition", "attachment; filename=\"" + request->getParam("name")->value() + "\"");
                    request->send(response);
                  }
                  else if (fileAction == "delete")
                    request->send(200, "text/plain", "deletion = " + String(storage::remove(path)));
                  else if (fileAction == "read")
                    request->send(SPIFFS, path, "text/plain", storage::get(path));
                  else if (fileAction == "edit") 
                    return;   // Do nothing here; handled in body handler below
                  else
                    request->send(400, "text/plain", "unknown action: " + fileAction); 
                }, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
                  {
                    if (!request->hasParam("action") && request->getParam("action")->value() != "edit") 
                      return;

                    const String path = "/" + request->getParam("name")->value();

                    File f = SPIFFS.open(path, FILE_WRITE);
                    if (!f) return;

                    f.write(data, len);
                    if (index + len == total) {
                      f.close();
                      request->send(200, "text/plain", "ok");
                } });

      // Minimal filesystem page
      server.on("/fs", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send_P(200, "text/html", fs_minimal); });
  }

} // namespace webFS
} // namespace network
