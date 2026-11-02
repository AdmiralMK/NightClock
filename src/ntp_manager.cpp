// ntp_manager.cpp
#include "ntp_manager.h"

void NtpManager::begin(const String servers[], int count,
                       const String& posixTz) {
    // В ESP8266 core >= 2.7.0 правильный способ установки TZ —
    // передать его напрямую в configTime(), а не через setenv().
    const char* tz = posixTz.length() ? posixTz.c_str() : "UTC0";

    if (count >= 3)
        configTime(tz, servers[0].c_str(), servers[1].c_str(), servers[2].c_str());
    else if (count == 2)
        configTime(tz, servers[0].c_str(), servers[1].c_str(), nullptr);
    else if (count == 1)
        configTime(tz, servers[0].c_str(), nullptr, nullptr);
    else
        configTime(tz, "pool.ntp.org", nullptr, nullptr);

    _started   = true;
    _startedAt = millis();
    Serial.printf("[NTP] started, TZ='%s'\n", tz);
}

time_t NtpManager::now() { return time(nullptr); }

bool NtpManager::getLocalTm(struct tm& t) {
    time_t n = time(nullptr);
    if (n < 1577836800UL) return false;
    localtime_r(&n, &t);
    return true;
}

void NtpManager::update() {
    if (!_started || _synced) return;
    if (time(nullptr) > 1577836800UL) {
        _synced = true;
        struct tm t;
        time_t now = time(nullptr);
        localtime_r(&now, &t);
        Serial.printf("[NTP] synced: TZ='%s' UTC=%ld local=%02d:%02d:%02d\n",
                      getenv("TZ"), (long)now,
                      t.tm_hour, t.tm_min, t.tm_sec);
    }
}

void NtpManager::setTimezone(const String& posixTz) {
    const char* tz = posixTz.length() ? posixTz.c_str() : "UTC0";
    // Перезапуск SNTP с новым TZ (серверы можно оставить прежние)
    configTime(tz, "pool.ntp.org", nullptr, nullptr);
    Serial.printf("[NTP] timezone changed to '%s'\n", tz);
}