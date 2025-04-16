//*****************************************************************************
//*****************************  C  Source Code  ******************************
//*****************************************************************************
//
//   DESIGNER NAME:  James Homan
//
//       FILE NAME:  Final_Proj_James_Josh.c
// 
//            DATE:  5/3/2023
//
// DESCRIPTION
//    
//
//*****************************************************************************

//---------------------------------------------------------------------
// Required Dragon12 Board support information
//---------------------------------------------------------------------

#include <hidef.h>                /* common defines and macros */
#include <mc9s12dg256.h>          /* derivative information */
#pragma LINK_INFO DERIVATIVE "mc9s12dg256b"

#include "main_asm.h"             /* interface to the assembly module */
#include  "csc202_lab_support.h"

// Method Prototypes
void displayMainMenu(void);
void displayTemp(void);
void myPrintln(sint8* message);
int getNum(void);
char get_sel_Num(int max_allowed);
void getString(uint8 length, char *test);
void configDevice(void);
void displayConfigMenu(void);
bool inputValidationDev(char c);
bool inputValidationMain(char c);
void phraseDisplay(char phrase[]);
bool inputValidationPIN(sint8* pin);
bool confirmPin(char* first, char* second);
void pin(int*);
void presentation(int*);
sint8* display_status(bool door, bool window, bool garage, int job);
uint16 getTemp(void); 
void garage_go_up(void);                                                                     
void garage_go_down(void);
bool doorLock(bool locked);
void disarm(int*);
void spotlight(void); 

// Global Constants used to communicate between interupts and main()
bool g_garage_flag = FALSE;
bool g_door_flag = FALSE;
bool g_window_flag = FALSE;
bool g_status_change = FALSE;
bool g_update_message = FALSE;
bool g_alarm_activate = FALSE;
bool g_timeout = FALSE;
int g_echo_time = ZERO;
bool garage_request_up = FALSE;
bool g_alarm_trigger = FALSE;

//----------------------------------------------------------------------------
// NAME: switches(ISR)
//
// DESCRIPTION:
//    This function monitors SW 3 (window) & 4 (Door). If either button is  
//    pressed, the global flag representing that entry is flagged, as well as
//    the status change flag (represents a change occured wihtin the while 
//    loop in main())
//
// INPUT:
//   none
//
// OUTPUT: 
//   none.
//   
// RETURN:
//   none
//----------------------------------------------------------------------------
void interrupt 25 switches()
{
  uint8 clear_bits = ZERO;
  uint8 switchvalue = PIFH;
  //monitors front door
  if((switchvalue & PTH_DOOR_BITMASK) == PTH_DOOR_BITMASK)
  {
    g_door_flag = TRUE;
    clear_bits |= PTH_DOOR_BITMASK;
    g_status_change = TRUE; 
  }
  // monitors main window
  if((switchvalue & PTH_WIN_BITMASK) == PTH_WIN_BITMASK) 
  {
    g_window_flag = TRUE;
    clear_bits |= PTH_WIN_BITMASK;
    g_status_change = TRUE;
  }
  PIFH = clear_bits;
}

//----------------------------------------------------------------------------
// NAME: sched_interupt (RTI)
//
// DESCRIPTION:
//    This timer handles the majority of scheduling and status checks for the
//    alarm to function. Garage: the garage is scheduled to check for joystick
//    activity every 50ms. If triggered the garage flag is set and 
//    garage_request_up is set depending on direction recorded. Messaging:
//    message count is set to trigger an update in message displayed (g_update
//    _message) every 4 seconds. Home Alarm Button: An external pushbutton is
//    connected to PTM bit 3 and alarm_activate is set if triggered. All above 
//    triggers set g_status_change for interpretation in main. Timeout: timeout 
//    is used to trigger a 15 second count when needed in main(). This timeout  
//    is used to trigger the buzzer alarm. Alarm trigger: This would be triggered.
//    by main() if timeout is not resolved in the alotted time. This causes an  
//    alarm to alternate sounding off/ silent within RTI so that main() can  
//    continue requesting the pin for validation..
//
// INPUT:
//   none
//
// OUTPUT:   .
//   none
//
// RETURN:
//   none
//----------------------------------------------------------------------------
void interrupt 7 sched_interupt()
{
  //static int variables are timers/counters
  static int garage_count = ONE;
  static int message_count = ONE;
  static int timeout_count = ZERO;
  static int alarm_counter = ZERO;
  uint16 js_reading = ZERO;
  asm
  {
    sei;
  }
  //monitors for activation of garage (joystick)
  if (garage_count == RTI_CONVERT_FIFTY_MS)
  {
    js_reading = ad1conv(JOYSTICK);
    if ((js_reading > MAX_JS_DEFAULT))
    {
      g_garage_flag = TRUE;
      g_status_change = TRUE;
      garage_request_up = TRUE;
    }
    if ((js_reading < MIN_JS_DEFAULT))
    {
      g_garage_flag = TRUE;
      g_status_change = TRUE;
      garage_request_up = FALSE;
    }
    garage_count = ZERO;
  }
  //controls time delayed message notification on LCD line 2
  if (message_count == RTI_CONVERT_FOUR_SEC)
  {
    g_update_message = TRUE;
    message_count = ZERO;
    g_status_change = TRUE;
  }
  //Monitors In-home alarm (external) pushbutton
  if((PTM | ALARM_BITMASK) == ALARM_BITMASK)
  {
    g_alarm_activate = TRUE;
    g_status_change = TRUE;
  }
  //If pin request triggered, timeout used to trigger alarm after timeframe
  if (g_timeout == TRUE)
  {
    timeout_count++;
    if (timeout_count == RTI_CONVERT_FIFTEEN_SEC)
    {
      g_timeout = FALSE;
    }
  }
  if (g_timeout == FALSE)
  {
    timeout_count = ZERO;
  }
  //Controls alarm within RTI so that pin can still be obtained
  if (g_alarm_trigger)
  {
    alarm_counter++; 
    if (alarm_counter == RTI_CONVERT_ONE_SEC)
    {
      motor5_init();
      motor5(ALARM_PITCH);
      led_enable();
      PORTB = ON;
    }
    if (alarm_counter == RTI_CONVERT_TWO_SEC) 
    {
      motor5(ZERO);
      led_disable();
      alarm_counter = ZERO;
    } 
  }
  //counter maintenaince/ RTI clear
  garage_count++; 
  message_count++;
  if ((g_alarm_trigger == FALSE) && (alarm_counter > ZERO))
  {
    alarm_counter = ZERO;
  }
  clear_RTI_flag();
  _asm CLI;
}

