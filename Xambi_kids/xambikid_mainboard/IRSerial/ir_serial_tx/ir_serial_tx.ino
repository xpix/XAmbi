// Tgis is a lowpower dirty sensor for DHT11 as example
// the measure Humidity and Temperature and send this
// via infrared with 1200 baud to a TSOP Gateway.
//
// Some Rules if you use a Arduino Pro Mini:
// 1.) Use the 3.3V version Arduino Pro Mini with the lower CPU clock speed (8MHz)
// 2.) Disconnect the Arduino's power indicator LED by cutting the track between the LED 
//     and the resistor in series. Saves about 1.5 mA. Please check this:
//     http://www.mysensors.org/build/battery

//dirt cheap wireless TX
//generates 38kHz carrier wave on pin 9 and 10
//sends data via TX every 500ms

// For Attiny8* please check this:
// http://www.ernstc.dk/arduino/38khz_timer.htm

#include "DHT.h"
#include "LowPower.h" // Lib: http://www.rocketscream.com/blog/2011/07/04/lightweight-low-power-arduino-library/

#define DHTPIN 8     // what pin we're connected to

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11 
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

DHT dht(DHTPIN, DHTTYPE);

void setup()
{
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);

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
  Serial.println('DHT11 IR Node 117.');
}


void loop() {
  // Enter idle state for 8 s with the rest of peripherals turned off
  // Each microcontroller comes with different number of peripherals
  // Comment off line of code where necessary

  // ATmega328P, ATmega168
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(t) || isnan(h)) {
    Serial.println("Failed to read from DHT");
  } else {
    Serial.print("Humidity: "); 
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: "); 
    Serial.print(t);
    Serial.println(" *C");
    delay(2000); // importent, cuz mc send and go too early in sleep mode
  }
}
