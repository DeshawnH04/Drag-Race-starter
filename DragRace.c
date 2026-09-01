// DragRace.c
// Kairi Rodriguez, Estefania, Alex, Deshawn
// Group 8
// October 26, 2025

#include "tm4c123gh6pm.h"
#include <stdint.h> // C99 data types
#include <stdbool.h> // provides boolean data type
#include "PLL.h"
#include "SysTick.h"
#include "Sensors_Lights.h"

// Function Prototypes
// External functions from startup.s
extern void DisableInterrupts(void); // Disable interrupts
extern void EnableInterrupts(void);  // Enable interrupts
extern void WaitForInterrupt(void);  // Go to low power mode while waiting for the next interrupt


// Functions defined in this file.
void System_Init(void);

// TODO: define bit addresses for two sensors, 8 color lights, and one reset button 
#define SENSORS (*((volatile unsigned long *) 0x40004030)) // bit addresses for 2 sensors 
#define LIGHTS  (*((volatile unsigned long *) 0x400053FC)) // bit addresses for 8 Race Lights 
#define RESET   (*((volatile unsigned long *) 0x40024020)) // bit address for one reset button

// TODO: define number of states for FSM
#define NUM_STATE            (11U)

// TODO: FSM definition
struct State { 
	uint8_t Out;
	uint8_t Time;     // multiple of 0.5 second
	uint8_t Next[4];  
};

typedef const struct State STyp;

// TODO: define reload value for half second
#define HALF_SEC   (8000000U)

// TODO: assign a value to all states in Drag Race FSM
// use all upper case for constants defined here
enum DragRace_states {
	INITIALIZE,
	WAIT_FOR_STAGING,
	COUNTDOWN_Y1,
	COUNTDOWN_Y2,
	GO,
	WIN_LEFT,
	WIN_RIGHT,
	WIN_BOTH,
	FALSE_START_LEFT,
	FALSE_START_RIGHT,

	FALSE_START_BOTH
};

// TODO: define Outputs for the FSM 
// Needs to be double checked please!!! 

//00001100 <= green LED
//11110000 <= yellow LED
//00000011 <= red LED

#define ALL_ON 	    0xFF  // Turns on all the LEDs 
#define ALL_OFF 	0x00  // Turns off all the LEDs 
#define YELLOW1_ON	(0xC0) // Turns on 1st set of yellow LEDs 
#define YELLOW2_ON  (0x30) // Turns on 2nd set of yellow LEDs
#define GREEN_BOTH  (0x0C) // Turns on the green LEDs 
#define GREEN_LEFT  (0x08) // Turns on left green LED
#define GREEN_RIGHT (0x04) // Turns on right green LED
#define RED_BOTH    (0x03) // Turns on both red LEDS
#define RED_LEFT    (0x02) // Turns on left red LED
#define RED_RIGHT   (0x01) // Turns on right red LED 

