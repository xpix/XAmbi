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
// http://www.trashboard.de/2014/01/23/avr-arduino-blumen-giessen-teil-1/
//----------------------------------------------------------------------------------------------------------------------
//#define RF69_COMPAT 1

#include <JeeLib.h> // https://github.com/jcw/jeelib
#include "xambikid_tools.h"

//#define USE_ACK           // Enable ACKs, comment out to disable

// Node defines
#define network 210       // RF12 Network group
#define myNodeID 3        // RF12 node ID in the range 1-30
#define mySubNodeID 15    // RF12 subnode ID in the range 1-255 (inside = >= 60)
#define myParamsSize 1    // How much values want to send (supplyV, value1, value2) = 3
#define INTERVAL 5        // Time in minutes to sleep

// Sensor defines
#define MOIS_DATA  A7      // Moisture sensor is connected on A1/ATtiny pin 13
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

int minutes = -1;

void setup() {
  Serial.begin(9600);
  Serial.println("Start Sensor...");
#ifdef USE_ACK
  ack(1); // enable use ack
#endif

  pinMode(MOIS_POWER, OUTPUT); // set power on for sensor
  pinMode(MOIS_DATA, INPUT);   // prepare data pin for input

  Serial.println("Init RFM ...");
  tools_init_rf24(myNodeID, network);
  Serial.println("RFM ready");
}

void loop() {
  // measure the value only every INTERVAL minutes
  if(minutes < 0 || minutes >= 5){

    // anable analog pins
    tools_enable_adc();

    // sensor Power on 
    digitalWrite(MOIS_POWER, HIGH); // turn Moisture sensor on
    delay(5); // Allow 5ms for the sensor to be ready

    // read analog value 
    int sensorValue = analogRead(MOIS_DATA);

    // sensor Power off 
    digitalWrite(MOIS_POWER, LOW); // turn Power off

    // disable analog pins 
    tools_disable_adc();
  
    Serial.println(sensorValue);
    Serial.println("----------");

    // set Payload  
    tinytx.paramsize = myParamsSize;   
    tinytx.moisture = sensorValue;
    tinytx.supplyV = tools_readVcc(); // Get supply voltage
    tools_rfwrite(myNodeID, &tinytx, sizeof tinytx); // Send data via RF 

    // reset minute timer
    minutes = 0; 
  } 

  minutes++;
  Sleepy::loseSomeTime(60000);
}


