#define freq         RF12_433MHZ // Frequency of RFM12B module
#define NODEIDSADR   10          // use NODEIDSADR for first byte and NODEIDSADR+1 for second byte 

#define RETRY_PERIOD 1           // How soon to retry (in seconds) if ACK didn't come in
#define RETRY_LIMIT  5           // Maximum number of times to retry
#define ACK_TIME     10          // Number of milliseconds to wait for an ack
#define NEEDNEWID    9999

void tools_init_rf24(byte myNodeID, byte network);

void tools_enable_adc();

void tools_disable_adc();

bool tools_ack(bool state);
bool tools_ack();

void get_nodeid (int startrangeid);

void looseSomeTimeInMinutes(byte minutes);

long tools_readVcc();

void tools_rfwrite(byte myNodeID, const void* ptr, uint8_t len);

byte waitForAck(byte myNodeID);