// -----------------------------------------------------------------------------
// DESCRIPTION
//   This ISR is used for input from the echo line of the distance sensor. When
//   this line goes high, the ISR is triggered to record the TCNT (running 
//   timer) and when echo goes low, the return time is recorded. This time
//   difference (analagous to distance) is recored in the global variable
//   g_echo_time for calculation in main.
//
// INPUT PARAMETERS:
//   none
//
// OUTPUT PARAMETERS:
//   none
//
// RETURN:
//   none
// -----------------------------------------------------------------------------
void interrupt 10 timer()
{
  static uint16 start_time = ZERO;
  if((PTT & TTIMER_MASK) == TTIMER_MASK)
  {
    start_time = TCNT;
  }
  else
  {
    g_echo_time = TCNT - start_time;
  }
  //clears interrupt flag
  TFLG1 = TTIMER_MASK;
}

//----------------------------------------------------------------------------
// NAME: int* system_config
//
// DESCRIPTION:
//    This function runs the SCI functionality (putty) when the program is 
//    started. Each peripheral chosen for use has its interupt set (or global
//    var for garage) and a pin is created and passed into the main function.
//
// INPUT:
//   pin_store - a pointer to an int array that stores the customized pin
//
// OUTPUT: -  
//   
//
// RETURN:
//   none
//----------------------------------------------------------------------------
void system_config(int* pin_store) {
  // Variables
  bool done = FALSE;
  bool run = FALSE;
  bool invalid_option = TRUE;
  char character;
  int i = ZERO;
  int count = ZERO;
  bool innerLoopVar = FALSE;
  bool loopVar = FALSE;
  bool pinset = FALSE;
  // Initialize functions
  PLL_init();                             
  lcd_init();
  SCI1_init(SCI0VAL);
  ad0_enable();
  seg7_disable();  
  clear_lcd();
  alt_clear();
  //while loop cycles entire menu  
  while(!done)
  {  
    clear_lcd();
    myPrintln("");
    type_lcd("  Startup Req.");
    set_lcd_addr(LCD_LINE2_ADDR);
    type_lcd("  Goto Portal.");  
    displayMainMenu();
    character = get_sel_Num(PIN_SIZE);
    switch(character)                              
    {
      case CONFIG_PIN:                                         
        pin(pin_store);
        pinset = TRUE;
        break;
        
      case DEV_CONFIG:                      
        configDevice();
        
        break;
        
      case DEMO_MODE:                       
        presentation(pin_store);
        done = TRUE;
        break;
      // Terminate config  
      case ENDPROGRAM:
        if(pinset)
        {
          done = TRUE;
        }
        else
        {
          myPrintln("Not Configured"); 
        }
        break;
    }
  }
  myPrintln("System Configured.");
  seg7_disable();
  clear_lcd();
  set_lcd_addr(LCD_LINE1_ADDR);
  type_lcd(END_PROGRAM);
}

