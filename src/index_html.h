// src/index_html.h
#pragma once
#include <Arduino.h>   // для PROGMEM
#include <pgmspace.h>  // явно подключаем, чтобы PROGMEM был определён[reference:1]

// Только объявление: extern сообщает компилятору, что массив
// определён в другом месте (в index_html.cpp)
extern const char INDEX_HTML[] PROGMEM;