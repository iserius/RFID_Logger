// htmlHelper.h

#ifndef _HTMLHELPER_h
#define _HTMLHELPER_h

#if defined(ARDUINO) && ARDUINO >= 100
    #include "arduino.h"
#else
    #include "WProgram.h"
#endif

#undef FOUND_BOARD
#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#define FOUND_BOARD ESP8266
#pragma message(Reminder "Target hardware: ESP8266")
#endif

#ifdef ARDUINO_ARCH_ESP32
#include <HTTPClient.h>
#include <WiFi.h>
#define FOUND_BOARD ESP32
#pragma message(Reminder "Target hardware: ESP32")
#endif

#ifndef FOUND_BOARD
#pragma message(Reminder "Error Target hardware not defined !")
#endif // ! FOUND_BOARD

String HTML_RequestText(String url);
void HTML_SendRequest(WiFiClientSecure *thisClient, String TheRequest, String& MovedToURL);

#endif