//main()
void main (void)
{  
  //local variables
  bool done = FALSE;
  bool run = FALSE;
  bool invalid_option = TRUE;
  int master_pin[PIN_SIZE];
  int keypad_read = ZERO;
  bool garage_active = FALSE;
  bool door_active = FALSE;
  bool window_active = FALSE;
  bool status_change = FALSE;
  bool alarm_active = FALSE;
  bool running = TRUE;
  bool garage_up = FALSE;
  bool windowOpen = FALSE;
  uint16 temp = ZERO;
  sint8* status_message = "WELCOME!";
  bool doorLocked = TRUE;
  //system configuration    
  PLL_init();                              
  lcd_init();
  RTI_init(); 
  PPSH = ZERO;
  DDRM = ZERO;
  SCI1_init(SCI0VAL);
  ad0_enable();
  ad1_enable();
  seg7_disable();
  keypad_enable();
  lcd_init();
  clear_lcd();
  alt_clear();
  while(!done)
  {
    PIEH = ZERO;
    PIFH = ZERO;
    //Create Pin/set up system
    system_config(master_pin);   
    //Determ alarm triggers desired and sets up interupt ports
    if (g_garage_flag == TRUE)
    {
      garage_active = TRUE;
      garage_up = FALSE;
    }
    g_garage_flag = FALSE;
    if ((PIEH & PTH_DOOR_BITMASK) == PTH_DOOR_BITMASK)
    {
      door_active = TRUE;
      PIFH |= (PTH_DOOR_BITMASK);
    }
    if ((PIEH & PTH_WIN_BITMASK) == PTH_WIN_BITMASK)
    {
      window_active = TRUE;
      PIFH |= (PTH_WIN_BITMASK);
    }  
    //setup master alarm functionality and activate
    DDRM |= ZERO;
    alarm_active = TRUE;
    //loop cycles between alarm active/ inactive
    while (running)
    {
      //while alarm is active, continuously prints status to screen
      //if status change is triggered, exits loop to address it 
      while(alarm_active)
      {
        seg7_disable();
        while((g_status_change == FALSE) && (alarm_active))       
        {
          set_lcd_addr(LCD_LINE1_ADDR);
          type_lcd(ACTIVE_ALARM);
          set_lcd_addr(LCD_LINE2_ADDR);
          type_lcd(status_message);
          //if statement adds details to temperature script
          if (status_message[ZERO] == TEMP_SCRIPT[ZERO])
          {
            temp = getTemp();
            write_int_lcd(temp);
            data8(DEGREE);
            type_lcd("F    ");
          }
          //motion sensor check
          spotlight();
          // if keypad read, request pin/trigger alarm
          keypad_read = keyscan();
          if (keypad_read != 16)
          {
            wait_keyup();
            disarm(master_pin);
            alarm_active = FALSE;
          }
        }
        //if status_change is triggered, if statements below
        //address the peripheral needing servicing
        if (g_door_flag) 
        {
          doorLocked = doorLock(doorLocked);
          status_message = display_status(door_active, window_active,
          garage_active, DOOR_DEV);
          g_door_flag = FALSE;
          if (doorLocked == FALSE)
          { 
            disarm(master_pin);
            alarm_active = FALSE;
          }
        }
        if (g_window_flag) 
        {
          status_message = display_status(door_active, window_active,
          garage_active, WIN_DEV);
          g_window_flag = FALSE;
          if (windowOpen)
          {
            windowOpen = FALSE;
          }
          else
          {
            windowOpen = TRUE;
            disarm(master_pin);
            alarm_active = FALSE;
          }
        }
        // garage if statement tracks status of door and ignores
        //requests to current disposition
        if (g_garage_flag) 
        {
          if(garage_request_up)                                                                  
          {                                                                                     
            if (!garage_up)
            {
              garage_go_up();                                                                     
              g_garage_flag = FALSE;
              garage_up = TRUE;
              status_message = display_status(door_active, window_active,
              garage_active, GAR_DEV);
              disarm(master_pin);
              alarm_active = FALSE;
            }
            else
            {
              status_message = display_status(door_active, window_active,
              garage_active, SCROLL);
            }
          }
          else                                                                                  
          {                                                                                     
            if (garage_up)
            {
              garage_go_down();                                                                   
              g_garage_flag = FALSE;
              garage_up = FALSE;
              status_message = display_status(door_active, window_active,
              garage_active, GAR_DEV);
            }
            else
            {
              status_message = display_status(door_active, window_active,
              garage_active, SCROLL);
            }
          }  
        }
        if (g_update_message)
        {
          status_message = display_status(door_active, window_active,
          garage_active, SCROLL);
          g_update_message = FALSE;

        }
        if (g_alarm_activate)
        {
          ms_delay(TWO_HUN_MS_DELAY);
          g_alarm_activate = FALSE;
          disarm(master_pin);
          alarm_active = FALSE;
          
        }
        g_status_change = FALSE;   
      }
      //alarm inactive loop behaves similarly to alarm active loop
      //however pin not required for most status changes
      while(alarm_active == FALSE)
      {
        seg7_disable();
        while((g_status_change == FALSE) && (alarm_active == FALSE))
        {
          set_lcd_addr(LCD_LINE1_ADDR);
          type_lcd(INACTIVE_ALARM);
          set_lcd_addr(LCD_LINE2_ADDR);
          type_lcd(status_message);
          if (status_message[ZERO] == TEMP_SCRIPT[ZERO])
          {
            temp = getTemp();
            write_int_lcd(temp);
            data8(DEGREE);
            type_lcd("F   ");
          }
          keypad_read = keyscan();
          //if alarm engaged, provides warnings about 
          //potential security issues
          if (keypad_read == STAR)
          {
            alarm_active = TRUE;
            if ((door_active) && (doorLocked == FALSE))
            {
              set_lcd_addr(LCD_LINE1_ADDR);
              type_lcd("Door Status:    " );
              set_lcd_addr(LCD_LINE2_ADDR);
              type_lcd("Not Secure      ");
              ms_delay(TWO_SEC_DELAY);
            }
            if ((window_active) && (windowOpen == TRUE))
            {
              set_lcd_addr(LCD_LINE1_ADDR);
              type_lcd("Window Status:  ");
              set_lcd_addr(LCD_LINE2_ADDR);
              type_lcd("Not Secure      ");
              ms_delay(TWO_SEC_DELAY);
            }
            if ((garage_active) && (garage_up))
            {
              set_lcd_addr(LCD_LINE1_ADDR);
              type_lcd("Garage Status:  ");
              set_lcd_addr(LCD_LINE2_ADDR);
              type_lcd("Not Secure      ");
              ms_delay(TWO_SEC_DELAY);
            }
          }
          //motion sensor check
          spotlight();
        }
        //Conditions to be addressed outside of display while loop
        if (g_door_flag) 
        {
          doorLocked = doorLock(doorLocked);
          g_door_flag = FALSE;
          status_message = display_status(door_active, window_active,
          garage_active, DOOR_DEV);
        }
        if (g_window_flag) 
        {
          g_window_flag = FALSE;
          if (windowOpen)
          {
            windowOpen = FALSE;
          }
          else
          {
            windowOpen = TRUE;
          }
          status_message = display_status(door_active, window_active,
          garage_active, WIN_DEV);
        }
        if (g_garage_flag) 
        {
          if(garage_request_up)                                                                  
          {                                                                                     
            if (!garage_up)
            {
              garage_go_up();                                                                     
              g_garage_flag = FALSE;
              garage_up = TRUE;
              status_message = display_status(door_active, window_active,
              garage_active, GAR_DEV);
            }
            else
            {
              status_message = display_status(door_active, window_active,
              garage_active, ZERO);
            }
          }
          else                                                                                  
          {                                                                                     
            if (garage_up)
            {
              garage_go_down();                                                                   
              g_garage_flag = FALSE;
              garage_up = FALSE;
              status_message = display_status(door_active, window_active,
              garage_active, GAR_DEV);
            }
            else
            {
              status_message = display_status(door_active, window_active,
              garage_active, ZERO);
            }
          }
        }
        if (g_update_message)
        {
          status_message = display_status(door_active, window_active,
          garage_active, ZERO);
          g_update_message = FALSE;
        }
        if (g_alarm_activate)
        {
          ms_delay(TWO_HUN_MS_DELAY);
          g_alarm_activate = FALSE;
          alarm_active = TRUE;
          if ((door_active) && (doorLocked == FALSE))
          {
            set_lcd_addr(LCD_LINE1_ADDR);
            type_lcd("Door Status:    " );
            set_lcd_addr(LCD_LINE2_ADDR);
            type_lcd("Not Secure      ");
            ms_delay(TWO_SEC_DELAY);
          }
          if ((window_active) && (windowOpen == TRUE))
          {
            set_lcd_addr(LCD_LINE1_ADDR);
            type_lcd("Window Status:  ");
            set_lcd_addr(LCD_LINE2_ADDR);
            type_lcd("Not Secure      ");
            ms_delay(TWO_SEC_DELAY);
          }
          if ((garage_active) && (garage_up))
          {
            set_lcd_addr(LCD_LINE1_ADDR);
            type_lcd("Garage Status:  ");
            set_lcd_addr(LCD_LINE2_ADDR);
            type_lcd("Not Secure      ");
            ms_delay(TWO_SEC_DELAY);
          }
        } 
        g_status_change = FALSE;
      }
    }
  }  
}

