//#ifdef INCLUDE 
  #define INCLUDE 

  #include "r307.h"

  /* These three were included to avoid error squiggles 
  for types like uint16_t, bool and functions like strcat()
  btw all these are in Arduino.h */
  #include <inttypes.h>
  #include <string.h>
  #include <stdbool.h>

  // struct to create UART-bourne packets
  struct Fingerprint_Packet_t{
    uint16_t start_code;          // "Wakeup" code for packet detection
    uint8_t address[4];           // 32-bit Fingerprint sensor address
    uint8_t type;                 // Type of packet
    uint16_t length;              // Equal to (size of data contents + sum)
    uint8_t* data;                // The raw buffer for packet payload
    uint16_t sum;                 // Equal to (type + length + data contents)
  };

  // starting value of a packet, assigned to start_code
  #define FINGERPRINT_HEADER 0XEF01

  // values indicating different packet types
  typedef enum{
    FINGERPRINT_PACKET_IDENTIFIER_COMMAND      = 0X1,                  // Command packet
    FINGERPRINT_PACKET_IDENTIFIER_DATA         = 0X2,                  // Data packet
    FINGERPRINT_PACKET_IDENTIFIER_ACKNOWLEDGE  = 0X7,                  // Acknowledge packet
    FINGERPRINT_PACKET_IDENTIFIER_END          = 0X8                   // End packet
  }EnumPacketIdentifier;

  // values for the different confirmation codes in the acknowledge packets
  typedef enum{
      FINGERPRINT_DATA_CONFIRMATION_SUCCESS = 0X0,                                        // Command execution is complete
      FINGERPRINT_DATA_CONFIRMATION_DATA_PACKAGE_RECEIVE_ERROR = 0X1,                     //! Error when receiving data package
      FINGERPRINT_DATA_CONFIRMATION_NO_FINGER = 0X2,                                      //! No finger on the sensor
      FINGERPRINT_DATA_CONFIRMATION_ENROLL_FINGER_FAIL = 0X3,                             //! Failed to enroll the finger
      FINGERPRINT_DATA_CONFIRMATION_IMAGE_DISORDERLY = 0X6,                               //! Failed to generate character file due to overly disorderly
                                                                                          //! fingerprint image
      FINGERPRINT_DATA_CONFIRMATION_FEATURE_FAIL = 0X7,                                   //!Failed to generate character file due to the lack of character point
                                                                                          //! or small fingerprint image
      FINGERPRINT_DATA_CONFIRMATION_NO_MATCH = 0X8,                                       //! Finger doesn't match
      FINGERPRINT_DATA_CONFIRMATION_SEARCH_FAIL = 0X9,                                    //! Failed to find matching finger
      FINGERPRINT_DATA_CONFIRMATION_ENROLL_MISMATCH = 0x0A,                 
      FINGERPRINT_DATA_CONFIRMATION_BAD_LOCATION = 0x0B,                  
      FINGERPRINT_DATA_CONFIRMATION_DB_RANGE_FAIL = 0x0C,                                 //! Error when reading template from library or invalid template
      FINGERPRINT_DATA_CONFIRMATION_UPLOAD_FEATURE_FAIL = 0x0D,                           //! Error when uploading template
      FINGERPRINT_DATA_CONFIRMATION_PACKET_RESPONSE_FAIL = 0x0E,                          //! Module failed to receive the following data package
      FINGERPRINT_DATA_CONFIRMATION_UPLOAD_IMAGE_DATA_PACKET_TRANSFER_FAIL = 0x0F,        //! Error when uploading image
      FINGERPRINT_DATA_CONFIRMATION_DELETEFAIL = 0x10,                                    //! Failed to delete the template
      FINGERPRINT_DATA_CONFIRMATION_DB_CLEAR_FAIL = 0x11,                                 //! Failed to clear finger library
      FINGERPRINT_DATA_CONFIRMATION_WRONG_PASSWORD = 0x13,                                //! Find whether the fingerprint passed or failed
      FINGERPRINT_DATA_CONFIRMATION_INVALID_IMAGE = 0x15,                                 //! Failed to generate image because of lac of valid primary image
      FINGERPRINT_DATA_CONFIRMATION_FLASH_ERRORR = 0x18,                                  //! Error when writing flash
      FINGERPRINT_DATA_CONFIRMATION_INVALID_REGISTER_NO = 0X1A,                           //! Invalid register number
      FINGERPRINT_DATA_CONFIRMATION_PORT_COMMUNICATION_FAILURE = 0X1D                     //! Invalid register number
  }EnumDataConfirmation;

  /* 
  fingerprint_module stores the different system parameter values that can be updated or read by the different functions. 
  
  in this translation unit only, hence preventing the user from changing critical data such as the module address.
  
  It is assigned the default values initially (as below)
  */
  static Fingerprint_Helper_t fingerprint_module = {
      {0xFF, 0XFF, 0XFF, 0XFF},
      {
          0x0000,
          0x0009,
          1000,
          FINGERPRINT_SECURITY_LEVEL_5,
          {0XFF, 0XFF, 0XFF, 0XFF},
          FINGERPRINT_PACKET_LENGTH_64_BYTE,
          FINGERPRINT_BAUDRATE_57600
      } 
  };

  // default value of mySerial is 0
  mySerial = NULL; 

  /* if mySerial == NULL, tell the user to assign a Serial object address to mySerial and return control back to the calling function */
  #define checkAddressZero() \
    if(mySerial == NULL){ \
      Serial.println("Error! Assign the address of a Serial Object to mySerial\n"); \
      return;
    }

  // size of an acknowledge packet with 1 byte of data
  #define FINGERPRINT_PACKET_SIZE_MINIMUM sizeof(struct Fingerprint_Packet_t) - (sizeof(uint8_t*) + sizeof(uint8_t))

  // For functions sending a command packet and receiving an acknowlege packet only
  #define RECEIVE_ACK_PACKET(packet_type, data_length, data_pointer) \
    /* Send the command packet */\
    packetSend(packet_type, data_length, data_pointer); \
    /* Receive the acknowledge packet */\
    struct Fingerprint_Packet_t ack_packet = packetReceive()
    

  extern "C"{    
    void setBaud(){
      mySerial->begin(fingerprint_module.system_parameter_store.baud_setting * FINGERPRINT_BAUD_RATE_BASE);
    }

    /* Sends UART-bourne packet to the sensor */
    static void packetSend(uint8_t packet_type, uint16_t data_length, uint8_t* data){
      /* do nothing if mySerial == NULL
         DO NOT add a SEMICOLON at the end */
      checkAddressZero()
      
      // construct packet to be trasmitted
      struct Fingerprint_Packet_t txPacket = {
        FINGERPRINT_HEADER, 
        {0xFF, 0xFF, 0xFF, 0xFF}, 
        packet_type,
        data_length + sizeof(txPacket.sum),
        data,
        packet_type + data_length
      };

      // final value of sum
      for (int i = 0; i < data_length; i++)
        txPacket.sum += *(data + i);

      // Transfer the header value with the high byte first
      mySerial->write(txPacket.start_code & 0xFF00);
      mySerial->write(txPacket.start_code & 0x00FF);

      // Transfer the address with the high byte first and low byte last
      mySerial->write(txPacket.address[0]);
      mySerial->write(txPacket.address[1]);
      mySerial->write(txPacket.address[2]);
      mySerial->write(txPacket.address[3]);

      // Transfer the packet ID 
      mySerial->write(txPacket.type);  

      // Transfer the package length with the high byte first
      mySerial->write(txPacket.length & 0xFF00);
      mySerial->write(txPacket.length & 0x00FF);

      // Transfer the data; not sure if it is the high byte first so I assumed low byte first
      for(int i = 0; i < data_length; i++)
        mySerial->write(txPacket.data[i]);

      // Transfer the sum with the high byte first
      mySerial->write(txPacket.sum & 0xFF00);
      mySerial->write(txPacket.sum & 0x00FF);
    }
    
    /* Receives UART-bourne packet from the sensor */
    static struct Fingerprint_Packet_t packetReceive(){
      /* do nothing if mySerial == NULL
         DO NOT add a SEMICOLON at the end */
      checkAddressZero()
  
      // wait until the packet sent is big enough to be read
      while (mySerial->available() < FINGERPRINT_PACKET_SIZE_MINIMUM){
        Serial.println("Waiting for data to be sent\n");
        // wait for 20 ms
        delay(20);
      }
      // construct packet to receive information sent
      struct Fingerprint_Packet_t rxPacket;
      
      // assign the first 2 bytes as the start code
      rxPacket_Packet.start_code = (((uint16_t)mySerial->read() << 8) | (uint16_t)mySerial->read());
      
      // assign the next 4 bytes as the address
      rxPacket.address[0] = mySerial->read();
      rxPacket.address[1] = mySerial->read();
      rxPacket.address[2] = mySerial->read();
      rxPacket.address[3] = mySerial->read();
      
      // assign the next byte as the type of packet
      rxPacket.type = mySerial->read();
      
      // assign the next 2 bytes as the length
      rxPacket.length = (((uint16_t)mySerial->read() << 8) | (uint16_t)mySerial->read());
      
      // data_buffer stores the data received
      static uint8_t data_buffer[rxPacket.length - sizeof(rxPacket.sum)]; 
      rxPacket.data = data_buffer;
      
      // assign the next no. of bytes = length of the data = txPacket.length - sizeof(txPacket.sum) to data_buffer
      for(int i = 0; i < rxPacket.length - sizeof(rxPacket.sum); i++)
        *(rxPacket.data + i) = mySerial->read();
      
      // assign the last 2 bytes as the sum
      rxPacket.sum = (((uint16_t)mySerial->read() << 8) | (uint16_t)mySerial->read());
      
      return rxPacket;
    }

    typedef enum{
      FINGERPRINT_COMMAND_PASSWORD_VERIFY = 0X13,
      FINGERPRINT_COMMAND_PASSWORD_SET = 0X12,
      FINGERPRINT_COMMAND_SYSTEM_PARAMETER_SET = 0XE,
      FINGERPRINT_COMMAND_SYSTEM_PARAMETER_READ = 0xF,
      FINGERPRINT_COMMAND_UART_PORT_CONTROL = 0X17, 
      FINGERPRINT_COMMAND_GET_RANDOM_CODE = 0X14,
      FINGERPRINT_COMMAND_SET_MODULE_ADDRESS = 0X15,
      FINGERPRINT_COMMAND_READ_VALID_TEMPLATE_NUM = 0X1D, 
      FINGERPRINT_COMMAND_FINGERPRINT_VERIFY = 0X32,
      FINGERPRINT_COMMAND_FINGERPRINT_AUTO_VERIFY = 0X34,
      FINGERPRINT_COMMAND_UPLOAD_IMAGE = 0XA,
      FINGERPRINT_COMMAND_DOWNLOAD_IMAGE = 0XB
    }EnumCommandCodes;

    // Send output to the terminal depending on the confirmation code received
    static bool handleReceivedData(uint8_t data_code, uint8_t* keywords){
      int buff_size = 0;
      while(*keywords++)
        buff_size++;
      
      uint8_t buffer[(sizeof("Success! ") - 1) + buff_size + sizeof('\n')] = "Success! ";
  
      switch(data_code){
        case FINGERPRINT_DATA_CONFIRMATION_SUCCESS:
          strcat(buffer, keywords);
          strcat(buffer, "\n");
          while(*buffer++)
            Serial.println(*buffer); 
          return 1;

        case FINGERPRINT_DATA_CONFIRMATION_DATA_PACKAGE_RECEIVE_ERROR:
          Serial.println("Error! Sensor Did Not Properly Receive Package.\n");  
          return 0;

        case FINGERPRINT_DATA_CONFIRMATION_UPLOAD_IMAGE_DATA_PACKET_TRANSFER_FAIL:
          Serial.println("Error! Sensor Failed to Upload the Data Package.\n");  
          return 0;
  
        case FINGERPRINT_DATA_CONFIRMATION_PORT_COMMUNICATION_FAILURE:
          Serial.println("Error! UART Port Communication Failure.\n");  
          return 0;

        case FINGERPRINT_DATA_CONFIRMATION_NO_FINGER:
          Serial.println("Error! Sensor Cannot Detect Finger.\n");  
          return 0;

        case FINGERPRINT_DATA_CONFIRMATION_ENROLL_FINGER_FAIL:
          Serial.println("Error! Sensor Failed to Detect Finger.\n");  
          return 0;

        case FINGERPRINT_DATA_CONFIRMATION_IMAGE_DISORDERLY: 
          Serial.println("Error! Fingerprint Is Overly Disorderly.\n");
          return 0;

        case FINGERPRINT_DATA_CONFIRMATION_FEATURE_FAIL:
          Serial.println("Error! Inefficient Fingerprint Data Collected.\n");
          return 0;

        case FINGERPRINT_DATA_CONFIRMATION_SEARCH_FAIL: 
          Serial.println("Error! Fingerprint Does Not Match.\n");
          return 0;

        case FINGERPRINT_DATA_CONFIRMATION_INVALID_REGISTER_NO: 
          Serial.println("Error! Wrong Register Number Inputted.\n");
          return 0;

        default:
          Serial.println("Error! Unusual Behaivior.\n");
          return 0;
      }
    }  
    
    void passwordVerify(uint8_t password[4]){
      // first byte is the instruction code, last 4 bytes are for the password
      uint8_t data[] = {
                          FINGERPRINT_COMMAND_PASSWORD_VERIFY,
                          password[0], 
                          password[1], 
                          password[2],
                          password[3]
                        };

      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data);

      // handle the data received in the acknowledge packet
      handleReceivedData(*ack_packet.data, "Correct Password.");
    }
    
    void passwordSet(uint8_t new_password[4]){
      // first byte is the instruction code, last 4 bytes are for the new password
      uint8_t data[] = {
                        FINGERPRINT_COMMAND_PASSWORD_SET,
                        new_password[0], 
                        new_password[1], 
                        new_password[2],
                        new_password[3]
                      };

      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data);
      
      // set the password in fingerprint_module to new_password if everything is ok
      if(handleReceivedData(*ack_packet.data, "Password Setting Complete.")){
        fingerprint_module.module_password[0] = new_password[0];
        fingerprint_module.module_password[1] = new_password[1];
        fingerprint_module.module_password[2] = new_password[2];
        fingerprint_module.module_password[3] = new_password[3];
      }  
    }

    void setModuleAddress(uint8_t new_address[4]){
      // The 1st byte is for the instruction code and last 4 bytes are for the new address
      uint8_t data[] = {
                          FINGERPRINT_COMMAND_SET_MODULE_ADDRESS,
                          new_address[0],
                          new_address[1],
                          new_address[2],
                          new_address[3]
                        }

      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data);

      // set the address in fingerprint_module to new_address if everything is ok
      if(handleReceivedData(*ack_packet.data, "Address Setting Complete.")){
        fingerprint_module.system_parameter_store.device_address[0] = new_address[0];
        fingerprint_module.system_parameter_store.device_address[1] = new_address[1];
        fingerprint_module.system_parameter_store.device_address[2] = new_address[2];
        fingerprint_module.system_parameter_store.device_address[3] = new_address[3];
      }
    }

    void systemBasicParameterSet(uint8_t parameter_number, uint8_t new_parameter){
      // 1st byte is for the instruction code, the last two bytes are for the inputs
      uint8_t data[] = {
                          FINGERPRINT_COMMAND_SYSTEM_PARAMETER_SET,
                          parameter_number,
                          new_parameter
                        };

      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data);
      
      // handle the data received in the acknowledge packet
      if(handleReceivedData(*ack_packet.data, "Parameter Setting Complete."))
        fingerprint_module.system_parameter_store.data_packet_size = new_parameter; 
    }

    void controlUARTPort(uint8_t port_state){
      // The 1st byte is for the instruction code and 2nd is for on/off byte
      uint8_t data[] = {
                          FINGERPRINT_COMMAND_UART_PORT_CONTROL,
                          port_state
                        };

      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data);
      
      // handle the data received
      handleReceivedData(*ack_packet.data, "Port Operation Complete.");
    }

    struct Fingerprint_Helper_t systemParameterRead(){
      // The 1 byte is for the instruction code
      uint8_t data = FINGERPRINT_COMMAND_SYSTEM_PARAMETER_READ;

      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data);
      
      // assign the data received to the helper variable
      if(handleReceivedData(*ack_packet.data, "Parameter Read Complete.")){
        fingerprint_module.system_parameter_store.status_register = ((uint16_t)ack_packet.data[0] << 8) | ack_packet.data[1];
        fingerprint_module.system_parameter_store.system_ID_code = ((uint16_t)ack_packet.data[2] << 8) | ack_packet.data[3];
        fingerprint_module.system_parameter_store.finger_lib_size = ((uint16_t)ack_packet.data[4] << 8) | ack_packet.data[5];
        fingerprint_module.system_parameter_store.security_level = ((uint16_t)ack_packet.data[6] << 8) | ack_packet.data[7];

        fingerprint_module.system_parameter_store.device_address[0] = ack_packet.data[8];
        fingerprint_module.system_parameter_store.device_address[1] = ack_packet.data[9];
        fingerprint_module.system_parameter_store.device_address[2] = ack_packet.data[10];
        fingerprint_module.system_parameter_store.device_address[3] = ack_packet.data[11];

        print_module.system_parameter_store.data_packet_size = ((uint16_t)ack_packet.data[12] << 8) | ack_packet.data[13];
        fingerprint_module.system_parameter_store.baud_setting = ((uint16_t)ack_packet.data[14] << 8) | ack_packet.data[15];
      }
      return fingerprint_module;
    }

    uint16_t templateNumberRead(){
      // The 1 byte is for the instruction code
      uint8_t data = FINGERPRINT_COMMAND_READ_VALID_TEMPLATE_NUM;

      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data);

      // return the template if it was generated
      if(handleReceivedData(*ack_packet.data, "Number Generation Success."))
        // return the template number
        return ((uint16_t)ack_packet.data[1] << 8) | ack_packet.data [2];
    }

    void fingerprintVerify(uint8_t capture_time, uint16_t start_bit_number,
                          uint16_t search_quantity, uint16_t (*result)[2]){
      /* The 1st byte is the instruction code 
        2nd byte is for capture_time
        3rd and 4th byte for start_bit_number
        last 2 bytes are for search_quantity
      */
      uint8_t data[] = {
                          FINGERPRINT_COMMAND_FINGERPRINT_VERIFY,
                          capture_time,
                          start_bit_number & 0XFF00,
                          start_bit_number & 0X00FF,
                          search_quantity & 0XFF00,
                          search_quantity & 0X00FF
                        };

      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_COMMAND_FINGERPRINT_VERIFY, sizeof(data), &data);

      // return the received data back to the user 
      if(handleReceivedData(*ack_packet.data, "Fingerprint Read Complete.")){
        result = uint8_t result_buff[4];
                              
        result[0] = (uint16_t)ack_packet.data[1] << 8;
        result[0] |= ack_packet.data[2];

        result[1] = (uint16_t)ack_packet.data[3] << 8;
        result[1] |= ack_packet.data[4];
      }
    }

    void fingerprintAutoVerify(uint16_t(*result)[2]){
      // The 1 byte is the instruction code 

      uint8_t data = FINGERPRINT_COMMAND_FINGERPRINT_AUTO_VERIFY;

      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_COMMAND_FINGERPRINT_VERIFY, sizeof(data), &data);

      // return the received data back to the user 
      if(handleReceivedData(*ack_packet.data, "Fingerprint Read Complete.")){
        result = uint8_t result_buff[4];
                              
        result[0] = (uint16_t)ack_packet.data[1] << 8;
        result[0] = ack_packet.data[2];

        result[1] = (uint16_t)ack_packet.data[3] << 8;
        result[1] = ack_packet.data[4];
      }
    }

    void imageCollect(){
      // The 1 byte is for the instruction code
      uint8_t data = FINGERPRINT_COMMAND_READ_VALID_TEMPLATE_NUM;

      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data);

      // handle the data received received
      handleReceivedData(*ack_packet.data, "Finger Collection Success.");
    }

    // For functions receiving a data packet after sending the command packet and receiving an acknowledge packet
    #define RECEIVE_DATA_PACKET(packet_type, data_length, data_pointer, keywords) \
      RECEIVE_ACK_PACKET(packet_type, data_length, data_pointer); \
      \
       /* Recieive the data packet only if all is well */\
      if (handleReceivedData(*ack_packet.data, keywords))\
        struct Fingerprint_Packet_t data_packet = packetReceive()

    void imageUpload(uint8_t* result){
      // The 1 byte is the instruction code
      uint8_t data = FINGERPRINT_COMMAND_UPLOAD_IMAGE;
      
      // get the acknowledge and data packets and handle the data as well
      RECEIVE_DATA_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data);
    }

    void imageUpload(uint8_t* result){
      // The 1 byte is the instruction code
      uint8_t data = FINGERPRINT_COMMAND_UPLOAD_IMAGE;
      
      // get the acknowledge and data packets and handle the data as well
      RECEIVE_DATA_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data);
    
      // get resultant image
      static uint8_t data[data_packet.length - sizeof(uint8_t)];
      result = data;
      for(int i = 0; i < sizeof(data); i++)
        *(result + i) = *(data_packet.data + i);
    }

    void imageUpload(uint8_t* result){
      // The 1 byte is the instruction code
      uint8_t data = FINGERPRINT_COMMAND_DOWNLOAD_IMAGE;
      
      // get the acknowledge and data packets and handle the data as well
      RECEIVE_DATA_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data);
      
      // get resultant image
      static uint8_t data[data_packet.length - sizeof(uint8_t)];
      result = data;
      for(int i = 0; i < sizeof(data); i++)
        *(result + i) = *(data_packet.data + i);
    }

    uint32_t generateRandomCode(){
      // The 1 byte is the instruction code
      uint8_t data = FINGERPRINT_COMMAND_GET_RANDOM_CODE;

      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data);

      // return the number if it was generated
      if (handleReceivedData(*ack_packet.data, "Number Generation Success."))
        // return the random number
        return (uint32_t)ack_packet.data[1] << 24 | (uint32_t)(ack_packet.data[2] << 16) | (uint32_t)ack_packet.data[3] << 8 | ack_packet.data[4];
    }
  };  
// #endif

/* functions: [ an indidcation of'*' means implemented above]
passwordVerify(); *
passwordSet(); *

systemParameterSet(); *
systemParameterRead(); *


generateRandomCode(); *

setModuleAddress(); *

controlUARTPort(); *


fingerprintVerify(); *
fingerprintAutoVerify(); *

imageCollect();
imageUpload();
imageDownload();
imageToCharFile();

characterFileUpload();
characterFileDownload();

templateMatchTwo();
templateNumberRead();

templateGenerate();
templateRead();
templateStore();
templateDelete();

fingerLibraryEmpty();
fingerLibrarySearch();

notepadWrite();
notepadRead();

 */
