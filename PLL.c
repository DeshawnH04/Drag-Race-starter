// PLL.c
// Kairi Rodriguez, Estefania, Alex, Deshawn
// Group 8
// October 26, 2025
 
#include "PLL.h"
#include "tm4c123gh6pm.h"
#include <stdint.h>

// The #define statement SYSDIV2 in PLL.h
// initializes the PLL to the desired frequency.

// bus frequency is 400MHz/(SYSDIV2+1) = 400MHz/(7+1) = 50 MHz
// see the table at the end of this file

// configure the system to get its clock from the PLL
void PLL_Init(void){

  // Step 0 enable RCC2 for advanced PLL configuration 400 MHz
  SYSCTL_RCC2_R |= SYSCTL_RCC2_USERCC2;

  // Step 1 bypass PLL during setup
  SYSCTL_RCC2_R |= SYSCTL_RCC2_BYPASS2;

  // Step 2 set crystal to 16 MHz use main oscillator
  SYSCTL_RCC_R  &= ~SYSCTL_RCC_XTAL_M;        // clear XTAL bits
  SYSCTL_RCC_R  |=  SYSCTL_RCC_XTAL_16MHZ;    // 16 MHz crystal
  SYSCTL_RCC2_R &= ~SYSCTL_RCC2_OSCSRC2_M;    // clear oscillator source
  SYSCTL_RCC2_R |=  SYSCTL_RCC2_OSCSRC2_MO;   // main oscillator source

  // Step 3 power up the PLL
  SYSCTL_RCC2_R &= ~SYSCTL_RCC2_PWRDN2;

  // Step 4 set divider to get 50 MHz system clock
  // Formula 400 MHz / (SYSDIV2 + 1)
  const uint32_t SYSDIV = 7; // 400/(7+1) = 50 MHz
  SYSCTL_RCC2_R |= SYSCTL_RCC2_DIV400;
  SYSCTL_RCC2_R = (SYSCTL_RCC2_R & ~0x1FC00000) | (SYSDIV << 22);
  SYSCTL_RCC2_R &= ~0x00200000; // integer division (clear bit 21)

  // Step 5 wait for PLL to lock
  while ((SYSCTL_RIS_R & SYSCTL_RIS_PLLLRIS) == 0) {}

  // Step 6 enable PLL by clearing BYPASS
  SYSCTL_RCC2_R &= ~SYSCTL_RCC2_BYPASS2;

}