//----------------------------------------------------------------------------
// NAME: void disarm
//
// DESCRIPTION:
//    This function sets the message to be displayed on the LCD, as well as 
//    maintaining status of peripheral for correct status'. A status change
//    is represented with the number for each peripheral [door(1) window(2)
//    and garage(3)]. a job 0 (ZERO) indicates a cyclical reading of all 
//    peripheral status (for LCD) allowing the function to serve for 2 
//    distinct  yet connected tasks.
//
// INPUT:
//   bool door - status representing use of door monitor
//   bool window - status representing use of window monitor
//   bool garage - status representing use of garage monitor
//
// OUTPUT:
//   none.
//   
//
// RETURN:
//   The status message requested/queued
//----------------------------------------------------------------------------
void disarm(int* unlock_pin)                                                                                                            
{
  bool no_input = FALSE;
  int i = ZERO;
  int pin_attempt;
  int pin_guess[PIN_SIZE];
  
  bool validated = TRUE;
  bool access_denied = TRUE;
  uint8 temp_ptb = PORTB;
  
  set_lcd_addr(LCD_LINE1_ADDR);
  type_lcd("Enter your PIN  ");
  set_lcd_addr(LCD_LINE2_ADDR);
  type_lcd("                ");
  while (access_denied)
  {
    g_timeout = TRUE;
    validated = TRUE;
    for(i = ZERO; i < PIN_SIZE; i++)
    {
      no_input = TRUE;
      while(no_input)
      {
        pin_attempt = keyscan();
        if(pin_attempt < LIMIT_DIGITS_TO_NINE)
        {
          wait_keyup();
          no_input = FALSE;
        }
        if (g_timeout == FALSE)
        {
          g_alarm_trigger = TRUE;
        }
      }
      pin_guess[i] = pin_attempt;
    }
    for (i = ZERO; i < PIN_SIZE; i++) 
    {
      if (unlock_pin[i] != pin_guess[i])
      {
        validated = FALSE;
      }
      if (g_timeout == FALSE)
      {
        g_alarm_trigger = TRUE;
      }
    }
    if (validated)
    {
      g_timeout = FALSE;
      access_denied = FALSE;
      g_alarm_trigger = FALSE;
      motor5(ZERO);
      led_disable();
      PORTB = temp_ptb;
    }
    else
    {
      set_lcd_addr(LCD_LINE2_ADDR);
      type_lcd("Invalid Attempt");
      ms_delay(TWO_SEC_DELAY);
      set_lcd_addr(LCD_LINE2_ADDR);
      type_lcd("                ");
      if (g_timeout == FALSE)
      {
        g_alarm_trigger = TRUE;
      }
    }
  }
}  
                                                                                

