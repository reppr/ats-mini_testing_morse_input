 /*
  menu_IO_configuration.h
*/

/*
  this version definines the menu INPUT routine int men_getchar();
  and the menu OUTPUT streams
  from the *program*
  not inside the Menu class

  so your program must #include
  #include "menu_IO_configuration.h"
*/


#ifndef MENU_CONFIGURATION_H

#if ! defined EOF32
  #define EOF32		(int) 0xffffffff
#endif

/*
// CONFIGURE one or more INPUTS and OUTPUTS:
#define MENU_USE_SERIAL_IN
#define MENU_USE_SERIAL_OUT

#define MENU_USE_WiFi_TELNET_IN
#define MENU_USE_WiFi_TELNET_OUT

// determine number of configured outstreams
#if defined(MENU_USE_SERIAL_OUT) + defined(MENU_USE_WiFi_TELNET_OUT) == 2
  #define MENU_OUTSTREAMS	2
#elif defined(MENU_USE_SERIAL_OUT) + defined(MENU_USE_WiFi_TELNET_OUT) == 1
  #define MENU_OUTSTREAMS	1
#elif defined(MENU_USE_SERIAL_OUT) + defined(MENU_USE_WiFi_TELNET_OUT) == 0
  #define MENU_OUTSTREAMS	0
  #error "MENU_OUTSTREAMS=0	not implemented yet"
#endif
*/

#ifdef ARDUINO
  /* ********************************
    ==> Serial menu output
  */

/*
  ################ FIXME: temporary:  use Serial unconditionally
  #if defined(MENU_USE_SERIAL_OUT) || defined(MENU_USE_SERIAL_IN)
*/

#if ! defined BAUDRATE
    /* BAUDRATE for Serial:	uncomment one of the following lines:	*/
    //#define BAUDRATE	1000000	// (I get many errors on some ESP32 boards)
    //#define BAUDRATE	500000	// fine on ESP32 with bad USB cable
  #if defined ESP32
    #define BAUDRATE	500000
  //#define BAUDRATE	115200	// allows to read the boot messages

  #else	// default on other processors
    #define BAUDRATE	115200	// works fine here on all tested Arduinos
  #endif
    //#define BAUDRATE	250000
    //#define BAUDRATE	230400
    //#define BAUDRATE	115200		// works fine here on all tested Arduinos
    //#define BAUDRATE	74880
    //#define BAUDRATE	57600
    //#define BAUDRATE	38400
    //#define BAUDRATE	19200
    //#define BAUDRATE	9600		// failsafe
    //#define BAUDRATE	31250		// MIDI
#endif // BAUDRATE


#define MENU_OUTSTREAM	Serial


#if defined USE_BLUETOOTH_SERIAL_MENU
  #if defined ESP32
    #define MENU_OUTSTREAM2	BLUEtoothSerial
  #else
    #undef USE_BLUETOOTH_SERIAL_MENU
    #warning USE_BLUETOOTH_SERIAL_MENU was switched off
  #endif
#endif // USE_BLUETOOTH_SERIAL_MENU

// do we need WiFi?
  #ifdef USE_WIFI_telnet_menu	// use WIFI as menu over telnet?
    #if defined(ESP8266)
	#include <ESP8266WiFi.h>	// breaks:  min() max()   use:  _min() _max()
    #elif defined(ESP32)
	#include <WiFi.h>	// might break:  min() max()   use:  _min() _max()
    #else
	#undef USE_WIFI_telnet_menu
	#warning "WiFi code unknown,  see: pulses_boards.h, WiFi telnet menu DEACTIVATED"
    #endif
  #endif

  #ifdef USE_WIFI_telnet_menu	/* ################ FIXME: #ifdef MENU_USE_WiFi_TELNET_OUT */
    /* ********************************
       ==> WiFi menu output
    */

    //  // how many clients should be able to telnet to this ESP8266
    //  #define MAX_SRV_CLIENTS 1
    //  WiFiClient server_client[MAX_SRV_CLIENTS];
    WiFiClient server_client;
    WiFiServer telnet_server(23);

    #define MENU_OUTSTREAM2	server_client
  #endif

#if ! defined MENU_OUTSTREAM2
  // see: https://stackoverflow.com/questions/11826554/standard-no-op-output-stream
  #include <iostream>
  class NullBuffer :  public std::streambuf
  {
  public:
    int overflow(int c) { return c; }
    // streambuf::overflow is the function called when the buffer has to output data to the actual destination of the stream.
    // The NullBuffer class above does nothing when overflow is called so any stream using it will not produce any output.
  };

  NullBuffer null_buffer;

  //#define MENU_OUTSTREAM2		std::ostream null_stream(&null_buffer)
  #define MENU_OUTSTREAM2	(Stream &) null_buffer
#endif // #if ! defined MENU_OUTSTREAM2

  /* ********************************
    ==> menu input
  */
  int men_getchar() {
    if (Serial.available())
      return Serial.read();

  #if defined USE_BLUETOOTH_SERIAL_MENU
    if (BLUEtoothSerial.available())
      return BLUEtoothSerial.read();
  #endif

  #ifdef USE_WIFI_telnet_menu
      if (server_client && server_client.connected() && server_client.available())
	return server_client.read();
  #endif

/*
    #ifdef MENU_USE_SERIAL_IN
      if (Serial.available())
	return Serial.read();
    #endif
*/
    return EOF32;
  }

#else	// PC
  #warning "Linux PC test version  *NOT SUPPORTED*  out of date"

  int men_getchar() {
    return getchar();		// c++ Linux PC test version
  }

  #define MENU_OUTSTREAM	cout
#endif // Arduino vs PC

#if ! defined CB_SIZE
  #if defined RAM_IS_SCARE	// for very old AVR Arduino programs, obsolete
    #define CB_SIZE	32
  #else
    #define CB_SIZE	128
  #endif
#endif // CB_SIZE

  #define MENU_CONFIGURATION_H
#endif
