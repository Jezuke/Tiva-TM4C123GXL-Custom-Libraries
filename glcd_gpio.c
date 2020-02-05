// Graphics LCD Example
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL with LCD/Keyboard Interface
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// Red Backlight LED:
//   PB5 drives an NPN transistor that powers the red LED
// Green Backlight LED:
//   PE5 drives an NPN transistor that powers the green LED
// Blue Backlight LED:
//   PE4 drives an NPN transistor that powers the blue LED
// ST7565R Graphics LCD Display Interface:
//   MOSI on PD3 (SSI1Tx)
//   SCLK on PD0 (SSI1Clk)
//   ~CS on PD1 (SSI1Fss)
//   A0 connected to PD2

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "tm4c123gh6pm.h"
#include "graphics_lcd.h"
#include "wait.h"
#include "gpio.h"

#define RED_BL_LED PORTB,5
#define GREEN_BL_LED PORTE,5
#define BLUE_BL_LED PORTE,4

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
void initHw()
{
	// Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 40 MHz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);

    // Enable clocks
    enablePort(PORTB);
    enablePort(PORTE);

    // Configure three backlight LEDs
    selectPinPushPullOutput(RED_BL_LED);
    selectPinPushPullOutput(GREEN_BL_LED);
    selectPinPushPullOutput(BLUE_BL_LED);
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)
{
    // Initialize hardware
    initHw();

    // Turn-on all LEDs to create white backlight
    setPinValue(RED_BL_LED, 1);
    setPinValue(GREEN_BL_LED, 1);
    setPinValue(BLUE_BL_LED, 1);

    // Initialize graphics LCD
    initGraphicsLcd();

    // Draw X in left half of screen
    uint8_t i;
    for (i = 0; i < 64; i++)
        drawGraphicsLcdPixel(i, i, SET);
    for (i = 0; i < 64; i++)
        drawGraphicsLcdPixel(63-i, i, INVERT);

    // Draw text on screen
    setGraphicsLcdTextPosition(84, 5);
    putsGraphicsLcd("Text");

    // Draw flashing block around the text
    while(true)
    {
        drawGraphicsLcdRectangle(83, 39, 25, 9, INVERT);
        waitMicrosecond(500000);
    }
}
