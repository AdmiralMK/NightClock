// wifi_manager.h
#pragma once
#include <Arduino.h>
#include <ESP8266WiFi.h>

class ConfigManager;

class WifiManager {
public:
    void begin(ConfigManager* cfg);
    void update();

    bool isConnected() const { return WiFi.status() == WL_CONNECTED; }
    bool isApMode()    const { return _apMode; }
    String ssid()      const { return WiFi.SSID(); }
    IPAddress ip()     const { return WiFi.localIP(); }

    void scheduleRestart(uint32_t delayMs = 1500);   // ← ЭТА СТРОКА
    void restartConnect();

private:
    ConfigManager* _cfg = nullptr;
    bool     _apMode = false;
    uint32_t _bootAt = 0;

    // ← НОВОЕ: отложенный перезапуск
    bool     _restartPending = false;
    uint32_t _restartAtMs    = 0;

    uint32_t _attemptStartMs = 0;
    uint8_t  _currentNetwork = 0;
    uint8_t  _totalAttempts  = 0;
    bool     _inProgress     = false;

    void _startApMode();
    void _tryNextNetwork();
    int  _networkCount();
};