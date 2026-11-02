// ntp_manager.h
#pragma once
#include <Arduino.h>
#include <time.h>

class NtpManager {
public:
    void begin(const String servers[], int count,
               const String& posixTz);

    void update();
    void setTimezone(const String& posixTz);
    bool isSynced()  const { return _synced; }
    bool isWaiting() const { return _started && !_synced; }
    bool isStarted() const { return _started; }

    time_t  now();
    bool    getLocalTm(struct tm& t);

private:
    bool     _started = false;
    bool     _synced  = false;
    uint32_t _startedAt = 0;
};

