//----------------------------------------------------------------------------------------------------------------------
// XAmbiKid/Mailbox - An ATtiny84 and RFM12B Wireless Mailbox Node
// By Frank Herrmann. For hardware design see http://xpix.de
//
// Licenced under the Creative Commons Attribution-ShareAlike 3.0 Unported (CC BY-SA 3.0) licence:
// http://creativecommons.org/licenses/by-sa/3.0/
//
// Requires Arduino IDE with arduino-tiny core: http://code.google.com/p/arduino-tiny/
//
// Please check http://forum.arduino.cc/index.php?topic=101156.0 for simple schematic and function
//----------------------------------------------------------------------------------------------------------------------

#include <JeeLib.h> // https://github.com/jcw/jeelib
#include "xambikid_tools.h"
#include <SensorOp.h>

//#define USE_ACK           // Enable ACKs, comment out to disable

// Node defines
#define network 210       // RF12 Network group
#define myNodeID 4        // RF12 node ID in the range 1-30
#define mySubNodeID 11    // RF12 subnode ID (Please check config file: cfg/sensortypes.cfg)
#define myParamsSize 2    // How much values want to send (supplyV, value1, value2) = 3

#define LED 3            // Powerpin from IR-LED
#define SENSOR A1        // Sensorpin on Fototransistor
#define THRESHOLD 50    // Threshold for full
#define SAMPLES 4

// Sensor defines
SensorOp sensor(LED, SENSOR, SAMPLES);

bool oldfull = false;
int sensor_dark = 0;
int minutes = 0;

// Data Structure to be sent
struct Payload_s{
  byte subnode;       // Subnode Id (standard value)
  byte paramsize;     // parameters size
  int supplyV;	      // Supply voltage (standard value)
  int box;	      // box event
  int val;	      // for debug
} Payload_default  = {mySubNodeID, myParamsSize, 0, 0, 0}; // Default Values!!
typedef struct Payload_s Payload;
Payload tinytx = Payload_default;
// --------------------------


void setup() {
#ifdef USE_ACK
  ack(1); // enable use ack
#endif
  Serial.begin(9600);
  Serial.println("Init...");
  pinMode(LED, OUTPUT);       // set power on for led

  tools_init_rf24(myNodeID, network);
  delay(10000);

  sensor_dark = calibration();
  Serial.println(sensor_dark);

  Serial.println("Start Loop...");
}

void loop() {
  // If Box empty then Calibrate every hour
  // wait till box are empty
  if(oldfull == false && minutes++ >= 60){
    sensor_dark = calibration();
    minutes = 0; 
  }
  // Measure Power 
   tinytx.supplyV = tools_readVcc(); // Get supply voltage

   tools_enable_adc();
   led_on();  // turn led on
   int sensorValue = sensor.readSensor();
   tinytx.val = sensorValue; // Get sensor value
   led_off();  // turn led off
   tools_disable_adc();

   Serial.println(sensorValue);

   // empty or full
   bool full = false;
   if(sensorValue > sensor_dark + THRESHOLD){
      full = true;
   }

   if(oldfull == false && full == true){
     // Event, Box was filled!
     tinytx.box = 1;
     Serial.println("Send full");
     //tools_rfwrite(myNodeID, &tinytx, sizeof tinytx); // Send data via RF 
   }
   if(oldfull == true && full == false){
     // Event, Box was emptied!
     tinytx.box = 0;
     Serial.println("Send empty");
     //tools_rfwrite(myNodeID, &tinytx, sizeof tinytx); // Send data via RF 
   }

   // for debug send data every minute
   tools_rfwrite(myNodeID, &tinytx, sizeof tinytx); // Send data via RF 
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

int calibration(){
  // Calibration
  int calib=0;

  Serial.println("Calibrierung...");
  tools_enable_adc();
  led_on();
  calib += sensor.readSensor();
  delay(50);
  calib += sensor.readSensor();
  delay(80);
  calib += sensor.readSensor();
  delay(100);
  calib += sensor.readSensor();
  delay(200);
  calib += sensor.readSensor();
  delay(500);
  calib += sensor.readSensor();
  led_off();
  tools_disable_adc();

  int result = calib/6;
  return result;
}
