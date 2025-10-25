#  Home Security System

This project stationery is designed to get you up and running
quickly with CodeWarrior for MC9S12DG256B.
It is set up for the selected CPU and target connection,
but can be easily modified.

Sample code for the following language(s) is at your disposal:
- Assembly
- C

The wizard has prepared CodeWarrior target(s) with the connection methods
of your choice:
- Freescale Full Chip Simulation:
  Select this connection to use the HCS12\HCS12X Full Chip Simulation.
  This connection also allows to use the Freescale Instruction Set Simulator.
  In case Full Chip Simulation for the selected derivative is not available,
  a basic Full Chip Simulation is available.   

- HCS12 Serial Monitor:
  This Connection allows to debug with the on-chip Freescale HCS12 serial monitor.
  This monitor uses on-chip resources and the SCI. See Freescale
  application note AN2140 for details.
  Using the on-chip DBG module of HCS12 devices (if available, 
  not present on all HCS12 derivatives), trace is supported
  You can load the trace component using the menu Component > Open in the debugger.


Additional connections can be chosen in the simulator/debugger,
use the menu Component > Set Target.
In the debugger menu Component > Open you can load additional components.

##  Project Summary

The Home Security System uses a variety of peripherals to keep your home secure. Secured with your custom made PIN code, you can keep your home safe. Lock your front door, open or close your garage door, and secure your home’s doors and windows with our PIN protected alarm. When your alarm is set, simply enter your 4 digit PIN to disarm the house within 15 seconds of entry. Our state of the art alarm will trigger an automated report to our systems during an unwanted trespassing. We will call to check with you, the owner, to see if this alarm was unwanted and call the authorities automatically reporting the incident of a break-in. 

Separating us from the competition would be our garage door monitoring system. If someone tries to force entry through your garage door, we will catch such action. Live reporting to your Home Security System, secure your external, or internal garage. 

While, on its own, this is just proof of concept, there are several included features to simulate this design’s use for a home. Features included: 

·        Alarm buzzer – after invalid attempts breaching the perimeter 

·        Flood Light – activated by motion sensor 

·        Door lock – By keypad to unlock/lock the front door 

·        Window monitoring – This system will track the status of all entryways 

·        Garage control – Open or close the garage 

·        Status display – Monitor the status of your entryways – and the temperature too. 

Below is a list of what is needed to build this device: 

1.    Dragonboard                                 x1 

2.    Ultrasonic Ranging Module                   x1 

3.    Joystick                                    x1 

4.    Servo Motor                                 x1 

5.    DC Motor                                    x1 

6. External Power supply                          x1 

7. Breadboard Power Module        	              x1 

8. Push button                                    x1 

9. Keypad                                         x1 

17. Breadboard                                    x1 

The ultrasonic ranging module serves as motion detection, triggering the floodlight (LED). The DC motor, motor driver chip and joystick simulate controlling a garage door in each direction. The pushbuttons simulate opening/closing a window and activating/deactivating the alarm. The servo motor simulates locking/unlocking the main entrance. The buzzer sounds the alarm when needed, while the LCD, temperature and humidity sensor, and keypad interact with the user to provide status updates, weather readings, and allows the user to enter their pin to enable/disable the alarm. 

##  Simulator/Debugger: Additional components

In the simulator/debugger, you can load additional components. Try the
menu Component > Open.

## Future Work Needed
In the future, we plan to integrate the following peripherals and security protocols: 

- RFID tag entry 
  - Enter your home with the ability to disarm your device using a RFID tag. 
- Video authentication 
  - Authenticate and unlock your home for visitors using a Video feed. 
- Online Hub access to home features 
  - See the status of your Home Security System through an online portal. 
- Home automation 
  - Control features that exist on your Home Security System through automated processes. 
- Wireless Garage Door connection 
  - With the power of a Zigbee wireless controller, we would get rid of those messy wires connecting your garage door opener to your Home Security System. 
- Mobile access to your Online Hub. 
  - On your mobile device, view and set the status of your Home Security System. 

##  Additional Documentation

Read the online documentation provided. Use in CodeWarrior IDE the menu 
Help > CodeWarrior Help.

Design Specification Manual to be posted


##  Contacting Metrowerks
For bug reports, technical questions, and suggestions, please use the
forms installed in the Release_Notes folder and send them to:
cw_support@freescale.com

### CREDIT TO James Homan AND Josh McLamore
