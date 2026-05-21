#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Display.hpp>
#include <Packet.hpp>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
unsigned long start = millis();

struct PINS{
  int SWITCH_1 = 16;
  int SWITCH_2 = 17;

  int STICK_2_Y = 34;
  int STICK_2_X = 35;

  int STICK_1_Y = 33;
  int STICK_1_X = 32;

  int POT1 = 39;
  int POT2 = 36;

  int STICK_1_BUTTON = 19;
  int STICK_2_BUTTON = 18;

  int BUTTONS[8] = {18, 19, 13, 12, 4, 2, 15, 23};

  int SDA = 21;
  int SCL = 22;

  int TX = 1;
  int RX = 3;
};
PINS Pins;

int POT_MAX = 3000;
int STICK_1_X_MEAN = 0;
int STICK_1_Y_MEAN = 0;
int STICK_2_X_MEAN = 0;
int STICK_2_Y_MEAN = 0;
int STICK_MAX = 3000;
int STICK_HALF = STICK_MAX/2;
float DEADZONE = STICK_MAX*0.1;

uint8_t RTT_check_period = 200 ; // ms
uint32_t ACKS_expected = 0 ;
uint32_t last_ping = millis(); //ms 
u_int32_t signal_latency = 100;
uint8_t receiverMAC[] = {0x3C, 0x0F, 0x02, 0xE4, 0x76, 0xD4};
PingPacket pong;
uint32_t last_ack_time = millis();


void read_switches();
void read_joysticks();
void read_potentiometers();
void read_buttons();
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
Display_Interface Display;
ControllerPacket packet;
bool updated = false;
uint32_t last_display = millis();
uint8_t display_period = 20; //ms


uint32_t clip_pot_value(uint32_t pot_val){
  if(pot_val > POT_MAX){
    return POT_MAX;
  }else if(pot_val < 0){
    return 0;
  }else{
    return pot_val;
  }
}

uint32_t clip_joystick_value(uint32_t j_val){
  if(j_val > STICK_MAX){
    return STICK_MAX;
  }else if(j_val < 0){
    return 0;
  }else{
    return j_val;
  }
}

void print_packet(){
  delay(10);
  // Serial.print(" --- PACKET --- \n");
  Serial.print("J1: ");
  Serial.print(packet.x1);
  Serial.print(" - ");
  Serial.println(packet.y1);

  // Serial.print("  J2: ");
  // Serial.print(analogRead(Pins.STICK_2_X));
  // Serial.print(" - ");
  // Serial.println(analogRead(Pins.STICK_2_Y));

  // Serial.print("P1: ");
  // Serial.print(clip_pot_value(analogRead(Pins.POT1)));
  // Serial.print("  P2: ");
  // Serial.println(clip_pot_value(analogRead(Pins.POT2)));
  
  // Serial.print(digitalRead(Pins.STICK_1_BUTTON));
  // Serial.print(" - ");
  // Serial.println(digitalRead(Pins.STICK_2_BUTTON));

  // Serial.print("S1: "); 
  // Serial.print(packet.sw1);

  // Serial.print("  S2: ");
  // Serial.println(packet.sw2);
  
}

// Callback function
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    memcpy(&pong, incomingData, sizeof(pong));
    signal_latency = ((millis() - pong.timestamp)/2)*0.25 + signal_latency*0.75;
    if (signal_latency > 100){
      signal_latency = 100;
    } 
    last_ack_time = millis();
    if(ACKS_expected > 0){
      ACKS_expected -=1;
    }
}


