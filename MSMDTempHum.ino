#define MY_NODE_ID AUTO
#define MY_PARENT_NODE_ID AUTO 

#define Min_Batt 595
#define Max_Batt 850
  
#define SKETCH_NAME "MSMD Temp_Hum"
#define SKETCH_MAJOR_VER "0"
#define SKETCH_MINOR_VER "1"

//===
#define MY_DEBUG 
#define DEBUG

#define MY_RADIO_RF24
#define MY_RF24_PA_LEVEL RF24_PA_MAX

#include <MySensors.h>
#include <Wire.h>
#include <Sodaq_SHT2x.h>
#include <avr/wdt.h>

//==== NODE_SENSORS
#define SensTemp      1   // V_TEMP
#define SensHum       2   // V_NUM

#define SensUID       200
#define SensNewID     201     // V_VAR1
#define SensSleepTime 202

#define cTimeCell     1   // Save sleep time value cell

#define BUTTON_PIN    4 
#define BATTERY_PIN   A0

#define cFirstSmartSleepCnt 2    // Start smartSleep count
#define cDefSleepTimeMin    15   // Default sleep time
//unsigned long cLongSmartSleepCnt  
#define cNoneVal 255

MyMessage msgTemp(SensTemp, V_TEMP);
MyMessage msgHum(SensHum,   V_HUM);
MyMessage msgSleepTime(SensSleepTime, V_VAR1);

unsigned long SLEEP_TIME;
byte StartSmartSleepCnt = cFirstSmartSleepCnt;
int DaySmartCnt = 0; // Day smartSleep counter
int DaySmartVal = 0; // Day smartSleep value

// Last values
byte  LastBattery = cNoneVal;
float LastTemp = cNoneVal;
float LastHum = cNoneVal;

void(* resetFunc) (void) = 0;

void before() {
  pinMode(BATTERY_PIN, INPUT); 
}

void setup(){
  Wire.begin();
  
  analogReference(INTERNAL);

  // Load sleep time
  byte L = loadState(cTimeCell);
  SetTime(L);
  
  wdt_enable(WDTO_8S);
  send(msgSleepTime.set(L)); // Send sleep time
}

void presentation(){
  sendSketchInfo(SKETCH_NAME, SKETCH_MAJOR_VER"."SKETCH_MINOR_VER);

  present(SensUID,        S_CUSTOM,   "ac6bc098fbd63afd");
  present(SensNewID,      S_CUSTOM,   "Node new ID");
  present(SensSleepTime,  S_CUSTOM,   "Time sleep_minute");

  present(SensTemp, S_TEMP, "Temperature");
  present(SensHum,  S_HUM,  "Humidity");
}

void loop(){
  wdt_reset();
  
  SendDevInfo();

  // Day smartSleep
  if (DaySmartVal != 0){
    DaySmartCnt--;
    if (DaySmartCnt <= 0) {
      LastTemp = cNoneVal;
      LastHum = cNoneVal;
      LastBattery = cNoneVal;
      
      DaySmartCnt = DaySmartVal;
      smartSleep(SLEEP_TIME);
      return;
    }
  }

  // Start smartSleep
  if (StartSmartSleepCnt > 0) {
    StartSmartSleepCnt--;
    smartSleep(SLEEP_TIME);
    return;
  }
  
  sleep(SLEEP_TIME);  
}

void receive(const MyMessage &message) {
  if (message.isAck()) {
     return;
  }

  byte L;
  byte Node_ID;

  switch (message.sensor) {
  // NodeID
  case SensNewID:
    Node_ID = message.getByte();
    hwWriteConfig(EEPROM_NODE_ID_ADDRESS, Node_ID);
    resetFunc();
    break;
    
  // Set time
  case SensSleepTime:
    L = message.getByte();
    SetTime(L);
    saveState(cTimeCell, L);

    break;
  }     

  // Reset counter for wait next config messages
  StartSmartSleepCnt = cFirstSmartSleepCnt;
}

float RoundEx(float val){
  return round( val / 0.25 ) * 0.25;
}

void SendDevInfo(){
  float Temp = RoundEx(SHT2x.GetTemperature());
  float Hum  = RoundEx(SHT2x.GetHumidity());
  
  // battery
  byte BattP = constrain(map(analogRead(BATTERY_PIN), Min_Batt, Max_Batt, 0, 100), 0, 100);
  
  // Temp
  if (LastTemp != Temp) {
    if (send(msgTemp.set(Temp, 1), true)) {
      LastTemp = Temp;
    } else {
      LastTemp = cNoneVal;
    }
    wait(1);
  }
  
  // Humidity
  if (LastHum != Hum) {
    if (send(msgHum.set(Hum, 0), true)) {
      LastHum = Hum;
    } else {
      LastHum = cNoneVal;
    }
    wait(1);
  }
  
  // Battery
  if (LastBattery != BattP){
    sendBatteryLevel(BattP);
    LastBattery = BattP;
  }
}

void SetTime(byte L){
  if ((L == 0x00) || (L == 0xFF)) {
    L = cDefSleepTimeMin;
  }
  
  SLEEP_TIME = L*60*1000ul;
  DaySmartVal = 1440 / L;
  DaySmartCnt = DaySmartVal; 
}