//TODO: Define Drag Race FSM
STyp DragRace_FSM[] = {};

  // INITIALIZE - State 0: All lights flash
    {ALL_ON, 2, {WAIT_FOR_STAGING, WAIT_FOR_STAGING, WAIT_FOR_STAGING, WAIT_FOR_STAGING}},
    
    // WAIT_FOR_STAGING - State 1: Wait for both racers
    {ALL_OFF, 1, {WAIT_FOR_STAGING, WAIT_FOR_STAGING, WAIT_FOR_STAGING, COUNTDOWN_Y1}},
    
    // COUNTDOWN_Y1 - State 2: First yellow (2 sec)
    {YELLOW1_ON, 4, {COUNTDOWN_Y2, FALSE_START_RIGHT, FALSE_START_LEFT, FALSE_START_BOTH}},
    
    // COUNTDOWN_Y2 - State 3: Second yellow (2 sec)
    {YELLOW2_ON, 4, {GO, FALSE_START_RIGHT, FALSE_START_LEFT, FALSE_START_BOTH}},
    
    // GO - State 4: Green lights, race begins
    {GREEN_BOTH, 4, {GO, WIN_RIGHT, WIN_LEFT, WIN_BOTH}},
    
    // WIN_LEFT - State 5
    {GREEN_LEFT, 4, {WAIT_FOR_STAGING, WAIT_FOR_STAGING, WAIT_FOR_STAGING, WAIT_FOR_STAGING}},
    
    // WIN_RIGHT - State 6
    {GREEN_RIGHT, 4, {WAIT_FOR_STAGING, WAIT_FOR_STAGING, WAIT_FOR_STAGING, WAIT_FOR_STAGING}},
    
    // WIN_BOTH - State 7
    {WINNER_BOTH, 4, {WAIT_FOR_STAGING, WAIT_FOR_STAGING, WAIT_FOR_STAGING, WAIT_FOR_STAGING}},
    
    // FALSE_START_LEFT - State 8
    {RED_LEFT, 4, {WAIT_FOR_STAGING, WAIT_FOR_STAGING, WAIT_FOR_STAGING, WAIT_FOR_STAGING}},
    
    // FALSE_START_RIGHT - State 9
    {RED_RIGHT, 4, {WAIT_FOR_STAGING, WAIT_FOR_STAGING, WAIT_FOR_STAGING, WAIT_FOR_STAGING}},
    
    // FALSE_START_BOTH - State 10
    {RED_BOTH, 4, {WAIT_FOR_STAGING, WAIT_FOR_STAGING, WAIT_FOR_STAGING, WAIT_FOR_STAGING}}
};

// TODO: define bit positions for left, right and reset buttons
#define RESET_MASK  	   0x08 // bit position for reset button
#define LEFT_SENSOR_MASK  (0x08) // bit position for left sensor
#define RIGHT_SENSOR_MASK (0x04) // bit position for left sensor
	
volatile uint8_t Input;
bool timesup;
bool reset;  // flag to reset the system, set by the reset button located at breadboard, not the launchpad reset button.
	
int main(void){
  uint8_t S;  // current state index
	
	System_Init();
		
  while(1){
    // TODO: reset FSM to its Initial state, reset globals to default values
     S = INITIALIZE ;
     reset = false ;
	 Input = 0x00 ;	
		
		while (!reset) {
			// TO Do: take care of FSM outputs and time in state.
			LIGHTS = DragRace_FSM[S].Out;
      SysTick_Start(DragRace_FSM[S].Time*HALF_SEC);
			while((!timesup)&&(!reset)){
			  WaitForInterrupt();
			}
			timesup=false;
			S = DragRace_FSM[S].Next[Input];
		}
	SysTick_Stop();	
  }
}

// Initialize the following hardware modules: PLL, Sensors, Reset button, Lights, SysTick timer
// Initialize the following golbal variables: timesup for FSM state time, reset for reset the whole system
void System_Init(void) {
	DisableInterrupts();
	PLL_Init();
  Sensors_Init(); 
	Reset_Init(); 
	Lights_Init();
	SysTick_Init(); 
  // TODO: reset global variables: timesup, reset, Input 
	Input = 0x00;
	reset = false;	
	timesup = false;		
	EnableInterrupts();
}

// Interrupt handler for the two sensors: update Input here 
void GPIOPortA_Handler(void){
	for (uint32_t time=0;time<100000;time++) {}
		if (GPIO_PORTA_RIS_R & LEFT_SENSOR_MASK){
			Input = SENSORS >>2;
			GPIO_PORTA_ICR_R = LEFT_SENSOR_MASK;
		}
		else if (GPIO_PORTA_RIS_R & RIGHT_SENSOR_MASK){
			Input = SENSORS >>2;
			GPIO_PORTA_ICR_R = RIGHT_SENSOR_MASK;
	}
}


// Interrupt handler for reset button:  
// update global variable: reset
void GPIOPortE_Handler(void) { 
	for (uint32_t time=0; time<10000000; time++) {}
    reset = true;
    GPIO_PORTE_ICR_R = 0x08;
}

// Systick interrupt handler:
// Stop systick timer and update global variable: timesup 
void SysTick_Handler(void) {
	timesup = true;
}

