// D:\ArduinoApp\NightClock\src\display_manager.cpp
#include "display_manager.h"

// Таблица сегментов цифр 0..9 (без бита двоеточия/DP)
static const uint8_t DIGIT_SEGS[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

// Бит двоеточия на 4-разрядном TM1637 обычно сидит на digit[1] (второй разряд).
// Если на вашем модуле двоеточие не появляется — см. примечание внизу.
static const uint8_t COLON_BIT = 0x80;

// ============================================================
void DisplayManager::begin(uint8_t maxBright) {
    _display.begin();
    _max = constrain(maxBright, 0, 7);
    _display.clear();
    _display.setBrightness(1, false);   // экран погашен
    _current = _target = 0;
}

// ============================================================
void DisplayManager::update() {
    // --- плавная яркость ---
    if (_fading && _current != _target) {
        if (millis() - _lastStep >= _stepInterval) {
            _lastStep = millis();
            _current += (_target > _current) ? 1 : -1;
            _display.setBrightness(_current, _current > 0);
            if (_current == _target) _fading = false;
        }
    }
    // --- анимации ---
    if (_anim != ANIM_NONE && millis() - _animLastStep >= 250) {
        _animLastStep = millis();
        _animFrame++;
        _renderAnimFrame();
    }
}

// ============================================================
void DisplayManager::setTargetBrightness(uint8_t target, uint32_t fadeMs) {
    target = constrain(target, 0, _max);
    if (target == _target && _fading) return;
    _target = target;

    if (fadeMs == 0 || _current == _target) {
        _current = _target;
        _display.setBrightness(_current, _current > 0);
        _fading = false;
        return;
    }
    int steps = abs((int)_target - (int)_current);
    _stepInterval = max<uint32_t>(1, fadeMs / steps);
    _lastStep = millis() - _stepInterval;
    _fading = true;
}

// ============================================================
//  Вывод времени с ручным управлением двоеточием
// ============================================================
void DisplayManager::showTime(uint8_t h, uint8_t m, bool colonOn) {
    uint8_t segs[4];
    segs[0] = DIGIT_SEGS[h / 10];
    segs[1] = DIGIT_SEGS[h % 10] | (colonOn ? COLON_BIT : 0);
    segs[2] = DIGIT_SEGS[m / 10];
    segs[3] = DIGIT_SEGS[m % 10];
    _display.setSegments(segs, 4, 0);
}

// ============================================================
void DisplayManager::showString(const char* s) { _display.showString(s); }
void DisplayManager::showBlank()               { _display.clear(); }

// ============================================================
void DisplayManager::startBootAnimation() {
    _anim = ANIM_NONE;
    _display.showString("PLAY");
    _current = 1;
    _display.setBrightness(1, true);
    setTargetBrightness(_max, 2000);
}

void DisplayManager::startWifiWaitAnimation() {
    _anim = ANIM_WIFI_H;
    _animFrame = 0;
    _animLastStep = millis();
    _current = _target = _max; _fading = false;
    _display.setBrightness(_max, true);
    _renderAnimFrame();
}

void DisplayManager::startNtpWaitAnimation() {
    _anim = ANIM_NTP_V;
    _animFrame = 0;
    _animLastStep = millis();
    _current = _target = _max; _fading = false;
    _display.setBrightness(_max, true);
    _renderAnimFrame();
}

void DisplayManager::stopAnimation() { _anim = ANIM_NONE; }

// ============================================================
void DisplayManager::_renderAnimFrame() {
    switch (_anim) {
        case ANIM_WIFI_H: {
            // горизонтальный бегущий «----»
            static const char* frames[] = {
                "----", " ---", "  --", "   -",
                "    ", "-   ", "--  ", "--- "
            };
            _display.showString(frames[_animFrame & 7]);
            break;
        }
        case ANIM_NTP_V: {
            // вертикальный «прыгающий» сегмент: верх → середина → низ → середина
            // SEG_A / SEG_G / SEG_D — макросы из TM1637TinyDisplay.h
            uint8_t pattern;
            switch (_animFrame & 3) {
                case 0: pattern = SEG_A; break;
                case 1: pattern = SEG_G; break;
                case 2: pattern = SEG_D; break;
                default: pattern = SEG_G; break;
            }
            uint8_t segs[4] = { pattern, pattern, pattern, pattern };
            _display.setSegments(segs, 4, 0);   // без 4-го аргумента
            break;
        }
        default: break;
    }
}