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
//#define RF69_COMPAT 1

#include <JeeLib.h> // https://github.com/jcw/jeelib
#include "xambikid_tools.h"

//#define USE_ACK           // Enable ACKs, comment out to disable

// Node defines
#define network 210       // RF12 Network group
#define myNodeID 2        // RF12 node ID in the range 1-30
//#define mySubNodeID 52    // RF12 subnode ID in the range 1-255 (outside >= 50)
#define mySubNodeID 60    // RF12 subnode ID in the range 1-255 (inside = >= 60)
#define myParamsSize 1    // How much values want to send (supplyV, value1, value2) = 3

// Sensor defines
#define MOIS_DATA  A1      // Moisture sensor is connected on A1/ATtiny pin 13
#define MOIS_POWER 10      // DHT Power pin is connected on D9/ATtiny pin 12

// Data Structure to be sent
struct Payload_s{
  byte subnode;       // Subnode Id (standard value)
  byte paramsize;     // parameters size
  int supplyV;	      // Supply voltage (standard value)
  int moisture;	      // moisture reading
} Payload_default  = {mySubNodeID, myParamsSize, 0, 0}; // Default Values!!
typedef struct Payload_s Payload;
Payload tinytx = Payload_default;
// --------------------------


void setup() {
  Serial.begin(9600);
  Serial.println("Start Sensor...");

  pinMode(MOIS_POWER, OUTPUT); // set power on for sensor
  pinMode(MOIS_DATA, INPUT);   // prepare data pin for input

  Serial.println("Init RFM ...");
  tools_init_rf24(myNodeID, network);
}

void loop() {

  tools_enable_adc();

  digitalWrite(MOIS_POWER, HIGH); // turn Moisture sensor on
  Sleepy::loseSomeTime(5); // Allow 5ms for the sensor to be ready
  int sensorValue = analogRead(MOIS_DATA);

  Serial.println(sensorValue);
  Serial.println("----------");

  // set Payload  
  tinytx.moisture = sensorValue;
  tinytx.supplyV = tools_readVcc(); // Get supply voltage
  rfwrite(); // Send data via RF 

  digitalWrite(MOIS_POWER, LOW); // turn Power off
  tools_disable_adc();

  Sleepy::loseSomeTime(10000);
  //looseSomeTimeInMinutes(5);
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

