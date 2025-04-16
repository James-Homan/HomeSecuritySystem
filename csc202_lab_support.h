//*****************************************************************************
//*****************************    C Source Code    ***************************
//*****************************************************************************
//
// DESIGNER NAME: James Homan
//
//     FILE NAME: csc202_lab_support.h
//
//          DATE: 01/05/2022
//
//-----------------------------------------------------------------------------
//
// DESCRIPTION:
//    This file contains the definitions of standard data types used and 
//    various CSC-202 support functions for Dragon12 Board. It adds some 
//    support for to display strings to the serial port.
//
//*****************************************************************************

#ifndef CSC202_SUPPORT_H_
#define CSC202_SUPPORT_H_

#include <stdio.h>

//-----------------------------------------------------------------------------
//                        Define symbolic constants
//-----------------------------------------------------------------------------

#define RTI_CONVERT_ONE_SEC 100
#define RTI_CONVERT_TWO_SEC 200
#define RTI_CONVERT_FOUR_SEC 400
#define RTI_CONVERT_FIFTEEN_SEC 1500
#define RTI_CONVERT_FIFTY_MS 50
#define DOOR_DEV 1
#define WIN_DEV 2
#define GAR_DEV 3
#define TEMP_MSG 4
#define STAR 14
#define LOCK_ENABLE 0xEFB
#define LIMIT_DIGITS_TO_NINE 10
#define TTIMER_MASK 0x04
#define SCROLL 0
#define FALSE     0
#define TRUE      1
#define LCD_LINE1_ADDR 0x00
#define LCD_LINE2_ADDR 0x40
#define ZERO 0
#define ONE 1
#define PTH_DOOR_BITMASK 0x02
#define PTH_WIN_BITMASK 0x04
#define ALARM_BITMASK     0xF7  //bits 3/5 of port M
#define SCI0VAL         9600
#define PIN_SIZE        4
#define CONFIG_PIN      '1'
#define DEV_CONFIG      '2'
#define DEMO_MODE       '3'
#define ENDPROGRAM      '4'
#define SELECT_DOOR     '1'
#define SELECT_WINDOW   '3'
#define SELECT_GARAGE   '2'
#define RETURN_MENU     '4'
#define ON              0xFF
#define DEGREE          0xDF
#define RETURN          '\r'
#define NEXT_LINE       '\n'
#define BACKSPACE       0x7F

#define INVALID_INPUT   "Incorrect input."
#define END_PROGRAM     "SETUP STOPPED"
#define ACTIVE_ALARM    "ALARM IS ACTIVE "
#define INACTIVE_ALARM  "ALARM INACTIVE  "

#define MIN_JS_DEFAULT 800
#define MAX_JS_DEFAULT 860
#define LIGHT_SENSOR 4
#define POTENTIOMETER 7
#define THERMOMETER 5
#define JOYSTICK 3

#define DOOR_UNLOCKED    "Door Unlocked   "
#define DOOR_LOCKED      "Door locked     "
#define WINDOW_CLOSED    "Window Closed   "
#define WINDOW_OPEN      "Window Open     "
#define GARAGE_CLOSED    "Garage Closed   "
#define GARAGE_OPEN      "Garage Open     "
#define TEMP_SCRIPT      "Temp:"

#define SELECT_DOOR     '1'
#define SELECT_WINDOW   '3'
#define SELECT_GARAGE   '2'

#define GARAGE_SPEED          200                                                            
#define GARAGE_OFF            0 
#define BIT_TWO_MASK          0x04
#define GARAGE_SPEED          200                                                           
#define GARAGE_OFF            0 
#define BIT_TWO_MASK 0x04
#define TIMER_ENABLE 0x80
#define BIT_TWO_UP_DWN_EDGE_TRIG 0x30
#define TRIGGER_HIGH 0x03
#define TRIGGER_LOW 0x02
#define FORCE_BIT_FOUR 0x10
#define NEXT_ISR_TIME 15
#define OUTPUT_CAPTURE_MODE 1
#define CHANNEL3_MODE (OUTPUT_CAPTURE_MODE << 3)
#define TIMER_PRESCALER_TWO 0x04
#define CHANNEL4_MODE (OUTPUT_CAPTURE_MODE << 4)
#define ENABLE_MOTOR 0xFF
#define ASCII_VALUE 0x30
 
#define ZERO_DEGREE       3000
#define NINETY_DEGREE     5150
#define ONEEIGHTY_DEGREE  7000
#define ALARM_PITCH       1000

#define BIT_TWO_MASK 0x04
#define TIMER_ENABLE 0x80
#define BIT_TWO_UP_DWN_EDGE_TRIG 0x30
#define TRIGGER_HIGH 0x03
#define TRIGGER_LOW 0x02
#define FORCE_BIT_FOUR 0x10
#define NEXT_ISR_TIME 15
#define DISTANCE_TOO_CLOSE 10
#define SPEED_OF_SOUND 340
#define CYCLES_PER_SECOND 1500000
#define CM_CONVERSION 100
#define TWO_WAY_DISTANCE_REDUCTION 2
#define PIN_LENGTH 4
#define TWO 2

