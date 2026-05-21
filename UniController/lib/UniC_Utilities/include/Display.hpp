#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <Packet.hpp>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

struct Display_Interface{
    public:
    Adafruit_SSD1306 display;
    bool update;
    const unsigned char* joystick_sprites[10];

    enum Joystick_state {
        INACTIVE  = 0,
        UP    = 1 << 0,
        DOWN  = 1 << 1,
        LEFT  = 1 << 2,
        RIGHT = 1 << 3,
        PRESS = 1 << 4
    };
    const int stateToIndex[32] = {
        0,2,3,0,4,6,9,0,5,7,8,
        0,0,0,0,0,1,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0
    };

    struct Activity{
        Joystick_state j1;
        Joystick_state j2;
        uint8_t p1;
        uint8_t p2;
        bool s1;
        bool s2;
    };
    Activity activity;
   


    Display_Interface();
    int setup_display();
    void update_display(u_int32_t signal_latency, uint32_t loop_latency, u_int32_t *packet_loss);
};


#endif