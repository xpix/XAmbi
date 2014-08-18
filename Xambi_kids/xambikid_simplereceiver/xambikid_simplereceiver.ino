/*
----------------------------------------------------------------------------------------------------------------------
 TinyTX Simple Receive Example
 By Frank Herrmann. 

 Licenced under the Creative Commons Attribution-ShareAlike 3.0 Unported (CC BY-SA 3.0) licence:
 http://creativecommons.org/licenses/by-sa/3.0/

 Receiver is very simple, he get all Value and print this space separated on every line to serial console.
 All Programs to publish this values to a service provider understand this simple format.
----------------------------------------------------------------------------------------------------------------------
*/

//#define RF69_COMPAT 1

#include <JeeLib.h> // https://github.com/jcw/jeelib
#include "xambikid_tools.h"

// Fixed RF12 settings
#define network 210       // RF12 Network group
#define myNodeID 30       // RF12 node ID in the range 1-30

typedef struct {
  byte subnode;
  byte paramsize;     // parameters size
  int supplyV;          // tx voltage
  int 
    ValueA, ValueB, ValueC,ValueD,ValueE,ValueF; // sensor valuea
} Payload;
Payload rx;

int nodeID;    //node ID of tx, extracted from RF datapacket. Not transmitted as part of structure

void setup () {
  char ausgabe[80];

  Serial.begin(9600);
  tools_init_rf24(myNodeID, network);

  sprintf(ausgabe, "Simple Receiver\nNode:%d Network:%d", myNodeID, network);
  Serial.println(ausgabe);

  Serial.println("Waiting for data ...\n");
}

void loop() {

  if (rf12_recvDone() && rf12_crc == 0 && (rf12_hdr & RF12_HDR_CTL) == 0) {
    nodeID = rf12_hdr & 0x1F;  // get node ID
    rx = *(Payload*) rf12_data;


    if (RF12_WANTS_ACK) {                  // Send ACK if requested
      rf12_sendStart(RF12_ACK_REPLY, 0, 0);
    }
    if(rx.paramsize > 10){
      // Ignore
      Serial.print("#Ignore Data from Node: ");
      Serial.print(nodeID);
      Serial.println(rx.subnode);
      return;
    }
    Serial.print(nodeID);
    Serial.print(rx.subnode);
    Serial.print(' ');
    Serial.print(rx.paramsize);
    Serial.print(' ');
    Serial.print(rx.supplyV);
    Serial.print(' ');
    Serial.print(rx.ValueA);
    Serial.print(' ');
    if(rx.paramsize > 2){
      Serial.print(rx.ValueB);
      Serial.print(' ');
    }
    if(rx.paramsize > 3){
      Serial.print(rx.ValueC);
      Serial.print(' ');
    }
    if(rx.paramsize > 4){
      Serial.print(rx.ValueD);
      Serial.print(' ');
    }
    if(rx.paramsize > 5){
      Serial.print(rx.ValueE);
      Serial.print(' ');
    }
    if(rx.paramsize > 6){
      Serial.print(rx.ValueF);
      Serial.print(' ');
    }
    if(rx.paramsize > 7){
      Serial.print("Received packet are too big!");
    }
    Serial.println();

  } // rf12_recvDone
}


