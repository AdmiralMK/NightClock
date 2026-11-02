// D:\ArduinoApp\NightClock\src\main.cpp
#include <Arduino.h>
#include <Ticker.h>

#include "config_manager.h"
#include "display_manager.h"
#include "ntp_manager.h"
#include "wifi_manager.h"
#include "brightness_engine.h"
#include "web_server.h"

// ============================================================
//  Глобалы, доступные из web_server.cpp
// ============================================================
bool    g_sleepMode         = false;   // режим «сон» (кнопка D7)
uint8_t g_currentBrightness = 0;       // текущая яркость (для превью в UI)

// ============================================================
//  Пины
// ============================================================
static const uint8_t PIN_CLK = D6;   // GPIO12
static const uint8_t PIN_DIO = D5;   // GPIO14
static const uint8_t PIN_BTN = D7;   // GPIO13

// ============================================================
//  Объекты
// ============================================================
ConfigManager    cfg;
DisplayManager   display(PIN_CLK, PIN_DIO);
NtpManager       ntp;
WifiManager      wifi;
BrightnessEngine brEngine;
WebServerApp     web;

Ticker           rebootTicker;

// ============================================================
//  Состояние приложения
// ============================================================
enum AppState : uint8_t {
    APP_BOOT,        // анимация "PLAY"
    APP_WIFI_WAIT,   // горизонтальная анимация "----"
    APP_NTP_WAIT,    // вертикальная анимация "----"
    APP_RUN          // рабочий режим
};
static AppState s_state = APP_BOOT;

static bool     s_btnPressed     = false;
static uint32_t s_btnDebounce    = 0;

static uint32_t s_lastBrightCheck = 0;
static uint32_t s_lastClockSecond = 0;
static bool     s_colonOn         = true;

// ============================================================
//  Публичная функция для web_server.cpp — отложенная перезагрузка
// ============================================================
void scheduleReboot() {
    rebootTicker.once_ms(1000, []() { ESP.restart(); });
}

// ============================================================
//  Кнопка «Сон»
// ============================================================
static void handleButton(uint32_t now) {
    const bool pressed = (digitalRead(PIN_BTN) == LOW);
    const uint32_t debounceMs =
        cfg.doc()["button"]["sleepDebounceMs"] | 300;

    if (pressed && !s_btnPressed && now - s_btnDebounce > debounceMs) {
        s_btnDebounce = now;
        s_btnPressed  = true;

        g_sleepMode = !g_sleepMode;
        Serial.printf("[BTN] sleep = %s\n", g_sleepMode ? "ON" : "OFF");

        if (g_sleepMode) {
            display.setTargetBrightness(0, 300);
        }
        // при выключении сна — движок пересчитает яркость
        s_lastBrightCheck = 0;
    }
    if (!pressed) s_btnPressed = false;
}

// ============================================================
//  Пересчёт целевой яркости
// ============================================================
static void recalcBrightness() {
    struct tm t;
    if (!ntp.getLocalTm(t)) return;

    if (g_sleepMode) {
        display.setTargetBrightness(0, 200);
        return;
    }

    auto r = brEngine.compute(t);
    uint32_t fadeMs = (uint32_t)r.fadeSec * 1000UL;
    display.setTargetBrightness(r.brightness, fadeMs);
    Serial.printf("[BR] -> %u (fade %us)\n", r.brightness, r.fadeSec);
}

// ============================================================
//  Вывод времени
// ============================================================
static void renderClock() {
    struct tm t;
    if (!ntp.getLocalTm(t)) return;
    display.showTime(t.tm_hour, t.tm_min, s_colonOn);
}

