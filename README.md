<p align="center">
    <img src="images/fingerprint.png" alt="ultraFinger title" />
</p>
    <p align="center">An alternative r307 Fingerprint library for Arduino</p>
</p>

## Yet another Fingerprint Library?

In 2024, I decided to make my own fingerprint sensor library out of frustration due to working with the poorly documented Adafruit Fingerprint Library while working on a cashless transaction module at the 
time. 
that
Later, I decided to revisit the idea to create it ain a re-usable manner to allow it to be used in other applications.

For example, I made the decision to write this library in "C-like C++", specifically a blend between C99 and C89 code. 

Since C is cross compatible with virtually any language, and this library exposes the basic fucntionality for working with this sensor, you can,
for example, write a smart door system that uses a database interfaces and with a Python ORM to query fingerprints using a custom API that uses this library to read / write fingerprint templates / character files / bitmap images to the fingerprint sensor.

I also made the decision to use only the SoftwareSerial library for UART communication. This was done for 2 reasons:

* To ensure that the HardwareSerial ports(0 and 1) are reserved for debugging.
* To ensure cross compatibility as all microcontollers supporting SoftwareSerial can carry out UART communication with the sensor using any port.

## Features

Here is a list of the current features:

* A container for the system parameter values.
* Debugging information conveyed to the terminal.
* Verifying the module password.
* Changing the module password.
* Reading compressed images from the Image Buffer.
* Reading templates from the Character Buffer.
* Writing templates to the Character Buffer.
* Reading templates to from the Notepad.
* Writing templates to the Notepad.
* Reading system parameter values.
* Modifying system parameter values.
* Random Number Generation.
* Comparing two templates
* Searching for a template
* Getting template ID's


### Dependencies

In order to use the library you will need:

* Arduino IDE
* SoftwareSerial library 


## Implementation details


### Specifications

The r307 fingerprtint sensor


### Functions
    Below are the different functions provided by the library

```C
extern void setBaud()
```    
Sets the baud rate of the upper computer

```C
extern void passwordVerify(uint8_t password[4])
```
Verifies the sensor's handshaking password
    
```C
extern void passwordSet(uint8_t new_password[4]);
```
Changes / Sets the sensor's handshaking password 

```C
extern void systemBasicParameterSet(uint8_t parameter_number, uint8_t new_parameter);
```
Changes either the baud rate, security level or packet length (Basic System Parameters) 

```C
extern Fingerprint_Helper_t systemParameterRead();
```
Reads the the current system parameter values     
        
```C
extern void setModuleAddress(uint8_t new_address[4]);
```
Sets the device address value 

```C
extern void controlUARTPort(uint8_t port_state);
```
Turns the UART port on the sensor on / off 

```C
extern void fingerprintVerify(uint8_t capture_time, uint16_t start_bit_number, uint16_t search_quantity, uint16_t(*result)[2]);
```
Matches captured fingerprint with fingerprint library and returns the result 

```C
extern void fingerprintAutoVerify(uint16_t(*result)[2]);
```
Collects fingerprint, matches it with the fingerprint library and returns the result     

```C
extern void imageReceive(uint8_t* output_image, uint16_t* image_size);
```
Receives an image from the image buffer, puts its address in *image and its size in image_size (uploads an image) 

```C
extern void imageToCharFile(uint8_t buff_number); 
```
Generates character file from the original finger image in the image buffer and stores it in either character buffer 1 or 2 
        
```C
extern void characterFileReceive(uint8_t buff_number, uint16_t* data_size, uint8_t* result);
```    
Receives a character file or template from the specified character buffer (uploads a character file) 

```C
extern void characterFileSend(uint8_t buff_number, uint16_t* data_size, uint8_t* result);
```    
Send character file or template to the specified character buffer (downloads a character file) 

```C
extern uint16_t templateNumberRead();   
```
 Reads a valid template number 

```C
extern void templateGenerate();
```    
Generates a template from character files in the character buffers 

```C
extern void templateStore(uint8_t buffer_number, uint16_t pageID);
```    
Stores the template of the specified character buffer in the designated location of the flash library 

```C
extern void templateRead(uint8_t buff_number, uint16_t pageID);
```    
Loads / Reads a template at the specified location (pageID) of the flash library and places it in either character buffer 1 or 2 

```C
extern void templateDelete(uint8_t pageID, uint16_t n_templates);
```   
Deletes n number of templates of the flash library starting from the specified location (pageID)  

```C
extern void templateMatch();
``` 
Carries out precise matching of templates, one in character buffer 1 and the other in character buffer 2    

```C
extern void fingerLibraryEmpty();
```    
Deletes all templates in the finger library 

```C
extern void fingerLibrarySearch(uint8_t buffer_number, uint16_t start_address, uint16_t page_num);
```    
Searches the whole finger library for the template that matches the one in the specified character buffer 

```C
extern void notepadWrite(uint8_t page_num, uint8_t(*data)[32]);
```   
Writes data to the specified flash page  

```C
extern void notepadRead(uint8_t page_num, uint8_t(*return_data)[32]);
```   
Reads the specified flash page's content  
        
```C
extern uint32_t generateRandomCode()
```
Generates a random number from the sensor and return it to the computer 

BLACKBOXAI

Sure! Here’s the complete README content in one file format:

markdown

<p align="center">

    <img src="images/fingerprint.png" alt="UltraFinger Title" />

</p>

<p align="center">An alternative R307 Fingerprint library for Arduino</p>


## Yet Another Fingerprint Library?


