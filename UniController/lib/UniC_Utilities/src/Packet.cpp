#include <Packet.hpp>

ControllerPacket::ControllerPacket(){
        padding = 0xAA;
        timestamp =0;
        id = 0;
        check_RTT = false;
}

PingPacket::PingPacket(){
        padding = 0xAA;
        timestamp = 0;
        id = 0;
}