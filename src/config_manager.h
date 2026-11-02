// D:\ArduinoApp\NightClock\src\config_manager.h
#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>

struct DaySchedule {
    String   mode;              // "time" | "sun" | "hybrid"
    String   start;             // "HH:MM"
    String   end;               // "HH:MM"
    int16_t  sunOffsetStart;    // мин. до восхода (обычно отрицательное)
    int16_t  sunOffsetEnd;      // мин. после заката
    String   sunHardLimit;      // "HH:MM" — не позднее этого момента
    uint8_t  dayBrightness;     // 0..7
    uint8_t  nightBrightness;   // 0..7
    uint16_t fadeSec;           // 0..60
};

class ConfigManager {
public:
    bool begin();
    bool load();
    bool save();
    void loadDefaults();

    // Полная перезапись конфигурации из JSON-строки (для POST /api/config)
    bool replaceFromJson(const String& json);

    JsonDocument& doc() { return _doc; }

    DaySchedule getDaySchedule(uint8_t dow);   // 0 = воскресенье
    bool        isHoliday(const String& dateStr);
    int         wifiCount();

    // Возвращает true один раз после изменения конфига — сигнал пересчёта
    bool consumeChanged();
    void markChanged() { _changed = true; }

private:
    JsonDocument _doc;
    bool _loaded  = false;
    bool _changed = false;
};