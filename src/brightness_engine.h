// brightness_engine.h
#pragma once
#include <Arduino.h>
#include <time.h>
#include "config_manager.h"

class BrightnessEngine {
public:
    void begin(ConfigManager* cfg) { _cfg = cfg; }

    struct Result { uint8_t brightness; uint16_t fadeSec; };
    Result compute(const struct tm& tm);

private:
    ConfigManager* _cfg = nullptr;

    time_t _sunCacheDate = 0;
    time_t _sunriseLocal = 0;
    time_t _sunsetLocal  = 0;

    void   _updateSunTimes(const struct tm& tm);
    static time_t _parseHHMM(const String& s, const struct tm& day);
    bool   _nightByTime(const struct tm& tm, const DaySchedule& ds);
    bool   _nightBySun(time_t now, const DaySchedule& ds);
    bool   _nightHybrid(time_t now, const struct tm& tm, const DaySchedule& ds);
    uint8_t _applyAdaptiveLight(uint8_t base);
};