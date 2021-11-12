/*
  ST7789 240x240 IPS (without CS pin) connections (only 6 wires required):
  #01 GND -> GND
  #02 VCC -> VCC (3.3V only!)
  #03 SCL -> D13/SCK
  #04 SDA -> D11/MOSI
  #05 RES -> D8 or any digital
  #06 DC  -> D7 or any digital
  #07 BLK -> NC

  MAX31855 K-Type thermocouple amplifier connections
  #01 VCC -> 3.3V
  #02 GND -> GND
  #03 SO  -> D3
  #04 CS  -> D4
  #05 SCK -> D5

  KEYS
  #01 PREV -> D9
  #02 NEXT -> D10
*/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Arduino_ST7789_Fast.h>
#include "Adafruit_MAX31855.h"

#define TFT_CS    9
#define TFT_DC    7
#define TFT_RST   8
#define SCR_WD   240
#define SCR_HT   240

#define MAXDO   4
#define MAXCS   5
#define MAXCLK  6

#define DEBOUNCE 30
#define NEXT_PIN 3
#define PREV_PIN 2

Arduino_ST7789 tft = Arduino_ST7789(TFT_DC, TFT_RST, TFT_CS);
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

bool linesDrawn = false;

float rawAFR, nowAFR, minAFR = 20, maxAFR = 10;
int nowEGT, minEGT = 2000, maxEGT = 10;
unsigned long lastTimeEGT = 0, lastTimeAFR = 0;

int nextLast, prevLast;
short mode = 0;

void setup(void)
{
  tft.init(SCR_WD, SCR_HT);
  tft.setRotation(1);
  
  pinMode(NEXT_PIN, INPUT_PULLUP);
  pinMode(PREV_PIN, INPUT_PULLUP);
}

void loop() {
//===================== MODE SELECT ========================
  if (digitalRead(NEXT_PIN) == 0 && nextLast == 1) {
    mode++;
    if(mode > 1) mode = 0;
    nextLast = 0;
    linesDrawn = false;
    delay(DEBOUNCE);
  }
  if (digitalRead(NEXT_PIN) == 1) nextLast = 1;
  
  if (digitalRead(PREV_PIN) == 0 && prevLast == 1) {
    mode--;
    if(mode < 0) mode = 1;
    prevLast = 0;
    linesDrawn = false;
    delay(DEBOUNCE);
  }
  if (digitalRead(PREV_PIN) == 1) prevLast = 1;
//===================== MODE0 EGT ==========================
  if (millis() - lastTimeEGT > 200) {
    nowEGT = thermocouple.readCelsius();
    if (isnan(nowEGT)) {
      tft.fillScreen(BLACK);
      tft.setTextSize(4);
      tft.setCursor(87, 30);
      tft.print("BLAD");
      tft.setCursor(47, 50);
      tft.print("TERMOPARY");
    } else {
      if (nowEGT < minEGT) minEGT = nowEGT;
      if (nowEGT > maxEGT) maxEGT = nowEGT;
      if (mode == 0) EGT_show(nowEGT, minEGT, maxEGT);
    }
    lastTimeEGT = millis();
  }
//===================== MODE1 AFR ==========================
  if (millis() - lastTimeAFR > 100) {
    rawAFR = analogRead(A0);
    nowAFR = (map(rawAFR, 0, 1023, 1090, 2020)) / 100.0;
    if (nowAFR < minAFR) minAFR = nowAFR;
    if (nowAFR > maxAFR) maxAFR = nowAFR;
    if (mode == 1) AFR_show(nowAFR, minAFR, maxAFR);
    lastTimeAFR = millis();
  }
}

void AFR_show(float nowAFR, float minAFR, float maxAFR) {
  if (linesDrawn == false) draw_lines();

  tft.setCursor(87, 5);
  tft.setTextColor(RED, BLACK);
  tft.setTextSize(4);
  tft.print("AFR");

  tft.setCursor(5, 175);
  tft.setTextSize(2);
  tft.print("MIN");

  tft.setCursor(125, 175);
  tft.setTextSize(2);
  tft.print("MAX");

  tft.setCursor(10, 70);
  tft.setTextSize(9);
  tft.print(nowAFR, 1);

  tft.setCursor(10, 198);
  tft.setTextSize(4);
  tft.print(minAFR, 1);

  tft.setCursor(130, 198);
  tft.setTextSize(4);
  tft.print(maxAFR, 1);
}

void EGT_show(int nowEGT, int minEGT, int maxEGT) {
  if (linesDrawn == false) draw_lines();

  tft.setCursor(87, 5);
  tft.setTextColor(RED, BLACK);
  tft.setTextSize(4);
  tft.print("EGT");

  tft.setCursor(5, 175);
  tft.setTextSize(2);
  tft.print("MIN");

  tft.setCursor(125, 175);
  tft.setTextSize(2);
  tft.print("MAX");

  tft.setCursor(40, 70);
  tft.setTextSize(9);
  tft.print(nowEGT, 1);

  tft.setCursor(23, 198);
  tft.setTextSize(4);
  tft.print(minEGT, 1);

  tft.setCursor(145, 198);
  tft.setTextSize(4);
  tft.print(maxEGT, 1);
}

void draw_lines() {
  tft.fillScreen(BLACK);
  tft.drawFastHLine(0, 40, tft.width(), RED);
  tft.drawFastHLine(0, 41, tft.width(), RED);
  tft.drawFastHLine(0, 170, tft.width(), RED);
  tft.drawFastHLine(0, 171, tft.width(), RED);
  tft.drawFastVLine(120, 170, tft.width(), RED);
  tft.drawFastVLine(121, 171, tft.width(), RED);
  linesDrawn = true;
}
