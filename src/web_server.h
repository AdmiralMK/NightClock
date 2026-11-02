// web_server.h
#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "config_manager.h"
#include "ntp_manager.h"
#include "wifi_manager.h"

class WebServerApp {
public:
    void begin(ConfigManager* cfg, NtpManager* ntp, WifiManager* wifi);
    void update() {}

    bool scanRequested = false;                 // выставляется true при POST /api/scan/start
    String lastScanResult = "[]";               // кеш последнего результата

private:
    AsyncWebServer _server{80};
    ConfigManager* _cfg  = nullptr;
    NtpManager*    _ntp  = nullptr;
    WifiManager*   _wifi = nullptr;
    String         _postBody;
};