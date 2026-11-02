// brightness_engine.cpp
#include "brightness_engine.h"
#include <SolarCalculator.h>

// Сравнение «время в интервале [start; end]» с переходом через полночь
static bool inRange(int nowMin, int startMin, int endMin) {
    if (startMin <= endMin) return nowMin >= startMin && nowMin < endMin;
    return nowMin >= startMin || nowMin < endMin;
}

void BrightnessEngine::_updateSunTimes(const struct tm& tm) {
    // Пересчитываем не чаще раза в день
    time_t today = mktime(const_cast<struct tm*>(&tm));
    time_t dayStart = today - (tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec);
    if (_sunCacheDate == dayStart) return;
    _sunCacheDate = dayStart;

    auto loc = _cfg->doc()["location"].as<JsonObject>();
    double lat = loc["lat"] | 56.8389;
    double lon = loc["lon"] | 60.6057;
    int    off = _cfg->doc()["ntp"]["offsetHours"] | 5;

    double transit, sunrise, sunset;
    calcSunriseSunset(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                      lat, lon, transit, sunrise, sunset);
    // Библиотека возвращает часы UTC (double). Переводим в локальные секунды.
    _sunriseLocal = (time_t)(dayStart + (int)((sunrise + off) * 3600.0));
    _sunsetLocal  = (time_t)(dayStart + (int)((sunset  + off) * 3600.0));
}

time_t BrightnessEngine::_parseHHMM(const String& s, const struct tm& day) {
    if (s.length() < 4) return 0;
    int h = s.substring(0, 2).toInt();
    int m = s.substring(3, 5).toInt();
    struct tm t = day;
    t.tm_hour = h; t.tm_min = m; t.tm_sec = 0;
    return mktime(&t);
}

bool BrightnessEngine::_nightByTime(const struct tm& tm, const DaySchedule& ds) {
    int nowMin   = tm.tm_hour * 60 + tm.tm_min;
    int startMin = ds.start.substring(0, 2).toInt() * 60 + ds.start.substring(3, 5).toInt();
    int endMin   = ds.end.substring(0, 2).toInt()   * 60 + ds.end.substring(3, 5).toInt();
    return inRange(nowMin, startMin, endMin);
}

bool BrightnessEngine::_nightBySun(time_t now, const DaySchedule& ds) {
    time_t sunrise = _sunriseLocal + ds.sunOffsetStart * 60;
    time_t sunset  = _sunsetLocal  + ds.sunOffsetEnd   * 60;
    return (now < sunrise) || (now >= sunset);
}

bool BrightnessEngine::_nightHybrid(time_t now, const struct tm& tm, const DaySchedule& ds) {
    // 1) базовый расчёт от солнца
    time_t sunrise = _sunriseLocal + ds.sunOffsetStart * 60;
    time_t sunset  = _sunsetLocal  + ds.sunOffsetEnd   * 60;

    // 2) жёсткий лимит на sunset
    if (ds.sunHardLimit.length() >= 4) {
        time_t limit = _parseHHMM(ds.sunHardLimit, tm);
        if (limit && sunset > limit) sunset = limit;
    }
    // 3) дополнительное ограничение: не позднее чем time-start
    //    (если пользователь заполнил time-start)
    if (ds.start.length() >= 4) {
        time_t t = _parseHHMM(ds.start, tm);
        if (t && sunset > t) sunset = t;
    }
    return (now < sunrise) || (now >= sunset);
}

uint8_t BrightnessEngine::_applyAdaptiveLight(uint8_t base) {
    auto al = _cfg->doc()["adaptiveLight"].as<JsonObject>();
    if (!(al["enabled"] | false)) return base;

    int raw     = analogRead(al["pin"] | 17);
    int minRaw  = al["minRaw"]  | 0;
    int maxRaw  = al["maxRaw"]  | 1023;
    uint8_t mnB = al["minBrightness"] | 1;
    uint8_t mxB = _cfg->doc()["display"]["maxBrightness"] | 7;

    if (maxRaw <= minRaw) return base;
    raw = constrain(raw, minRaw, maxRaw);
    // Предполагаем: pull-up + фоторезистор к GND → свет ярче = raw меньше
    float f = 1.0f - (float)(raw - minRaw) / (float)(maxRaw - minRaw);
    uint8_t out = (uint8_t)lroundf(base * f);
    return constrain(out, mnB, mxB);
}

BrightnessEngine::Result BrightnessEngine::compute(const struct tm& tm) {
    Result r { 0, 30 };
    if (!_cfg) return r;

    auto disp = _cfg->doc()["display"].as<JsonObject>();
    uint8_t maxB = disp["maxBrightness"] | 7;

    // Определяем, какой день недели использовать
    char dateStr[16];   // было 11 — добавили запас
    int yr = constrain(tm.tm_year + 1900, 1970, 2099);
    int mo = constrain(tm.tm_mon + 1, 1, 12);
    int dy = constrain(tm.tm_mday, 1, 31);
    snprintf(dateStr, sizeof(dateStr), "%04d-%02d-%02d", yr, mo, dy);

    uint8_t dow = tm.tm_wday; // 0=Sun
    // Праздники → используем расписание воскресенья
    if (_cfg->isHoliday(String(dateStr))) dow = 0;

    DaySchedule ds = _cfg->getDaySchedule(dow);

    _updateSunTimes(tm);
    time_t now = mktime(const_cast<struct tm*>(&tm));

    bool night = false;
    if      (ds.mode == "time")   night = _nightByTime(tm, ds);
    else if (ds.mode == "sun")    night = _nightBySun(now, ds);
    else if (ds.mode == "hybrid") night = _nightHybrid(now, tm, ds);

    uint8_t base = night ? ds.nightBrightness : ds.dayBrightness;
    if (base > maxB) base = maxB;

    r.brightness = _applyAdaptiveLight(base);
    r.fadeSec    = ds.fadeSec;
    return r;
}