/*
functions to handle voltages control of solar charging and battery
TK5EP 2025/02/14
*/
#include <Arduino.h>
#include <avr/sleep.h>
#include "boards.h"
#include "settings.h"
#include "digi.h"
#include <sht31.h>

bool panel_connected, was_charged;
uint8_t BatLowFlag =0;
uint16_t sht_status, v_bat, v_pv;

/************************************************************************
*   Measures the supply voltage of Arduino. Doesn't need any external hardware
*************************************************************************/
// https://forum.arduino.cc/t/can-an-arduino-measure-the-voltage-of-its-own-power-source/669954/4

uint32_t readVcc() { 
     #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) // For mega boards
      const long InternalReferenceVoltage = 1115L;  // Adjust this value to your boards specific internal BG voltage x1000
      ADMUX = (0<<REFS1) | (1<<REFS0) | (0<<ADLAR) | (0<<MUX5) | (1<<MUX4) | (1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (0<<MUX0);
    #else // For 168/328 boards
      const long InternalReferenceVoltage = 1100L; // Adjust this value to your boards specific internal BG voltage x1000
      ADMUX = (0<<REFS1) | (1<<REFS0) | (0<<ADLAR) | (1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (0<<MUX0);
      delay(5);
    #endif
    ADCSRA |= _BV( ADSC ); // Start a conversion 
    while( ( (ADCSRA & (1<<ADSC)) != 0 ) ); // Wait for it to complete
    int result = (((InternalReferenceVoltage * 1024L) / ADC) + 5L) / 10L; // Scale the value; calculates for straight line value
    return result*10; // convert from centivolts to millivolts
}


/******************************************
 * readVoltage()
 * read ADC on pin
 * result in mV
 *****************************************/
uint32_t readVoltage(uint8_t pin) {
  analogReference(INTERNAL);
	uint32_t adc_value = 0;

  analogRead(pin);  // wake up the ADC and let it settle
  delay(50);
	for(uint8_t i=0; i<5; i++) adc_value = analogRead(pin); // make some measures and keep last one
  //Serial.println(adc_value);
	uint32_t volt = adc_value * 1100 / 1024;  // convert adc value to mVolts refered to reference voltage. Adjust 1100 to you board
  //Serial.println(volt);
  volt = volt * SOLAR_VOLTAGE_DIVIDER;  // apply the voltage divider multiplication factor (R1+R2)/R2
  return volt;
}

/******************************************
 * solarPanel_ON
 * connect the solar panel
 *****************************************/
void solarPanel_ON() {
  // switch ON solar panel
  pinMode(SOLAR_POWER_PIN, OUTPUT);
  digitalWrite(SOLAR_POWER_PIN, HIGH);
  panel_connected = 1;
  #ifdef DEBUG
    Serial.println(F("Solar panel ON"));
    Serial.flush();
#endif
}

/******************************************
 * solarPanel_OFF
 * disconnect the solar panel
 *****************************************/
void solarPanel_OFF() {
// switch OFF solar panel
pinMode( SOLAR_POWER_PIN, OUTPUT);
digitalWrite( SOLAR_POWER_PIN, LOW);
panel_connected = 0;
#ifdef DEBUG
  Serial.println(F("Solar panel OFF"));
  Serial.flush();
#endif
}

/******************************************
 * sensors_ON
 * connect Vcc to the sensors power line
 *****************************************/
void sensors_ON(){
    // switch ON sensors power line
    pinMode( SENSORS_PWR, OUTPUT);
    digitalWrite( SENSORS_PWR, LOW);
    #ifdef DEBUG
      Serial.println(F("Sensors ON"));
      Serial.flush();
    #endif
}

/******************************************
 * sensors_OFF
 * disconnect Vcc from the sensors power line
 *****************************************/
void sensors_OFF(){
  // switch OFF sensors power line
  pinMode( SENSORS_PWR, OUTPUT);
  digitalWrite( SENSORS_PWR, HIGH);
  #ifdef DEBUG
    Serial.println(F("Sensors OFF"));
    Serial.flush();
  #endif
}

/******************************************
 * checkBattery()
 * check the battery and solar panel voltage, handles the battery charging
 * puts the digi in sleep mode when battery is low
 *****************************************/
void checkBattery(){
  // if battery drops below a level, stop transmitting put in sleep mode, wake up every 15min and check
  // if solar voltage > battery voltage and battery voltage < battery full, charging
  // if solar panel voltage < battery voltage, disconnect the panel
  // 
  v_pv = readVoltage(SOLAR_VOLTAGE_PIN);
  v_bat = readVcc();

  #ifdef DEBUG
    Serial.print(F("Solar voltage = "));
    Serial.print(v_pv);
    Serial.println(F(" mV"));
    Serial.print(F("Battery voltage = "));
    Serial.print(v_bat);
    Serial.println(F(" mV"));
    Serial.flush();
  #endif

  // disonnect panel if not enough solar output
    if (v_pv < v_bat ) {
    solarPanel_OFF();
    was_charged = 0;
 }

    // disconnect solar panel if battery fully loaded and flag as fully charged once
    if (v_bat>=BAT_FULL) {
      solarPanel_OFF();
      was_charged = 1;
    }

  // if the battery reached full charge and the voltage drops below a reasonable voltage, change flag to allow charging
  if ( was_charged == 1 && v_bat< BAT_OK) {
    was_charged = 0;
  }

  // connect solar panel if not fully charged once and voltage > battery voltage + 0.1V until is battery fully loaded
  if ( (was_charged == 0 ) && (v_pv > v_bat + BAT_HYST)  && (v_bat < BAT_FULL) ) {
    solarPanel_ON();
    //was_charged = 0;
  }

  // if battery low
  // go into sleep mode with LoRa module desactivated
  // and send beacon 
  if (v_bat < BAT_LOW){
    #ifdef DEBUG
    Serial.println(F("WARNING ! Battery LOW"));
    Serial.flush();
  #endif
    BatLowFlag++;             // number of low battery measures, just to be sure that the battery is really low
    if (BatLowFlag>=5) {
      sleep_flag=1;
      lora.setPower(13);		  // put beacon in low power 20mW
      DigiSendBeacon(1);    	// send system beacon for sleep mode
      lora.setPower(LORA_POWER);
      #ifdef DEBUG
        Serial.println(F("Going into sleep"));
        Serial.flush();
      #endif
      DigiSleep();          // Put lora radio module in sleep

      // sleep for 15 min
      uint32_t t = wdt_clk + (60 * 15);
      while(wdt_clk < t) {
          wdt_flag = 0;
          set_sleep_mode(SLEEP_MODE_PWR_DOWN);
          sleep_enable();
          sleep_mode();
          sleep_disable();
      }
    }
    else sleep_flag = 0; 
}
else {
  BatLowFlag = 0; // reset the low battery flag
}
}

#ifdef SHT31_ENABLE
/******************************************
 * init_SHT31()
 * initialize the SHT31 sensor
 *****************************************/
SHT31 sht(SHT31_ADDRESS);
void init_SHT31(){
    sensors_ON();
    Wire.begin();
    Wire.setClock(100000);
    sht.begin();
    sht_status = sht.isConnected();
    if (!sht_status) {
        Serial.println(F("Couldn't find SHT31"));
    }
    else Serial.println(F("SHT31 found."));
    sensors_OFF();
}

/******************************************
 * readSHT31()
 * reads the SHT31 sensor
 *****************************************/
void read_SHT31() {
    if (sht.isConnected()) {
        sensors_ON();
        sht.read();
        int_temp = sht.getTemperature();
        //Serial.print(sht.getTemperature(), 1);
        Serial.print(int_temp);
        Serial.print("\t");
        humidity=sht.getHumidity();
        Serial.println(humidity);
        //Serial.println(sht.getHumidity(), 1);
        sensors_OFF();
    }
}
#endif

/******************************************
 * sleepfor()
 * go into sleep for X seconds
 *****************************************/
void sleepfor(uint32_t nbs){
          uint32_t t;
          t = wdt_clk + (nbs);
          while(wdt_clk < t) {
              wdt_flag = 0;
              set_sleep_mode(SLEEP_MODE_PWR_DOWN);
              sleep_enable();
              sleep_mode();
              sleep_disable();
          }
}