void setup() {

  // Setting up display
  Display.setup_display();

  // Setting up Serial
  Serial.begin(921600);
  
  // Setting up Wifi for ESP-NOW
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA); // station mode needed for ESP-NOW

  // Setting up Pins
  pinMode(Pins.SWITCH_1, INPUT_PULLUP); 
  pinMode(Pins.SWITCH_2, INPUT_PULLUP);

  pinMode(Pins.STICK_1_BUTTON, INPUT_PULLUP);
  pinMode(Pins.STICK_1_X, INPUT);
  pinMode(Pins.STICK_1_Y, INPUT);

  pinMode(Pins.STICK_2_BUTTON, INPUT_PULLUP);
  pinMode(Pins.STICK_2_X, INPUT);
  pinMode(Pins.STICK_2_Y, INPUT);

  pinMode(Pins.POT1, INPUT);
  pinMode(Pins.POT2, INPUT);


  // Calibrating Joysticks
  int reps = 200;
  for(int i=0; i<reps; i++){
    STICK_1_X_MEAN += analogRead(Pins.STICK_1_X)/reps;
    STICK_1_Y_MEAN += analogRead(Pins.STICK_1_Y)/reps;
    STICK_2_X_MEAN += analogRead(Pins.STICK_2_X)/reps;
    STICK_2_Y_MEAN += analogRead(Pins.STICK_2_Y)/reps;
  }
  Display.display.println("Joysticks Calibrated.");
  Display.display.display();

  // Connecting to Receiver ESP
  esp_now_peer_info_t peerInfo = {}; // Add receiver as peer
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_init() != ESP_OK) {
      Display.display.println("ESP-NOW init failed!");
      return;
  }

  // Register receive callback
  esp_now_register_recv_cb(OnDataRecv);

  esp_now_add_peer(&peerInfo);
  Display.display.println("ESP-NOW initialized.");
  Display.display.display();

  delay(100);
  Display.display.println("Sender MAC: ");
  Display.display.println(WiFi.macAddress());
  Display.display.display();

  Display.display.print("\nSETUP COMPLETE");
  Display.display.display();
  delay(300);
  
  Display.display.print(".");
  Display.display.display();
  delay(500);

  Display.display.print(".");
  Display.display.display();
  delay(500);

  Display.display.println(".");
  Display.display.display();
  delay(500);
}


void loop() {
  read_switches();
  read_joysticks();
  read_potentiometers();
  read_buttons();
  if(millis() - last_display >= display_period){
    Display.update_display(signal_latency, millis() - start, &ACKS_expected);
    last_display = millis();
  }
  start = millis();
  packet.timestamp = millis();
  if(packet.id >= 999999){
    packet.id = 1;
  }else{
    packet.id += 1;
  }

  if (millis() - last_ping >= RTT_check_period){
    packet.check_RTT = true;
    last_ping = millis();
    esp_now_send(receiverMAC, (uint8_t *)&packet, sizeof(packet));
    ACKS_expected +=1;
    packet.check_RTT = false;
  }else{
    if(updated == true){
      Serial.println("Sent");
      esp_now_send(receiverMAC, (uint8_t *)&packet, sizeof(packet));
      updated = false;
    }
  }
  
  if (millis() - last_ack_time > 4 * RTT_check_period) {
      signal_latency = 100; // SIGNAL LOST
  }

  // print_packet();
  delay(20);
}

int8_t prev_sw1, prev_sw2;
void read_switches(){
  packet.sw1 = digitalRead(Pins.SWITCH_1);  
  Display.activity.s1 = packet.sw1;
  if(prev_sw1 != packet.sw1){
    updated = true;
    prev_sw1 = packet.sw1;
  }

  packet.sw2 = digitalRead(Pins.SWITCH_2);
  Display.activity.s2 = packet.sw2;
  if(prev_sw2 != packet.sw2){
    updated = true;
    prev_sw2 = packet.sw2;
  }
}

float pot1_filtered = 0;
float pot2_filtered = 0;

void read_potentiometers(){
  int raw = clip_pot_value(analogRead(Pins.POT1));
  float normalized = (raw * 100.0f) / POT_MAX;
  pot1_filtered = pot1_filtered * 0.7f + normalized * 0.3f;
  int new_pot1 = (int)round(pot1_filtered);
  if (new_pot1 != packet.pot1) {
      updated = true;
      packet.pot1 = new_pot1;
      Display.activity.p1 = packet.pot1;
  }

  raw = clip_pot_value(analogRead(Pins.POT2));
  normalized = 100- (raw * 100.0f) / POT_MAX;
  pot2_filtered = pot2_filtered * 0.7f + normalized * 0.3f;
  int new_pot2 = (int)round(pot2_filtered);
  if (new_pot2 != packet.pot2) {
      updated = true;
      packet.pot2 = new_pot2;
      Display.activity.p2 = packet.pot2;
  }
}

