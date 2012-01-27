#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

/* driverlib library includes */
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/i2c.h"


void I2C0_init() {
	//
    // The I2C0 peripheral must be enabled before use.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);


    //
    // Configure the pin muxing for I2C1 functions on port B2 and B3.
    //
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);

    //
    // Select the I2C function for these pins.  This function will also
    // configure the GPIO pins for I2C operation, setting them to
    // open-drain operation with weak pull-ups.
    //
    GPIOPinTypeI2C(GPIO_PORTB_BASE, I2CSCL_PIN | I2CSDA_PIN);


    // Enable and initialize the I2C0 master module.  Use the system clock for
    // the I2C0 module.  The last parameter sets the I2C data transfer rate.
    // If false the data rate is set to 100kbps and if true the data rate will
    // be set to 400kbps.  For this example we will use a data rate of 100kbps.
    //
    I2CMasterInitExpClk(I2C0_MASTER_BASE, SysCtlClockGet(), false);

    //
    // Enable the I2C0 slave module.
    //
    I2CSlaveEnable(I2C0_SLAVE_BASE);
}

//*****************************************************************************
// Non Interupt driven Write from the Eeprom.
// \param slave_address is the Control byte shift right once. (0x50=0xA0>>1)  
// \param data is the value to be written in the Eeprom
// \param reg_address is the memory address which will be written at
//*****************************************************************************
void I2C_write(unsigned char slave_address, unsigned char *data, unsigned long length, unsigned short reg_address) {
	int i;
    ///////////////////////////////////////////////////////////////////
    // Start with the control byte (address) to the Eeprom.
    I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, slave_address, false);
    
    // Place the address to be written in the data register.
    I2CMasterDataPut(I2C0_MASTER_BASE, (unsigned char)(reg_address));
    I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_START);    
    while(I2CMasterBusy(I2C0_MASTER_BASE)) {}    //wait

	for(i=0; i < (length-1); i++) {
	    I2CMasterDataPut(I2C0_MASTER_BASE, *(data+i));
	    I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
	    while(I2CMasterBusy(I2C0_MASTER_BASE)) {}    //wait
	}
	
    I2CMasterDataPut(I2C0_MASTER_BASE, *(data+i));
    I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
    while(I2CMasterBusy(I2C0_MASTER_BASE)) {}    //wait

    // here must be a delay to wait the write complete or to poll the ACK
}

//*****************************************************************************
// Non Interupt driven Read from the Eeprom device.
// \param slave_address is the Control byte shift right once. (0x50=0xA0>>1)  
// \param reg_address is the memory address to be read
//*****************************************************************************
void I2C_read(unsigned char slave_address, unsigned char *data, unsigned long length, unsigned short reg_address){
	int i;
     unsigned long ret = 0;
    ///////////////////////////////////////////////////////////////////
    // Start with a write to set the address in the Eeprom.
    I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, slave_address, false);
    
    // Place the address to be written in the data register.
    I2CMasterDataPut(I2C0_MASTER_BASE, (unsigned char)reg_address);
    I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_SINGLE_SEND);    
    while(I2CMasterBusy(I2C0_MASTER_BASE)) {}    //wait

    // Put the I2C master into receive mode.
    I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, slave_address, true);

	i=0;
    I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
    while(I2CMasterBusy(I2C0_MASTER_BASE)) {}    //wait
    ret = I2CMasterDataGet(I2C0_MASTER_BASE);
	*data=ret;
	
	for(i = 1; i < (length-1); i++)	{
		I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
        while(I2CMasterBusy(I2C0_MASTER_BASE)){}
        // get further bytes
        ret = I2CMasterDataGet(I2C0_MASTER_BASE);
        *(data + i) = ret;
	}

    // finish reading sequence
    I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
    while(I2CMasterBusy(I2C0_MASTER_BASE)){}
    // get last byte
    ret = I2CMasterDataGet(I2C0_MASTER_BASE);
    *(data + i) = ret;
}