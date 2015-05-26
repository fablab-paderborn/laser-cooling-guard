/*
 An example analogue meter using a ILI9341 TFT LCD screen

 This example uses the hardware SPI only and a board based
 on the ATmega328

 Needs Fonts 2, and 7 (also Font 4 if using large scale label)

 Comment out lines 153 and 197 to reduce needle flicker and
 to remove need for Font 4 (which uses ~8k of FLASH!)
 
 Alan Senior 23/2/2015
 */

/*
  Removed some code not used in this project.
  Check http://www.instructables.com/id/Arduino-sketch-for-a-retro-analogue-meter-graphic-/
  for Alan Senior's original code.

  Christopher Creutzig, 2015-05-21
*/

// These are the connections for the UNO to display
#define sclk 13  // Don't change
#define mosi 11  // Don't change
#define cs   10  // If cs and dc pin allocations are changed then 
#define dc   9   // comment out #define F_AS_T line in "Adafruit_ILI9341_FAST.h"
                 // (which is inside "Adafriit_ILI9341_AS" library)

#define rst  7  // Can alternatively connect this to the Arduino reset

#include <Adafruit_GFX_AS.h>     // Core graphics library
#include <Adafruit_ILI9341_AS.h> // Fast hardware-specific library
#include <SPI.h>

#define ILI9341_GREY 0x5AEB

Adafruit_ILI9341_AS tft = Adafruit_ILI9341_AS(cs, dc, rst); // Invoke custom library

float ltx = 0;    // Saved x coord of bottom of needle
uint16_t osx = 120, osy = 120; // Saved x & y coords

int old_analog =  -999; // Value last displayed

// position parameters
static const int xmin = 0, xmax = 280, ymin = 0, ymax = 160;
static const int xmid = (xmin+xmax)/2;
static const int needle_length = 123, scale_dia = 132;
static const int needle_outside = 20;
static const int yoffset = ymax+needle_outside-2;

static const double degToAngle = 0.0174532925;

// value parameters
static const int val_min = 0, val_max = 40;

// #########################################################################
// Update needle position
// This function is blocking while needle moves, time depends on ms_delay
// 10ms minimises needle flicker if text is drawn within needle sweep area
// Smaller values OK if text not in sweep area, zero for instant movement but
// does not look realistic... (note: 100 increments for full scale deflection)
// #########################################################################
void plotNeedle(int value, byte ms_delay)
{
  tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
  char buf[8]; dtostrf(value, 4, 0, buf);
  tft.drawRightString(buf, xmin + 42, ymax - 23, 2);

  if (value < val_min) value = val_min; // Limit value to emulate needle end stops
  if (value > val_max) value = val_max;

  // Move the needle until new value reached
  while (!(value == old_analog)) {
    if (old_analog < value) {
      old_analog++;
    } else {
      old_analog--;
    }
    
    if (ms_delay == 0) {
      old_analog = value; // Update immediately if delay is 0
    }
    
    float sdeg = map(old_analog, val_min, val_max, -140, -40); // Map value to angle 
    // Calculate tip of needle coords
    float sx = cos(sdeg * degToAngle);
    float sy = sin(sdeg * degToAngle);

    // Calculate x delta of needle start (does not start at pivot point)
    float tx = tan((sdeg+90) * degToAngle);
    
    // Erase old needle image
    tft.drawLine(xmid + needle_outside * ltx - 1, yoffset - needle_outside, osx - 1, osy, ILI9341_WHITE);
    tft.drawLine(xmid + needle_outside * ltx, yoffset - needle_outside, osx, osy, ILI9341_WHITE);
    tft.drawLine(xmid + needle_outside * ltx + 1, yoffset - needle_outside, osx + 1, osy, ILI9341_WHITE);
    
    // Re-plot text under needle
    tft.setTextColor(ILI9341_BLACK);
    
    // Store new needle end coords for next erase
    ltx = tx;
    osx = sx * needle_length + xmid;
    osy = sy * needle_length + yoffset;
    
    // Draw the needle in the new position, magenta makes needle a bit bolder
    // draws 3 lines to thicken needle
    tft.drawLine(xmid + needle_outside * ltx - 1, yoffset - needle_outside, osx - 1, osy, ILI9341_RED);
    tft.drawLine(xmid + needle_outside * ltx, yoffset - needle_outside, osx, osy, ILI9341_MAGENTA);
    tft.drawLine(xmid + needle_outside * ltx + 1, yoffset - needle_outside, osx + 1, osy, ILI9341_RED);
    
    // Slow needle down slightly as it approaches new position
    if (abs(old_analog - value) < 10) ms_delay += ms_delay / 5;
    
    // Wait before next update
    delay(ms_delay);
  }
}

