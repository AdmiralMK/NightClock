// D:\ArduinoApp\NightClock\src\config_manager.cpp
#include "config_manager.h"
#include <LittleFS.h>

static const char* CONFIG_PATH = "/config.json";

// ============================================================
bool ConfigManager::begin() {
    if (!LittleFS.begin()) {
        Serial.println(F("[CFG] LittleFS mount failed"));
        return false;
    }
    if (!LittleFS.exists(CONFIG_PATH)) {
        Serial.println(F("[CFG] config.json not found, writing defaults"));
        loadDefaults();
        return save();
    }
    return load();
}

// ============================================================
bool ConfigManager::load() {
    File f = LittleFS.open(CONFIG_PATH, "r");
    if (!f) {
        Serial.println(F("[CFG] open for read failed"));
        return false;
    }
    DeserializationError err = deserializeJson(_doc, f);
    f.close();
    if (err) {
        Serial.printf("[CFG] parse error: %s\n", err.c_str());
        return false;
    }
    _loaded = true;
    Serial.println(F("[CFG] loaded"));
    return true;
}

// ============================================================
bool ConfigManager::save() {
    File f = LittleFS.open(CONFIG_PATH, "w");
    if (!f) {
        Serial.println(F("[CFG] open for write failed"));
        return false;
    }
    size_t n = serializeJsonPretty(_doc, f);
    f.close();
    Serial.printf("[CFG] saved %u bytes\n", (unsigned)n);
    _changed = true;
    return true;
}

// ============================================================
bool ConfigManager::replaceFromJson(const String& json) {
    // Резервная копия — на случай битого запроса
    String backup;
    serializeJson(_doc, backup);

    _doc.clear();
    DeserializationError err = deserializeJson(_doc, json);
    if (err) {
        Serial.printf("[CFG] replace parse: %s\n", err.c_str());
        _doc.clear();
        deserializeJson(_doc, backup);
        return false;
    }
    return save();
}

// ============================================================
void ConfigManager::loadDefaults() {
    _doc.clear();

    // ---- Wi-Fi ----
    auto wifi = _doc["wifi"].to<JsonArray>();
    auto w = wifi.add<JsonObject>();
    w["ssid"] = "XXXXXXXXXXXXX";
    w["pass"] = "XXXXXXXXXXXXX";

    // ---- NTP ----
    auto ntp = _doc["ntp"].to<JsonObject>();
    auto srv = ntp["servers"].to<JsonArray>();
    srv.add("0.ru.pool.ntp.org");
    srv.add("time.windows.com");
    srv.add("192.168.0.1");
    ntp["timezone"]    = "Asia/Yekaterinburg";
    ntp["posixTz"] = "YEKT-5";
    ntp["offsetHours"] = 5;

    // ---- Координаты (Екатеринбург) ----
    auto loc = _doc["location"].to<JsonObject>();
    loc["lat"] = 56.8389;
    loc["lon"] = 60.6057;

    // ---- Дисплей ----
    _doc["display"]["maxBrightness"] = 7;

    // ---- Кнопка сна ----
    auto btn = _doc["button"].to<JsonObject>();
    btn["pin"]             = 13;    // D7
    btn["sleepDebounceMs"] = 300;

    // ---- Фоторезистор ----
    auto al = _doc["adaptiveLight"].to<JsonObject>();
    al["enabled"]       = false;
    al["pin"]           = 17;       // A0
    al["minRaw"]        = 0;
    al["maxRaw"]        = 1023;
    al["minBrightness"] = 1;

    // ---- Расписание на все 7 дней ----
    auto sch = _doc["schedule"].to<JsonObject>();
    for (int i = 0; i < 7; i++) {
        char key[2];
        snprintf(key, sizeof(key), "%d", i);
        auto d = sch[key].to<JsonObject>();
        d["mode"]            = "time";
        d["start"]           = "23:00";
        d["end"]             = "07:00";
        d["sunOffsetStart"]  = -60;
        d["sunOffsetEnd"]    =  60;
        d["sunHardLimit"]    = "";
        d["dayBrightness"]   = 7;
        d["nightBrightness"] = 1;
        d["fadeSec"]         = 30;
    }

    // ---- Праздники ----
    auto hol = _doc["holidays"].to<JsonArray>();
    hol.add("2026-01-01");
    hol.add("2026-01-07");
    hol.add("2026-02-23");
    hol.add("2026-03-08");
    hol.add("2026-05-01");
    hol.add("2026-05-09");
}

// ============================================================
DaySchedule ConfigManager::getDaySchedule(uint8_t dow) {
    DaySchedule ds;
    char key[2];
    snprintf(key, sizeof(key), "%d", dow % 7);

    auto sch = _doc["schedule"].as<JsonObject>();
    if (sch[key].isNull()) {
        // ничего не настроено — вернуть «дневные» настройки по умолчанию
        ds.mode            = "time";
        ds.start           = "23:00";
        ds.end             = "07:00";
        ds.sunOffsetStart  = -60;
        ds.sunOffsetEnd    =  60;
        ds.sunHardLimit    = "";
        ds.dayBrightness   = 7;
        ds.nightBrightness = 1;
        ds.fadeSec         = 30;
        return ds;
    }

    auto d = sch[key].as<JsonObject>();
    ds.mode            = d["mode"]            | "time";
    ds.start           = d["start"]           | "23:00";
    ds.end             = d["end"]             | "07:00";
    ds.sunOffsetStart  = d["sunOffsetStart"]  | -60;
    ds.sunOffsetEnd    = d["sunOffsetEnd"]    |  60;
    ds.sunHardLimit    = d["sunHardLimit"]    | "";
    ds.dayBrightness   = d["dayBrightness"]   | 7;
    ds.nightBrightness = d["nightBrightness"] | 1;
    ds.fadeSec         = d["fadeSec"]         | 30;
    return ds;
}

// ============================================================
bool ConfigManager::isHoliday(const String& dateStr) {
    auto arr = _doc["holidays"].as<JsonArray>();
    for (JsonVariant v : arr) {
        if (v.as<String>() == dateStr) return true;
    }
    return false;
}

// ============================================================
int ConfigManager::wifiCount() {
    return _doc["wifi"].as<JsonArray>().size();
}

// ============================================================
bool ConfigManager::consumeChanged() {
    bool c = _changed;
    _changed = false;
    return c;
}