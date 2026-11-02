// wifi_manager.cpp
#include "wifi_manager.h"
#include "config_manager.h"
#include <ESP8266WiFiGratuitous.h>

// ── 1 минута до AP-режима ──
static const uint32_t AP_TIMEOUT_MS         = 60UL * 1000UL;
// ── 15 секунд на одну сеть, чтобы успел пройти полный handshake + DHCP ──
static const uint32_t PER_NETWORK_TIMEOUT_MS = 15000;
static const char*    AP_SSID                = "NightClock-Setup";

// ============================================================
void WifiManager::begin(ConfigManager* cfg) {
    _cfg = cfg;
    _bootAt = millis();

    WiFi.persistent(false);
    WiFi.setAutoReconnect(false);   // переподключением управляем сами
    WiFi.setAutoConnect(false);     // не пытаться подключиться при boot
    WiFi.mode(WIFI_STA);
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    WiFi.setPhyMode(WIFI_PHY_MODE_11N);

    restartConnect();
}

// ============================================================
void WifiManager::restartConnect() {
    if (!_cfg) return;

    WiFi.disconnect();              // сбросить текущее
    _apMode           = false;
    _currentNetwork   = 0;
    _totalAttempts    = 0;
    _attemptStartMs   = 0;
    _inProgress       = false;

    WiFi.mode(WIFI_STA);
    Serial.printf("[WIFI] restartConnect, %d сет(и) в списке\n", _networkCount());
}

// ============================================================
int WifiManager::_networkCount() {
    if (!_cfg) return 0;
    return _cfg->doc()["wifi"].as<JsonArray>().size();
}

// ============================================================
void WifiManager::_tryNextNetwork() {
    auto arr = _cfg->doc()["wifi"].as<JsonArray>();
    int n = arr.size();
    if (n == 0) {
        Serial.println(F("[WIFI] нет сохранённых сетей"));
        return;
    }

    // Пробуем по очереди, начиная с _currentNetwork
    for (int i = 0; i < n; i++) {
        int idx = (_currentNetwork + i) % n;
        JsonObject w = arr[idx];
        String ssid = w["ssid"] | "";
        String pass = w["pass"] | "";

        if (ssid.length() == 0) continue;   // пропускаем пустые

        _currentNetwork  = idx + 1;         // след. раз начнём со следующей
        _totalAttempts++;
        _attemptStartMs  = millis();
        _inProgress      = true;

        Serial.printf("[WIFI] попытка #%u → '%s' (passlen=%u)\n",
                      _totalAttempts, ssid.c_str(), pass.length());

        WiFi.begin(ssid.c_str(), pass.c_str());
        return;
    }
}

// ============================================================
void WifiManager::update() {
    if (_apMode) return;

    wl_status_t st  = WiFi.status();
    uint32_t    now = millis();

    // Периодический лог раз в 3 сек
    static uint32_t lastLog = 0;
    if (now - lastLog > 3000) {
        lastLog = now;
        const char* sname = "?";
        switch (st) {
            case WL_IDLE_STATUS:    sname = "IDLE";         break;
            case WL_NO_SSID_AVAIL:  sname = "NO_SSID";      break;
            case WL_CONNECT_FAILED: sname = "AUTH_FAIL";    break;
            case WL_CONNECTED:      sname = "CONNECTED";    break;
            case WL_DISCONNECTED:   sname = "DISCONNECTED"; break;
            default: break;
        }
        Serial.printf("[WIFI] st=%s rssi=%d ssid='%s' attempts=%u\n",
                      sname, WiFi.RSSI(), WiFi.SSID().c_str(), _totalAttempts);
    }

    // 1) Подключились
    if (st == WL_CONNECTED) {
        if (_inProgress) {
            Serial.printf("[WIFI] ✓ CONNECTED: '%s' ip=%s\n",
                          WiFi.SSID().c_str(),
                          WiFi.localIP().toString().c_str());
            // Keep-alive: 30 сек без трафика — не даём роутеру «забыть» нас
            experimental::ESP8266WiFiGratuitous::stationKeepAliveSetIntervalMs(30000);
            _inProgress = false;
        }
        return;
    }

    // 2) Ещё не начинали — стартуем первую попытку
    if (!_inProgress) {
        _tryNextNetwork();
        return;
    }

    // 3) Немедленный провал (auth / no ssid) — не ждём таймаута
    if (st == WL_CONNECT_FAILED || st == WL_NO_SSID_AVAIL) {
        Serial.printf("[WIFI] попытка #%u провалилась мгновенно\n", _totalAttempts);
        _inProgress = false;
        WiFi.disconnect();
        // на следующей итерации loop() будет _tryNextNetwork()
        return;
    }

    // 4) Таймаут текущей попытки
    if (now - _attemptStartMs > PER_NETWORK_TIMEOUT_MS) {
        Serial.printf("[WIFI] попытка #%u таймаут (%lums)\n",
                      _totalAttempts, (unsigned long)PER_NETWORK_TIMEOUT_MS);
        _inProgress = false;
        WiFi.disconnect();
        return;
    }

    // 5) Общий таймаут — уходим в AP
    if (now - _bootAt > AP_TIMEOUT_MS) {
        Serial.printf("[WIFI] общий таймаут (%u попыток) → AP mode\n", _totalAttempts);
        _startApMode();
    }
}

// ============================================================
void WifiManager::_startApMode() {
    if (_apMode) return;
    _apMode = true;
    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID);
    Serial.printf("[WIFI] AP mode: %s / %s\n",
                  AP_SSID, WiFi.softAPIP().toString().c_str());
}

// ============================================================
//  Отложенный перезапуск (безопасен для вызова из web-хендлеров)
// ============================================================
void WifiManager::scheduleRestart(uint32_t delayMs) {
    _restartPending = true;
    _restartAtMs    = millis() + delayMs;
    Serial.printf("[WIFI] перезапуск через %lu мс\n", (unsigned long)delayMs);
}