#include "GlobalDefine.h"

#include "WiFiHelper.h"

//#include <ESP8266HTTPClient.h> // includes WiFiClient.h
//#include <ESP8266WiFi.h>


#undef FOUND_BOARD
#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
extern "C" {
#include "user_interface.h"
#include "wpa2_enterprise.h"
}
#define FOUND_BOARD ESP8266
#endif

#ifdef ARDUINO_ARCH_ESP32
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "esp_wpa2.h"
#define FOUND_BOARD ESP32
#endif

#ifdef ARDUINO_SAMD_MKRWIFI1010
#include <WiFiNINA.h>
#define FOUND_BOARD ARDUINO_SAMD_MKRWIFI1010
#endif

#ifndef FOUND_BOARD
#pragma message(Reminder "Error Target hardware not defined !")
#endif // ! FOUND_BOARD


String myMacAddress;

String HexString(byte buffer[], byte bufferSize) {
	String res = "";
	for (byte i = 0; i < bufferSize; i++) {
		res += HEX_CHARS[(buffer[i] >> 4) & 0xF];
		res += HEX_CHARS[buffer[i] & 0xF];
	}
	return res;
}

String wifiMacAddress() {
	if (!myMacAddress || (myMacAddress == "")) {
		myMacAddress = "";
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
		myMacAddress = WiFi.macAddress(); // this returns 6 hex bytes, delimited by colons
#endif
#if defined(ARDUINO_SAMD_MKRWIFI1010)
		byte mac[6];
		WiFi.macAddress(mac);
		myMacAddress = HexString(mac, 6);
#endif

		myMacAddress.replace(":", "");
		myMacAddress.replace("-", ""); // probably not used, but just in case they MAC address starts returning other well known delimiters such as dash
		myMacAddress.replace(" ", ""); // or perhaps even a space
	}

    return myMacAddress;
}

void WiFiStart(bool EnterpriseMode) {
	WIFI_DEBUG_PRINTLN(DEBUG_SEPARATOR);
	WIFI_DEBUG_PRINT("wifiConnect: Connecting to ");
	WIFI_DEBUG_PRINTLN(SECRET_WIFI_SSID);
	WIFI_DEBUG_PRINTLN(DEBUG_SEPARATOR);


#ifdef ARDUINO_ARCH_ESP8266
	WIFI_DEBUG_PRINTLN("ARDUINO_ARCH_ESP8266");
#endif

	if (EnterpriseMode) {
		// WPA2 Connection starts here
		// Setting ESP into STATION mode only (no AP mode or dual mode)
		WIFI_DEBUG_PRINTLN("Enterprise mode configured...");

#ifdef ARDUINO_ARCH_ESP8266
		WiFi.disconnect(true);

		// WPA2 Connection starts here
	// Setting ESP into STATION mode only (no AP mode or dual mode)
		WIFI_DEBUG_PRINTLN("Starting ESP8266 Enterprise WiFi...");
		wifi_set_opmode(STATION_MODE);

		struct station_config wifi_config;
		memset(&wifi_config, 0, sizeof(wifi_config));
		strcpy((char*)wifi_config.ssid, SECRET_WIFI_SSID);

		wifi_station_set_config(&wifi_config);

		wifi_station_clear_cert_key();
		wifi_station_clear_enterprise_ca_cert();
		wifi_station_set_wpa2_enterprise_auth(1);
		wifi_station_set_enterprise_identity((uint8*)SECRET_EAP_ID, strlen(SECRET_EAP_ID));
		wifi_station_set_enterprise_username((uint8*)SECRET_EAP_USERNAME, strlen(SECRET_EAP_USERNAME));
		wifi_station_set_enterprise_password((uint8*)SECRET_EAP_PASSWORD, strlen(SECRET_EAP_PASSWORD));
		wifi_station_connect();
		// WPA2 Connection ends here
#endif

#ifdef ARDUINO_ARCH_ESP32
		WiFi.disconnect(true);

		WIFI_DEBUG_PRINTLN("Starting ESP32 Enterprise WiFi...");
		WiFi.mode(WIFI_STA); // be sure to set mode FIRST
		esp_wifi_sta_wpa2_ent_set_identity((uint8_t*)SECRET_EAP_ID, strlen(SECRET_EAP_ID)); //provide identity
		esp_wifi_sta_wpa2_ent_set_username((uint8_t*)SECRET_EAP_USERNAME, strlen(SECRET_EAP_USERNAME)); //provide username
		esp_wifi_sta_wpa2_ent_set_password((uint8_t*)SECRET_EAP_PASSWORD, strlen(SECRET_EAP_PASSWORD)); //provide password
		esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT();
		esp_wifi_sta_wpa2_ent_enable(&config);
		WiFi.begin(SECRET_WIFI_SSID);
#endif

#ifdef ARDUINO_SAMD_MKRWIFI1010
		WiFi.disconnect();

		WIFI_DEBUG_PRINTLN("Starting WiFiNINA Enterprise WiFi...(not implemented in early WiFiNINA Versions, see Version 1.5.0 or later)");
		WiFi.beginEnterprise(SECRET_WIFI_SSID, SECRET_EAP_USERNAME, SECRET_EAP_PASSWORD);
#endif // ARDUINO_SAMD_MKRWIFI1010

	}
	else {
		WIFI_DEBUG_PRINTLN("Starting regular Wi-Fi...");
		HEAP_DEBUG_PRINTLN(DEFAULT_DEBUG_MESSAGE);
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
		WiFi.mode(WIFI_STA);
#else
		// station mode assumed.
#endif
		HEAP_DEBUG_PRINTLN(DEFAULT_DEBUG_MESSAGE);
		WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PWD);
		HEAP_DEBUG_PRINTLN(DEFAULT_DEBUG_MESSAGE);
	}
}

//*******************************************************************************************************************************************
// wifiConnect 
// 
//   WiFi.begin with repeated attempts with TFT screen and optional serial progress indication
//
//*******************************************************************************************************************************************
int wifiConnect(int maxAttempts) {
	int countAttempt = 0;
	WiFiStart(IS_EAP); // see GlobalDefine.h to set Enterprise Access Point on or off


	
	WIFI_DEBUG_PRINTLN("Starting WiFi Connection Loop...");
	while (WiFi.status() != WL_CONNECTED) {  // try to connect wifi for 6 sec then reset

											 // this tft code is not actualy DOING anything yet

		WIFI_DEBUG_PRINT(".");
		delay(250);
        if (IS_EAP) {
			// TODO - do we ever give up on EAP?
        }
        else
        {
            countAttempt++;
            if (countAttempt > maxAttempts) {
				countAttempt = 0;
				WIFI_DEBUG_PRINTLN(F("WiFi Disconnect... "));
				WiFi.disconnect();
				delay(5000);
				WIFI_DEBUG_PRINT(F("WiFi Retrying. "));
				WIFI_DEBUG_PRINTLN(F(SECRET_WIFI_SSID));

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
				WiFi.mode(WIFI_STA);
#else
				// station mode assumed.
#endif
				WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PWD);
				// TODO reboot?
			}
		}
	}
	WIFI_DEBUG_PRINTLN("WiFi Connected!");
	delay(5000); // TODO why wait?

	Serial.println("MAC Address=" + wifiMacAddress());
	HEAP_DEBUG_PRINTLN(DEFAULT_DEBUG_MESSAGE);
	WIFI_DEBUG_PRINT("wifiConnect: Done!");
	return 0;
}