//----------------------------------------------------------------------------
// NAME: void spotlight
//
// DESCRIPTION:
//    This function sets the message to be displayed on the LCD, as well as 
//    maintaining status of peripheral for correct status'. A status change
//    is represented with the number for each peripheral [door(1) window(2)
//    and garage(3)]. a job 0 (ZERO) indicates a cyclical reading of all 
//    peripheral status (for LCD) allowing the function to serve for 2 
//    distinct  yet connected tasks.
//
// INPUT:
//   bool door - status representing use of door monitor
//   bool window - status representing use of window monitor
//   bool garage - status representing use of garage monitor
//
// OUTPUT:
//   none.
//   
//
// RETURN:
//   The status message requested/queued
//----------------------------------------------------------------------------
void spotlight(void)
{
  double distance = ZERO;
  TSCR2 = ZERO;
  TIOS = ZERO;
  TIE = ZERO;
  TSCR1 = ZERO;
  TCTL1 = ZERO;
  TCTL2 = ZERO;
  TCTL3 = ZERO;
  TCTL4 = ZERO; 
  TIOS |= CHANNEL4_MODE;
  TSCR2 |= TIMER_PRESCALER_TWO;
  TSCR1 |= TIMER_ENABLE;
  TCTL4 = BIT_TWO_UP_DWN_EDGE_TRIG;
  TIE |= BIT_TWO_MASK;
  TCTL1 = TRIGGER_HIGH;
  CFORC = FORCE_BIT_FOUR;
  TCTL1 = TRIGGER_LOW;
  TC4 = TCNT + NEXT_ISR_TIME;
  ms_delay(FIVE_MS_DELAY);
  distance = ((double)g_echo_time * SPEED_OF_SOUND)/CYCLES_PER_SECOND ;
  distance = distance/TWO_WAY_DISTANCE_REDUCTION;
  distance = distance * CM_CONVERSION;
  seg7_disable();
  if(distance <= DISTANCE_TOO_CLOSE)
  {
    led_enable();
    PORTB = OUTPUT;
  }
  else
  {
    led_disable();
    PORTB = OFF;
  }
  TSCR2 = ZERO;
  TIOS = ZERO;
  TIE = ZERO;
  TSCR1 = ZERO;
  TCTL1 = ZERO;
  TCTL2 = ZERO;
  TCTL3 = ZERO;
  TCTL4 = ZERO; 
}

