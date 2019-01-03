#define MY_SIGNING_ATSHA204_PIN 17 // A3

#define MY_DEBUG
#define MY_CORE_ONLY // Not change

//#define MY_SIGNING_SOFT
#define MY_SIGNING_ATSHA204 // Hardware signing using ATSHA204A

// Enable and select radio type attached
#define MY_RADIO_RF24 // MySensor 2.3.1

#include <MySensors.h>
#include <Wire.h>
#include <Sodaq_SHT2x.h>

#define CONNECT_PIN  4
#define BAT_PIN      A0 
  
bool testSha204() {
  uint8_t rx_buffer[SHA204_RSP_SIZE_MAX];
  uint8_t ret_code;
  if (Serial) {
    Serial.print("SHA204: ");
  }
  atsha204_init(MY_SIGNING_ATSHA204_PIN);
  ret_code = atsha204_wakeup(rx_buffer);

  if (ret_code == SHA204_SUCCESS) {
    ret_code = atsha204_getSerialNumber(rx_buffer);
    if (ret_code != SHA204_SUCCESS) {
      if (Serial) {
        Serial.println(F("Failed to obtain device serial number. Response: "));
      }
      Serial.println(ret_code, HEX);
    } else {
      if (Serial) {
        Serial.print(F("OK (serial : "));
        for (int i = 0; i < 9; i++) {
          if (rx_buffer[i] < 0x10) {
            Serial.print('0'); // Because Serial.print does not 0-pad HEX
          }
          Serial.print(rx_buffer[i], HEX);
        }
        Serial.println(")");
      }
      return true;
    }
  } else {
    if (Serial) {
      Serial.println(F("Failed to wakeup SHA204"));
    }
  }
  return false;
} 

void setup() {
  int BatVal = 0;
  int buttonState = 0;

  Wire.begin();
  Serial.begin(115200);

  // Setup pins
  pinMode(CONNECT_PIN, INPUT_PULLUP);
  pinMode(BAT_PIN, INPUT); 

  // CPU
  Serial.println("Test CPU: OK");

  // NRF24/RFM69
  if (transportInit()) { // transportSanityCheck
    Serial.println("Radio: OK");
  } else {
    Serial.println("Radio: ERROR");
  }

  // ATASHA
  testSha204(); 

  // Sensor
  if (SHT2x.GetHumidity()==0){
    Serial.println("STH: Error");
  } else {
    Serial.println("STH: OK");
  }

  while (true) {
    unsigned long currentMillis = millis();     
    
    // Battery
    BatVal = analogRead(BAT_PIN);
    //TODO: Check value
    Serial.print("Battery: ");
    Serial.println(BatVal);

    // Button
    buttonState = digitalRead(CONNECT_PIN);
    Serial.print("Connect: ");
    Serial.println(buttonState);

    // Sensor
    Serial.print("SHT (%RH): ");
    Serial.print(SHT2x.GetHumidity());
    Serial.print("     SHT (C): ");
    Serial.println(SHT2x.GetTemperature()); 

    delay(1000);
  }
}

void loop() {

}
