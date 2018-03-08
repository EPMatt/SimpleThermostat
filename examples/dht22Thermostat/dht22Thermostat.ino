/*
  SimpleThermostat - Library for handling a simple thermostat with cooling and heating functions, compatible with DHT11, DHT22 and DS18B20 temperature sensors.
  Created by EPMatt (M. Agnoletto), February 19, 2018.
  Released to the public domain.
  Version: 0.1.0
  URL: https://github.com/EPMatt/SimpleThermostat
  History:
        0.1.0 2018-02-19 first release
  Example - DHT22 Thermostat
  The following sketch creates a thermostat with a DHT22 temperature sensor, sets desired temperature and threesolds and handles it, shows temperature and humidity values got from the sensor.
*/
#include <SimpleThermostat.h>
SimpleThermostat thermostat(2, 3, 4, TYPE_DHT22); //create a thermostat with cooling device on pin 2, heating device on pin 3 and a DHT22 sensor on pin 4
void setup() {
  Serial.begin(9600);
  Serial.println("SimpleThermostat library");
  Serial.println("Created by EPMatt");
  Serial.println("DHT22 Thermostat Example");
  //set desired temperature
  thermostat.setTemp(22.5);
  //set upper threesold: the cooling pin will be turned on when temperature reaches 22.5+2.0 = 24.5 °C
  thermostat.setUpperThreesold(2.0);
  //set lower threesold: the heating will be turned on when temperature reaches 22.5-1.5 = 21.0 °C
  thermostat.setLowerThreesold(1.5);
}

void loop() {
  //get current temperature and humidity values from the thermostat sensor and print them on serial
  float temperature = thermostat.getCurrentTemp();
  float humidity = thermostat.getCurrentHum();
  Serial.print("DHT22 Temperature: ");
  //check for sensor errors the return value of getCurrentTemp() and getCurrentHum() methods
  if (temperature == ERROR_UNABLE_TO_READ_SENSOR || humidity == ERROR_UNABLE_TO_READ_SENSOR) {
    Serial.println("Error while reading sensor values!");  //sensor is unplugged or not working properly
  } else {
    //everything clear, just print temperature and humidity values
    Serial.print(temperature);
    Serial.print(" C   Humidity: ");
    Serial.print(humidity);
    Serial.println("%");
  }
  //the handle method checks current temperature and turns on/off cooling and heating pins associated with the thermostat
  thermostat.handle();
  //check heating and cooling pins status: this can be achieved by retrieving the cooling and heating pin numbers from the thermostat and calling the standard digitalRead() function
  Serial.print("Cool pin is ");
  Serial.print(digitalRead(thermostat.getCoolingPin()));
  Serial.print(", heat pin is ");
  Serial.println(digitalRead(thermostat.getHeatingPin()));
  delay(1000);
}
