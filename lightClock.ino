#ifdef ESP8266
#  include <ArduinoOTA.h>
#  include <ESP8266WiFi.h>
#  include <ESP8266mDNS.h>
#  include <WiFiManager.h>
#endif

#include <Adafruit_NeoPixel.h>
#include <Time.h>
#include <Timezone.h>  // https://github.com/JChristensen/Timezone

#include "NtpClient.h"

Adafruit_NeoPixel pixels = { 60, 5, NEO_GRB + NEO_KHZ800 };

static uint32_t PIXEL_COLOR_MINUTES;
static uint32_t PIXEL_COLOR_HOURS;

void infoLight(uint32_t color) {
  for (int i = 0; i < 60; i++) {
    pixels.setPixelColor(i, color);
    pixels.show();
    delay(10);

    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    pixels.show();
  }
}

time_t getLocalTime() {
  //Central European Time (Frankfurt, Paris)
  TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
  TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       //Central European Standard Time
  Timezone localTimezone(CEST, CET);
  return localTimezone.toLocal(now());
}

void setup() {
  Serial.begin(115200);

  pixels.begin();
  pixels.clear();
  pixels.show();

  PIXEL_COLOR_MINUTES = pixels.Color(192, 0, 64);
  PIXEL_COLOR_HOURS = pixels.Color(32, 0, 224);

  char clock_name[32];
  sprintf(clock_name, "lightclock-%06x", ESP.getChipId());
  ArduinoOTA.setHostname(clock_name);
  
  WiFi.hostname(clock_name);

  Serial.println("Starting WiFiManager ...");
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(120);
  wifiManager.autoConnect(clock_name);

  Serial.println("Initializing ...");
  infoLight(pixels.Color(255, 255, 255));
    
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Waiting for WiFi ...");
    infoLight(pixels.Color(255, 0, 0));
    delay(500);
  }

  // Show that we are connected
  Serial.println("WiFi connected...");
  infoLight(pixels.Color(0, 255, 0));

  ArduinoOTA.begin();

  syncTime();

  Serial.println("Setup complete.");
}

uint8_t p(uint8_t i) {
  return (i + 15) % 60;
}

void loop() {
  ArduinoOTA.handle();

  time_t t = getLocalTime();
  uint8_t h = (hour(t) % 12) * 5;
  uint8_t m = minute(t);

  h += m / 12;


  h = p(h);
  m = p(m);

  if (h == m) {
    paintOverlayClockHands(m);
  } else {
    paintClockHands(h, m);
  }
  
  pixels.show();
  delay(2500);
}

void paintOverlayClockHands(uint8_t m) {
  for (uint8_t i = 0; i < 60; i ++) {
    pixels.setPixelColor(i, PIXEL_COLOR_HOURS);
  }

  pixels.setPixelColor(m, PIXEL_COLOR_MINUTES);
  
}

void paintClockHands(uint8_t h, uint8_t m) {
  for (uint8_t i = h; i != m; i = (i + 1) % 60) {
    pixels.setPixelColor(i, PIXEL_COLOR_MINUTES);
  }

  for (uint8_t i = m; i != h; i = (i + 1) % 60) {
    pixels.setPixelColor(i, PIXEL_COLOR_HOURS);
  }
}

