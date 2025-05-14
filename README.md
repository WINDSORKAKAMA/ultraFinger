Find a way how to differenciate HardwareSerial and SoftwareSerial using C++
****************************************************************************


    You can create pointers to either a HardwareSerial object or SoftwareSerial

object depending on whether the hardware avialable has multiple or one pair of 

UART ports respectively.


    You then create a pointer to a Serial object and write the address of either 

the HardwareSerial or SoftwareSerial pointer to it.

    Afterword, write to the Serial port to send data


    To receive data, check if there is available data and receive it


For More Info : 
https://www.circuitstate.com/libraries/r307-optical-fingerprint-scanner-library-for-arduino-documentation/
