
#include <OneWire.h>
#include <DallasTemperature.h>
#include "needle.h"


// pins:
const int flowmeter = 5;
const int relais = 8;

OneWire ds(4);
DallasTemperature sensors(&ds);
DeviceAddress     sensor_address;

void setup() {
  Serial.begin(9600); // debug output only
  pinMode(flowmeter, INPUT);
  pinMode(relais, OUTPUT);
  digitalWrite(relais, HIGH); // start with laser OFF

  sensors.begin();
  sensors.setResolution(9);
  sensors.getAddress(sensor_address, 0);
  sensors.requestTemperatures();

  setupAnalogMeter();
}

int loopctr = 0;
int pulses = 0;
int lastread = 0;

static const double tmin = 10, tmax = 28;
static const int flow_min = 2;

void loop() {
  int thisread = digitalRead(flowmeter);
  pulses += (lastread != thisread) ? 1 : 0;
  lastread = thisread;

  ++loopctr;
  if (loopctr > 0x1000 && sensors.isConversionAvailable(sensor_address)) {
    float celsius = sensors.getTempC(sensor_address);
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
    loopctr = 0;
  } else if (loopctr > 0x10000) {
    // something went wrong
    Serial.println("timeout waiting for temp");
    sensors.getAddress(sensor_address, 0);
    sensors.requestTemperatures();
    loopctr = 0;
  }
}

