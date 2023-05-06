/*
 * University:     Universidad del Valle de Guatemala
 * Course:         Electrónica Digital 2
 * Author:         Carlos Molina (#21253)
 * Description:    Laboratorio 08 - TivaWare
 * Date:           05 de mayo de 2023
 */

//-----------------------------------------------------------------------------
//                                 Libraries
//-----------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/rom.h"
#include "grlib/grlib.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/timer.h"
//#include "drivers/cfal96x64x16.h"


//-----------------------------------------------------------------------------
//                           Variables declarations
//-----------------------------------------------------------------------------
uint8_t cont = 0;
uint8_t LED = 2;
uint8_t i;
uint32_t period;

unsigned char recompilado;
//-----------------------------------------------------------------------------
//                           Functions declarations
//-----------------------------------------------------------------------------
void main(void);
void semaforo(void);
void InitUART(void);
void Timer0IntHandler(void);
void UART0IntHandler(void);

//-----------------------------------------------------------------------------
//                                 Main Code
//-----------------------------------------------------------------------------
void main(void){


    // Configuramos el oscilador a uno interno de 40MHz
    //SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN); //External Oscillator (or Crystal) --> 40MHz
    //SysCtlClockSet(SYSCTL_SYSDIV_1|SYSCTL_USE_OSC|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN); //External Oscillator (or Crystal) --> 16MHz
    //SysCtlClockSet(SYSCTL_SYSDIV_2|SYSCTL_USE_OSC|SYSCTL_OSC_INT); //Precision Internal Oscillator (16MHz) --> 8MHz
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_INT); //Precision Internal Oscillator (16MHz) --> 40MHz

    // Peripherals from Port B are enabled
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // Peripherals from Port F are enabled
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    /*
    // Pin 2, Pin 3, Pin 4 of PORT A are assigned as OUTPUTS
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4);
    */

    // Pin 1 of PORT B is assigned as OUTPUT
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_2);

    // Pin 1, Pin 2, Pin 3 of PORT F (LEDs pins) are assigned as OUTPUTS
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

    /*
    // Pin 1 of PORT B is assigned as Input
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_0);

    // Activate Weak Pull-Up for Pin 1
    //GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    */

    // TMR0 Configuration

    // Normal (16/32 bit) TMR0 is enabled
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    // TMR0 as a Full Width (Non-Individual -> Solely TIMERA -> No Pre-scaler Added) and Periodic Timer
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

    // TMR0 Period Calculus
    period = (SysCtlClockGet()/2);

    // Establish TMR0 Period
    TimerLoadSet(TIMER0_BASE, TIMER_A, period-1);

    // TIMER0A Interruption is enabled
    IntEnable(INT_TIMER0A);

    // Timeout is set as the reason of the interruption
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // Global Interruptions are enabled
    IntMasterEnable();

    // TIMER0 is enabled
    TimerEnable(TIMER0_BASE, TIMER_A);

    InitUART();

    while(1)
    {
        ;
    }
}

//-----------------------------------------------------------------------------
//                                Functions
//-----------------------------------------------------------------------------
void semaforo(void){

    // Turn the Green LED solely
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4, 0x04);
    // Delay for 3s
    SysCtlDelay(40000000);  // A delay takes 3 clock cycles long. Each clock cycle (or instruction cycle as it is referred in the manual) is equivalent to (1/Oscillator Used).

    // Turn the Green LED blinking
    for(i = 0; i<3; i++){

        // Turn the Green LED off
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4, 0x00);

        // Delay for 500ms or 0.5s
        SysCtlDelay(6666666.667);  // A delay takes 3 clock cycles long. Each clock cycle (or instruction cycle as it is referred in the manual) is equivalent to (1/Oscillator Used).

        // Turn the Green LED solely
        GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4, 0x04);

        // Delay for 500ms or 0.5s
        SysCtlDelay(6666666.667);  // A delay takes 3 clock cycles long. Each clock cycle (or instruction cycle as it is referred in the manual) is equivalent to (1/Oscillator Used).
    }

    // Turn on the Yellow LED solely
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4, 0x08);

    // Delay for 3s
    SysCtlDelay(40000000);  // A delay takes 3 clock cycles long. Each clock cycle (or instruction cycle as it is referred in the manual) is equivalent to (1/Oscillator Used).

    // Turn on the Red LED solely
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4, 0x10);

    // Delay for 3s
    SysCtlDelay(40000000);  // A delay takes 3 clock cycles long. Each clock cycle (or instruction cycle as it is referred in the manual) is equivalent to (1/Oscillator Used).

    // Turn the Red LED off
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4, 0x00);

    cont = 0;
}

void InitUART(void){
    // Peripherals from Port A are enabled
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Peripherals for UART0 are enabled
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    // Pin 0, Pin 1 of PORTA are assigned as UART
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0|GPIO_PIN_1);

    // Configuration of UART0 Parameters (Module UART0, Baud Rate 115200, 8 data bits, 1 stop bit, None parity)
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE) );

    // UART0 Interruption Enabled
    IntEnable(INT_UART0);

    // Receive is set as the reason of the interruption
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
}

//-----------------------------------------------------------------------------
//                                 Handler
//-----------------------------------------------------------------------------
void Timer0IntHandler(void){

    // TIMER0 Interruption is cleared
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // Toggle of PORTB GPIO
    if (!GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_2)){
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0x04);
    }

    else{
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0x00);
    }

    // Toggle of LED
    if (cont == 0){
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);
    }
    else if (cont == 1){
        if (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1)){
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);
        }
        else{
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x02);
        }
    }
    else if (cont == 2){
        if (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_2)){
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);
        }
        else{
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x04);
        }
    }
    else if (cont == 3){
        if (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_3)){
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);
        }
        else{
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x08);
        }
    }
    else{
        ;
    }

}
void UART0IntHandler(void){
    UARTIntClear(UART0_BASE, UART_INT_RX | UART_INT_RT);

    recompilado = UARTCharGet(UART0_BASE);

    if (recompilado == 'r'){
        if (cont == 1){
            cont = 0;
        }
        else{
            cont = 1;
        }
    }

    else if (recompilado == 'b'){
        if (cont == 2){
            cont = 0;
        }
        else{
            cont = 2;
        }
    }

    else if (recompilado == 'g'){
        if (cont == 3){
            cont = 0;
        }
        else{
            cont = 3;
        }
    }

    else{
        ;
    }

    /*
    // Si hay algo en el buffer lo leemos, si no, seguimos de largo
    newchar = UARTCharGetNonBlocking(UART0_BASE);
    */


}
