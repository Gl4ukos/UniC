#ifndef PACKET_HPP
#define PACKET_HPP

#include <Arduino.h>

struct ControllerPacket {
    int8_t padding;
    uint32_t id;
    uint32_t timestamp;
    
    // perform a RRT (flag)
    bool check_RTT;

    //joystick 1
    int8_t x1;       // -128 to 127
    int8_t y1;       
    int8_t b1;

    //joystick 2
    int8_t x2;       
    int8_t y2;       
    int8_t b2;

    //potentiometers
    int8_t pot1;
    int8_t pot2;

    //switches
    int8_t sw1;
    int8_t sw2;

    ControllerPacket();
};


struct PingPacket{
    int8_t padding;
    uint32_t id;
    uint32_t timestamp;

    PingPacket();
};
#endif
