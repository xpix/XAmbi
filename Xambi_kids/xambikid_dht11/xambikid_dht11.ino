//----------------------------------------------------------------------------------------------------------------------
// XAmbiKid/DHT11 - An ATtiny84 and RFM12B Wireless Temperature/Moisture Sensor Node
// By Frank Herrmann. For hardware design see http://xpix.de
//
// Using a this simple temp/moisture sensor with DHTXX
//
// Licenced under the Creative Commons Attribution-ShareAlike 3.0 Unported (CC BY-SA 3.0) licence:
// http://creativecommons.org/licenses/by-sa/3.0/
//
// Requires Arduino IDE with arduino-tiny core: http://code.google.com/p/arduino-tiny/
//
//----------------------------------------------------------------------------------------------------------------------

#include <JeeLib.h> // https://github.com/jcw/jeelib
#include "xambikid_tools.h"

//#define USE_ACK           // Enable ACKs, comment out to disable

// Node defines
#define network 210       // RF12 Network group
#define myNodeID 1        // RF12 node ID in the range 1-30
#define mySubNodeID 55    // RF12 subnode ID in the range 1-255 (outside >= 50)
#define myParamsSize 2    // How much values want to send (supplyV, value1, value2) = 3

// Sensor defines
#define DHT_DATA  3      // DHT11 sensor is connected to D3/ATtiny pin 6
#define DHT_POWER 0      // DHT11 Power pin is connected on D0/ATtiny pin 2

#define LED 9            // Powerpin from LED

DHTxx dht (DHT_DATA);

// Data Structure to be sent
struct Payload_s{
  byte subnode;       // Subnode Id (standard value)
  byte paramsize;     // parameters size
  int supplyV;	      // Supply voltage (standard value)
  int temp;	      // temperature reading
  int moist;	      // moisture reading
} Payload_default  = {mySubNodeID, myParamsSize, 0, 0}; // Default Values!!
typedef struct Payload_s Payload;
Payload tinytx = Payload_default;
// --------------------------


void setup() {
#ifdef USE_ACK
  ack(1); // enable use ack
#endif
  pinMode(LED, OUTPUT);       // set power on for led
  pinMode(DHT_POWER, OUTPUT); // set power on for sensor
  tools_init_rf24(myNodeID, network);
}

void loop() {
  // Measure Power for solar low power nodes with super cap
  // The Node switch on at ~1.8V. This is not enough for 
  // the RFM12B (need min 2.2V). Wait for enough saved 
  // solar power from supercap or sleep again
  tinytx.supplyV = tools_readVcc(); // Get supply voltage

  digitalWrite(DHT_POWER, HIGH); // turn sensor on
  digitalWrite(LED, HIGH);  // turn led on
  delay(50); // only a flash
  digitalWrite(LED, LOW);  // turn led off

  // Read sensor values with 1 sec break to DHT11 warm up
  delay(950); // let sensor some time to warmup
  int t, h;
  dht.reading(t, h);
  digitalWrite(DHT_POWER, LOW); // turn sensor off

  tinytx.paramsize = myParamsSize;   
  tinytx.temp = t*10; // temp i.e. 21C == 2100
  tinytx.moist = h/10; // moist 55% == 55

  tools_rfwrite(myNodeID, &tinytx, sizeof tinytx); // Send data via RF 
  
  //Sleepy::loseSomeTime(60000); //JeeLabs power save function: enter low power mode for 60 seconds (valid range 16-65000 ms)
  looseSomeTimeInMinutes(5);

}

