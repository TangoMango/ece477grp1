#include "inc/hw_ints.h"
#include "inc/hw_i2c.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/flash.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom.h"
#include "driverlib/i2c.h"

// Slave Port
#define GPIO_PA6_I2C1SCL		0x00001803
#define GPIO_PA7_I2C1SDA		0x00001C03
#define SLAVE_ADDRESS 			0x4C // 10011 A0 0

void testHeartBeat(void);
void testFlash(void);
void testDAC(void);
void send_data(int);

int main(void)
{
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

	// testHeartBeat();
	testFlash();
	// testDAC();

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

void testDAC()
{
	int data;
	int datahi;
	int datalo;
	int PinData = 4;

	// push button stuff
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);

	// I2C stuff
	SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	GPIOPinTypeI2CSCL(GPIO_PORTA_BASE, GPIO_PIN_6);
	GPIOPinTypeI2C(GPIO_PORTA_BASE, GPIO_PIN_7);

	GPIOPinConfigure(GPIO_PA6_I2C1SCL);
	GPIOPinConfigure(GPIO_PA7_I2C1SDA);

	I2CMasterInitExpClk(I2C1_MASTER_BASE, SysCtlClockGet(), false);
	SysCtlDelay(10000);  // delay for SlaveAddrSet()

	I2CMasterSlaveAddrSet(I2C1_MASTER_BASE, SLAVE_ADDRESS, false); // Write

	//while(1)
	//{
	I2CMasterControl(I2C1_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_START);
	data = 0x98;
	send_data(data); // slave address
	data = 0x10;
	send_data(data); // 0x10 updates register with data and outputs the data

	datahi = 0x00;
	datalo = 0x00;

	while(1)
	{
		if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4))
		{
			if(datalo >= 0xFF)
			{
				if(datahi <= 0xFF)
				{
					datahi = datahi + 0x02;
				}
				else
				{
					datahi = 0x00;
				}
				datalo = 0x00;
			}
			else
			{
				datalo = datalo + 0x02;
			}
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, PinData);
		}
		else
		{
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);
		}
		send_data(datahi);
		send_data(datalo);
	}

	I2CMasterControl(I2C1_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
}

void send_data(int data)
{
	while(I2CMasterBusy(I2C1_MASTER_BASE)) {}
	I2CMasterDataPut(I2C1_MASTER_BASE, data);
	I2CMasterControl(I2C1_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
	while(I2CMasterBusy(I2C1_MASTER_BASE)) {}
}