//----------------------------------------------------------------------------
// NAME: sint8* display_status
//
// DESCRIPTION:
//    This function sets the message to be displayed on the LCD, as well as 
//    maintaining status of peripheral for correct status'. A status change
//    is represented with the number for each peripheral [door(1) window(2)
//    and garage(3)]. a job 0 (ZERO) indicates a cyclical reading of all 
//    peripheral status via a static counter (for LCD) allowing the function 
//    to serve for 2 distinct  yet connected tasks.
//
// INPUT:
//   bool door - status representing use of door monitor
//   bool window - status representing use of window monitor
//   bool garage - status representing use of garage monitor
//   int job - tells function to update status or cycle message
//
// OUTPUT:
//   none.
//   
//
// RETURN:
//   The status message requested/queued
//----------------------------------------------------------------------------
sint8* display_status(bool door, bool window, bool garage, int job)
{
  
  static int message_number = 1;
  static bool garage_opened = FALSE;
  static bool window_opened = FALSE;
  static bool door_lockeded = TRUE;
  sint8* message;
  //uint16 temp;
  //indicates a status change, jump to script message
  if (job != ZERO)
  {
    message_number = job;
  }
  if (message_number == DOOR_DEV)
  {
    //check if door settings in use
    if (door)
    {
      //check if status change, update bool and save message
      if ((job == ZERO))
      {
        if (door_lockeded)
        {
          door_lockeded = FALSE;
          message = DOOR_UNLOCKED; 
        }
        else
        {
          door_lockeded = TRUE;
          message = DOOR_LOCKED; 
        }
      }
      //display message, no change in status (ZERO)
      else
      {
        if (door_lockeded)
        {
          message = DOOR_LOCKED;
        }
        else
        {
          message = DOOR_UNLOCKED;
        }
      }
    }
    //door settings not in use, move on
    else
    {
      message_number++;
    }
  }
  if (message_number == WIN_DEV)
  {
    if (window)
    {
      if (job == WIN_DEV)
      {
        if (window_opened)
        {
          window_opened = FALSE;
          message = WINDOW_CLOSED;
        }
        else
        {
          window_opened = TRUE;
          message = WINDOW_OPEN;
        }
      }
      else
      {
        if (window_opened)
        {
          message = WINDOW_OPEN;
        }
        else
        {
          message = WINDOW_CLOSED;
        }
      }
    }
    else
    {
      message_number++;
    }
  }
  if (message_number == GAR_DEV)
  {
    if (garage)
    {
      if (job == GAR_DEV)
      {
        if (garage_opened)
        {
          garage_opened = FALSE;
          message = GARAGE_CLOSED;
        }
        else
        {
          garage_opened = TRUE;
          message = GARAGE_OPEN;
        }
      }
      else
      {
        if (garage_opened)
        {
          message = GARAGE_OPEN;
        }
        else
        {
          message = GARAGE_CLOSED;
        }
      }
    }
    else
    {
      message_number++;
    }
  }
  if(message_number == TEMP_MSG)
  {
    //check temp and set to message
    message = TEMP_SCRIPT;
    message_number = ZERO;
  }
  message_number++;
  return ((sint8*)(message));
}//end method
   
//----------------------------------------------------------------------------
// NAME: displayMainMenu
//
// DESCRIPTION:
//    This function prints a string message to the terminal window. The 
//    string message will be a menu list of options.
//
// INPUT:
//   none
//
// OUTPUT: -  the string message menu.
//   
//
// RETURN:
//   none
//----------------------------------------------------------------------------
void displayMainMenu(void)
{
  myPrintln("Menu:");
  myPrintln("  1. Configure PIN");
  myPrintln("  2. Device Configuration");
  myPrintln("  3. DEMO Mode");
  myPrintln("  4. Exit Setup");
  myPrintln("Enter your selection: ");
}

//----------------------------------------------------------------------------
// NAME: displayConfigMenu
//
// DESCRIPTION:
//    This function prints a string message to the terminal window. The 
//    string message will be a menu list of options.
//
// INPUT:
//   none
//
// OUTPUT: -  the string message menu.
//   
//
// RETURN:
//   none
//----------------------------------------------------------------------------
void displayConfigMenu(void)
{
  myPrintln("Menu/n/rPlease Select a device to Enable/Disable:");
  myPrintln("  1. Main Entrance Lock");
  myPrintln("  2. Garage Control");
  myPrintln("  3. Window Security");
  myPrintln("  4. Return to Main Menu");
  myPrintln("Enter your selection: ");
}

//----------------------------------------------------------------------------
// NAME: getTemp
//
// DESCRIPTION:
//    This function gets the current temperature and returns it where called
//
// INPUT:
//   none
//
// OUTPUT:
//   none
//
// RETURN:
//   temp_var - temperature data
//----------------------------------------------------------------------------
uint16 getTemp(void)                                                                       
{                                                                                          
  uint16 temp_var;                                                                         
  temp_var = ad0conv(THERMOMETER);                                                                   
  temp_var /= TWO;                                                                           
  temp_var = (uint16)(temp_var *
  ((double)TEMP_CONVERSION_FACTOR_TWO/TEMP_CONVERSION_FACTOR_THREE)
  + TEMP_CONVERSION_FACTOR_ONE);  // Convert from C to F                  
  return temp_var;                                                                        
}   

//----------------------------------------------------------------------------
// NAME: garage_go_up
//
// DESCRIPTION:
//    This function activates the garage, causing it to open.
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
void garage_go_up(void)                                                                      
{                                                                                            
  uint8 portB_capture = PORTB;                                                               
  uint8 portP_capture = PTP;
  uint8 portP_dir = DDRP;
  uint8 portB_dir = DDRB;
  DDRP = OUTPUT;
  DDRB = OUTPUT;
  PTP = GARAGE_PTP_SETUP;                                                                 
  motor0_init();                                                                             
  PORTB = GARAGE_ENABLE_UP;                                                                             
  motor0(GARAGE_SPEED);                                                                      
  ms_delay(FIVE_SEC_DELAY);                                                                         
  motor0(ZERO); 
  PWME = ZERO;                                                                               
  PORTB = portB_capture;                                                                    
  PTP = portP_capture; 
  DDRB = portB_dir;
  DDRP = portP_dir;
  seg7_disable();                                                                     
}                                                                                            
                                                                                             
