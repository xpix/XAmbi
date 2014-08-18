#include <JeeLib.h>
#include "xambikid_tools.h"

// Fixed RF12 settings
#define network 210       // RF12 Network group
#define myNodeID 30       // RF12 node ID in the range 1-30


int vbat;

typedef struct {
int lp;
int bat;
} Payload;
Payload pomiar;

//#define rf12_sleep(x)

void setup () {
  
  char ausgabe[80];

  Serial.begin(9600);
  tools_init_rf24(myNodeID, network);

  sprintf(ausgabe, "Simple Sender\nNode:%d Network:%d", myNodeID, network);
  Serial.println(ausgabe);

  rf12_sleep(RF12_SLEEP);
}


void loop () {
   

  pomiar.bat = getbat();
  rf12_sleep(RF12_WAKEUP);

    

  while (!rf12_canSend())
          rf12_recvDone();
        
  rf12_sendStart(RF12_HDR_ACK, &pomiar, sizeof pomiar);

    // set the sync mode to 2 if the fuses are still the Arduino default
    // mode 3 (full powerdown) can only be used with 258 CK startup fuses
    rf12_sendWait(2);
    rf12_sleep(RF12_SLEEP);
  
  pomiar.lp += 1;
  
  Sleepy::loseSomeTime(1000); // czas uśpienia czujki do kolejnego pimiaru i wysyłki danych
  

}



// pomiar napięcia na zasilaniu
int getbat(void){
  
  analogReference(INTERNAL);
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
  delay(20);
  vbat=map(analogRead(A0),0,1023,0,3416); // potrzebny dzielnik z rezystorów 10k (D5-A0) oraz 4,7k (A0-GND), układ dobrany dla zasilania od2,0V do 3,3V
  digitalWrite(5, LOW);
  pinMode(5, INPUT);
  analogReference(DEFAULT);
  return vbat;
}