// ============================================================
//  setup()
// ============================================================
void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println();
    Serial.println(F("=== NightClock boot ==="));

    // --- кнопка ---
    pinMode(PIN_BTN, INPUT_PULLUP);

    // --- конфигурация ---
    if (!cfg.begin()) {
        Serial.println(F("[MAIN] Config init failed — defaults used"));
    }

    // --- дисплей ---
    uint8_t maxB = cfg.doc()["display"]["maxBrightness"] | 7;
    display.begin(maxB);

    // --- движок яркости ---
    brEngine.begin(&cfg);

    // --- 1. Анимация «PLAY» ---
    display.startBootAnimation();
    Serial.println(F("[MAIN] state = BOOT (PLAY)"));

    // --- 2. Wi-Fi ---
    wifi.begin(&cfg);

    // --- 3. NTP ---
    String servers[3];
    int sCount = 0;
    for (JsonVariant v : cfg.doc()["ntp"]["servers"].as<JsonArray>()) {
        if (sCount >= 3) break;
        servers[sCount++] = v.as<String>();
    }
    const char* posixTz = cfg.doc()["ntp"]["posixTz"] | "YEKT-5";
    ntp.begin(servers, sCount, posixTz);
    Serial.printf("[MAIN] NTP: %d server(s)\n", sCount);

    // --- 4. Веб-сервер ---
    web.begin(&cfg, &ntp, &wifi);
    Serial.printf("[MAIN] Web UI: http://%s/\n",
                  WiFi.localIP().toString().c_str());

    // --- 5. Дождаться окончания анимации PLAY ---
    while (display.currentBrightness() != maxB) { display.update(); }

    // --- 6. Ожидание Wi-Fi ---
    s_state = APP_WIFI_WAIT;
    display.startWifiWaitAnimation();
    Serial.println(F("[MAIN] state = WIFI_WAIT"));
}

// ============================================================
//  loop()
// ============================================================
void loop() {
    const uint32_t now = millis();

    // --- фоновые сервисы ---
    display.update();
    ntp.update();
    wifi.update();
    handleButton(now);

    // публикуем текущую яркость для web-ui
    g_currentBrightness = display.currentBrightness();

    // --- конечный автомат ---
    switch (s_state) {

        case APP_WIFI_WAIT:
            if (wifi.isConnected() || wifi.isApMode()) {
                s_state = APP_NTP_WAIT;
                display.startNtpWaitAnimation();
                Serial.printf("[MAIN] state = NTP_WAIT (%s)\n",
                              wifi.isApMode() ? "AP" : "STA");
            }
            break;

        case APP_NTP_WAIT:
            if (ntp.isSynced()) {
                display.stopAnimation();
                s_state = APP_RUN;
                s_lastBrightCheck = now;   // ← не 0, а now
                recalcBrightness();         // ← применить яркость сразу
                Serial.println(F("[MAIN] state = RUN"));
            }
            break;

        case APP_RUN: {
            // 1) конфиг обновлён через web-панель
            if (cfg.consumeChanged()) {
                uint8_t mb = cfg.doc()["display"]["maxBrightness"] | 7;
                display.setMaxBrightness(mb);
                Serial.printf("[MAIN] config reloaded, max=%u\n", mb);

                // форсируем немедленный пересчёт яркости по расписанию
                if (ntp.isSynced() && !g_sleepMode) {
                    struct tm t;
                    if (ntp.getLocalTm(t)) {
                        auto r = brEngine.compute(t);
                        // короткий переход 500 мс — чтобы изменения были видны сразу
                        display.setTargetBrightness(r.brightness, 500);
                        Serial.printf("[MAIN] brightness -> %u\n", r.brightness);
                    }
                }
                s_lastBrightCheck = now;
            }

            // 2) раз в 30 секунд — проверка расписания яркости
            if (now - s_lastBrightCheck > 30000UL) {
                s_lastBrightCheck = now;
                if (ntp.isSynced()) recalcBrightness();
            }

            // 3) раз в секунду — обновление времени + мигающее двоеточие
            if (now - s_lastClockSecond > 1000UL) {
                s_lastClockSecond = now;
                s_colonOn = !s_colonOn;
                if (!g_sleepMode) renderClock();
                else              display.showBlank();
            }
            break;
        }

        default:
            break;
    }
}