//----------------------------------------------------------------------------               
// NAME: garage_go_down                                                                      
//                                                                                           
// DESCRIPTION:                                                                              
//    This function activates the garage, causing it to close.                                         
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
void garage_go_down(void)                                                                    
{                                                                                            
  uint8 portB_capture = PORTB;                                                               
  uint8 portP_capture = PTP;
  uint8 portP_dir = DDRP;
  uint8 portB_dir = DDRB;
  DDRP = OUTPUT;
  DDRB = OUTPUT;
  PTP = GARAGE_PTP_SETUP;                                                                 
  motor0_init();                                                                            
  PORTB = GARAGE_ENABLE_DOWN;                                                                             
  motor0(GARAGE_SPEED);                                                                     
  ms_delay(FIVE_SEC_DELAY);                                                                          
  motor0(ZERO);
  PWME = ZERO;                                                                                
  PORTB = portB_capture;                                                                     
  PTP = portP_capture; 
  DDRB = portB_dir;
  DDRP = portP_dir;
  seg7_disable();                                                                       
}                                                                                            
                                                                                          

//----------------------------------------------------------------------------
// NAME: myPrintln
//
// DESCRIPTION:
//    This function prints a string message to the terminal window. The 
//    string message can be up to 70 characters long.
//
// INPUT:
//   message - the message programmer wants to display to Putty
//
// OUTPUT:
//   message - the message programmer wants to display to Putty
//
// RETURN: 
//   none
//----------------------------------------------------------------------------
void myPrintln(sint8* message)
{
  uint16 idx = ZERO;
  while(message[idx] != '\0')
  {
    outchar1(message[idx++]);      
  }
  outchar1(RETURN);
  outchar1(NEXT_LINE);
}


//----------------------------------------------------------------------------
// NAME: getNum
//
// DESCRIPTION:
//    This function returns a number entered from an SCI connection (PUTTY).
//    The data is converted into an int before it is returne and the function
//    loops until a number is entered.
//
// INPUT:
//   none
//
// OUTPUT:
//   invalid response - will indicate that user inputted wrong input.
//
// RETURN: 
//   character - an int entered by the user 
//----------------------------------------------------------------------------
int getNum(void)
{
  char character;
  bool NUM = FALSE;
  
  while (!NUM)
  {
    character = inchar1();
    if(!((character == '1') || (character == '2') || (character == '3') || 
    (character == '4') || (character == '5') || (character == '6') || 
    (character == '7') || (character == '8') || (character == '9') || 
    (character == '0')))
    {
      myPrintln(INVALID_INPUT);
    }
    else
    {
      NUM = TRUE;
    }
  }
  character-=ASCII_VALUE;
  return (int) character;
}

//----------------------------------------------------------------------------
// NAME: getNum
//
// DESCRIPTION:
//    This function returns a character the user inputs. and ensures a number
//    in the specificed range (upperbound) is returned as a char.. This is 
//    meant for use in Menu navigation.
//
// INPUT:
//   none
//
// OUTPUT:
//   invalid response - will indicate that user inputted wrong input.
//
// RETURN: 
//   signed integer pointer with a character array stored inside of it.
//----------------------------------------------------------------------------
char get_sel_Num(int max_allowed)
{
  char character;
  int i = ZERO;
  bool NUM = FALSE;
  char num_array[] = 
  {
    '0','1', '2', '3', '4', '5', '6', '7', '8', '9', '0'
  };
  while (!NUM)
  {
    character = inchar1();
    outchar1(character);
    myPrintln("");
    for (i = ONE; i <= max_allowed; i++)
    {
      if ((character) == num_array[i])
      {
        NUM = TRUE;
      }
    } 
    if(NUM == FALSE)
    {
      myPrintln(INVALID_INPUT);
    }

  }
  return character;
}
 //----------------------------------------------------------------------------
// NAME: int* pin
//
// DESCRIPTION:
//    This function returns a pin number (a pointer to a 4 digit int[])
//    is returned as a char.
//
// INPUT:
//   none
//
// OUTPUT:
//   invalid response - will indicate that user inputted wrong input.
//
// RETURN: 
//   *pin - int[] (4 digits)
//----------------------------------------------------------------------------
void pin(int* pin) 
{
   int i = ZERO;
   int j = ZERO;
   int p = ZERO;
   int user_pin[PIN_SIZE];
   int validate_pin[PIN_SIZE];
   
   bool innerLoopVar = FALSE;
   bool loopVar = FALSE;
  
 
   while(!loopVar)
          {
            myPrintln("");
            myPrintln("Enter your PIN: ");
            myPrintln("");  
            while(!innerLoopVar)
            {
              for(i = ZERO; i < PIN_LENGTH; i++)
              {
                validate_pin[i] = getNum();
                outchar1('*');
              }
              innerLoopVar = TRUE;
            }
            myPrintln("");
            innerLoopVar = FALSE;
            myPrintln("Re-enter your PIN: ");
            for(i = ZERO; i < PIN_LENGTH; i++)
            {
              user_pin[i] = getNum();
              outchar1('*'); 
            }
            loopVar = TRUE;
            
            // VALIDATES USER ENTERED PIN TO RE-ENTRY
            for (i = ZERO; i < PIN_SIZE; i++)
            {
              if(validate_pin[i] != user_pin[i])
              {
                loopVar = FALSE;
              }
            }
            if (loopVar == FALSE)
            {
              myPrintln("Pin does not match. Please try again");
              ms_delay(ONE_SEC_DELAY);
            }
            else
            {
              for(i = ZERO; i < PIN_SIZE; i++)
              {
                pin[i] = user_pin[i];
              }
            }
          }
          for(i = ZERO; i < PIN_SIZE; i++)
          {
            outchar1((char)pin[i]);
          }
          myPrintln("");
          myPrintln("Your PIN has been set.");
}

