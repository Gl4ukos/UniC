#include <windows.h>
#include <iostream>
#include <string>
#include <chrono>


struct ControllerPacket {
    int8_t padding = 0xAA;
    int8_t type;
    int8_t x;
    int8_t y;
};
ControllerPacket packet;
const size_t PACKET_SIZE =  sizeof(ControllerPacket);


void print_byte(int8_t byte){
    for (int b=7; b>=0; b--){
        std::cout<<((byte>>b)&1);
    }
    std::cout<<" ";
}

void tapKey(WORD vkCode) {
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vkCode;      // virtual key code
    input.ki.dwFlags = 0;       // 0 for key down
    SendInput(1, &input, sizeof(INPUT));

    // key release
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

void keyDown(WORD vk) {
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk;
    input.ki.dwFlags = 0;
    SendInput(1, &input, sizeof(INPUT));
}

void keyUp(WORD vk) {
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk;
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

void releaseAllKeys() {
    for (int k = 0x01; k <= 0xFE; k++) {
        INPUT input = {0};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = k;
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
    }
}
bool killSwitchPressed() {
    // 0x51 is the virtual key code for 'Q'
    return (GetAsyncKeyState(0x51) & 0x8000) != 0;
}



bool rightPressed = false;
bool leftPressed = false;

void handle_packet(int8_t* data, size_t size){
    int8_t type = data[0];
    int8_t x = data[1];
    int8_t y = data[2];

    if(static_cast<int>(x)>10){
        tapKey(VK_UP);
    }else if(static_cast<int>(x)<-10){
        tapKey(VK_DOWN);
    }
    if(static_cast<int>(y)>10){
        tapKey(VK_LEFT);
    }else if(static_cast<int>(y)<-10){
        tapKey(VK_RIGHT);
    }
    
}

int main() {
    HANDLE hSerial = CreateFile(
        "COM8",                // replace with your ESP32 COM port
        GENERIC_READ,
        0,
        0,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        0
    );

    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening serial port\n";
        return 1;
    }

    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error getting state\n";
        return 1;
    }

    dcbSerialParams.BaudRate = 921600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity   = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error setting state\n";
        return 1;
    }

    std::cout<<"PACKET SIZE: "<<PACKET_SIZE<<"\n";

    int8_t byte;
    int8_t byte_array[PACKET_SIZE-1];
    int8_t header = 0xAA;
    size_t bufferPos = 0;
    DWORD bytesRead = 0;

    while (true) {
        // 1. Check for kill switch
        if (killSwitchPressed()) {
            releaseAllKeys();
            std::cout<<"Kill switch!\n";
            break;  // exit loop or set a global stop flag
        }
        
        ReadFile(hSerial, &byte, sizeof(byte), &bytesRead, NULL);
        if(byte == header){ //if header is found then read the next PACKET_SIZE-1 bytes
            ReadFile(hSerial, byte_array, PACKET_SIZE-1, &bytesRead, NULL);
            handle_packet(byte_array, PACKET_SIZE-1);
        }

    }
    CloseHandle(hSerial);
    return 0;
}