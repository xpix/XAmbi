//----------------------------------------------------------------------------------------------------------------------
// TinyTX - An ATtiny84 and RFM12B Wireless Plant moisture Sensor Node
// By Frank Herrmann. For hardware design see http://xpix.de
//
// Using a this simple moisture sensor
// http://www.dfrobot.com/image/data/SEN0114/Moisture%20Sensor%20Schematic.pdf
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
#define mySubNodeID 50    // RF12 subnode ID in the range 1-255 (outside >= 50)
//#define mySubNodeID 60    // RF12 subnode ID in the range 1-255 (inside = >= 60)
#define myParamsSize 1    // How much values want to send (supplyV, value1, value2) = 3

// Sensor defines
#define ONE_WIRE_BUS 10   // DS18B20 Temperature sensor is connected o0qn D10/ATtiny pin 13
#define ONE_WIRE_POWER 9  // DS18B20 Power pin is connected on D9/ATtiny pin 12

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
  pinMode(ONE_WIRE_POWER, OUTPUT); // set power on for sensor
  tools_init_rf24(myNodeID, network);
}

void loop() {
  tinytx.paramsize = myParamsSize;   
  digitalWrite(ONE_WIRE_POWER, HIGH); // turn DS18B20 sensor on

  //Sleepy::loseSomeTime(5); // Allow 5ms for the sensor to be ready
  delay(5); // The above doesn't seem to work for everyone (why?)
 
  sensors.begin(); //start up temp sensor
  sensors.requestTemperatures(); // Get the temperature
  tinytx.temp=(sensors.getTempCByIndex(0)*100); // Read first sensor and convert to integer, reversed at receiving end
  
  digitalWrite(ONE_WIRE_POWER, LOW); // turn DS18B20 off
  
  tinytx.supplyV = tools_readVcc(); // Get supply voltage

  rfwrite(); // Send data via RF 

  //Sleepy::loseSomeTime(1000); //Testing 1sec break
  Sleepy::loseSomeTime(10000); //JeeLabs power save function: enter low power mode for 60 seconds (valid range 16-65000 ms)

}
//--------------------------------------------------------------------------------------------------
// Send payload data via RF
//-------------------------------------------------------------------------------------------------
 static void rfwrite(){
  #ifdef USE_ACK
   for (byte i = 0; i <= RETRY_LIMIT; ++i) {  // tx and wait for ack up to RETRY_LIMIT times
     rf12_sleep(-1);              // Wake up RF module
      while (!rf12_canSend())
      rf12_recvDone();
      rf12_sendStart(RF12_HDR_ACK, &tinytx, sizeof tinytx); 
      rf12_sendWait(2);           // Wait for RF to finish sending while in standby mode
      byte acked = waitForAck(myNodeID);  // Wait for ACK
      rf12_sleep(0);              // Put RF module to sleep
      if (acked) { return; }      // Return if ACK received
  
      Sleepy::loseSomeTime(RETRY_PERIOD * 1000);     // If no ack received wait and try again
   }
  #else
     rf12_sleep(-1);              // Wake up RF module
     while (!rf12_canSend())
       rf12_recvDone();
     rf12_sendStart(0, &tinytx, sizeof tinytx); 
     rf12_sendWait(2);           // Wait for RF to finish sending while in standby mode
     rf12_sleep(0);              // Put RF module to sleep
     return;
  #endif
 }

