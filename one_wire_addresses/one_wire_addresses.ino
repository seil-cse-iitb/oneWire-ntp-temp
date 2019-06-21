#include <OneWire.h>
#include <DallasTemperature.h>

#define SENSOR_PIN 22

OneWire oneWire(SENSOR_PIN);
DallasTemperature sensor(&oneWire);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("____ONE WIRE SENSOR ADDRESSES____");

  sensor.begin();

  int sensorCount = sensor.getDeviceCount();
  Serial.print("Number of sensors are ");
  Serial.println(sensorCount);

  DeviceAddress sensorAddress[sensorCount];
  String addressString;

  for (int index = 0; index < sensorCount; index++)
  {
    sensor.getAddress(sensorAddress[index], index);
    Serial.print("Sensor ");
    Serial.print(index+1);
    Serial.print(" : ");

    for (int bitIndex = 0; bitIndex < 8; bitIndex++)
    {
      if (bitIndex == 0)
      {
        addressString = String(sensorAddress[index][bitIndex], HEX);
        addressString += '-';
      }
      else if (bitIndex != 1)
      {
        if (sensorAddress[index][8 - bitIndex] < 16) addressString += '0';

        addressString += String(sensorAddress[index][8 - bitIndex], HEX);
      }
    }

    Serial.println(addressString);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
