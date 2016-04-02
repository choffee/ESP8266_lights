/*
 *  This sketch connect to a mqtt server and set the two neopixel lights connectd
 *  based on the setting received.
 *  
 *  Copyright John Cooper 2016
 *  
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <Adafruit_NeoPixel.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <ESP8266WiFi.h>
#include "wireless_creds.h"

// TODO: Move this to a .h config file
const char* ssid     = WIRELESS_SSID;
const char* password = WIRELESS_KEY;

// Store the MQTT server, username, and password in flash memory.
// This is required for using the Adafruit MQTT library.
// TODO: Move this to a .h file 
const char MQTT_SERVER[] PROGMEM    = "mqtt.local";

const char MQTT_USERNAME[] PROGMEM  = MQTT_USERNAME;
const char MQTT_PASSWORD[] PROGMEM  = MQTT_PASSWORD;

#define PIXEL_PIN 2 //GPIO 2
#define PIXEL_COUNT 2
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;


// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, 1883, MQTT_USERNAME, MQTT_PASSWORD);

// Setup a feed called 'color' for subscribing to changes.
const char LIGHT_FEED[] PROGMEM = _MQTT_USERNAME "/lights/esplights/";
Adafruit_MQTT_Subscribe lightscolor = Adafruit_MQTT_Subscribe(&mqtt, LIGHT_FEED);

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();


void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Setup MQTT subscription for onoff feed.
  //mqtt.subscribe(&lightscolor);

  pixels.begin();
  pixels.setPixelColor(0, pixels.Color(0,150,0));
  pixels.show();

}

int value = 0;

void loop() {
  delay(500);
  ++value;
  if (value > 255 ) { value = 0; };
  pixels.setPixelColor(0, pixels.Color(0,value,0));
  pixels.show();
  
}