In 2024, I decided to create my own fingerprint sensor library out of frustration with the poorly documented Adafruit Fingerprint Library while working on a cashless transaction module. I aimed to develop a reusable library that could be applied in various applications.


This library is written in a "C-like C++" style, blending C99 and C89 code. Since C is cross-compatible with virtually any language, this library exposes the basic functionality for working with the sensor. For example, you can write a smart door system that uses a database interface with a Python ORM to query fingerprints using a custom API that utilizes this library to read/write fingerprint templates, character files, and bitmap images to the fingerprint sensor.


I also decided to use only the SoftwareSerial library for UART communication for two reasons:

- To reserve the HardwareSerial ports (0 and 1) for debugging.

- To ensure cross-compatibility, as all microcontrollers supporting SoftwareSerial can carry out UART communication with the sensor using any port.


## Features


Here is a list of the current features:


- A container for system parameter values.

- Debugging information conveyed to the terminal.

- Verifying the module password.

- Changing the module password.

- Reading compressed images from the image buffer.

- Reading templates from the character buffer.

- Writing templates to the character buffer.

- Reading templates from the notepad.

- Writing templates to the notepad.

- Reading system parameter values.

- Modifying system parameter values.

- Random number generation.

- Comparing two templates.

- Searching for a template.

- Getting template IDs.


### Dependencies


To use the library, you will need:


- Arduino IDE

- SoftwareSerial library


## Implementation Details


### Specifications

Visit the manuals in the manuals directory for more information


### Functions


Here are the key functions provided by the library:


```cpp

extern void setBaud();

Sets the baud rate of the upper computer.

cpp1 lines
Click to expand

extern void passwordVerify(uint8_t password[4]);

Verifies the sensor's handshaking password.

cpp1 lines
Click to close

extern void passwordSet(uint8_t new_password[4]);

Changes or sets the sensor's handshaking password.

cpp1 lines
Click to expand

extern void systemBasicParameterSet(uint8_t parameter_number, uint8_t new_parameter);

Changes the baud rate, security level, or packet length (basic system parameters).

cpp1 lines
Click to expand

extern Fingerprint_Helper_t systemParameterRead();

Reads the current system parameter values.

cpp1 lines
Click to expand

extern void setModuleAddress(uint8_t new_address[4]);

Sets the device address value.

cpp1 lines
Click to expand

extern void controlUARTPort(uint8_t port_state);

Turns the UART port on the sensor on/off.

cpp1 lines
Click to expand

extern void fingerprintVerify(uint8_t capture_time, uint16_t start_bit_number, uint16_t search_quantity, uint16_t(*result)[2]);

Matches the captured fingerprint with the fingerprint library and returns the result.

cpp1 lines
Click to expand

extern void fingerprintAutoVerify(uint16_t(*result)[2]);

Collects a fingerprint, matches it with the fingerprint library, and returns the result.

cpp1 lines
Click to expand

extern void imageReceive(uint8_t* output_image, uint16_t* image_size);

Receives an image from the image buffer, putting its address in *output_image and size in image_size.

cpp1 lines
Click to expand

extern void imageToCharFile(uint8_t buff_number);

Generates a character file from the original finger image in the image buffer and stores it in either character buffer 1 or 2.

cpp1 lines
Click to expand

extern void characterFileReceive(uint8_t buff_number, uint16_t* data_size, uint8_t* result);

Receives a character file or template from the specified character buffer.

cpp1 lines
Click to expand

extern void characterFileSend(uint8_t buff_number, uint16_t* data_size, uint8_t* result);

Sends a character file or template to the specified character buffer.

cpp1 lines
Click to expand

extern uint16_t templateNumberRead();

Reads a valid template number.

cpp1 lines
Click to expand

extern void templateGenerate();

Generates a template from character files in the character buffers.

cpp1 lines
Click to expand

extern void templateStore(uint8_t buffer_number, uint16_t pageID);

Stores the template of the specified character buffer in the designated location of the flash library.

cpp1 lines
Click to expand

extern void templateRead(uint8_t buff_number, uint16_t pageID);

Loads a template at the specified location (pageID) of the flash library to either character buffer 1 or 2.

cpp1 lines
Click to expand

extern void templateDelete(uint8_t pageID, uint16_t n_templates);

Deletes n number of templates from the flash library starting from the specified location (pageID).

cpp1 lines
Click to expand

extern void templateMatch();

Carries out precise matching of templates, one in character buffer 1 and the other in character buffer 2.

cpp1 lines
Click to expand

extern void fingerLibraryEmpty();

Deletes all templates in the fingerprint library.

cpp1 lines
Click to expand

extern void fingerLibrarySearch(uint8_t buffer_number, uint16_t start_address, uint16_t page_num);

Searches the whole fingerprint library for the template that matches the one in the specified character buffer.

cpp1 lines
Click to expand

extern void notepadWrite(uint8_t page_num, uint8_t(*data)[32]);

Writes data to the specified flash page.

cpp1 lines
Click to expand

extern void notepadRead(uint8_t page_num, uint8_t(*return_data)[32]);

Reads the specified flash page's content.

cpp1 lines
Click to expand

extern uint32_t generateRandomCode();

Generates a random number from the sensor and returns it to the computer.

## Side Note
The function, ```imageSend()``` is under construction. Avoid using it


## Conclusion


This library aims to provide a robust and flexible solution for working with fingerprint sensors in various applications. Whether you're building a cashless transaction system, a smart door lock, or any other project requiring fingerprint authentication, UltraFinger is designed to meet your needs.


Feel free to contribute, report issues, or suggest features!
