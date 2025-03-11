/***************************************************************************
 * Lora digipeater + WX station
 * by Patrick EGLOFF aka TK5EP
 * 
 * Forked from "Digipro" project of Remi Bilodeau VE2YAG and with his permission.
 * 
 * This is a small low power, low budget LoRa APRS digipeater build around an Arduino pro-mini and a RFM95W module
 * A cheap PCB can be made using the Gerber files found here : https://github.com/CongducPham/PRIMA-Intel-IrriS
 * I used the version IISS_V4 that has simple solar panel charger on it
 * There is also a V3 that doesn't have the solar charger part.
 * 
 * The digipeater can send WX datas measured with a SHT31 and/or a DS1820
 * It can send APRS WX packets and telemetry messages
 * of course, this costs energy !
 * 
 * Calibrate:
 * Batt voltage
 * RF Frequency, using SDR receiver and set frequency centered.
 * 
 * Pinout:
 * See boards.h 
 * Parameters :
 * See settings.h
 * 
 * The watchdog must be set to always ON with the corresponding fuse
 * 
 * Current draw:
 * 12.5 mA on RX (95% is lora Module), 113 mA in TX, 0.43 mA when radio module sleeping  
 * 
 ***************************************************************************/
#include <Arduino.h>
#include "settings.h"
#include "utils.h"
#include "boards.h"
#include <avr/sleep.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <SHT31.h>
#include "digi.h"

/* TELEMETRY */
//uint16_t batt_volt;
//uint32_t solar_voltage;
float ext_temp,int_temp,pressure,humidity;
//uint16_t v_bat;

bool sleep_flag;

/* EXTERIOR TEMPERATURE SENSOR */ 
#if DS_ENABLE==1
    OneWire oneWire(DS_SENSOR); 
    DallasTemperature sensors(&oneWire);
#endif

/*****************************
 _____      _               
/  ___|    | |              
\ `--.  ___| |_ _   _ _ __  
 `--. \/ _ \ __| | | | '_ \ 
/\__/ /  __/ |_| |_| | |_) |
\____/ \___|\__|\__,_| .__/ 
                     | |    
                     |_|    
******************************/
void setup() {
    #ifdef DEBUG
        Serial.begin(115200);
        while (!Serial);
        Serial.print(F("DigiPro LoRa digipeater "));
        Serial.println(VERSION);
        Serial.flush();
    #endif    

    /* CONFIGURE WATCHDOG FOR 1HZ INTERRUPT */
    Watchdog_setup();
    // switch OFF solar panel and sensors OFF
    solarPanel_OFF();
    sensors_OFF();

    /* IF MODEM CONFIGURE FAIL, RESET BOARD AT NEXT WATCHDOG IRQ */
    if(DigiInit() == 0) {
      #ifdef DEBUG  
        Serial.println(F("SX1278 init problem"));
        Serial.flush();
      #endif  
        wdt_flag = 31;
        while(1);       // Watchdog will reset
    }
    else {
        #ifdef DEBUG
            Serial.println(F("SX1278 init OK"));
            Serial.flush();
        #endif
    }

    /* SENSORS INIT */ 
	#if DS_ENABLE==1
        sensors.begin();
	#endif

    #if SHT31_ENABLE==1
        init_SHT31();
        read_SHT31();
    #endif

    // check the battery status and charging logic
    checkBattery();
    // send beacon at startup, just to be sure that everything works
    DigiSendBeacon(2);
    
    // if the WX beacon is enabled send it at startup
    #if WX_ENABLE==1
        if (SHT31_ENABLE==0 && DS_ENABLE==0) {
            Serial.println (F("Telemetry enabled, but no sensor enabled!"));
            Serial.println (F("Please correct this. Program halted."));
            while(1) {};
        }
        sleepfor(5);        // wait for 5s
        DigiSendBeacon(3);
    #endif

    // is status beacon enabled, send it at startup
    #if B1_ENABLE == 1 
        
    
    sleepfor(5);
        DigiSendBeacon(1);
    #endif

    // if telemetry is enabled send it at startup
    #if TELEMETRY_ENABLE==1
        if (SHT31_ENABLE==0 && DS_ENABLE==0) {
             Serial.println (F("Telemetry enabled, but no sensor enabled!"));
             Serial.println (F("Please correct this. Program halted."));
             while(1) {};
        }
        sleepfor(5);
        DigiSendTelem();
    #endif
}   // SETUP END

/*****************************
 _                       
| |                      
| |     ___   ___  _ __  
| |    / _ \ / _ \| '_ \ 
| |___| (_) | (_) | |_) |
\_____/\___/ \___/| .__/ 
                  | |    
                  |_|    
*****************************/
void loop() {
    //uint32_t t;
    static uint32_t sensor_to;

    /* GO TO SLEEP FOR 1 SECOND IF NO REQUEST FROM MODULE */
    if(digitalRead(LORA_DIO) == LOW) {

        /* ENABLE INTERRUPT ON DIO0 TO WAKE-UP FROM SLEEP */
        PCMSK2 = 0x08;    // PCINT19 pin-on-change interrupt for PD3
        PCICR = 0x04;     // PCINT2 ENABLE
    
        /* POWER DOWN CPU, WAKE-UP WITH 1Hz WATCHDOG INTERRUPT OR INCOMING PACKET */
        wdt_flag = 0;
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        sleep_enable();

        sleep_mode();		// Watchdog wake CPU or DIO0 rising level, set in SX1278.CPP
        sleep_disable();

        /* DISABLE DIO0 INTERRUPT */
        PCICR = 0;
        PCMSK2 = 0;
    }

	/* CHECK SENSOR EVERY PERIOD DEFINED WITH SENSOR_PERIOD */
    if(wdt_clk > sensor_to) {
        sensor_to = wdt_clk + SENSOR_PERIOD;

		/* DS18B20 EXTERIOR TEMP */
		#if DS_ENABLE==1
            // switch DS sensor ON
            sensors_ON();
            sensors.requestTemperatures();
            delay(100); 
            ext_temp = sensors.getTempCByIndex(0); 
            if (ext_temp<-126) Serial.println("Error reading 18BS20");
		#endif

        // is SHT31 sensor enabled
        #if SHT31_ENABLE==1
            read_SHT31();
        #endif

        checkBattery(); // check battery state
    }

    /* SEND AND RECEIVE PACKET, UNTIL NOTHING TO DO */
    while(DigiPoll());
}