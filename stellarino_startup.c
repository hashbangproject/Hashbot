/*  stellarino_timer.c
    Copyright (C) 2012 Sultan Qasim Khan

    This is part of Stellarino.

    Stellarino is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stellarino is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Stellarino.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stellarino.h>

void ResetISR(void);
static void GenericISR(void);

// Linker variable for top of stack
extern unsigned long __STACK_TOP;

// main
extern int main(void);

// Initial vector table

#pragma DATA_SECTION(g_pfnVectors, ".intvecs");

void (* const g_pfnVectors[])(void) =
{
    (void (*)(void))((unsigned long)&__STACK_TOP),  // Initial stack pointer
    ResetISR,                                       // Reset handler
    GenericISR                                      // Everything else handler
};

// Initializes C and starts program
void ResetISR(void)
{
    void (**p)() = (void (**)())0x200009ac;

    // Set system clock to 80 MHz
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

    // Enable FPU and allow floating point operations in interrupts
    ROM_FPUEnable();
    ROM_FPULazyStackingEnable();

    // Call all the global constructors
    (*p)();

    main();
}

// This interrupt handler does nothing
static void GenericISR(void)
{
    while(1);
}
