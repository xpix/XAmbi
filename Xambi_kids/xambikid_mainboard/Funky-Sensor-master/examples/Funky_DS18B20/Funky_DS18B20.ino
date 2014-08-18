//--------------------------------------------------------------------------------------
// Funky Wireless Temperature Sensor Node
// http://harizanov.com/2012/10/funky-as-remote-temperature-sensing-node-with-ds18b20/
// To use with DS18B20, a 4K7 resistor is needed between pads 4 and 5 of the Funky (Vdd and DQ)
// Martin Harizanov http://harizanov.com
// GNU GPL V3
//--------------------------------------------------------------------------------------

#include <JeeLib.h> // https://github.com/jcw/jeelib
#include "pins_arduino.h"

#include <OneWire.h>   // http://www.pjrc.com/teensy/arduino_libraries/OneWire.zip
#include <DallasTemperature.h>  // http://download.milesburton.com/Arduino/MaximTemperature/DallasTemperature_371Beta.zip
 
#define ONE_WIRE_BUS 3  // pad 5 of the Funky
#define tempPower 7     // Power pin is connected pad 4 on the Funky


// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
//addresses of sensors, MAX 4!!
byte allAddress [4][8];  // 8 bytes per address
#define TEMPERATURE_PRECISION 12
#define ASYNC_DELAY 750 // 9bit requres 95ms, 10bit 187ms, 11bit 375ms and 12bit resolution takes 750ms


//#define LEDpin PB0 new revisions use this pin as CLKI
#define LEDpin 10


ISR(WDT_vect) { Sleepy::watchdogEvent(); } // interrupt handler for JeeLabs Sleepy power saving

#define myNodeID 19      // RF12 node ID in the range 1-30
#define network 210      // RF12 Network group
#define freq RF12_868MHZ // Frequency of RFM12B module


//########################################################################################################################
//Data Structure to be sent, it is variable in size and we only send 2+n*2 bytes where n is the number of DS18B20 sensors attached
//########################################################################################################################

 typedef struct {
    int supplyV;	// Supply voltage
	int temp;	// Temperature reading
  	int temp2;	// Temperature 2 reading
  	int temp3;	// Temperature 3 reading
  	int temp4;	// Temperature 4 reading	
 } Payload;

 Payload temptx;

 int numSensors;
 
 static void setPrescaler (uint8_t mode) {
    cli();
    CLKPR = bit(CLKPCE);
    CLKPR = mode;
    sei();
}

//########################################################################################################################

void setup() {

  pinMode(LEDpin,OUTPUT);
  digitalWrite(LEDpin,LOW);  // LED on
  Sleepy::loseSomeTime(100); 
  digitalWrite(LEDpin,HIGH);   // LED off
  
  rf12_initialize(myNodeID,freq,network); // Initialize RFM12 with settings defined above 
  rf12_control(0xC000);					  // Adjust low battery voltage to 2.2V
  
  // !mp,90kHz,last byte=power level: 0=highest, 7=lowest
  byte txPower=7; //LOWEST possible
  rf12_control(0x9850 | (txPower > 7 ? 7 : txPower));

  rf12_sleep(0);                          // Put the RFM12 to sleep
 
  PRR = bit(PRTIM1); // only keep timer 0 going
  ADCSRA &= ~ bit(ADEN); // Disable the ADC
  bitSet (PRR, PRADC);   // Power down ADC
  bitClear (ACSR, ACIE); // Disable comparitor interrupts
  bitClear (ACSR, ACD);  // Power down analogue comparitor
  bitSet(PRR, PRUSI); // disable USI h/w  
  
  pinMode(tempPower, OUTPUT); // set power pin for DS18B20 to output
  digitalWrite(tempPower, HIGH); // turn sensor power on
  Sleepy::loseSomeTime(50); // Allow 50ms for the sensor to be ready
  // Start up the library
  sensors.begin(); 
  sensors.setWaitForConversion(false);     
  numSensors=sensors.getDeviceCount(); 

  byte j=0;                                        // search for one wire devices and
                                                   // copy to device address arrays.
  while ((j < numSensors) && (oneWire.search(allAddress[j]))) {        
    j++;
  }
    

  
}

void loop() {
  pinMode(tempPower, OUTPUT); // set power pin for DS18B20 to output  
  digitalWrite(tempPower, HIGH); // turn DS18B20 sensor on
  Sleepy::loseSomeTime(20); // Allow 20ms for the sensor to be ready
  
  /*
  for(int j=0;j<numSensors;j++) {
    sensors.setResolution(allAddress[j], TEMPERATURE_PRECISION);      // and set the a to d conversion resolution of each.
  }  
  */
  
  sensors.requestTemperatures(); // Send the command to get temperatures  
  Sleepy::loseSomeTime(ASYNC_DELAY); //Must wait for conversion, since we use ASYNC mode
  
                     temptx.temp=(sensors.getTempC(allAddress[0])*100);  
  if (numSensors>1) { temptx.temp2=(sensors.getTempC(allAddress[1])*100); } 
  if (numSensors>2) { temptx.temp3=(sensors.getTempC(allAddress[2])*100); } 
  if (numSensors>3) { temptx.temp4=(sensors.getTempC(allAddress[3])*100); }
  digitalWrite(tempPower, LOW); // turn DS18B20 sensor off
  pinMode(tempPower, INPUT); // set power pin for DS18B20 to input before sleeping, saves power
  
  digitalWrite(LEDpin,LOW);  //LED on for some time, to show we are alive

  bitClear(PRR, PRADC); // power up the ADC
  ADCSRA |= bit(ADEN); // enable the ADC  
  Sleepy::loseSomeTime(2); 
  temptx.supplyV = readVcc(); // Get supply voltage
  ADCSRA &= ~ bit(ADEN); // disable the ADC
  bitSet(PRR, PRADC); // power down the ADC

  digitalWrite(LEDpin,HIGH);  //LED off
  
  rfwrite(); // Send data via RF   
  
  for(byte j = 0; j < 5; j++) {    // Sleep for j minutes
    Sleepy::loseSomeTime(60000); //JeeLabs power save function: enter low power mode for 60 seconds (valid range 16-65000 ms)
  }

  
}

//--------------------------------------------------------------------------------------------------
// Send payload data via RF
//--------------------------------------------------------------------------------------------------
 static void rfwrite(){
   bitClear(PRR, PRUSI); // enable USI h/w  
   rf12_sleep(-1);     //wake up RF module
   while (!rf12_canSend())
   rf12_recvDone();
   rf12_sendStart(0, &temptx, numSensors*2 + 2); // two bytes for the battery reading, then 2*numSensors for the number of DS18B20s attached to Funky
   rf12_sendWait(2);    //wait for RF to finish sending while in standby mode
   rf12_sleep(0);    //put RF module to sleep
   bitSet(PRR, PRUSI); // disable USI h/w
}

//--------------------------------------------------------------------------------------------------
// Read current supply voltage
//--------------------------------------------------------------------------------------------------

 long readVcc() {
   long result;
   // Read 1.1V reference against Vcc
   ADMUX = _BV(MUX5) | _BV(MUX0);
   delay(2); // Wait for Vref to settle
   ADCSRA |= _BV(ADSC); // Convert
   while (bit_is_set(ADCSRA,ADSC));
   result = ADCL;
   result |= ADCH<<8;
   result = 1126400L / result; // Back-calculate Vcc in mV
   return result;
}
