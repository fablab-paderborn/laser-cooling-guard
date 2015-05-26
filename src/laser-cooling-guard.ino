
#include <OneWire.h>
#include <DallasTemperature.h>
#include "needle.h"


// pins:
const int flowmeter = 5;
const int relais = 8;

OneWire ds(4);
DallasTemperature sensors(&ds);

void setup() {
  Serial.begin(9600); // debug output only
  pinMode(flowmeter, INPUT);
  pinMode(relais, OUTPUT);
  digitalWrite(relais, HIGH); // start with laser OFF

  sensors.begin();
  sensors.requestTemperatures();

  setupAnalogMeter();
}

int loopctr = 0;
int pulses = 0;
int lastread = 0;

static const double tmin = 10, tmax = 28;
static const int flow_min = 23;

void loop() {
  int thisread = digitalRead(flowmeter);
  pulses += (lastread != thisread) ? 1 : 0;
  lastread = thisread;

  ++loopctr;
  loopctr %= 0xffff;
  if (0==loopctr) {
    float celsius = sensors.getTempCByIndex(0);
    sensors.requestTemperatures();

    plotNeedle(celsius, 5);
    barLines(pulses);

    bool everything_is_fine = true;
    everything_is_fine &= celsius >= tmin;
    everything_is_fine &= celsius <= tmax;
    everything_is_fine &= pulses >= flow_min;

    digitalWrite(relais, everything_is_fine ? LOW : HIGH);

    Serial.print("  Temperature = ");
    Serial.print(celsius);
    Serial.print(" Celsius, ");

    Serial.print(pulses);
    Serial.print(" pulses -> ");
    Serial.println(everything_is_fine ? "OK" : "PROBLEM");
    pulses = 0;
  }
}

