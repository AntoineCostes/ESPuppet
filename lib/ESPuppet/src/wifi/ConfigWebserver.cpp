#include "ConfigWebserver.h"

String processor(const String &var)
{

  if (var == "TITLE") return "ESPuppet Config";
  if (var == "CONFIG") {
    Preferences prefs;
    prefs.begin("ESPuppet");
    String configFileName = prefs.getString("config", "notfound");
    prefs.end();
    return configFileName;
  }
  return "[???]";
}

String infoProcessor(const String &var)
{
  if (var.equals("uptime")) return (String)(millis() / 1000 / 60) + " mn " + (String)((millis() / 1000) % 60) + "s";
  if (var.equals("chipid")) return String((uint32_t)ESP.getEfuseMac(),HEX);
  if (var.equals("chiprev")) return (String)ESP.getChipRevision();
  if (var.equals("idesize")) return (String)ESP.getFlashChipSize();
  if (var.equals("flashsize")) return (String)ESP.getPsramSize();
  if (var.equals("cpufreq")) return (String)ESP.getCpuFreqMHz();
  if (var.equals("freeheap")) return (String)ESP.getFreeHeap();
  if (var.equals("memsketch")) return (String)(ESP.getSketchSize())+" / "+ (String)(ESP.getFreeSketchSpace());
  if (var.equals("memsketch_used")) return (String)(ESP.getSketchSize());
  if (var.equals("memsketch_free")) return (String)(ESP.getFreeSketchSpace());
  if (var.equals("memsmeter_max")) return (String)(ESP.getSketchSize()+ESP.getFreeSketchSpace());
  if (var.equals("temp")) return (String)temperatureRead();
  if (var.equals("conx")) return WiFi.isConnected() ?"Yes":"No";
  if (var.equals("staip")) return WiFi.localIP().toString();
  if (var.equals("stagw")) return WiFi.gatewayIP().toString();
  if (var.equals("stasub")) return WiFi.subnetMask().toString();
  if (var.equals("dnss")) return WiFi.dnsIP().toString();
  if (var.equals("host")) return WiFi.getHostname();
  if (var.equals("stamac")) return WiFi.macAddress();
  if (var.equals("apip")) return WiFi.softAPIP().toString();
  if (var.equals("apmac")) return (String)WiFi.softAPmacAddress();
  if (var.equals("aphost")) return WiFi.softAPgetHostname();
  if (var.equals("apbssid")) return (String)WiFi.BSSIDstr() ;
  return "[???]";
}

ConfigWebserver::ConfigWebserver(bool serialDebug) : Component("webserver", serialDebug)
{
}

void ConfigWebserver::start()
{
  dnsServer = new DNSServer();
  dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
  // dnsServer->setTTL(300); TODO what is this for ?
  dnsServer->start(53, "*", WiFi.softAPIP());
  dbg("started on :" + WiFi.softAPIP().toString());

  server = new AsyncWebServer(80);
  // does not work ?
  server->addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER); // only when requested from AP

  // TODO CSS in separate file or change placeholder
  // TODO test wifi change
  server->on("/", HTTP_GET, std::bind(&ConfigWebserver::serveIndex, this, std::placeholders::_1));
  server->on("/info", HTTP_GET, std::bind(&ConfigWebserver::serveInfo, this, std::placeholders::_1));
  server->on("/wifi", HTTP_GET, std::bind(&ConfigWebserver::serveWifi, this, std::placeholders::_1));
  server->on("/custom", HTTP_GET, std::bind(&ConfigWebserver::serveCustomIndex, this, std::placeholders::_1));
  // server->onNotFound(std::bind(&ConfigWebserver::serveIndex, this, std::placeholders::_1));
  server->onNotFound([](AsyncWebServerRequest *request)
                     {
                      Serial.println("NOT FOUND"); 
                      // TODO not found page
                      request->send(LittleFS, "/info.html", String(), false, processor); });
  server->begin();
}

void ConfigWebserver::serveInfo(AsyncWebServerRequest *request)
{
  dbg("serve info");
  request->send(LittleFS, "/info.html", String(), false, infoProcessor);
}

void ConfigWebserver::serveWifi(AsyncWebServerRequest *request)
{
  dbg("serve wifi");
  request->send(LittleFS, "/wifi.html", String(), false, infoProcessor);
}

