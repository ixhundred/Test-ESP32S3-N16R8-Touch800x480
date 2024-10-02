#include <Arduino.h>
#include <EEPROM.h>
#include <SPI.h>
#include <SPIFFS.h>
#include <Wire.h>

#include <Arduino_GFX_Library.h>
#define GFX_BL 2 // default backlight pin, you may replace DF_GFX_BL to actual backlight pin
#if defined(DISPLAY_DEV_KIT)
Arduino_GFX *gfx = create_default_Arduino_GFX();
#else /* !defined(DISPLAY_DEV_KIT) */

/* More data bus class: https://github.com/moononournation/Arduino_GFX/wiki/Data-Bus-Class */
Arduino_DataBus *bus = create_default_Arduino_DataBus();

/* More display class: https://github.com/moononournation/Arduino_GFX/wiki/Display-Class */
Arduino_GFX *gfx = new Arduino_ILI9341(bus, DF_GFX_RST, 0 /* rotation */, false /* IPS */);

#endif /* !defined(DISPLAY_DEV_KIT) */


/* uncomment for XPT2046 */
#define TOUCH_XPT2046
#define TOUCH_XPT2046_SCK 12
#define TOUCH_XPT2046_MISO 13
#define TOUCH_XPT2046_MOSI 11
#define TOUCH_XPT2046_CS 38
#define TOUCH_XPT2046_INT 18
#define TOUCH_XPT2046_ROTATION 0
#define TOUCH_XPT2046_SAMPLES 50

#include <XPT2046_Touchscreen.h>
XPT2046_Touchscreen ts(TOUCH_XPT2046_CS, TOUCH_XPT2046_INT);

// Please fill below values from Arduino_GFX Example - TouchCalibration
bool touch_swap_xy = false;
int16_t touch_map_x1 = -1;
int16_t touch_map_x2 = -1;
int16_t touch_map_y1 = -1;
int16_t touch_map_y2 = -1;

int16_t touch_max_x = 0, touch_max_y = 0;
int16_t touch_raw_x = 0, touch_raw_y = 0;
int16_t touch_last_x = 0, touch_last_y = 0;

uint32_t    tsec = 0;

void touch_init(int16_t w, int16_t h, uint8_t r)
{
  touch_max_x = w - 1;
  touch_max_y = h - 1;
  if (touch_map_x1 == -1)
  {
    switch (r)
    {
    case 3:
      touch_swap_xy = true;
      touch_map_x1 = touch_max_x;
      touch_map_x2 = 0;
      touch_map_y1 = 0;
      touch_map_y2 = touch_max_y;
      break;
    case 2:
      touch_swap_xy = false;
      touch_map_x1 = touch_max_x;
      touch_map_x2 = 0;
      touch_map_y1 = touch_max_y;
      touch_map_y2 = 0;
      break;
    case 1:
      touch_swap_xy = true;
      touch_map_x1 = 0;
      touch_map_x2 = touch_max_x;
      touch_map_y1 = touch_max_y;
      touch_map_y2 = 0;
      break;
    default: // case 0:
      touch_swap_xy = false;
      touch_map_x1 = 0;
      touch_map_x2 = touch_max_x;
      touch_map_y1 = 0;
      touch_map_y2 = touch_max_y;
      break;
    }
  }

  SPI.begin(TOUCH_XPT2046_SCK, TOUCH_XPT2046_MISO, TOUCH_XPT2046_MOSI, TOUCH_XPT2046_CS);
  ts.begin();
  ts.setRotation(TOUCH_XPT2046_ROTATION);
}

bool touch_has_signal()
{
  return ts.tirqTouched();
}

void translate_touch_raw()
{
  if (touch_swap_xy)
  {
    touch_last_x = map(touch_raw_y, touch_map_x1, touch_map_x2, 0, touch_max_x);
    touch_last_y = map(touch_raw_x, touch_map_y1, touch_map_y2, 0, touch_max_y);
  }
  else
  {
    touch_last_x = map(touch_raw_x, touch_map_x1, touch_map_x2, 0, touch_max_x);
    touch_last_y = map(touch_raw_y, touch_map_y1, touch_map_y2, 0, touch_max_y);
  }
  Serial.printf("touch_raw_x: %d, touch_raw_y: %d, touch_last_x: %d, touch_last_y: %d\n", touch_raw_x, touch_raw_y, touch_last_x, touch_last_y);
}

bool touch_touched()
{
  if (ts.touched())
  {
    TS_Point p = ts.getPoint();
    touch_raw_x = p.x;
    touch_raw_y = p.y;
    int max_z = p.z;
    int count = 0;
    while ((ts.touched()) && (count < TOUCH_XPT2046_SAMPLES))
    {
      count++;

      TS_Point p = ts.getPoint();
      if (p.z > max_z)
      {
        touch_raw_x = p.x;
        touch_raw_y = p.y;
        max_z = p.z;
      }
      //Serial.printf("touch_raw_x: %d, touch_raw_y: %d, p.z: %d\n", touch_raw_x, touch_raw_y, p.z);
    }
    translate_touch_raw();
    return true;
  }

  return false;
}

bool touch_released()
{
  return true;
}

void setup()
{
  delay(100);
  Serial.begin(115200);
  EEPROM.begin(4096);
  delay(1000);
  Serial.printf("#PSRAM Free = %lu/%lu kBytes\r\n",ESP.getFreePsram()/1024,ESP.getPsramSize()/1024);
  touch_init(800,480,0);
#ifdef GFX_EXTRA_PRE_INIT
  GFX_EXTRA_PRE_INIT();
#endif

  // Init Display
  if (!gfx->begin())
  {
    Serial.println("gfx->begin() failed!");
  }
  else {
    Serial.println("gfx->begin() okay");
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
    tsec = millis();
    gfx->fillScreen(BLACK);
    Serial.printf("#fill time = %ld ms\r\n",millis()-tsec);

    gfx->displayOn();
    gfx->setCursor(10, 10);
    gfx->setTextColor(RED);
    gfx->println("Hello World!");
    gfx->flush();
  }
  tsec = millis();
} 

void loop()
{
  //boolean istouched = touch_touched();
  //if (istouched) {
  //  TS_Point p = ts.getPoint();
  //  Serial.printf("#Touch %d,%d  z=%d\r\n",p.x,p.y,p.z);
  //}
  if ((uint32_t)(millis()-tsec) >= 1000) {
    tsec = millis();
    gfx->setCursor(random(gfx->width()), random(gfx->height()));
    gfx->setTextColor(random(0xffff), random(0xffff));
    gfx->setTextSize(random(6) /* x scale */, random(6) /* y scale */, random(2) /* pixel_margin */);
    gfx->println("Hello World!");
    //digitalWrite(GFX_BL, digitalRead(GFX_BL)^1);
    Serial.println("#Hello World!!");
  }

}