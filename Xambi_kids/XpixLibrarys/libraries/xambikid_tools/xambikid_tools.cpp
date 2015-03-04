#include "Arduino.h"
#include <EEPROM.h>
#include <JeeLib.h> // https://github.com/jcw/jeelib
#include "xambikid_tools.h" 

bool USE_ACK = 0;
volatile bool adcDone;
// for low-noise/-power ADC readouts, we'll use ADC completion interrupts
ISR(ADC_vect) { adcDone = true; }
ISR(WDT_vect) { Sleepy::watchdogEvent(); } // interrupt handler for JeeLabs Sleepy power saving

// Data Structure to be sent
struct Payload_s{
  byte subnode;   // Subnode Id (standard value)
  int sign;       // Sign to signal, i need a node id (9999)
  int startid;	   // Start id from segement to need an id
} PayloadNeedId_default  = {0, NEEDNEWID, 0}; // Default Values!!
typedef struct Payload_s Payload;
Payload PayloadNeedId = PayloadNeedId_default;
// --------------------------


// Wait a few milliseconds for proper ACK
byte waitForAck(byte myNodeID) {
   MilliTimer ackTimer;
   while (!ackTimer.poll(ACK_TIME)) {
     if (rf12_recvDone() && rf12_crc == 0 &&
        rf12_hdr == (RF12_HDR_DST | RF12_HDR_CTL | myNodeID))
        return 1;
     }
   return 0;
}


void tools_init_rf24(byte myNodeID, byte network){
   delay(100);
   rf12_initialize(myNodeID,freq,network); // Initialize RFM12 with settings defined above 
   rf12_sleep(0);                          // Put the RFM12 to sleep

   rf12_control(0xC691); //bitrate 2.395kbps
// rf12_control(0xC688); // bitrate 4.789kbps
// rf12_control(0xC623); // data rate 9.579kbps
   rf12_control(0x94C2); // Rx control 67 kHz bandwidth
   rf12_control(0x9820); // Tx control 45 kHz deviation

   PRR = bit(PRTIM1); // only keep timer 0 goingh
   tools_disable_adc();
}

void tools_enable_adc(){
  bitClear(PRR, PRADC); ADCSRA |= bit(ADEN); // Enable the ADC
}

void tools_disable_adc(){
  ADCSRA &= ~ bit(ADEN); bitSet(PRR, PRADC); // Disable the ADC to save power
}

void get_nodeid (int startrangeid){
  // global function to get a NodeID if not exists
  // Flow:
  // 1.  try to read myNodeID and mySubNodeID from EEPROM
  // 2.  if not exists set myNodeID = 9 and mySubNodeID = 99: 999
  byte myNodeID      = EEPROM.read(NODEIDSADR);
  byte mySubNodeID   = EEPROM.read(NODEIDSADR + 1);
  
  // 3.  send a "give me a node":GMN message to gateway
  if(! myNodeID && ! mySubNodeID){
     myNodeID         = 9;
     mySubNodeID      = 99;
     PayloadNeedId.sign       = NEEDNEWID; // set signal to need an id
     PayloadNeedId.startid    = startrangeid;  // set startid to discover free id's in range
     tools_rfwrite(myNodeID, &PayloadNeedId, sizeof PayloadNeedId); // Send data via RF 

     // 4.  wait for an answer i.e. "GMN:1 14" from gateway
     while(! rf12_recvDone()){
        String answer = "";
        if (rf12_recvDone() && rf12_crc == 0) {
           for (byte i = 0; i < rf12_len; ++i)
               answer += String(rf12_data[i]);
           if(answer.substring(0,3) == "GSM:"){
               myNodeID = answer.substring(3,4).toInt();
               mySubNodeID = answer.substring(5).toInt();
           }
        }
     }
     // 5.  set new node id in EEPROM
     if(myNodeID != 9){
         EEPROM.write(NODEIDSADR,   myNodeID);
         EEPROM.write(NODEIDSADR+1, mySubNodeID);
     }
  }
  // 6.  go in loop
}

void looseSomeTimeInMinutes(byte minutes){
  for(byte i=0; i<=minutes; i++){
      Sleepy::loseSomeTime(60000); //JeeLabs power save function: enter low power mode for 60 seconds (valid range 16-65000 ms)
  }
}

bool tools_ack(bool state){
   USE_ACK = state;
   return USE_ACK;
}

bool tools_ack(){
   return USE_ACK;
}


//--------------------------------------------------------------------------------------------------
// Send payload data via RF
//-------------------------------------------------------------------------------------------------
void tools_rfwrite (byte myNodeID, const void* ptr, uint8_t len) {
   if(tools_ack()){
      for (byte i = 0; i <= RETRY_LIMIT; ++i) {  // tx and wait for ack up to RETRY_LIMIT times
        rf12_sleep(-1);              // Wake up RF module
         while (!rf12_canSend())
         rf12_recvDone();
         rf12_sendStart(RF12_HDR_ACK, ptr, len); 
         rf12_sendWait(2);           // Wait for RF to finish sending while in standby mode
         byte acked = waitForAck(myNodeID);  // Wait for ACK
         rf12_sleep(0);              // Put RF module to sleep
         if (acked) { return; }      // Return if ACK received
     
         Sleepy::loseSomeTime(RETRY_PERIOD * 1000);     // If no ack received wait and try again
      }
   }
   else {
      rf12_sleep(-1);              // Wake up RF module
      while (!rf12_canSend())
         rf12_recvDone();
      rf12_sendStart(0, ptr, len); 
      rf12_sendWait(2);           // Wait for RF to finish sending while in standby mode
      rf12_sleep(0);              // Put RF module to sleep
      return;
   }
}

//--------------------------------------------------------------------------------------------------
// Read current supply voltage
//--------------------------------------------------------------------------------------------------
long tools_readVcc() {
   bitClear(PRR, PRADC); ADCSRA |= bit(ADEN); // Enable the ADC
   long result;
   // Read 1.1V reference against Vcc
   #if defined(__AVR_ATtiny84__) 
    ADMUX = _BV(MUX5) | _BV(MUX0); // For ATtiny84
   #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  // For ATmega328
   #endif 
   delay(2); // Wait for Vref to settle
   ADCSRA |= _BV(ADSC); // Convert
   while (bit_is_set(ADCSRA,ADSC));
   result = ADCL;
   result |= ADCH<<8;
   result = 1126400L / result; // Back-calculate Vcc in mV
   ADCSRA &= ~ bit(ADEN); bitSet(PRR, PRADC); // Disable the ADC to save power
   return result;
} 

//########################################################################################################################