//----------------------------------------------------------------------------
// NAME: configDevice
//
// DESCRIPTION:
//    This function handles the initial setup of the program through SCI
//    PUTTY.
//
// INPUT:
//   none
//
// OUTPUT:
//   invalid response - will indicate that user inputted wrong input.
//
// RETURN: 
//   signed integer pointer with a character array stored inside of it.
//----------------------------------------------------------------------------
void configDevice(void)
{
  bool done = FALSE;
  char character;
  while(!done)
  {  
    displayConfigMenu();
    character = get_sel_Num(4);
    switch(character)                               
    {
      case SELECT_DOOR:                       
        if ((PTH_DOOR_BITMASK & PIEH) == (PTH_DOOR_BITMASK))
        {
          PIEH &= (~PTH_DOOR_BITMASK);
          myPrintln("");
          myPrintln("Door Disabled.");
          myPrintln("");
        }
        else
        {
          PIEH |= PTH_DOOR_BITMASK;
          myPrintln("");
          myPrintln("Door Enabled.");
          myPrintln("");
        }
        break;
        
      case SELECT_GARAGE:                      
          if (g_garage_flag == FALSE)
          {
            g_garage_flag = TRUE;
            myPrintln("");
            myPrintln("Garage Enabled.");
            myPrintln("");
          }
          else
          {
            g_garage_flag = FALSE;
            myPrintln("");
            myPrintln("Garage Disabled.");
            myPrintln("");
          }
        break;
        
      case SELECT_WINDOW:
        if ((PTH_WIN_BITMASK & PIEH) == (PTH_WIN_BITMASK))
        {
          PIEH &= (~PTH_WIN_BITMASK);
          myPrintln("");
          myPrintln("Window Security Disabled.");
          myPrintln("");
        }
        else
        {
          PIEH |= PTH_WIN_BITMASK;
          myPrintln("");
          myPrintln("Window Security Enabled.");
          myPrintln("");
        }
        break;
      case RETURN_MENU:                      // Terminate program
        done = TRUE;
        break;
    }
  }
}
//----------------------------------------------------------------------------
// NAME: presentation
//
// DESCRIPTION:
//    This function is used to set up "Demo Mode" for presenting the features
//    of this program. This function auto assigns a pin and enables all 
//    features.
//
// INPUT:
//   pin - variable used for returning the default pin created
//
// OUTPUT:
//   none
//
// RETURN: 
//   none
//----------------------------------------------------------------------------
void presentation(int* pin)
{
  int i;
  for (i = ZERO; i < PIN_SIZE; i++)
  {
    pin[i] = 2;
  }
  g_garage_flag = TRUE;
  PIEH |= PTH_DOOR_BITMASK;
  PIEH |= PTH_WIN_BITMASK;
  myPrintln("");
  myPrintln("Door Enabled.");
  myPrintln("");
  myPrintln("Garage Enabled.");
  myPrintln("");
  myPrintln("Window Security Enabled.");
  myPrintln("");
}

//----------------------------------------------------------------------------          
// NAME: doorLock                                                                       
//                                                                                      
// DESCRIPTION:                                                                         
//    This function locks the door.                                                     
//                                                                                     
// INPUT:                                                                              
//   bool - the status of the lock                                                      
//                                                                                      
// OUTPUT:                                                                              
//   none                                                                               
//                                                                                      
// RETURN:                                                                              
//   none                                                                               
//----------------------------------------------------------------------------
bool doorLock(bool locked)
{
  uint8 temp_ddrp = DDRP;
  uint8 temp_ptp = PTP;
  uint8 temp_ptb = PORTB;
  int position;
  DDRP  = OUTPUT;
  PTP = ENABLE_MOTOR;
  PTM &= LOCK_ENABLE;
  DDRB = OUTPUT;
  PORTB = ZERO;
  ad0_enable();
  servo76_init();
  if(locked)
  {
    position = ONEEIGHTY_DEGREE;
    set_servo76(position);
    locked = FALSE; 
  }
  
  else
  {
    position = ZERO_DEGREE;
    set_servo76(position);
    locked = TRUE; 
  }
  return locked;
}

