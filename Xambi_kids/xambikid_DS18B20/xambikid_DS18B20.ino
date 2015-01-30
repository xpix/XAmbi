//----------------------------------------------------------------------------------------------------------------------
// XAmbiKid/DS18B20 - An ATtiny84 and RFM12B Wireless Temperature Sensor Node
// By Frank Herrmann. For hardware design see http://xpix.de
//
// Licenced under the Creative Commons Attribution-ShareAlike 3.0 Unported (CC BY-SA 3.0) licence:
// http://creativecommons.org/licenses/by-sa/3.0/
//
// Requires Arduino IDE with arduino-tiny core: http://code.google.com/p/arduino-tiny/
//
//----------------------------------------------------------------------------------------------------------------------

#include <JeeLib.h> // https://github.com/jcw/jeelib
#include "xambikid_tools.h"

#include <OneWire.h>
#include <DallasTemperature.h>

//#define USE_ACK           // Enable ACKs, comment out to disable

// Node defines
#define network 210       // RF12 Network group
#define myNodeID 1        // RF12 node ID in the range 1-30
#define mySubNodeID 12    // RF12 subnode ID (Please check config file: cfg/sensortypes.cfg)
#define myParamsSize 1    // How much values want to send (supplyV, value1, value2) = 3

#define LED 9            // Powerpin from LED

// Sensor defines
#define ONE_WIRE_BUS 3   // DS18B20 Temperature sensor is connected o0qn D10/ATtiny pin 13
#define ONE_WIRE_POWER 0  // DS18B20 Power pin is connected on D0/ATtiny pin 2

OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature

// Data Structure to be sent
struct Payload_s{
  byte subnode;       // Subnode Id (standard value)
  byte paramsize;     // parameters size
  int supplyV;	      // Supply voltage (standard value)
  int temp;	      // moisture reading
} Payload_default  = {mySubNodeID, myParamsSize, 0, 0}; // Default Values!!
typedef struct Payload_s Payload;
Payload tinytx = Payload_default;
// --------------------------


void setup() {
#ifdef USE_ACK
  ack(1); // enable use ack
#endif
  pinMode(LED, OUTPUT);       // set power on for led
  pinMode(ONE_WIRE_POWER, OUTPUT); // set power on for sensor
  tools_init_rf24(myNodeID, network);
}

void loop() {
   // Measure Power for solar low power nodes with super cap
   // The Node switch on at ~1.8V. This is not enough for 
   // the RFM12B (need min 2.2V). Wait for enough saved 
   // solar power from supercap
   tinytx.supplyV = tools_readVcc(); // Get supply voltage

   digitalWrite(ONE_WIRE_POWER, HIGH); // turn DS18B20 sensor on
   digitalWrite(LED, HIGH);  // turn led on
   delay(50); // only a flash
   digitalWrite(LED, LOW);  // turn led off
   
   delay(5); // The above doesn't seem to work for everyone (why?)
   
   sensors.begin(); //start up temp sensor
   sensors.requestTemperatures(); // Get the temperature
   tinytx.temp=(sensors.getTempCByIndex(0)*100); // Read first sensor and convert to integer, reversed at receiving end
   tinytx.paramsize = myParamsSize;   
   
   digitalWrite(ONE_WIRE_POWER, LOW); // turn DS18B20 off
   
   tools_rfwrite(myNodeID, &tinytx, sizeof tinytx); // Send data via RF 

   Sleepy::loseSomeTime(60000); //JeeLabs power save function: enter low power mode for 60 seconds (valid range 16-65000 ms)
}

