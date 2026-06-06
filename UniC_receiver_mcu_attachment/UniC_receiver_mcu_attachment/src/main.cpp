#include <Arduino.h>
#include <esp_now.h>
#define RXD1 3
#define TXD1 1

#define LED2 19
#define LED1 18

struct receiver_command {
  int8_t x1,y1;
  int8_t x2,y2;
  int8_t p1;
  int8_t p2;
  int8_t sw1,sw2;
};
receiver_command command;
uint8_t prelude = 0xAA;

void extract_command(){
  command.x1 = Serial.read();
  command.y1 = Serial.read();
  command.x2 = Serial.read();
  command.y2 = Serial.read();
  
  command.p1 = Serial.read();
  command.p2 = Serial.read();

  command.sw1 = Serial.read();
  command.sw2 = Serial.read();
}

void print_command(){
  Serial.println(" ======= PKT =======");
  Serial.print("J1: ");
  Serial.print(command.x1);
  Serial.print(" - ");
  Serial.println(command.y1);

  Serial.print("J2: ");
  Serial.print(command.x2);
  Serial.print(" - ");
  Serial.println(command.y2);

  Serial.print("P1: ");
  Serial.print(command.p1);
  Serial.print(" P2: ");
  Serial.println(command.p2);

  Serial.print("S1: ");
  Serial.print(command.sw1);
  Serial.print(" S2: ");
  Serial.println(command.sw2);
}

void update_leds(){
  if (command.x1 !=0 || command.x2 !=0){
    digitalWrite(LED1, HIGH);
  }else{
    digitalWrite(LED1, LOW);
  }
  if(command.y1 !=0 || command.y2 !=0){
    digitalWrite(LED2, HIGH);
  }else{
    digitalWrite(LED2, LOW);
  }
}

void setup() {
    // USB Serial Monitor
    Serial.begin(115200);

    pinMode(LED1, OUTPUT); 
    pinMode(LED2, OUTPUT); 

    Serial.println("UART Receiver Started");
}

unsigned long print_period = 500;
void loop() {
    static uint8_t state = 0;
    static uint8_t idx = 0;
    static uint8_t buffer[sizeof(receiver_command)];
    unsigned long start_t = millis();

    while(true){
      if(Serial.available()){
        uint8_t b = Serial.read();

        if(b == prelude){
          extract_command();
          update_leds();
          if(millis() - start_t > print_period){
            print_command();
            start_t = millis();
          }
        }

      }

    }

}