void ConfigWebserver::serveIndex(AsyncWebServerRequest *request)
{
  dbg("SERVE INDEX");
  request->send(LittleFS, "/index.html", String(), false, infoProcessor);
}

  void ConfigWebserver::serveCustomIndex(AsyncWebServerRequest *request)
  {
  dbg("SERVE CUSTOM INDEX");

  // TODO handleRoot()
  // getHTTPHead(_title);
  String page;
  page += "<!DOCTYPE html>"
          "<html lang='en'><head>"
          "<meta name='format-detection' content='telephone=no'>"
          "<meta charset='UTF-8'>"
          "<meta  name='viewport' content='width=device-width,initial-scale=1,user-scalable=no'/>"
          "<title>ESPUPPET CONFIG</title>";
  page += "<script>function c(l){"
          "document.getElementById('s').value=l.getAttribute('data-ssid')||l.innerText||l.textContent;"
          "p = l.nextElementSibling.classList.contains('l');"
          "document.getElementById('p').disabled = !p;"
          "if(p)document.getElementById('p').focus();};"
          "function f() {var x = document.getElementById('p');x.type==='password'?x.type='text':x.type='password';}"
          "</script>";
  page +=
      "<style>"
      ".c,body{text-align:center;font-family:verdana}div,input,select{padding:5px;font-size:1em;margin:5px 0;box-sizing:border-box}"
      "input,button,select,.msg{border-radius:.3rem;width: 100%}input[type=radio],input[type=checkbox]{width:auto}"
      "button,input[type='button'],input[type='submit']{cursor:pointer;border:0;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%}"
      "input[type='file']{border:1px solid #1fa3ec}"
      ".wrap {text-align:left;display:inline-block;min-width:260px;max-width:500px}"
      // links
      "a{color:#000;font-weight:700;text-decoration:none}a:hover{color:#1fa3ec;text-decoration:underline}"
      // quality icons
      ".q{height:16px;margin:0;padding:0 5px;text-align:right;min-width:38px;float:right}.q.q-0:after{background-position-x:0}.q.q-1:after{background-position-x:-16px}.q.q-2:after{background-position-x:-32px}.q.q-3:after{background-position-x:-48px}.q.q-4:after{background-position-x:-64px}.q.l:before{background-position-x:-80px;padding-right:5px}.ql .q{float:left}.q:after,.q:before{content:'';width:16px;height:16px;display:inline-block;background-repeat:no-repeat;background-position: 16px 0;"
      "background-image:url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAGAAAAAQCAMAAADeZIrLAAAAJFBMVEX///8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADHJj5lAAAAC3RSTlMAIjN3iJmqu8zd7vF8pzcAAABsSURBVHja7Y1BCsAwCASNSVo3/v+/BUEiXnIoXkoX5jAQMxTHzK9cVSnvDxwD8bFx8PhZ9q8FmghXBhqA1faxk92PsxvRc2CCCFdhQCbRkLoAQ3q/wWUBqG35ZxtVzW4Ed6LngPyBU2CobdIDQ5oPWI5nCUwAAAAASUVORK5CYII=');}"
      // icons @2x media query (32px rescaled)
      "@media (-webkit-min-device-pixel-ratio: 2),(min-resolution: 192dpi){.q:before,.q:after {"
      "background-image:url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAALwAAAAgCAMAAACfM+KhAAAALVBMVEX///8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADAOrOgAAAADnRSTlMAESIzRGZ3iJmqu8zd7gKjCLQAAACmSURBVHgB7dDBCoMwEEXRmKlVY3L//3NLhyzqIqSUggy8uxnhCR5Mo8xLt+14aZ7wwgsvvPA/ofv9+44334UXXngvb6XsFhO/VoC2RsSv9J7x8BnYLW+AjT56ud/uePMdb7IP8Bsc/e7h8Cfk912ghsNXWPpDC4hvN+D1560A1QPORyh84VKLjjdvfPFm++i9EWq0348XXnjhhT+4dIbCW+WjZim9AKk4UZMnnCEuAAAAAElFTkSuQmCC');"
      "background-size: 95px 16px;}}"
      // msg callouts
      ".msg{padding:20px;margin:20px 0;border:1px solid #eee;border-left-width:5px;border-left-color:#777}.msg h4{margin-top:0;margin-bottom:5px}.msg.P{border-left-color:#1fa3ec}.msg.P h4{color:#1fa3ec}.msg.D{border-left-color:#dc3630}.msg.D h4{color:#dc3630}.msg.S{border-left-color: #5cb85c}.msg.S h4{color: #5cb85c}"
      // lists
      "dt{font-weight:bold}dd{margin:0;padding:0 0 0.5em 0;min-height:12px}"
      "td{vertical-align: top;}"
      ".h{display:none}"
      "button{transition: 0s opacity;transition-delay: 3s;transition-duration: 0s;cursor: pointer}"
      "button.D{background-color:#dc3630}"
      "button:active{opacity:50% !important;cursor:wait;transition-delay: 0s}"
      // invert
      "body.invert,body.invert a,body.invert h1 {background-color:#060606;color:#fff;}"
      "body.invert .msg{color:#fff;background-color:#282828;border-top:1px solid #555;border-right:1px solid #555;border-bottom:1px solid #555;}"
      "body.invert .q[role=img]{-webkit-filter:invert(1);filter:invert(1);}"
      ":disabled {opacity: 0.5;}"
      "</style>";
  page += "</head><body class='invert'><div class='wrap'>";

  String boardName = "name";
  String str = "<h1>{ESPUPPET}</h1><h3>" + boardName + "-" + WiFi.localIP().toString() + "</h3>";

  page += str;

  String menuWifi= "<form action='/wifi'   method='get'><button>Configure WiFi (No scan)</button></form><br/>\n";
String menuInfo = "<form action='/info'    method='get'><button>Info</button></form><br/>\n";
String menuParam = "<form action='/param'   method='get'><button>Setup</button></form><br/>\n"; 
String menuSep = "<hr><br/>";
String menuUpdate = "<form action='/update'  method='get'><button>Update</button></form><br/>\n";
  
  page += menuWifi;
  page += menuInfo;
  // page += menuParam;
  page += menuSep;
  page += menuUpdate;

  // reportStatus(page);

  page += String("</div></body></html>");

  request->send(200, "text/html", page);
}

void ConfigWebserver::stop()
{
  log("STOP");
  dnsServer->stop();
  server->end();
}

void ConfigWebserver::update()
{
  if (millis() % 500 < 1)
    Serial.print(".");
  dnsServer->processNextRequest();
}