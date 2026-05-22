#include <Arduino.h>
#include <esp_now.h>
#include <Packet.hpp>
#include <WiFi.h>
#include "USB.h"
#include "USBHIDGamepad.h"
#include "USBHIDKeyboard.h"
#include "USBHIDMouse.h"

#define WHITE_LED_PIN 6
#define BLUE_LED_PIN 7

// USBHIDGamepad gamepad;
USBHIDMouse mouse;
int8_t mouse_x, mouse_y; 
int8_t sensitivity;
USBHIDKeyboard keyboard;
uint8_t controllerMAC [] = {0xE0, 0x8C, 0xFE, 0x60, 0xF6, 0x64};
ControllerPacket packet;
PingPacket pong;

enum ControllerMode{
    GAMEPAD,
    MOUSE_KEYBOARD
};

uint8_t loop_index = 0;

struct Led{
    uint8_t pin_no;
    bool active;
    uint32_t start_time;
    uint32_t blink_period;
    uint32_t on_t;
    uint32_t off_t;

    Led(uint8_t pin, uint32_t on_time, uint32_t off_time){
        pin_no = pin;
        on_t = on_time;
        off_t = off_time;
        pinMode(pin_no, OUTPUT);
        active = false;
    }

    public:

    void activate(){
        active = true;
        digitalWrite(pin_no, HIGH);
    }

    void deactivate(){
        active = false;
        digitalWrite(pin_no, LOW);
    }

    void toggle(){
        if(active == false){
            activate();
        }else{
            deactivate();
        }
    }
    
    void blink(){
        if(active == true){
            if(millis() - start_time > on_t){
                deactivate();
                start_time = millis();
            }
        }else{
            if(millis() - start_time > off_t){
                activate();
                start_time = millis();
            }
        }
    }

};

uint8_t scroll_threshold = 10;
uint8_t scroll_softcap = 20;
void update_scrolller(){
    uint8_t scroll_value = packet.pot1;

    if(scroll_value > 50 + scroll_threshold){
        if(scroll_value > 50 + scroll_threshold + scroll_softcap){
            mouse.move(0,0,1);
        }else{
            mouse.move(0,0,2);
        }
    }else if(scroll_value < 50 - scroll_threshold){
        if(scroll_value < 50 - scroll_threshold - scroll_softcap){
            mouse.move(0,0,-1);
        }else{
            mouse.move(0,0,-2);
        }
    }
}

void update_mouse() {
    int8_t dx,dy;
    int8_t sign;

    if(packet.x2 != 0){
        if(packet.x2 > 0){
            sign = 1;
        }else{
            sign = -1;
        }

        if (abs(packet.x2) < 80){
            dx = 1*sign;
        }else{
            dx = 3*sign;
        }
    }

    if(packet.y2 != 0){
        if(packet.y2 > 0){
            sign = 1;
        }else{
            sign = -1;
        }

        if (abs(packet.y2) < 80){
            dy = 1*sign;
        }else{
            dy = 3*sign;
        }
    }

    dx *= round(1 + (sensitivity/5) );
    dy *= round(1 + (sensitivity/5) );

    mouse.move(dx, dy);
}

bool last_b2 = 0;
bool last_b1 = 0;


void update_arrows() {
    if(packet.y1 < -50){
        keyboard.press(KEY_UP_ARROW);
    } else {
        keyboard.release(KEY_UP_ARROW);
    }

    if(packet.y1 > 50){
        keyboard.press(KEY_DOWN_ARROW);
    } else {
        keyboard.release(KEY_DOWN_ARROW);
    }

    if(packet.x1 > 50){
        keyboard.press(KEY_RIGHT_ARROW);
    } else {
        keyboard.release(KEY_RIGHT_ARROW);
    }

    if(packet.x1 < -50){
        keyboard.press(KEY_LEFT_ARROW);
    } else {
        keyboard.release(KEY_LEFT_ARROW);
    }
}

void update_keystrokes(){
    sensitivity = packet.pot2;

    // left mouse button
    bool current = packet.b2;
    // button just pressed
    if (current == 1 && last_b2 == 0) {
        mouse.press(MOUSE_LEFT);
    }
    // button just released
    if (current == 0 && last_b2 == 1) {
        mouse.release(MOUSE_LEFT);
    }
    last_b2 = current;

    // right mouse button
    current = packet.b1;
    // button just pressed
    if (current == 1 && last_b1 == 0) {
        mouse.press(MOUSE_RIGHT);
    }
    // button just released
    if (current == 0 && last_b1 == 1) {
        mouse.release(MOUSE_RIGHT);
    }
    last_b1 = current;

    update_arrows();
    

}

// void update_gamepad()
// {
//     int8_t lx = packet.x1;   // already -100..100?
//     int8_t ly = packet.y1;

//     int8_t rx = packet.x2;
//     int8_t ry = packet.y2;

//     int8_t z  = packet.pot1; // throttle
//     int8_t rz = packet.pot2;  // brake

//     uint8_t hat = 0; // neutral

//     uint32_t buttons = 0;

//     gamepad.send(lx, ly, z, rz, rx, ry, hat, buttons);
// }

Led white_led(WHITE_LED_PIN, 50, 4000);
Led blue_led(BLUE_LED_PIN, 25, 5000);
bool got_packet = false;

// Callback function
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    memcpy(&packet, incomingData, sizeof(packet));
    packet.x1 = -packet.x1;
    packet.y1 = -packet.y1;

    got_packet = true;
    // send back packet for RTT check
    if(packet.check_RTT == true){
        blue_led.blink();
        pong.timestamp = packet.timestamp;
        pong.id = packet.id;
        esp_now_send(controllerMAC, (uint8_t *)&pong, sizeof(pong));
        blue_led.blink();
    }
}

void print_MAC(){
    Serial.print("\nReceiver MAC: ");
    Serial.println(WiFi.macAddress());
}

void setup(){
    Serial.begin(921600);
    WiFi.mode(WIFI_STA); // station mode needed for ESP-NOW

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Register receive callback
    esp_now_register_recv_cb(OnDataRecv);

    // setting up usb gamepad interface
    USB.begin();
    // gamepad.begin();
    mouse.begin();
    keyboard.begin();

    // Configuring the controller esp as receiver
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, controllerMAC, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);


    for(int i=0; i<4; i++){
        blue_led.activate();
        white_led.deactivate();
        delay(100);
        blue_led.deactivate();
        white_led.activate();        
        delay(100);
        Serial.println("Booting...");
    }
    white_led.deactivate();
    blue_led.deactivate();
    Serial.println("Boot complete.");
    Serial.println("Listening for messages.");\
    print_MAC();
}

void loop() {
    if(got_packet == true){
        got_packet = false;
        if(packet.sw1 == false){
            // update_gamepad();
        }else{
            update_mouse();
            update_keystrokes();
            update_scrolller();
        }
    }
}
