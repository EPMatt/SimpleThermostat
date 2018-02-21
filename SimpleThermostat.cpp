/*
  SimpleThermostat.cpp - Library for handling a simple thermostat with cooling and heating functions, compatible with DHT11, DHT22 and DS18B20 temperature sensors.
  Created by EPMatt (M. Agnoletto), February 19, 2018.
  Version 0.1.0
  Released to the public domain.
  URL: https://github.com/EPMatt/SimpleThermostat
  History:
        0.1.0 2018-19-02 first release
*/
#include "SimpleThermostat.h"
/*
    Method:       SimpleThermostat::SimpleThermostat(unsigned short coolingPin, unsigned short heatingPin, unsigned short sensorPin,int type)
    Description:  Constructor for a thermostat object
    Parameters:   -coolingPin the pin number to control the cooling device
                  -heatingPin the pin number to control the heating device
                  -sensorPin the pin the temperature sensor is attached to
                  -type temperature sensor type, currently supported sensors are DHT11, DHT22 and DS18B20
    Return value: none
*/
SimpleThermostat::SimpleThermostat(unsigned short coolingPin, unsigned short heatingPin, unsigned short sensorPin,int type) {
    _coolingPin=coolingPin;
    _heatingPin=heatingPin;
    _sensorPin=sensorPin;
    _type=type;
    _lastRead=-1100;
    if(_type==TYPE_DS18B20) _dsSensor=new OneWire(_sensorPin);
    pinMode(_coolingPin,OUTPUT);
    pinMode(_heatingPin,OUTPUT);
}
/*
    Method:       float SimpleThermostat::getCurrentTemp()
    Description:  get current temperature read from the attached sensor
    Parameters:   none
    Return value: current temperature or ERROR_UNABLE_TO_READ_SENSOR if the sensor is disconnected or unresponding
*/
float SimpleThermostat::getCurrentTemp() {
    if(millis()-_lastRead>READ_DELAY) {
        if(_type==TYPE_DS18B20) {
            byte i;
            byte data[12];
            byte addr[8];
            if (!(*_dsSensor).search(addr)) {
                (*_dsSensor).reset_search();
                return ERROR_UNABLE_TO_READ_SENSOR;
            }
            if (OneWire::crc8( addr, 7) != addr[7]) {
                return ERROR_UNABLE_TO_READ_SENSOR;
            }
            if (addr[0] != 0x28) {
                return ERROR_UNABLE_TO_READ_SENSOR;
            }
            (*_dsSensor).reset();
            (*_dsSensor).select(addr);
            (*_dsSensor).write(0x44, 1);
            (*_dsSensor).reset();
            (*_dsSensor).select(addr);
            (*_dsSensor).write(0xBE);
            for ( i = 0; i < 9; i++) {
                data[i] = (*_dsSensor).read();
            }
            _currentTemp = ( (data[1] << 8) + data[0] )*0.0625;
            (*_dsSensor).reset_search();
        } else {
           if(!readDHT())return ERROR_UNABLE_TO_READ_SENSOR;
        }
    }
    return _currentTemp;
}
/*
    Method:       float SimpleThermostat::getCurrentHum()
    Description:  get current humidity read from the attached sensor
    Parameters:   none
    Return value: current humidity % or ERROR_UNABLE_TO_READ_SENSOR if the sensor is disconnected or unresponding or ERROR_UNSUPPORTED_OPERATION if the sensor does not support humidity reading
*/
float SimpleThermostat::getCurrentHum() {
    if(_type==TYPE_DS18B20) return ERROR_UNSUPPORTED_OPERATION;
    if(millis()-_lastRead>READ_DELAY) {
       if(!readDHT())return ERROR_UNABLE_TO_READ_SENSOR;
    }
    return _currentHum;
}
/*
    Method:       float SimpleThermostat::getTemp()
    Description:  get optimal temperature
    Parameters:   none
    Return value: thermostat optimal temperature
*/
float SimpleThermostat::getTemp() {
    return _temp;
}
/*
    Method:       float SimpleThermostat::getUpperThreesold()
    Description:  get temperature upper threesold. Upper threesold is used to activate the cooling device. (see handle() method)
    Parameters:   none
    Return value: the temperature upper threesold
*/
float SimpleThermostat::getUpperThreesold() {
    return _upperThreesold;
}
/*
    Method:       float SimpleThermostat::getLowerThreesold()
    Description:  get temperature lower threesold. Lower threesold is used to activate the heating device. (see handle() method)
    Parameters:   none
    Return value: the temperature lower threesold
*/
float SimpleThermostat::getLowerThreesold() {
    return _lowerThreesold;
}
/*
    Method:       void SimpleThermostat::setTemp(float temp)
    Description:  Set desired temperature for the thermostat. This is the temperature which the system will try to reach balancing heating and cooling devices.
    Parameters:   -temp the new desired temperature
    Return value: none
*/
void SimpleThermostat::setTemp(float temp) {
    _temp=temp;
}
/*
    Method:       void SimpleThermostat::setUpperThreesold(unsigned float threesold)
    Description:  set temperature upper threesold. Upper threesold is used to activate the cooling device. (see handle() method)
    Parameters:   -threesold the new upper threesold
    Return value: none
*/
void SimpleThermostat::setUpperThreesold(unsigned float threesold) {
    _upperThreesold=threesold;
}
/*
    Method:       void SimpleThermostat::setLowerThreesold(unsigned float threesold)
    Description:  set temperature lower threesold. Lower threesold is used to activate the heating device. (see handle() method)
    Parameters:   -threesold the new lower threesold
    Return value: none
*/
void SimpleThermostat::setLowerThreesold(unsigned float threesold) {
    _lowerThreesold=threesold;
}
/*
    Method:       unsigned short SimpleThermostat::getSensorPin()
    Description:  get temperature sensor pin
    Parameters:   none
    Return value: the Arduino pin number the  thermostat sensor is attached to
*/
unsigned short SimpleThermostat::getSensorPin() {
    return _sensorPin;
}
/*
    Method:       unsigned short SimpleThermostat::getCoolingPin()
    Description:  get cooling device pin
    Parameters:   none
    Return value: the Arduino pin number the thermostat cooling device should be attached to
*/
unsigned short SimpleThermostat::getCoolingPin() {
    return _coolingPin;
}
/*
    Method:       unsigned short SimpleThermostat::getHeatingPin()
    Description:  get heating device pin
    Parameters:   none
    Return value: the Arduino pin number the thermostat heating device should be attached to
*/
unsigned short SimpleThermostat::getHeatingPin() {
    return _heatingPin;
}
/*
    Method:       void SimpleThermostat::handle()
    Description:  Read <current temperature> from the attached sensor and handle cooling and heating devices as it follow.
                  If the <current temperature> is higher than the <desired temperature>+<the upper threesold>, turn on the cooling device only.
                  If the <current temperature> is lower than the <desired temperature>-<the lower threesold>, turn on the heating device only.
                  If the <current temperature> is equal to the <desired temperature> (in limits of the sensor precision, of course) turn off heating and cooling devices.
                  If the sensor temperature sensor is unplugged or unresponding, turn off heating and cooling devices.
    Parameters:   none
    Return value: none
*/
void SimpleThermostat::handle() {
    float currentTemp=getCurrentTemp();
    if(currentTemp==ERROR_UNABLE_TO_READ_SENSOR)turnOffAll();
    else if(currentTemp>=_temp+_upperThreesold) {
        digitalWrite(_coolingPin,HIGH);
        digitalWrite(_heatingPin,LOW);
    } else if(currentTemp<=_temp-_lowerThreesold) {
        digitalWrite(_coolingPin,LOW);
        digitalWrite(_heatingPin,HIGH);
    } else {
        if(_type==TYPE_DHT11&&currentTemp<_temp+DHT11_PRECISION&&currentTemp>_temp-DHT11_PRECISION)turnOffAll();
        if(_type==TYPE_DHT22&&currentTemp<_temp+DHT22_PRECISION&&currentTemp>_temp-DHT22_PRECISION)turnOffAll();
        if(_type==TYPE_DS18B20&&currentTemp<_temp+DS18B20_PRECISION&&currentTemp>_temp-DS18B20_PRECISION)turnOffAll();
    }
}
/*
    Private methods(for internal usage only)
*/
void SimpleThermostat::turnOffAll() {
    digitalWrite(_coolingPin,LOW);
    digitalWrite(_heatingPin,LOW);
}
bool SimpleThermostat::readDHT(){
 int chk = (_type==TYPE_DHT11)?_dhtSensor.read11(_sensorPin):_dhtSensor.read22(_sensorPin);
        switch (chk) {
        case DHTLIB_OK:
            _currentHum=(float) _dhtSensor.humidity;
            _currentTemp=(float) _dhtSensor.temperature;
            _lastRead=millis();
            break;
        default:
            return false;
        }
        return true;
}