void read_joysticks(){
  float x1_centered = ((float)clip_joystick_value(analogRead(Pins.STICK_1_X)) - STICK_1_X_MEAN);
  float y1_centered = ((float)clip_joystick_value(analogRead(Pins.STICK_1_Y)) - STICK_1_Y_MEAN);
  float x2_centered = ((float)clip_joystick_value(analogRead(Pins.STICK_2_X)) - STICK_2_X_MEAN);
  float y2_centered = ((float)clip_joystick_value(analogRead(Pins.STICK_2_Y)) - STICK_2_Y_MEAN);  

  uint8_t temp;

  Display.activity.j1 = Display_Interface::INACTIVE;
  if(packet.b1 != 0){
    Display.activity.j1 = Display_Interface::PRESS;
    updated = true;
  }
  else if (abs(x1_centered) <= DEADZONE){
    if(packet.x1 != 0){
      updated = true;
    }
    packet.x1 = (int8_t)(0);
  }else if (x1_centered >= 0){ // LEFT
    updated = true;
    packet.x1 = (int8_t)((x1_centered/(STICK_MAX-STICK_1_X_MEAN))*100);
    Display.activity.j1 = Display_Interface::LEFT;
  }else{  //RIGHT
    updated = true;
    packet.x1 = (int8_t)((x1_centered/(STICK_1_X_MEAN))*100);
    Display.activity.j1 = Display_Interface::RIGHT;
  }
  if(abs(y1_centered) <= DEADZONE){
    packet.y1 = (int8_t)(0);
  }else if(y1_centered >= 0){
    updated = true;
    packet.y1 = (int8_t)((y1_centered/(STICK_MAX-STICK_1_Y_MEAN))*100);
    Display.activity.j1 = (Display_Interface::Joystick_state)(Display.activity.j1 | Display_Interface::UP); // UP
  }else{
    updated = true;
    packet.y1 = (int8_t)((y1_centered/(STICK_1_Y_MEAN))*100);
    Display.activity.j1 = (Display_Interface::Joystick_state)(Display.activity.j1 | Display_Interface::DOWN); // DOWN
  }

  Display.activity.j2 = Display_Interface::INACTIVE;
  if(packet.b2 != 0){
    updated = true;
    Display.activity.j2 = Display_Interface::PRESS;
  }
  else if (abs(x2_centered) <= DEADZONE){
    if(packet.x2 != 0){
      updated = true;
    }
    packet.x2 = (int8_t)(0);
  }else if (x2_centered >= 0){
    updated = true;
    packet.x2 = (int8_t)((x2_centered/(STICK_MAX-STICK_2_X_MEAN))*100);
    Display.activity.j2 = Display_Interface::RIGHT; // RIGHT
  }else{
    updated = true;
    packet.x2 = (int8_t)((x2_centered/(STICK_2_X_MEAN))*100);
    Display.activity.j2 = Display_Interface::LEFT; // LEFT
  }
  if(abs(y2_centered) <= DEADZONE){
    packet.y2 = (int8_t)(0);
  }else if(y2_centered >= 0){
    updated = true;
    packet.y2 = (int8_t)((y2_centered/(STICK_MAX-STICK_2_Y_MEAN))*100);
    Display.activity.j2 = (Display_Interface::Joystick_state)(Display.activity.j2 | Display_Interface::DOWN); // DOWN
  }else{
    updated = true;
    packet.y2 = (int8_t)((y2_centered/(STICK_2_Y_MEAN))*100);
    Display.activity.j2 = (Display_Interface::Joystick_state)(Display.activity.j2 | Display_Interface::UP); // UP
  }

}

int8_t prev_b1, prev_b2;
void read_buttons(){
  packet.b1 = (digitalRead(Pins.STICK_1_BUTTON) == 0);
  if(packet.b1 != prev_b1){
    updated = true;
    prev_b1 = packet.b1;
  }
  packet.b2 = (digitalRead(Pins.STICK_2_BUTTON) == 0);
  if(packet.b2 != prev_b2){
    updated = true;
    prev_b2 = packet.b2;
  }
}
