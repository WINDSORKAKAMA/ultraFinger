#ifndef R307
#define R307

#include "Arduino.h"
#include <SoftwareSerial.h>


typedef enum{
    FINGERPRINT_PARAM_NO_BAUDRATE = 0X4,     // Baud Rate parameter number

    FINGERPRINT_BAUD_RATE_BASE = 9600,       // Multiply this and one of the values below 
                                             // to get the corresponding baud rate

    FINGERPRINT_BAUDRATE_9600 = 0X1,         // UART baud 9600
    FINGERPRINT_BAUDRATE_19200 = 0X2,        // UART baud 19200
    FINGERPRINT_BAUDRATE_28800 = 0X3,        // UART baud 28800
    FINGERPRINT_BAUDRATE_38400 = 0x4,        // UART baud 38400
    FINGERPRINT_BAUDRATE_48000 = 0x5,        // UART baud 48000
    FINGERPRINT_BAUDRATE_57600 = 0x6,        // UART baud 57600
    FINGERPRINT_BAUDRATE_67200 = 0x7,        // UART baud 67200
    FINGERPRINT_BAUDRATE_76800 = 0x8,        // UART baud 76800
    FINGERPRINT_BAUDRATE_86400 = 0x9,        // UART baud 86400
    FINGERPRINT_BAUDRATE_96000 = 0xA,        // UART baud 96000
    FINGERPRINT_BAUDRATE_105600 = 0xB,       // UART baud 105600
    FINGERPRINT_BAUDRATE_115200 = 0xC        // UART baud 115200
}EnumBaudRate;

typedef enum{
    FINGERPRINT_PARAM_NO_SECURITY_LEVEL = 0X5,           // Security Level parameter number
   
    FINGERPRINT_SECURITY_LEVEL_1 = 0X1,                  // Security Level 1
    FINGERPRINT_SECURITY_LEVEL_2 = 0X2,                  // Security Level 2
    FINGERPRINT_SECURITY_LEVEL_3 = 0X3,                  // Security Level 3
    FINGERPRINT_SECURITY_LEVEL_4 = 0X4,                  // Security Level 4
    FINGERPRINT_SECURITY_LEVEL_5 = 0X5                   // Security Level 5
}EnumSecurityLevel;

typedef enum{
    FINGERPRINT_PARAM_NO_PACKET_LENGTH = 0X6,          // Packet Length parameter number
   
    FINGERPRINT_PACKET_LENGTH_32_BYTE = 0X0,           // 32 byte packet
    FINGERPRINT_PACKET_LENGTH_64_BYTE = 0X1,           // 64 byte packet
    FINGERPRINT_PACKET_LENGTH_128_BYTE = 0X2,          // 128 byte packet
    FINGERPRINT_PACKET_LENGTH_256_BYTE = 0X3           // 256 byte packet
}EnumPacketLength;

typedef enum{
  FINGERPRINT_COMMAND_UART_PORT_OFF = 0X0,              // turn UART port off
  FINGERPRINT_COMMAND_UART_PORT_ON = 0X1                // turn UART port on
}EnumPortState;

typedef enum{
    // capture time can be any 8-bit value
    FINGERPRINT_CAPTURE_TIME_1 = 0X20,                  // about 4.5s
    FINGERPRINT_CAPTURE_TIME_2 = 0X25,                  // about 5.5s
    FINGERPRINT_CAPTURE_TIME_3 = 0X30,                  // about 6.5s
}EnumFingerprintVerify;

// forward declaration of SoftwareSerial
typedef struct SoftwareSerial SoftwareSerial;

// global pointer to the SoftwareSerial object used by user
extern SoftwareSerial *mySerial;

// global variable, fingerprint_module with fields containing the current system parameter values that can be changed by the different functions
extern struct Fingerprint_Helper_t{
    uint8_t module_password[4];
  
    union SystemParameter{
        struct Contents{
            uint16_t status_register;
            uint16_t system_ID_code;
            uint16_t finger_lib_size;
            uint16_t security_level;
            uint8_t device_address[4];
            uint16_t data_packet_size;
            uint16_t baud_setting; 
        };

        struct Align{
            uint8_t padding : 0;
        }align[16];

    }system_parameter_store;
}fingerprint_module;

extern "C"{
    /* Sets the baud rate of the computer */
    extern void setBaud(int initializer);

    /* Verify the sensor's handshaking password */
    extern void passwordVerify(uint8_t password[4]);

    /* Change / Set the sensor's handshaking password */
    extern void passwordSet(uint8_t new_password[4]);

    /* Change either the baud rate, security level or packet length (Basic System Parameters) */
    extern void systemBasicParameterSet(uint8_t parameter_number, uint8_t new_parameter);

    /* Read the current system parameter values into the system_parameter_store section of the fingerprint_module variable */
    extern void systemParameterRead();

    /* Turn the UART port on the sensor on/off */
    extern void controlUARTPort(uint8_t port_state);

    /* Generate a random number from the sensor and return it to the computer */
    extern uint32_t generateRandomCode();

    /* Set the device address value */
    extern void setModuleAddress(uint8_t new_address[4]);

    /* Read a valid template number */
    extern uint16_t templateNumberRead();    
};

#endif
