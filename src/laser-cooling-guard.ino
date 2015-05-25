
#include <OneWire.h>
#include "needle.h"


// digital pin 2 has a flowmeter attached to it. Give it a name:
int flowmeter = 5;

OneWire ds(4);
byte tempAddr[8];

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  // make the flowmeter's pin an input:
  pinMode(flowmeter, INPUT);

  ds.search(tempAddr);
  ds.reset();
  ds.select(tempAddr);

  setupAnalogMeter();
}

// the loop routine runs over and over again forever:
int loopctr = 0;
int pulses = 0;
int lastread = 0;

byte data[12];

void loop() {
  int thisread = digitalRead(flowmeter);
  pulses += (lastread != thisread) ? 1 : 0;
  lastread = thisread;

  ++loopctr;
  loopctr %= 0xffff;
  if (0==loopctr) {
    ds.reset();
    ds.select(tempAddr);
    ds.write(0xBE);

    for (int i=0; i<9; ++i) {
      data[i] = ds.read();
    }
    ds.write(0x44,1);

    // Convert the data to actual temperature
    // because the result is a 16 bit signed integer, it should
    // be stored to an "int16_t" type, which is always 16 bits
    // even when compiled on a 32 bit processor.
    int16_t raw = (data[1] << 8) | data[0];
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
    float celsius = (float)raw / 16.0;

    plotNeedle(celsius, 8); // Update analogue meter, 8ms delay per needle increment
    barLines(pulses);

    Serial.print("  Temperature = ");
    Serial.print(celsius);
    Serial.print(" Celsius, ");

    Serial.print(pulses);
    Serial.println(" pulses");
    pulses = 0;
  }
}

