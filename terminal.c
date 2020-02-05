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
    uint16_t delimPos[MAX_CHARS]; //Holds the position of delimiters for turning into null chars later
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

    for(j = 0; j<delimCount; j++)
    {
        data->buffer[delimPos[j]] = 0;
    }

}

char* getFieldString(USER_DATA * data, uint8_t fieldNumber)
{

    if(fieldNumber>data->fieldCount || fieldNumber<1 )
        return NULL;

    return &data->buffer[data->fieldPosition[fieldNumber]];
}

int strToInt(char a[]) { // Obtained from https://www.programmingsimplified.com/c/source-code/c-program-convert-string-to-integer-without-using-atoi-function
    int c, sign, offset, n;

    if (a[0] == '-') {  // Handle negative integers
        sign = -1;
    }

    if (sign == -1) {  // Set starting position to convert
        offset = 1;
    }
    else {
        offset = 0;
    }

    n = 0;

    for (c = offset; a[c] != '\0'; c++) {
        n = n * 10 + a[c] - '0';
    }

    if (sign == -1) {
        n = -n;
    }

  return n;
}


int32_t getFieldInteger(USER_DATA * data, uint8_t fieldNumber)
{

    if(fieldNumber>data->fieldCount || fieldNumber<1 )
        return NULL;

    return strToInt(&data->buffer[data->fieldPosition[fieldNumber]]);
}


int strCompare(char* arg1, char* arg2) // Returns 0 for equal strings
{
    uint8_t i = 0;
    while(arg1[i]==arg2[i])
    {
        if(arg1[i]=='\0' && arg2[i]=='\0')
            break;
        i++;
    }

    return arg1[i]-arg2[i];
}

bool isCommand(USER_DATA * data, const char strCommand[],uint8_t minArguments)
{
    if(strCompare(&data->buffer[data->fieldPosition[0]],strCommand)==0)
    {
        if(data->fieldCount-1 == minArguments)
            return true;
    }

    return false;
}



//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)
{
    // Initialize hardware
    initHw();
    initUart0();

    setUart0BaudRate(115200, 40e6);

    //int32_t gotInt;

    while(1)
    {
        USER_DATA command;
        getsUart0(&command);
        putsUart0("Command typed in: ");
        putsUart0(command.buffer);
        putcUart0('\n');
        putcUart0('\r');


        parseFields(&command);
       /* uint8_t i;
        for(i = 0; i<command.fieldCount;i++)
        {
            putcUart0(command.fieldType[i]);
            putcUart0('\t');
            putsUart0(&command.buffer[command.fieldPosition[i]]);
            putcUart0('\n');
        }

        char* gotString = getFieldString(&command, 2);
        putsUart0(gotString); */

        // set add, data -> add and data are integers
        bool val = false;

        if(isCommand(&command,"set",2))
        {
            int32_t add = getFieldInteger(&command,1);
            int32_t data = getFieldInteger(&command,2);
            putcUart0(add+data);
            val = true;
        }
        else if(isCommand(&command, "alert",1))
        {
            char* str = getFieldString(&command,1);
            if(strCompare(str,"on")==0)
                putsUart0("Alert is on.");
            if(strCompare(str,"off")==0)
                putsUart0("Alert is off.");
            val = true;
        }

        if(!val)
            putsUart0("Invalid Command");
        putcUart0('\n');
        putcUart0('\r');
    }

}