#define FIVE_MS_DELAY 5
#define TEN_MS_DELAY 10
#define FIFTY_MS_DELAY 50
#define TWO_HUN_MS_DELAY 200
#define TWO_HUN_FIF_MS_DELAY 250
#define ONE_HUN_MS_DELAY 100
#define TEN_SEC_DELAY 10000
#define TWO_SEC_DELAY 2000
#define ONE_SEC_DELAY 1000
#define FIVE_SEC_DELAY 5000
#define OUTPUT 0xFF
#define OFF 0x00
#define TEMP_CONVERSION_FACTOR_ONE 32
#define TEMP_CONVERSION_FACTOR_TWO 9
#define TEMP_CONVERSION_FACTOR_THREE 5
#define GARAGE_ENABLE_DOWN 0x02
#define GARAGE_ENABLE_UP 0x01
#define GARAGE_PTP_SETUP 0xEF



#define alt_println   alt_printf("\r\n", 0)


//-----------------------------------------------------------------------------
//                        Define Embedded Data Types
//-----------------------------------------------------------------------------

//  Integer Types
typedef   signed char       sint8;      // signed 8 bit values
typedef unsigned char       uint8;      // unsigned 8 bit values
typedef   signed short int  sint16;     // signed 16 bit values
typedef unsigned short int  uint16;     // unsigned 16 bit values
typedef   signed long  int  sint32;     // signed 32 bit values
typedef unsigned long  int  uint32;     // unsigned 32 bit values

//  Floating Point Types
typedef float  real32;                  // single precision floating values
typedef double real64;                  // double precision floating values


//  Register Types
typedef volatile uint8*  register8;     //  8-bit register
typedef volatile uint16* register16;    // 16-bit register
typedef volatile uint32* register32;    // 32-bit register

typedef unsigned short bool;            // Boolean




//-----------------------------------------------------------------------------
//                      Define Public Functions
//-----------------------------------------------------------------------------

void alt_printf(sint8 buffer[70], uint16 value);
void alt_printfL(sint8 buffer[70], uint32 value);
void alt_clear();



//-----------------------------------------------------------------------------
//                 Define Public Global Variables
//-----------------------------------------------------------------------------

uint8  test_reg8  = 0xC0;
uint16 test_reg16 = 0x0000;


#ifdef USE_SCI0
  #define send_char   outchar0
#else
  #define send_char   outchar1
#endif


//-----------------------------------------------------------------------------
//                             private functions
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// NAME: Print a string message
//
// DESCRIPTION:
//    This function prints a string message to the terminal window. The 
//    string message can be up to 70 characters long. The string to print 
//    supports basic format of the C printf function. This version of the 
//    print function allows you to display 1 or 2 byte integer in the string.
//
// INPUT:
//   buffer  - the string array up to 70 characters to print
//   buffer  - a long integer value to be substituted in string formatting
//
// OUTPUT:
//   none
//
// RETURN:
//   none
//----------------------------------------------------------------------------
void alt_printf(sint8 buffer[70], uint16 value)
{
  uint16 i = 0;
  uint16 len = 0;
  sint8  string[80];

  len = sprintf(string, buffer, value);

  // Walk through array to send each character to serial port
  for (i = 0; i< len; i++)
  {
    send_char(string[i]);      
  } /* for */
  
} /* alt_printfL */


//----------------------------------------------------------------------------
// NAME: Print a string message
//
// DESCRIPTION:
//    This function prints a string message to the terminal window. The 
//    string message can be up to 70 characters long. The string to print 
//    supports basic format of the C printf function. This version of the 
//    print function allows you to display a long integer in the string.
//
// INPUT:
//   buffer  - the string array up to 70 characters to print
//   buffer  - a long integer value to be substituted in string formatting
//
// OUTPUT:
//   none
//
// RETURN:
//   none
//----------------------------------------------------------------------------
void alt_printfL(sint8 buffer[70], uint32 value)
{
  uint16 i = 0;
  uint16 len = 0;
  sint8  string[80];

  len = sprintf(string, buffer, value);

  // Walk through array to send each character to serial port
  for (i = 0; i< len; i++)
  {
    send_char(string[i]);      
  } /* for */
  
} /* alt_printfL */


//----------------------------------------------------------------------------
// NAME: Clear Terminal Screen
//
// DESCRIPTION:
//    This function sends the escape sequence to terminal window to clear 
//    the screen. Then it places the cursor in the home location. This works 
//    best when using a terminal emulator like PuTTY.
//
// INPUT:
//   none
//
// OUTPUT:
//   none
//
// RETURN:
//   none
//----------------------------------------------------------------------------
void alt_clear()
{
  // ESC to alt_clear screen
  send_char(0x1B);
  send_char('[');
  send_char('2');
  send_char('J');

  // ESC to home cursor
  send_char(0x1B);
  send_char('[');
  send_char('H');

}  /* alt_clear */








#endif /* CSC202_SUPPORT_H_ */