// #########################################################################
//  Draw the analogue meter on the screen
// #########################################################################

void analogMeter()
{
  // Meter outline
  // tft.fillRect(xmin, ymin, xmax, ymax, ILI9341_GREY);
  // tft.fillRect(xmin+5, ymin+3, xmax-5, ymax-3, ILI9341_WHITE);
  
  tft.setTextColor(ILI9341_BLACK);  // Text colour
  
  // Draw ticks every 5 degrees from -50 to +50 degrees (100 deg. FSD swing)
  for (int i = -50; i < 51; i += 5) {
    // Long scale tick length
    int tl = 15;

    
    // Coordinates of tick to draw
    float sx = cos((i - 90) * degToAngle);
    float sy = sin((i - 90) * degToAngle);
    uint16_t x0 = sx * (scale_dia + tl) + xmid;
    uint16_t y0 = sy * (scale_dia + tl) + yoffset;
    uint16_t x1 = sx * scale_dia + xmid;
    uint16_t y1 = sy * scale_dia + yoffset;
    
    // Coordinates of next tick for zone fill
    float sx2 = cos((i + 5 - 90) * degToAngle);
    float sy2 = sin((i + 5 - 90) * degToAngle);
    int x2 = sx2 * (scale_dia + tl) + xmid;
    int y2 = sy2 * (scale_dia + tl) + yoffset;
    int x3 = sx2 * scale_dia + xmid;
    int y3 = sy2 * scale_dia + yoffset;
    
    // Yellow zone limits
    if (i < -25 || (i >= 10 && i < 25)) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, ILI9341_YELLOW);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, ILI9341_YELLOW);
    }

    // Orange zone limits
    if (i >= 25 && i < 50) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, ILI9341_ORANGE);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, ILI9341_ORANGE);
    }
    
    // Red zone limits
    if (i >= 25 && i < 50) {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, ILI9341_RED);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, ILI9341_RED);
    }
    
    // Short scale tick length
    if (i % 25 != 0) tl = 8;
    
    // Recalculate coords in case tick length changed
    x0 = sx * (scale_dia + tl) + xmid;
    y0 = sy * (scale_dia + tl) + yoffset;
    x1 = sx * scale_dia + xmid;
    y1 = sy * scale_dia + yoffset;
    
    // Draw tick
    tft.drawLine(x0, y0, x1, y1, ILI9341_BLACK);
    
    // Check if labels should be drawn, with position tweaks
    if (i % 25 == 0) {
      // Calculate label positions
      x0 = sx * (scale_dia + tl + 10) + xmid;
      y0 = sy * (scale_dia + tl + 10) + yoffset;
      switch (i / 25) {
        case -2: tft.drawCentreString("0", x0, y0 - 12, 2); break;
        case -1: tft.drawCentreString("10", x0, y0 - 9, 2); break;
        case 0: tft.drawCentreString("20", x0, y0 - 6, 2); break;
        case 1: tft.drawCentreString("30", x0, y0 - 9, 2); break;
        case 2: tft.drawCentreString("40", x0, y0 - 12, 2); break;
      }
    }
    
    // Now draw the arc of the scale
    sx = cos((i + 5 - 90) * degToAngle);
    sy = sin((i + 5 - 90) * degToAngle);
    x0 = sx * scale_dia + xmid;
    y0 = sy * scale_dia + yoffset;
    // Draw scale arc, don't draw the last part
    if (i < 50) tft.drawLine(x0, y0, x1, y1, ILI9341_BLACK);
  }
  
  tft.drawString("Temp", xmax - 42, ymax - 23, 2); // Font doesn't have a ° symbol
  tft.drawRect(xmin+5, ymin+3, xmax-5, ymax-3, ILI9341_BLACK); // Draw bezel line
  
  plotNeedle(0,0); // Put meter needle at 0
}

static int bar_min = 0;
static int bar_max = 80;
static int bar_warnbelow = 20;
static int bar_xmin = xmax+1, bar_xmax = 320, bar_ymin = 0, bar_ymax = 240;
void barLines(int value) {
  tft.fillRect(bar_xmin, bar_ymin, bar_xmax, bar_ymax, ILI9341_GREY);

  if (value < bar_min) {
    value = bar_min;
  }
  if (value > bar_max) {
    value = bar_max;
  }

  tft.fillRect(bar_xmin, map(value, bar_min, bar_max, bar_ymax, bar_ymin),
    bar_xmax, bar_ymax,
    value < bar_warnbelow ? ILI9341_RED : ILI9341_GREEN);
}

void setupAnalogMeter(void) {
  tft.init();
  tft.setRotation(3);

  tft.fillScreen(ILI9341_WHITE);

  analogMeter(); // Draw analogue meter
  barLines(0);
}
