//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// Red LED:
//   PF1 drives an NPN transistor that powers the red LED
// Green LED:
//   PF3 drives an NPN transistor that powers the green LED
// Pushbutton:
//   SW1 pulls pin PF4 low (internal pull-up is used)

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "tm4c123gh6pm.h"
#include "gpio.h"
#include "uart0.h"

// Pins
#define RED_LED PORTF,1
#define GREEN_LED PORTF,3

// Data structure for terminal
#define MAX_CHARS 80
#define MAX_FIELDS 5

typedef struct _USER_DATA
{
    char buffer[MAX_CHARS+1];
    uint8_t fieldCount;
    uint8_t fieldPosition[MAX_FIELDS];
    char fieldType[MAX_FIELDS];
} USER_DATA;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
void initHw()
{
    // Configure HW to work with 16 MHz XTAL, PLL enabled, sysdivider of 5, creating system clock of 40 MHz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);

    // Enable clocks
    enablePort(PORTF);

    // Configure LED and pushbutton pins
    selectPinPushPullOutput(GREEN_LED);
    selectPinPushPullOutput(RED_LED);

}

void getsUart0(USER_DATA* data)
{
    uint8_t count = 0;


    while(1){
        char c = getcUart0();

        if(((c==8) || (c==127))&&(count!=0)) //Backspace
        {
            count--;
        }
        else if(c==13 || c==10) //CR
        {
            data->buffer[count]=0;
            return;
        }
        else if((c==32)||(c>32)) //Space and special characters
        {
            if(count==MAX_CHARS-1)
            {
                data->buffer[count+1]='\n';
                return;
            }
            else if((c>=65)&&(c<=90))
            {
                c+=32;
            }
            data->buffer[count] = c;
            count++;
        }
    }

}


void parseFields(USER_DATA * data) // Populate fieldCount, fieldPosition, fieldType
{
    // numerics: ASCII 48-57
    // alpha: ASCII 97-122

    uint8_t i = 0;
    char prev = 'd'; // Assume previous char is a delimiter
    data->fieldCount = 0;
    char delimPos[MAX_CHARS]; //Holds the position of delimiters for turning into null chars later
    uint16_t delimIndex = 0;
    uint16_t delimCount = 0;

    for(i = 0; data->buffer[i] != 0; ++i)
    {
        if(data->fieldCount == MAX_FIELDS)
        {
            if((data->buffer[i]>=97 && data->buffer[i]<=122) || (data->buffer[i]>=48 && data->buffer[i]<=57))
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if((prev == 'd') && (data->buffer[i]>=48 && data->buffer[i]<=57))
        {
            data->fieldType[data->fieldCount] = 'n';
            data->fieldPosition[data->fieldCount] = i;
            data->fieldCount++;
            prev = 'n';
        }
        else if((prev == 'd') && (data->buffer[i]>=97 && data->buffer[i]<=122))
        {
            data->fieldType[data->fieldCount] = 'a';
            data->fieldPosition[data->fieldCount] = i;
            data->fieldCount++;
            prev = 'a';
        }
        else if((prev != 'd') && !(data->buffer[i]>=97 && data->buffer[i]<=122) && !(data->buffer[i]>=48 && data->buffer[i]<=57))
        {
            delimPos[delimIndex] = i;
            prev = 'd';

            if(delimIndex!=MAX_CHARS-1)
            {
                delimIndex++;
                delimCount++;
            }
        }
    }

    delimPos[delimIndex] = i;

    uint8_t j;

    for(j = 0; delimPos[j]!=0; j++)
    {
        data->buffer[delimPos[j]] = 0;
    }

}
/*
char* getFieldString(USER_DATA * data, uint8_t fieldNumber)
{

}

int32_t getFieldInteger(USER_DATA * data, uint8_t filedNumber)
{

}

bool isCommand(USER_DATA * data, const char strCommand[],uint8_t minArguments)
{

}
*/


//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)
{
    // Initialize hardware
    initHw();
    initUart0();

    setUart0BaudRate(115200, 40e6);

    USER_DATA command;

    while(1)
    {
        getsUart0(&command);
        putsUart0(command.buffer);
        putcUart0('\n');


        parseFields(&command);
        uint8_t i;
        for(i = 0; i<command.fieldCount;i++)
        {
            putcUart0(command.fieldType[i]);
            putcUart0('\t');
            putsUart0(&command.buffer[command.fieldPosition[i]]);
            putcUart0('\n');
        }
    }

}
