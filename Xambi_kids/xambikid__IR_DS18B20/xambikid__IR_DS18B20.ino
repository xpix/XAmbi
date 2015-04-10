//----------------------------------------------------------------------------------------------------------------------
// XAmbiKid/DS18B20 - An ATtiny84 and RFM12B Wireless Temperature Sensor Node
// By Frank Herrmann. For hardware design see http://xpix.de
//
// Licenced under the Creative Commons Attribution-ShareAlike 3.0 Unported (CC BY-SA 3.0) licence:
// http://creativecommons.org/licenses/by-sa/3.0/
//
// Requires Arduino IDE with arduino-tiny core: http://code.google.com/p/arduino-tiny/
//
// !!! Use the Infrared Communication with a gateway with installed TSOP !!!
//
// For Attiny8* please check this:
// http://www.ernstc.dk/arduino/38khz_timer.htm
//
//----------------------------------------------------------------------------------------------------------------------

#include <JeeLib.h> // https://github.com/jcw/jeelib
#include "xambikid_tools.h"

#include <OneWire.h>
#include <DallasTemperature.h>

//#define USE_ACK           // Enable ACKs, comment out to disable

// Node defines
#define network 100       // RF12 Network group (IR == 100)
#define myNodeID 1        // RF12 node ID in the range 1-30
#define mySubNodeID 15    // RF12 subnode ID (Please check config file: cfg/sensortypes.cfg)
#define myParamsSize 1    // How much values want to send (supplyV, value1, value2) = 3

#define IR_A   9           // Powerpin for IR LED
#define IR_B  10           // Alternative Powerpin for IR LED
#define LED   13           // Flash for debug

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
  pinMode(IR_A, OUTPUT);
  pinMode(IR_B, OUTPUT);
  pinMode(ONE_WIRE_POWER, OUTPUT); // set power on for sensor
  
  //tools_init_rf24(myNodeID, network);

  // IR send logic with 38khz Carrier and 2400 baud
  // 
  // Clear Timer on Compare Match (CTC) Mode
  bitWrite(TCCR1A, WGM10, 0);
  bitWrite(TCCR1A, WGM11, 0);
  bitWrite(TCCR1B, WGM12, 1);
  bitWrite(TCCR1B, WGM13, 0);

  // Toggle OC1A and OC1B on Compare Match.
  bitWrite(TCCR1A, COM1A0, 1);
  bitWrite(TCCR1A, COM1A1, 0);
  bitWrite(TCCR1A, COM1B0, 1);
  bitWrite(TCCR1A, COM1B1, 0);

  // No prescaling
  bitWrite(TCCR1B, CS10, 1);
  bitWrite(TCCR1B, CS11, 0);
  bitWrite(TCCR1B, CS12, 0);

  OCR1A = 210;
  OCR1B = 210;
  
  Serial.begin(1200);
}

void loop() {
   digitalWrite(ONE_WIRE_POWER, HIGH); // turn DS18B20 sensor on
   digitalWrite(LED, HIGH);  // turn led on
   delay(50); // only a flash
   // Measure Power for solar low power nodes with super cap
   // The Node switch on at ~1.8V. This is not enough for 
   // the RFM12B (need min 2.2V). Wait for enough saved 
   // solar power from supercap
   tinytx.supplyV = tools_readVcc(); // Get supply voltage
   digitalWrite(LED, LOW);  // turn led off
   
   delay(5); // The above doesn't seem to work for everyone (why?)
   
   sensors.begin(); //start up temp sensor
   sensors.requestTemperatures(); // Get the temperature
   tinytx.temp=(sensors.getTempCByIndex(0)*100); // Read first sensor and convert to integer, reversed at receiving end
   tinytx.paramsize = myParamsSize;   
   
   digitalWrite(ONE_WIRE_POWER, LOW); // turn DS18B20 off
   
   // tools_rfwrite(myNodeID, &tinytx, sizeof tinytx); // Send data via RF 
   // emulate RF send string
   // i.e.:  112 1 2987 1806
   Serial.print(myNodeID);      // 1
   Serial.print(mySubNodeID);   // 12
   Serial.print(' ');
   Serial.print(myParamsSize);  // 1
   Serial.print(' ');
   Serial.print(tinytx.supplyV);// 2987
   Serial.print(' ');
   Serial.print(tinytx.temp);    // 1806
   Serial.println();

   Sleepy::loseSomeTime(10000); //JeeLabs power save function: enter low power mode for 60 seconds (valid range 16-65000 ms)
}

