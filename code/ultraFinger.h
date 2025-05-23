/*
 * ultraFinger – R307 Fingerprint Sensor Library
 * 
 * Copyright (c) 2025 John Terry
 * Licensed under the BSD 3-Clause License.
 * 
 * See the LICENSE file for full terms.
 */


#ifndef ULTRA_LIB
    #define ULTRA_LIB

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
        FINGERPRINT_CHAR_BUFFER_1 = 0X1,                    // ID for Character Buffer 1
        FINGERPRINT_CHAR_BUFFER_2 = 0X2                     // ID for Character Buffer 2
    }EnumBufferID;

    typedef enum{
        /* capture time can be any 8-bit value but these are the reference values */
        FINGERPRINT_CAPTURE_TIME_REFERENCE_1 = 0X20,                  // about 4.5s
        FINGERPRINT_CAPTURE_TIME_REFERENCE_2 = 0X25,                  // about 5.5s
        FINGERPRINT_CAPTURE_TIME_REFERENCE_3 = 0X30,                  // about 6.5s
    }EnumFingerprintVerify;

    typedef enum{
        FINGERPRINT_FLASH_PAGE_1 = 0,                                 // Page 1 number
        FINGERPRINT_FLASH_PAGE_2,                                     // Page 2 number
        FINGERPRINT_FLASH_PAGE_3,                                     // Page 3 number
        FINGERPRINT_FLASH_PAGE_4,                                     // Page 4 number
        FINGERPRINT_FLASH_PAGE_5,                                     // Page 5 number
        FINGERPRINT_FLASH_PAGE_6,                                     // Page 6 number
        FINGERPRINT_FLASH_PAGE_7,                                     // Page 7 number
        FINGERPRINT_FLASH_PAGE_8,                                     // Page 8 number
        FINGERPRINT_FLASH_PAGE_9,                                     // Page 9 number
        FINGERPRINT_FLASH_PAGE_10,                                    // Page 10 number
        FINGERPRINT_FLASH_PAGE_11,                                    // Page 11 number
        FINGERPRINT_FLASH_PAGE_12,                                    // Page 12 number
        FINGERPRINT_FLASH_PAGE_13,                                    // Page 13 number
        FINGERPRINT_FLASH_PAGE_14,                                    // Page 14 number
        FINGERPRINT_FLASH_PAGE_15,                                    // Page 15 number
        FINGERPRINT_FLASH_PAGE_16                                     // Page 16 number
    }EnumFlashPages;

    // Maximum number of templates that can be stored in the flash memory
    #define FINGERPRINT_FLASH_MAXIMUM_TEMPLATES 1000

    // Forward decalaration of SoftwareSerial handle type
    typedef class SoftwareSerial* SoftwareSerial_H;

    // global pointer to the SoftwareSerial object used by user
    extern SoftwareSerial_H *mySerial;

    // return type of systemParameterRead(): information on the current values of the system parameters
    typedef struct Fingerprint_Helper_t{
        uint8_t module_password[4];
    
        union{
            struct{
                uint16_t status_register;
                uint16_t system_ID_code;
                uint16_t finger_lib_size;
                uint16_t security_level;
                uint8_t device_address[4];
                uint16_t data_packet_size;
                uint16_t baud_setting; 
            }contents;

            struct{
                uint8_t padding : 0;
            }align[16];

        }system_parameter_store;
    }Fingerprint_Helper_t;

    extern "C"{
        /* Sets the baud rate of the upper computer */
        extern void setBaud();
    
        /* Verify the sensor's handshaking password */
        extern void passwordVerify(uint8_t password[4]);
    
        /* Change / Set the sensor's handshaking password */
        extern void passwordSet(uint8_t new_password[4]);
    
        /* Change either the baud rate, security level or packet length (Basic System Parameters) */
        extern void systemBasicParameterSet(uint8_t parameter_number, uint8_t new_parameter);
    
        /* Read the the current system parameter values */
        extern Fingerprint_Helper_t systemParameterRead();
    
        /* Set the device address value */
        extern void setModuleAddress(uint8_t new_address[4]);
    
        /* Turn the UART port on the sensor on / off */
        extern void controlUARTPort(uint8_t port_state);
    
        /* Match captured fingerprint with fingerprint library and return the result */
        extern void fingerprintVerify(uint8_t capture_time, uint16_t start_bit_number, uint16_t search_quantity, uint16_t(*result)[2]);
        
        /* Collect fingerprint, match with the fingerprint library and return the result */    
        extern void fingerprintAutoVerify(uint16_t(*result)[2]);
    
        /*  Receive an image from the image buffer, put its address in *image and size in image_size (upload an image) */
        extern void imageReceive(uint8_t* output_image, uint16_t* image_size);
    
        /* Generate character file from the original finger image in the image buffer and store it in either character buffer 1 or 2 */
        extern void imageToCharFile(uint8_t buff_number); 
    
        /* Receive a character file or template from the specified character buffer (upload a character file) */
        extern void characterFileReceive(uint8_t buff_number, uint16_t* data_size, uint8_t* result);
    
        /* Send character file or template to the specified character buffer (download a character file) */
        extern void characterFileSend(uint8_t buff_number, uint16_t* data_size, uint8_t* result);
    
        /* Read a valid template number */
        extern uint16_t templateNumberRead();   
    
        /* Generate a template from character files in the character buffers */
        extern void templateGenerate();
    
        /* Store the template of the specified character buffer in the designated location of the flash library */
        extern void templateStore(uint8_t buffer_number, uint16_t pageID);
    
        /* Load a template at the specified location (pageID) of the flash library to either character buffer 1 or 2 */
        extern void templateRead(uint8_t buff_number, uint16_t pageID);
    
        /* Delete n number of templates of the flash library starting from the specified location (pageID) */
        extern void templateDelete(uint8_t pageID, uint16_t n_templates);
    
        /* Carry out precise matching of templates, one in character buffer 1 and the other in character buffer 2 */
        extern void templateMatch();
    
        /* Delete all templates in the finger library */
        extern void fingerLibraryEmpty();
    
        /* Search the whole finger library for the template that matches the one in the specified character buffer */
        extern void fingerLibrarySearch(uint8_t buffer_number, uint16_t start_address, uint16_t page_num);
    
        /* Write data to the specified flash page */
        extern void notepadWrite(uint8_t page_num, uint8_t(*data)[32]);
        
        /* Read the specified flash page's content */
        extern void notepadRead(uint8_t page_num, uint8_t(*return_data)[32]);
    
        /* Generate a random number from the sensor and return it to the computer */
        extern uint32_t generateRandomCode();
    
    
        /* NB: Do not use; Under construction */
        extern void imageSend(uint8_t* input_image, uint32_t image_size);
            
    };
#endif
