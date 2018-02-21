/*
  SimpleThermostat.h - Library for handling a simple thermostat with cooling and heating functions, compatible with DHT11, DHT22 and DS18B20 temperature sensors.
  Created by EPMatt (M. Agnoletto), February 19, 2018.
  Version 0.1.0
  Released to the public domain.
  URL: https://github.com/EPMatt/SimpleThermostat
  History:
        0.1.0 2018-19-02 first release
  See SimpleThermostat.cpp for method documentation.
*/
#ifndef thermo_h
#define thermo_h
//supported sensors
#define TYPE_DHT11 1000
#define TYPE_DHT22 1001
#define TYPE_DS18B20 1002
//error codes
#define ERROR_UNABLE_TO_READ_SENSOR -500
#define ERROR_UNSUPPORTED_OPERATION -501
//the library reads directly from the thermostat attached sensor at a max frequency of 1Hz
#define READ_DELAY 1000
//sensors precision
#define DHT11_PRECISION 0.5
#define DHT22_PRECISION 0.05
#define DS18B20_PRECISION 0.03
//include libraries
#include "dht.h"
#include "OneWire.h"
#include "Arduino.h"
class SimpleThermostat {
public:
    SimpleThermostat(unsigned short coolingPin, unsigned short heatingPin, unsigned short sensorPin,int type);
    float getCurrentTemp();
    float getCurrentHum();
    float getTemp();
    float getUpperThreesold();
    float getLowerThreesold();
    void setTemp(float temp);
    void setUpperThreesold(unsigned float threesold);
    void setLowerThreesold(unsigned float threesold);
    void handle();
    unsigned short getHeatingPin();
    unsigned short getCoolingPin();
    unsigned short getSensorPin();
private:
    float _currentTemp;
    float _currentHum;
    float _temp;
    unsigned float _upperThreesold;
    unsigned float _lowerThreesold;
    unsigned short _sensorPin;
    unsigned short _coolingPin;
    unsigned short _heatingPin;
    dht _dhtSensor;
    OneWire* _dsSensor;
    long _lastRead;
    int _type;
    void turnOffAll();
    bool readDHT();
};
#endif
