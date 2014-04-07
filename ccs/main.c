#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/debug.h"
#include "driverlib/flash.h"

void testHeartBeat(void);
void testFlash(void);

int main(void)
{
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

	// testHeartBeat();
	testFlash();

	while(1);
}

void testHeartBeat()
{
	int PinData = 8;

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_3);

	while(1)
	{
		// 4th bit lights up heartbeat LED
		GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3, PinData);
		SysCtlDelay(1000000);
		GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3, 0x00);
		SysCtlDelay(1000000);
	}
}

void testFlash()
{
	unsigned long pulData[2];
	unsigned long pulRead[2];
	pulData[0] = 0x12345678;
	pulData[1] = 0x56789abc;

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);
	SysCtlDelay(20000000);

	FlashErase(0x10000);
	FlashProgram(pulData, 0x10000, sizeof(pulData));
	GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x02);
	SysCtlDelay(20000000);

	while(1);
}
