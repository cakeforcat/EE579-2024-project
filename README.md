# EE579-2024-project

A project which focused on the development of an autonomous RC car which is capable of playing a game of skittles. The rules of the game are simple: knock over the black cans as fast as possible while avoiding the white cans to prevent disqualification.

/RC Car 2024: Includes all project files required to configure the vehicle's peripherals and carry out the movement algorithm. A CCS workspace configured for loading code onto the MSP430G2553 is also included in the folder.
/TestingComponents: A python application which may be used to ensure correct operation of the vehicle's peripherals. 

User Guide

The repository includes both the project files and a CCS workspace configured for running the software on the MSP430G2553. Once downloaded, the workspace can be easily loaded on CCS File > Switch Workspace > Select downloaded folder>Open. Alternative IDE users must instead import each of the source and header files into a correctly configured workspace. 

A connection diagram necessary for flashing code, debugging, and UART communication is provided below.  

![Picture1](https://github.com/cakeforcat/EE579-2024-project/assets/92934215/1a149301-9818-4f72-b172-a0051508d0a2)

Using a MSP430 launchpad, connect the SBWCLK and SBWTDIO pins on the eZ-FET side of the launchpad to the respective jumpers on the car. Additionally, a common ground can be provided by connected the GND pin on the launchpad to the appropriate jumper.  Use the switch located at the back of the car, and switch to the on position prior to flashing code. The 3.3V connection is not required when the switch is in the ON position and the system is powered by the battery. When utilising UART functionality, the Rx pin on the launchpad’s eZ-FET side is connected to the vehicles Tx inlet. Consequently, the launchpad’s Tx pin is then connected to vehicle’s Rx pin. This allows for two-way communication with the vehicle, allowing the system to receive and transmit data. These UART connections are not necessary when flashing code without any UART requirements.

Preview of Car

A look at the build
![Picture2](https://github.com/cakeforcat/EE579-2024-project/assets/92934215/f44f22cb-0318-4a5a-a6d9-a51287e41bfb)

Knocking over a black can


https://github.com/cakeforcat/EE579-2024-project/assets/92934215/37cdb7a4-683a-428a-8e6a-4f19128a56d4



Avoiding a white can



https://github.com/cakeforcat/EE579-2024-project/assets/92934215/9e5caf48-db68-45de-914f-3f480af79177







