// display_manager.h
#pragma once
#include <Arduino.h>
#include <TM1637TinyDisplay.h>

class DisplayManager
{
public:
    DisplayManager(uint8_t clk, uint8_t dio) : _display(clk, dio) {}

    void begin(uint8_t maxBright);
    void update(); // вызывать в loop()

    void setTargetBrightness(uint8_t target, uint32_t fadeMs);
    void setMaxBrightness(uint8_t max)
    {
        _max = constrain(max, 0, 7);
        // если текущая/целевая яркость выше нового максимума — приглушаем сразу
        if (_target > _max)
            _target = _max;
        if (_current > _max)
        {
            _current = _max;
            _display.setBrightness(_current, _current > 0);
            _fading = false;
        }
    }
    uint8_t currentBrightness() const { return _current; }

    void showTime(uint8_t h, uint8_t m, bool colonOn);
    void showString(const char *s);
    void showBlank();

    void startBootAnimation();
    void startWifiWaitAnimation();
    void startNtpWaitAnimation();
    void stopAnimation();

private:
    TM1637TinyDisplay _display;
    uint8_t _current = 0, _target = 0, _max = 7;
    bool _fading = false;
    uint32_t _lastStep = 0, _stepInterval = 0;

    enum Anim : uint8_t
    {
        ANIM_NONE,
        ANIM_WIFI_H,
        ANIM_NTP_V
    };
    Anim _anim = ANIM_NONE;
    uint32_t _animLastStep = 0;
    uint8_t _animFrame = 0;

    void _renderAnimFrame();
};