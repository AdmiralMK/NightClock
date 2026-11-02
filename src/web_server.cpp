// web_server.cpp
#include "web_server.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include "index_html.h"

extern bool g_sleepMode; // см. main.cpp

// ============================================================================
//  Регистрация маршрутов
// ============================================================================
void WebServerApp::begin(ConfigManager *cfg, NtpManager *ntp, WifiManager *wifi)
{
  _cfg = cfg;
  _ntp = ntp;
  _wifi = wifi;

  // --- статика ---
  _server.on("/", HTTP_GET, [](AsyncWebServerRequest *r)
             { r->send_P(200, "text/html", INDEX_HTML); });

  // --- GET /api/config ---
  _server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest *r)
             {
        String out;
        serializeJson(_cfg->doc(), out);
        r->send(200, "application/json", out); });

  // --- POST /api/config ---
  _server.on("/api/config", HTTP_POST, [this](AsyncWebServerRequest *r)
             {
                 bool ok = _cfg->replaceFromJson(_postBody);
                 _postBody = "";
                 if (!ok) {
                     r->send(400, "application/json", F("{\"error\":\"parse\"}"));
                     return;
                 }

                 // ↓↓↓ ДОБАВЛЕНО ↓↓↓
                 // 1) Применить новый часовой пояс НЕМЕДЛЕННО
                 _ntp->setTimezone(_cfg->doc()["ntp"]["posixTz"] | "UTC0");

                 // 2) Перезапуск Wi-Fi — ОТЛОЖЕННО, чтобы не порвать TCP-ответ
                 _wifi->scheduleRestart(1500);
                 // ↑↑↑ ДОБАВЛЕНО ↑↑↑

                 r->send(200, "application/json", F("{\"ok\":true}")); }, nullptr, [this](AsyncWebServerRequest *, uint8_t *data, size_t len, size_t index, size_t total)
             {
                 if (index == 0) _postBody = "";
                 if (total > 8192) { _postBody = ""; return; }
                 _postBody.concat(reinterpret_cast<const char*>(data),
                                  (unsigned int)len); });

  // --- GET /api/status ---
  _server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *r)
             {
        JsonDocument d;
        d["wifiConnected"] = (WiFi.status() == WL_CONNECTED);
        d["ssid"]          = WiFi.SSID();
        d["ip"]            = WiFi.localIP().toString();
        d["apMode"]        = _wifi->isApMode();
        d["ntpSynced"]     = _ntp->isSynced();
        d["sleep"]         = g_sleepMode;
        d["uptime"]        = millis();
        d["heap"]          = ESP.getFreeHeap();

        struct tm t;
        if (_ntp->getLocalTm(t)) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
            d["time"] = buf;
            char dateBuf[16];
            int yr = constrain(t.tm_year + 1900, 1970, 2099);
            int mo = constrain(t.tm_mon + 1, 1, 12);
            int dy = constrain(t.tm_mday, 1, 31);
            snprintf(dateBuf, sizeof(dateBuf), "%04d-%02d-%02d", yr, mo, dy);
            d["date"] = dateBuf;
        } else {
            d["time"] = "";
            d["date"] = "";
        }

        // текущая яркость — читаем из main через extern (упрощённо)
        extern uint8_t g_currentBrightness;
        d["brightness"] = g_currentBrightness;

        String out;
        serializeJson(d, out);
        r->send(200, "application/json", out); });

  // --- POST /api/scan/start  (запуск сканирования) ---
  _server.on("/api/scan/start", HTTP_POST, [this](AsyncWebServerRequest *r)
             {
        WiFi.scanNetworks(true);        // асинхронный режим — возвращается мгновенно
        scanRequested = true;
        r->send(200, "application/json", F("{\"status\":\"started\"}")); });

  // --- GET /api/scan/result  (опрос результата) ---
  _server.on("/api/scan/result", HTTP_GET, [this](AsyncWebServerRequest *r)
             {
        if (!scanRequested) {
            // нет активного скана — отдаём кеш
            r->send(200, "application/json", lastScanResult);
            return;
        }
        int n = WiFi.scanComplete();
        if (n == WIFI_SCAN_RUNNING) {
            r->send(200, "application/json", F("{\"status\":\"running\"}"));
            return;
        }
        if (n == WIFI_SCAN_FAILED) {
            scanRequested = false;
            WiFi.scanDelete();
            r->send(200, "application/json", F("[]"));
            return;
        }
        // скан завершён — собираем JSON и сразу отвечаем
        scanRequested = false;
        JsonDocument doc;
        JsonArray arr = doc.to<JsonArray>();
        for (int i = 0; i < n; i++) {
            JsonObject o = arr.add<JsonObject>();
            o["ssid"]      = WiFi.SSID(i);
            o["rssi"]      = WiFi.RSSI(i);
            o["encrypted"] = (WiFi.encryptionType(i) != ENC_TYPE_NONE);
        }
        WiFi.scanDelete();
        serializeJson(doc, lastScanResult);
        r->send(200, "application/json", lastScanResult); });

  // --- GET /api/light ---
  _server.on("/api/light", HTTP_GET, [this](AsyncWebServerRequest *r)
             {
        int pin = _cfg->doc()["adaptiveLight"]["pin"] | 17;
        JsonDocument d;
        d["value"] = analogRead(pin);
        String out;
        serializeJson(d, out);
        r->send(200, "application/json", out); });

  // --- POST /api/reboot ---
  extern void scheduleReboot(); // определена в main.cpp

  _server.on("/api/reboot", HTTP_POST, [](AsyncWebServerRequest *r)
             {
               r->send(200, "application/json", F("{\"ok\":true}"));
               scheduleReboot(); // ESP.restart() через 1 секунду
             });

  _server.begin();
  Serial.println(F("WebServer: запущен на :80"));
}
