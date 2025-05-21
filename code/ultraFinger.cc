//#ifdef INCLUDE
  #define INCLUDE
  #include "ultraFinger.h"
  #include <stdlib.h>

  // The values indicating different packet types
  typedef enum{
    FINGERPRINT_PACKET_IDENTIFIER_COMMAND      = 0X1,                  // Command packet
    FINGERPRINT_PACKET_IDENTIFIER_DATA         = 0X2,                  // Data packet
    FINGERPRINT_PACKET_IDENTIFIER_ACKNOWLEDGE  = 0X7,                  // Acknowledge packet
    FINGERPRINT_PACKET_IDENTIFIER_END          = 0X8                   // End packet
  }EnumPacketIdentifier;

  /* 
    *** Command codes for the different functions. ****

    A command code is the first byte of data in a connand packet. 
  */
  typedef enum{
    FINGERPRINT_COMMAND_COLLECT_FINGERPRINT_IMAGE = 0X1,
    FINGERPRINT_COMMAND_GENERATE_CHAR_FILE_FROM_IMAGE = 0X2,
    FINGERPRINT_COMMAND_MATCH_TWO_TEMPLATES = 0X3,
    FINGERPRINT_COMMAND_SEARCH_FINGER_LIBRARY = 0X4,
    FINGERPRINT_COMMAND_GENERATE_TEMPLATE = 0X5,
    FINGERPRINT_COMMAND_STORE_TEMPLATE = 0X6,
    FINGERPRINT_COMMAND_READ_TEMPLATE = 0X7,
    FINGERPRINT_COMMAND_UPLOAD_CHAR_FILE = 0X8,
    FINGERPRINT_COMMAND_DOWNLOAD_CHAR_FILE = 0X9,
    FINGERPRINT_COMMAND_UPLOAD_IMAGE = 0XA,
    FINGERPRINT_COMMAND_DOWNLOAD_IMAGE = 0XB,
    FINGERPRINT_COMMAND_DELETE_TEMPLATE = 0XC,
    FINGERPRINT_COMMAND_EMPTY_FINGER_LIBRARY = 0XD,
    FINGERPRINT_COMMAND_SET_SYSTEM_PARAMETER = 0XE,
    FINGERPRINT_COMMAND_READ_SYSTEM_PARAMETER = 0xF, 
    FINGERPRINT_COMMAND_SET_PASSWORD = 0X12, 
    FINGERPRINT_COMMAND_VERIFY_PASSWORD = 0X13,
    FINGERPRINT_COMMAND_GET_RANDOM_CODE = 0X14,
    FINGERPRINT_COMMAND_SET_MODULE_ADDRESS = 0X15,
    FINGERPRINT_COMMAND_CONTROL_UART_PORT = 0X17,
    FINGERPRINT_COMMAND_WRITE_TO_NOTEPAD = 0X18,
    FINGERPRINT_COMMAND_READ_FROM_NOTEPAD = 0X19,
    FINGERPRINT_COMMAND_READ_VALID_TEMPLATE_NUM = 0X1D,
    FINGERPRINT_COMMAND_VERIFY_FINGERPRINT = 0X32,
    FINGERPRINT_COMMAND_AUTO_VERIFY_FINGERPRINT = 0X34
  }EnumCommandCodes;

    /* 
    *** Different confirmation codes in the different acknowledge packets.  ***

    A confirmation code is the first byte of an acknowledge packet.
  */
  typedef enum{
    FINGERPRINT_DATA_CONFIRMATION_SUCCESS = 0X0,
    FINGERPRINT_DATA_CONFIRMATION_DATA_PACKAGE_RECEIVE_ERROR = 0X1,
    FINGERPRINT_DATA_CONFIRMATION_NO_FINGER = 0X2,
    FINGERPRINT_DATA_CONFIRMATION_COLLECT_FINGER_FAIL = 0X3,
    FINGERPRINT_DATA_CONFIRMATION_DISORDELY_FINGERPRINT_IMAGE = 0X6,
    FINGERPRINT_DATA_CONFIRMATION_FINGERPRINT_FEATURE_ERROR = 0X7,
    FINGERPRINT_DATA_CONFIRMATION_TEMPLATES_NO_MATCH = 0X8,
    FINGERPRINT_DATA_CONFIRMATION_FINGERPRINT_IDENTIFICTAION_ERROR = 0X9,
    FINGERPRINT_DATA_CONFIRMATION_CHARACTER_FILE_COMBINE_FAIL = 0XA,
    FINGERPRINT_DATA_CONFIRMATION_INVALID_ADDRESSING_PAGE_ID = 0XB,
    FINGERPRINT_DATA_CONFIRMATION_TEMPLATE_READ_ERROR = 0XC,
    FINGERPRINT_DATA_CONFIRMATION_TEMPLATE_UPLOAD_ERROR = 0XD,
    FINGERPRINT_DATA_CONFIRMATION_DATA_PACKET_TRANSFER_FAIL_DOWNLOAD = 0XE,
    FINGERPRINT_DATA_CONFIRMATION_DATA_PACKET_TRANSFER_FAIL_UPLOAD = 0XF,
    FINGERPRINT_DATA_CONFIRMATION_TEMPLATE_DELETE_FAIL = 0X10,
    FINGERPRINT_DATA_CONFIRMATION_WRONG_PASSWORD = 0X13,
    FINGERPRINT_DATA_CONFIRMATION_NO_VALID_PRIMARY_IMAGE = 0x15,
    FINGERPRINT_DATA_CONFIRMATION_FLASH_WRITING_ERROR = 0X18,
    FINGERPRINT_DATA_CONFIRMATION_INVALID_REGISTER_NUMBER = 0X1A,
    FINGERPRINT_DATA_CONFIRMATION_PORT_OPERATION_FAIL = 0X1D
  }EnumDataConfirmation;

  // The "Wakeup" code for packet detection
  #define FINGERPRINT_HEADER 0XEF01

  // struct to create UART-bourne packets
  struct Fingerprint_Packet_t{
    uint16_t start_code;          // equal to FINGERPRINT_HEADER
    uint8_t address[4];           // 32-bit Fingerprint sensor address
    uint8_t type;                 // Type of packet
    uint16_t length;              // Equal to (size of data contents + sum)
    uint8_t* data;                // The raw buffer for packet payload
    uint16_t sum;                 // Equal to (type + length + data contents)
  };

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
                                                        \
    /* Receive the acknowledge packet */\
    struct Fingerprint_Packet_t ack_packet = packetReceive()

  // For functions sending a command packet and receiving an acknowledge packet followed by a data packet
  #define RECEIVE_DATA_PACKET(packet_type, data_length, data_pointer, keywords) \
  RECEIVE_ACK_PACKET(packet_type, data_length, data_pointer); \
                                                              \
  /* Recieive the data packet only if all is well */\
  if (handleReceivedData(*ack_packet.data, keywords))\
    struct Fingerprint_Packet_t data_packet = packetReceive()

  extern "C"{    
    void setBaud(){
      mySerial->begin(fingerprint_module.system_parameter_store.contents.baud_setting * FINGERPRINT_BAUD_RATE_BASE);
    }

    /* Sends UART-bourne packet to the sensor */
    static void packetSend(uint8_t packet_type, uint16_t data_length, uint8_t* data){
      /* do nothing if mySerial == NULL
         DO NOT add a SEMICOLON at the end */
      checkAddressZero()
      
      // construct packet to be trasmitted
      struct Fingerprint_Packet_t txPacket = {
        FINGERPRINT_HEADER, 
        {
          fingerprint_module.system_parameter_store.device_address[0],
          fingerprint_module.system_parameter_store.device_address[1], 
          fingerprint_module.system_parameter_store.device_address[2],
          fingerprint_module.system_parameter_store.device_address[3]
        }
        packet_type,
        data_length + sizeof(txPacket.sum),
        data,
        packet_type + data_length
      };

      // final value of sum
      for (int i = 0; i < data_length; i++)
        txPacket.sum += *(data + i);

      // Transfer the header value with the high byte first
      mySerial->write((txPacket.start_code & 0xFF00) >> 8);
      mySerial->write(txPacket.start_code & 0x00FF);

      // Transfer the address with the high byte first and low byte last
      mySerial->write(txPacket.address[0]);
      mySerial->write(txPacket.address[1]);
      mySerial->write(txPacket.address[2]);
      mySerial->write(txPacket.address[3]);

      // Transfer the packet ID 
      mySerial->write(txPacket.type);  

      // Transfer the package length with the high byte first
      mySerial->write((txPacket.length & 0xFF00) >> 8);
      mySerial->write(txPacket.length & 0x00FF);

      // Transfer the data; not sure if it is the high byte first so I assumed low byte first
      for(int i = 0; i < data_length; i++)
        mySerial->write(txPacket.data[i]);

      // Transfer the sum with the high byte first
      mySerial->write((txPacket.sum & 0xFF00) >> 8);
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
        // wait for 100 ms
        delay(100);
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
      
      // Store the data received on the heap
      rxPacket.data = malloc(rxPacket.length - sizeof(rxPacket.sum));
      
      if(data == NULL){
        printf("Memory Allocation Failed");
        return;
      }
      
      // assign the next no. of bytes = length of the data = txPacket.length - sizeof(txPacket.sum) to data_buffer
      for(int i = 0; i < rxPacket.length - sizeof(rxPacket.sum); i++)
        *(rxPacket.data + i) = mySerial->read();
      
      // assign the last 2 bytes as the sum
      rxPacket.sum = (((uint16_t)mySerial->read() << 8) | (uint16_t)mySerial->read());
      
      return rxPacket;
    }

    // free the data on the heap placed by packetReceive()
    static void dealloc(struct Fingerprint_Packet_t* ptr){
      if(ptr == NULL){
        printf("Memory Deallocation Failed");
        return;
      }
      free(ptr->data);
    }

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
          Serial.println("Error! Sensor Did Not Properly Receive the Package.\n");
          return 0;        

        case FINGERPRINT_DATA_CONFIRMATION_NO_FINGER:
          Serial.println("Error! Cannot Detect Finger.\n");
          return 0;        

        case FINGERPRINT_DATA_CONFIRMATION_COLLECT_FINGER_FAIL:
          Serial.println("Error! Failed to Collect the Fingerprint.\n");
          return 0;          

        case FINGERPRINT_DATA_CONFIRMATION_DISORDELY_FINGERPRINT_IMAGE:
          Serial.println("Error! Failed to Generate Character File due to Overly Disorderly Image.\n");
          return 0;    

        case FINGERPRINT_DATA_CONFIRMATION_FINGERPRINT_FEATURE_ERROR:
          Serial.println("Error! Failed to Generate Character File due to Lack of a Character Point or a Very Small Fingerprint Image.\n");
          return 0;    

        case FINGERPRINT_DATA_CONFIRMATION_TEMPLATES_NO_MATCH:
          Serial.println("Error! The Two Templates Do Not Match.\n");
          return 0;    

        case FINGERPRINT_DATA_CONFIRMATION_FINGERPRINT_IDENTIFICTAION_ERROR:
          Serial.println("Error! Fingerprint Has No Match in the Library.\n");
          return 0;         

        case FINGERPRINT_DATA_CONFIRMATION_CHARACTER_FILE_COMBINE_FAIL:
          Serial.println("Error! Failed to Combine Character Files due to their lack of a belonging to One Finger.\n");
          return 0;         
        
        case FINGERPRINT_DATA_CONFIRMATION_INVALID_ADDRESSING_PAGE_ID:
          Serial.println("Error! Page ID is Beyond the Boundaries of the Finger Library.\n");
          return 0;      

        case FINGERPRINT_DATA_CONFIRMATION_TEMPLATE_READ_ERROR:
          Serial.println("Error! Either the Template Could Not Be Read From the Finger Library or the Readout Template is Invalid.\n");
          return 0;      

        case FINGERPRINT_DATA_CONFIRMATION_TEMPLATE_UPLOAD_ERROR:
          Serial.println("Error! Sensor Could Not Send Template.\n");
          return 0;         

        case FINGERPRINT_DATA_CONFIRMATION_DATA_PACKET_TRANSFER_FAIL_DOWNLOAD:
          Serial.println("Error! Sensor Failed to Receive the following Data Packet.\n");
          return 0;         

        case FINGERPRINT_DATA_CONFIRMATION_DATA_PACKET_TRANSFER_FAIL_UPLOAD:
          Serial.println("Error! Sensor Failed to Send the following Data Packet.\n");
          return 0;         

        case FINGERPRINT_DATA_CONFIRMATION_TEMPLATE_DELETE_FAIL:
          Serial.println("Error! Failed to Delete Template.\n");
          return 0;      

        case FINGERPRINT_DATA_CONFIRMATION_WRONG_PASSWORD:
          Serial.println("Error! Wrong Password Inputted.\n");
          return 0;     

        case FINGERPRINT_DATA_CONFIRMATION_NO_VALID_PRIMARY_IMAGE:
          Serial.println("Error! Wrong Password Inputted.\n");
          return 0;     
        
        case FINGERPRINT_DATA_CONFIRMATION_FLASH_WRITING_ERROR:
          Serial.println("Error! Could Not Write to Sensor's Flash.\n");
          return 0;     

        case FINGERPRINT_DATA_CONFIRMATION_INVALID_REGISTER_NUMBER:
          Serial.println("Error! Sensor Did Not Properly Receive the Package.\n");
          return 0;        

        case FINGERPRINT_DATA_CONFIRMATION_PORT_OPERATION_FAIL:
          Serial.println("Error! Failure to do Port Operation.\n");
          return 0;  

        default:
          Serial.println("Error! Unusual Behaivior.\n");
          return 0;
      }
    }  

    void passwordVerify(uint8_t password[4]){
      // first byte is the instruction code, last 4 bytes are for the password
      uint8_t data[] = {
                          FINGERPRINT_COMMAND_VERIFY_PASSWORD,
                          password[0], 
                          password[1], 
                          password[2],
                          password[3]
                        };

      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data);

      // handle the data received in the acknowledge packet
      handleReceivedData(*ack_packet.data, "Correct Password.");
      
      // return the data containing heap - array to the adruino
      dealloc(&ack_packet);
    }
    
    void passwordSet(uint8_t new_password[4]){
      // first byte is the instruction code, last 4 bytes are for the new password
      uint8_t data[] = {
                        FINGERPRINT_COMMAND_SET_PASSWORD,
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
                  
      // return the data containing heap - array to the adruino
      dealloc(&ack_packet);
    }

    void setModuleAddress(uint8_t new_address[4]){
      // The 1st byte is for the instruction code and last 4 bytes are for the new address
      uint8_t data[] = {
                          FINGERPRINT_COMMAND_SET_MODULE_ADDRESS,
                          new_address[0],
                          new_address[1],
                          new_address[2],
                          new_address[3]
                        };

      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data);

      // set the address in fingerprint_module to new_address if everything is ok
      if(handleReceivedData(*ack_packet.data, "Address Setting Complete.")){
        fingerprint_module.system_parameter_store.device_address[0] = new_address[0];
        fingerprint_module.system_parameter_store.device_address[1] = new_address[1];
        fingerprint_module.system_parameter_store.device_address[2] = new_address[2];
        fingerprint_module.system_parameter_store.device_address[3] = new_address[3];
      }
                  
      // return the data containing heap - array to the adruino
      dealloc(&ack_packet);
    }

    void systemBasicParameterSet(uint8_t parameter_number, uint8_t new_parameter){
      // 1st byte is for the instruction code, the last two bytes are for the inputs
      uint8_t data[] = {
                          FINGERPRINT_COMMAND_SET_SYSTEM_PARAMETER,
                          parameter_number,
                          new_parameter
                        };

      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data);
      
      // handle the data received in the acknowledge packet
      if(handleReceivedData(*ack_packet.data, "Parameter Setting Complete."))
        fingerprint_module.system_parameter_store.data_packet_size = new_parameter; 
            
      // return the data containing heap - array to the adruino
      dealloc(&ack_packet);
    }

    void controlUARTPort(uint8_t port_state){
      // The 1st byte is for the instruction code and 2nd is for on/off byte
      uint8_t data[] = {
                          FINGERPRINT_COMMAND_CONTROL_UART_PORT,
                          port_state
                        };

      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data);
      
      // handle the data received
      handleReceivedData(*ack_packet.data, "Port Operation Complete.");
      
      // return the data containing heap - array to the adruino
      dealloc(&ack_packet);
    }

    struct Fingerprint_Helper_t systemParameterRead(){
      // The 1 byte is for the instruction code
      uint8_t data = FINGERPRINT_COMMAND_READ_SYSTEM_PARAMETER;

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
            
      // return the data containing heap - array to the adruino
      dealloc(&ack_packet);

      return fingerprint_module;
    }

    uint16_t templateNumberRead(){
      // The 1 byte is for the instruction code
      uint8_t data = FINGERPRINT_COMMAND_READ_VALID_TEMPLATE_NUM;

      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data);

      // Stores the data to be returned. If the sensor received the command successfully, store the data it sent, otherwise, its value of 0 is maintained 
      uint16_t store = 0; 

      // return the template if it was generated
      if(handleReceivedData(*ack_packet.data, "Number Generation Success."))
        store = ((uint16_t)ack_packet.data[1] << 8) | ack_packet.data [2];

      // return the data containing heap - array to the adruino
      dealloc(&ack_packet);
      return store;
    }

    void fingerprintVerify(uint8_t capture_time, uint16_t start_bit_number,
                          uint16_t search_quantity, uint16_t (*result)[2]){
      /* The 1st byte is the instruction code 
        2nd byte is for capture_time
        3rd and 4th byte for start_bit_number
        last 2 bytes are for search_quantity
      */
      uint8_t data[] = {
                          FINGERPRINT_COMMAND_VERIFY_FINGERPRINT,
                          capture_time,
                          start_bit_number & 0XFF00,
                          start_bit_number & 0X00FF,
                          search_quantity & 0XFF00,
                          search_quantity & 0X00FF
                        };

      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_COMMAND_VERIFY_FINGERPRINT, sizeof(data), &data);

      // return the received data back to the user 
      if(handleReceivedData(*ack_packet.data, "Fingerprint Read Complete.")){
        result = uint8_t result_buff[4];
                              
        result[0] = (uint16_t)ack_packet.data[1] << 8;
        result[0] |= ack_packet.data[2];

        result[1] = (uint16_t)ack_packet.data[3] << 8;
        result[1] |= ack_packet.data[4];
      }

      // return the data containing heap - array to the adruino
      dealloc(&ack_packet);
    }

    void fingerprintAutoVerify(uint16_t(*result)[2]){
      // The 1 byte is the instruction code 
      uint8_t data = FINGERPRINT_COMMAND_AUTO_VERIFY_FINGERPRINT;

      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_COMMAND_VERIFY_FINGERPRINT, sizeof(data), &data);

      // return the received data back to the user 
      if(handleReceivedData(*ack_packet.data, "Fingerprint Read Complete.")){
        uint8_t result_buff[4];
        result = result_buff;
                              
        result[0] = (uint16_t)ack_packet.data[1] << 8 | ack_packet.data[2];
        result[1] = (uint16_t)ack_packet.data[3] << 8 | ack_packet.data[4];
      }

      // return the data containing heap - array to the adruino
      dealloc(&ack_packet);
    }

    void imageCollect(){
      // The 1 byte is for the instruction code
      uint8_t data = FINGERPRINT_COMMAND_COLLECT_FINGERPRINT_IMAGE;

      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data);

      // handle the data received
      handleReceivedData(*ack_packet.data, "Finger Collection Success.");

      // return the data containing heap - array to the adruino
      dealloc(&ack_packet);
    }

    void imageReceive(uint8_t* image, uint16_t* image_size){
      // The 1 byte is the instruction code
      uint8_t data = FINGERPRINT_COMMAND_UPLOAD_IMAGE;
      
      // get the acknowledge and data packets and handle the data received
      RECEIVE_DATA_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data, "Ready to Transfer the Data Packet.");
    
      // get the address and size of the resultant image
      image = data_packet.data;
      *image_size = data_packet.length - sizeof(uint16_t);

      // return the data containing heap - array to the adruino
      dealloc(&ack_packet);
      dealloc(&data_packet);
    }

    void imageToCharFile(uint8_t buffer_number){
      // The 1st byte is the instruction code and 2nd byte is the buffer number / ID 
      uint8_t data[] ={ 
                      FINGERPRINT_COMMAND_GENERATE_CHAR_FILE_FROM_IMAGE,
                      buffer_number
                    };
      
      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data);

      // handle the data received
      handleReceivedData(*ack_packet.data, "Character file generated.");

      // return the data containing heap - array to the adruino
      dealloc(&ack_packet.data);    
    }

    void templateGenerate(){
      // The 1 byte is the instruction code
      uint8_t data = FINGERPRINT_COMMAND_GENERATE_TEMPLATE;
      
      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data);

      // handle the data received
      handleReceivedData(*ack_packet.data, "Template generated.");

      // return the data containing heap - array to the adruino
      dealloc(&ack_packet);
    }

    void characterFileReceive(uint8_t buffer_number, uint16_t* data_size, uint8_t* result){
      // The 1st byte is the instruction code and 2nd byte is the buffer number / ID 
      uint8_t data[] ={ 
                      FINGERPRINT_COMMAND_UPLOAD_CHAR_FILE,
                      buffer_number
                    };
      
      // get the acknowledge and data packets and handle the data received
      RECEIVE_DATA_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data, "Ready to Transfer the Data Packet.");
      
      // get the address and size of the resultant image
      result = data_packet.data;
      *data_size = data_packet.length - sizeof(uint16_t);

      // return all the heap allocated data back to the adruino
      dealloc(&ack_packet);
      dealloc(&data_packet);
    }

    void characterFileSend(uint8_t buffer_number, uint16_t* data_size, uint8_t* result){
      // The 1st byte is the instruction code and 2nd byte is the buffer number / ID 
      uint8_t data[] ={ 
                      FINGERPRINT_COMMAND_DOWNLOAD_CHAR_FILE,
                      buffer_number
                    };
      
      // get the acknowledge and data packets and handle the data received
      RECEIVE_DATA_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data, "Ready to Transfer the Data Packet.");
      
      // get the address and size of the resultant image
      result = data_packet.data;
      *data_size = data_packet.length - sizeof(uint16_t);

      // return all the heap allocated data back to the adruino
      dealloc(&ack_packet);
      dealloc(&data_packet);
    }

     void templateStore(uint8_t buffer_number, uint16_t pageID){
      // The 1st byte is the instruction code, 2nd buffer number and last 2 bytes are pageID
      uint8_t data[] ={
                        FINGERPRINT_COMMAND_STORE_TEMPLATE,
                        buffer_number,
                        pageID
                      };

      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data);

      // handle the data received
      handleReceivedData(*ack_packet.data, "Storage Success.");

      // return the data containing heap - array to the adruino
      dealloc(&ack_packet.data);
     }

    void templateRead(uint8_t buff_number, uint16_t pageID){
      // The 1st byte is the instruction code, 2nd buffer number and last 2 bytes are pageID
      uint8_t data[] ={
                         FINGERPRINT_COMMAND_READ_TEMPLATE,
                         buff_number,
                         pageID
                       };

      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data);
      
      // handle the data received
      handleReceivedData(*ack_packet.data, "Template Read Success.");

      // return the data containing heap - array to the adruino
      dealloc(&ack_packet);
    }

    void templateDelete(uint8_t pageID, uint16_t n_templates){
      // The 1st byte is the instruction code, 2nd is pageID, last 2 are the number of templates
      uint8_t data[] ={
                          FINGERPRINT_COMMAND_DELETE_TEMPLATE,
                          pageID,
                          n_templates
                        };
       
      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data); 
       
      // handle the data received
      handleReceivedData(*ack_packet.data, "Template Deletion Success."); 

      // return the data containing heap - array to the adruino
      dealloc(&ack_packta);
    }
    void fingerLibraryEmpty(){
      // The 1 byte is the instruction code
      uint8_t data[] = FINGERPRINT_COMMAND_EMPTY_FINGER_LIBRARY;
       
      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data); 
       
      // handle the data received
      handleReceivedData(*ack_packet.data, "Emptied Fingerprint Library Successfully."); 
    
      // return the data containing heap - array to the adruino
      dealloc(&ack_packet.data);
    }

    void templateMatch(){
      // The 1 byte is the instruction code
      uint8_t data = FINGERPRINT_COMMAND_MATCH_TWO_TEMPLATES;
       
      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data); 
       
      // handle the data received
      handleReceivedData(*ack_packet.data, "Templates of the Two Buffers Are Matching."); 
    
      // return the data containing heap - array to the adruino
      dealloc(&ack_packet);
    }
    
    void fingerLibrarySearch(uint8_t buffer_number, uint16_t start_address, uint16_t page_num){.
      // The 1st byte is the instruction code
      uint8_t data = FINGERPRINT_COMMAND_SEARCH_FINGER_LIBRARY;
       
      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data) 
       
      // handle the data received
      handleReceivedData(*ack_packet.data, "The Templates in the Two Buffers Are Matching."); 
    
      // return the data containing heap - array to the adruino
      dealloc(&ack_packet);
    }

    uint32_t generateRandomCode(){
      // The 1 byte is the instruction code
      uint8_t data = FINGERPRINT_COMMAND_GET_RANDOM_CODE;

      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data);

      // Stores the data to be returned. If the sensor received the command successfully, store the data it sent , otherwise, its value of 0 is maintained 
      uint32_t store = 0;

      // return the number if it was generated
      if (handleReceivedData(*ack_packet.data, "Number Generation Success."))
        // store the random number
        uint32_t store = (uint32_t)ack_packet.data[1] << 24 | (uint32_t)(ack_packet.data[2] << 16) | (uint32_t)ack_packet.data[3] << 8 | ack_packet.data[4];

      // return the data containing heap - array to the adruino
      dealloc(&ack_packet);
      return store;
    }

    void notepadWrite(uint8_t page_num, uint8_t(*data)[32]){
      // The 1st byte is the instruction code, 2nd is page_num, last 32 are for the data
      uint8_t data_buff[34];
      data_buff[0] =  FINGERPRINT_COMMAND_WRITE_TO_NOTEPAD;
      data_buff[1] = page_num;
      
      memset(data, data_buff + 2, sizeof(data) - sizeof(data[0]) - sizeof(data[1]));

      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data_buff), &data_buff); 
       
      // handle the data received
      handleReceivedData(*ack_packet.data, "Write Success."); 

      // return the data containing heap - array to the adruino
      dealloc(&ack_packet.data);
    }

    void notepadRead(uint8_t page_num, uint8_t(*return_data)[32]){
      // The 1st byte is the instruction code, 2nd is page_num, last 32 are for the data
      uint8_t data[] = {
                          FINGERPRINT_COMMAND_READ_FROM_NOTEPAD,
                          page_num
                       };

      // get the acknowledge packet
      RECEIVE_ACK_PACKET(FINGERPRINT_PACKET_IDENTIFIER_COMMAND, sizeof(data), &data); 
       
      // handle the data received
      if(handleReceivedData(*ack_packet.data, "Read Success.")){
        return_data = ack_packet.data;
      }

      // return the data containing heap - array to the adruino
      dealloc(&ack_packet.data);
    }

  };  
#endif
