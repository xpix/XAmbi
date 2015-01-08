//----------------------------------------------------------------------------------------------------------------------
// XAmbiKid/Mailbox - An ATtiny84 and RFM12B Wireless Mailbox Node
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
#include <SensorOp.h>

//#define USE_ACK           // Enable ACKs, comment out to disable

// Node defines
#define network 210       // RF12 Network group
#define myNodeID 4        // RF12 node ID in the range 1-30
#define mySubNodeID 11    // RF12 subnode ID (Please check config file: cfg/sensortypes.cfg)
#define myParamsSize 1    // How much values want to send (supplyV, value1, value2) = 3

#define LED 9            // Powerpin from IR-LED
#define SENSOR A0        // Sensorpin on Fototransistor
#define THRESHOLD 100    // Threshold for full
#define SAMPLES 4

// Sensor defines
SensorOp sensor(LED, SENSOR, SAMPLES);

bool full = false;
bool oldfull = false;
int sensor_dark = 0;

// Data Structure to be sent
struct Payload_s{
  byte subnode;       // Subnode Id (standard value)
  byte paramsize;     // parameters size
  int supplyV;	      // Supply voltage (standard value)
  int box;	      // box event
} Payload_default  = {mySubNodeID, myParamsSize, 0, -1}; // Default Values!!
typedef struct Payload_s Payload;
Payload tinytx = Payload_default;
// --------------------------


void setup() {
#ifdef USE_ACK
  ack(1); // enable use ack
#endif
  pinMode(LED, OUTPUT);       // set power on for led
  // Send Sequence o(2s)/x(1s)/o(1s)/x(1s)
  led_on(); // on 2 sec
  delay(2000);
  led_off();
  tools_init_rf24(myNodeID, network);
  delay(1000); // off 1 sec
  led_on();
  delay(1000); // on 1 sec
  led_off();
  delay(1000); // off 1 sec

  // Calibration
  int calib=0;
  for(int i; i <= SAMPLES; i++){
    led_on();
    calib += sensor.readSensor();
    led_off();
    delay(200);
  }
  sensor_dark = calib/SAMPLES;
}

void loop() {
   // Measure Power for solar low power nodes with super cap
   // The Node switch on at ~1.8V. This is not enough for 
   // the RFM12B (need min 2.2V). Wait for enough saved 
   // solar power from supercap
   tinytx.supplyV = tools_readVcc(); // Get supply voltage

   led_on();  // turn led on
   int sensorValue = sensor.readSensor();
   led_off();  // turn led off
   
   delay(5); // The above doesn't seem to work for everyone (why?)

   // empty or full
   if(sensorValue < sensor_dark + THRESHOLD){
      full = false;
   }
   else {
      full = true;
   }
   if(oldfull == false && full == true){
     // Event, Box was filled!
     tinytx.box = 1;
     tools_rfwrite(myNodeID, &tinytx, sizeof tinytx); // Send data via RF 
   }
   if(oldfull == true && full == false){
     // Event, Box was emptied!
     tinytx.box = 0;
     tools_rfwrite(myNodeID, &tinytx, sizeof tinytx); // Send data via RF 
   }
   oldfull = full;

   Sleepy::loseSomeTime(60000); //JeeLabs power save function: enter low power mode for 60 seconds (valid range 16-65000 ms)
}

void led_on(){
   digitalWrite(LED, HIGH);  // turn led on
   delay(5); // only a flash
}

void led_off(){
   digitalWrite(LED, LOW);  // turn led off
}
