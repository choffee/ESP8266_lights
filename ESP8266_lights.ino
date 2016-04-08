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
const char MQTT_SERVER[] PROGMEM    = MQTT_HOST;

const char _MQTT_USERNAME[] PROGMEM  = MQTT_USERNAME;
const char _MQTT_PASSWORD[] PROGMEM  = MQTT_PASSWORD;

#define PIXEL_PIN 2 //GPIO 2
#define PIXEL_COUNT 2
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;


// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, 1883, _MQTT_USERNAME, _MQTT_PASSWORD);

// Setup a feed called 'color' for subscribing to changes.
const char LIGHT_FEED[] PROGMEM = "lights/esplights/rgb";
Adafruit_MQTT_Subscribe lightsrgb = Adafruit_MQTT_Subscribe(&mqtt, LIGHT_FEED);

const char STATUS_FEED[] PROGMEM = "lights/esplights/status";
Adafruit_MQTT_Publish status_feed = Adafruit_MQTT_Publish(&mqtt, STATUS_FEED);

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
  mqtt.subscribe(&lightsrgb);

  pixels.begin();
  pixels.setPixelColor(0, pixels.Color(0,150,0));
  pixels.show();

}

int value = 0;

int string_rgb_to_int(String input, byte rgb[3]) {
  int result = 0;
  for ( int i = 0; i < 3 ; i++ ) {
      if (rgb[i] = atoi((char *)input[i, i+2]) ) {
        result = 3; // If we get a zero above it's a fail
      }
  }
  return result;
}

void loop() {
  
  MQTT_connect();

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    // Check if its the rgb feed
    if (subscription == &lightsrgb) {
      // Expecting 6 padded bytes of RGB.
      // eg. 255255000128001255
      Serial.print(F("Lights RGB: "));
      Serial.println((char *)lightsrgb.lastread);
      status_feed.publish((char *)lightsrgb.lastread);

      byte this_rgb[3];
      byte result = 0;
      byte color_val;
      int offset = 0;
      for (int light = 0; light < 2 ; light++) {
        for (int colour = 0; colour < 3; colour++) {
          color_val = 0;
          for (int i = 2; i >= 0; i--) {
            char this_val = lightsrgb.lastread[offset];
            color_val = color_val + atoi(&this_val) * pow( 10, i );
            offset++;
          }
          this_rgb[colour] = color_val;
        }
        pixels.setPixelColor(light, pixels.Color(this_rgb[0], this_rgb[1], this_rgb[2]));
      }
      pixels.show();
      
    }
  }

  // ping the server to keep the mqtt connection alive
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }  
  
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  status_feed.publish("Connected");
  Serial.println("MQTT Connected!");
}
