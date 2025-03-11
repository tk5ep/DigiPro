/*
configuration file for DigiPro
*/
#include <Arduino.h>
#include "digi.h"
#include "sx1278.h"
#include "watchdog.h"
#include "ax25_util.h"

//extern uint16_t batt_volt;
extern float ext_temp,int_temp,pressure,humidity;
extern uint16_t v_bat,v_pv;
extern bool panel_connected;

/* SET WHEN BOARD ARE UNDER SLEEP MODE */
extern bool sleep_flag;
extern SX1278 lora;			// to set low power on sleep beacon

#define VERSION ("250305")
#define DEBUG               // uncomment if debugging is wanted on the serial port
#define BOARD_IIRIS_V4      // declare you board defined in boards.h

/* BATTERY ADC CALIBRATION */
//#define BAT_CAL 4558L   // (float)batt_volt * BAT_CAL / 1023  In millivolts
#define BATTERY_NIMH   // comment for LiIon battery

#define SOLAR_VOLTAGE_DIVIDER 5.4   // this is the ratio of the voltage divider to measure the solar panel voltage (R1+R2)/R2

#ifdef BATTERY_NIMH
    #define BAT_LOW     3300    // 3 * 1.10V  
    #define BAT_OK      3600    // 3 * 1.20V
    #define BAT_FULL    4350    // 3 * 1.45V
    #define BAT_HYST    100
#else // BATTERY_LIION
    #define BAT_LOW 3000    // 3500 for a Li-Ion battery, 
    #define BAT_OK 4000
    #define BAT_FULL 4200
    #define BAT_HYST 3700
#endif

/* WATCHDOG DAILY REBOOT VALUE (86400 is around 28h, not 24) */
#define WD_REBOOT_VALUE 74060L

/* LORA RADIO PARAMETER */
#define FREQ 433.775                                // TX freq in MHz
#define FREQ_ERR -21000	                            // Freq error in Hz. Start with 0, and measure real frequency and correct this.
#define LORA_POWER 20                               // Power of radio (dbm)
#define PPM_ERR lround(0.95 * (FREQ_ERR/FREQ))      // 25khz offset, 0.95*ppm = 25Khz / 433.3 = 57.65 * 0.95 = 55

/* RADIO CHANNEL COLLISION */
#define CHANNEL_SLOTTIME 100                        // 100ms slottime
#define CHANNEL_PERSIST 63                          // 25% persistance

/* DIGIPEATER CONFIG */
#define OE_TYPE_PACKET_ENABLE 1		                // Enable ASCII and binary dual mode 
#define MYCALL   "TK5EP-8"		                    // Put your call here
#define BCN_DEST "APEP03"		                    // beacon destination, enter digi callsign or a 
#define BCN_PATH "WIDE2-2"				            // Set to "" to disable,  and "WIDE2-2" for std path
// Put your position and symbol here. Coordinates can be displayed here https://www.egloff.eu/qralocator/ and overlay + symbol see at end of this file
#define BCN_POSITION PSTR("!4156.95NL00845.27Ea")    // symbol red losange
#define WX_POSITION PSTR("!4156.95NL00845.27E_")   // symbol blue circle when the WX_ENABLE si enabled
// status beacon
#define B1_ENABLE           1                        // if status beacon is desired
#define B1_INTERVAL         3650                     // beacon 1 interval in seconds 1800
// normal beacon
#define B2_ENABLE 1        
#define B2_COMMENT (" 433.775MHz Digipro v.")       // change your comment data here
#define B2_INTERVAL         3600                    // beacon 2 interval in seconds
// WX APRS beacon
#define WX_ENABLE           0                       // if the digi should send WX infos  NOTE: SHT31_ENABLE or DS_ENABLE should be enabled !
#define B3_INTERVAL         1750
#define WIDEN_MAX           3

/* SENSOR CONFIG */
#define DS_ENABLE           0
#define SHT31_ENABLE        0
#define SHT31_ADDRESS       0x44
#define SENSOR_PERIOD       180      // Period at which the sensors will be read

/* FRAME DUPLICATE TABLE CONFIG */
#define DUP_DELAY 40          /* Delay in sec to keep frame in memory */
#define DUP_MAXFRAME 5        /* Maximum duplicate frame memory */

/* TELEMETRY */
#define TELEMETRY_ENABLE 0      // if telemetry sending is wanted = 1
#define TELEM_INTERVAL 900     // period of telemetry transmission

/*
 * When using L as primary table symbol, here symbol ID icon:
 * 'a' is red losange 
 * '>' is car 
 * 'A' is white box 
 * 'H' is orange box 
 * 'O' is baloon 
 * 'W' is green circle 
 * '[' is person 
 * '_' is blue circle
 * 'n' is red triangle 
 * 's' is boat 
 * 'k' SUV 
 * 'u' is Truck 
 * 'v' is Van
 * 'z' is Red house
*/
