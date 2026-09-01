// SysTick.h
// Runs on TM4C123
// Kairi Rodriguez, Estefania, Alex, Deshawn
// Group 8
// October 26, 2025

#include <stdint.h> // C99 data types

void SysTick_Init(void);

void SysTick_Start(uint32_t period);

void SysTick_Stop(void);