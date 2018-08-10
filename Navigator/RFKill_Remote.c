void uart_setup() {
    UART1_Init_Advanced(115200, _UART_8_BIT_DATA, _UART_NOPARITY, _UART_ONE_STOPBIT, &_GPIO_MODULE_USART1_PA9_10); // RF module
    UART2_Init_Advanced(19200, _UART_8_BIT_DATA, _UART_NOPARITY, _UART_ONE_STOPBIT, &_GPIO_MODULE_USART2_PA23); // FTDI
}

void pin_setup() {
    // Inputs
    GPIO_Config(&GPIOC_BASE, _GPIO_PINMASK_0, _GPIO_CFG_MODE_INPUT | _GPIO_CFG_PULL_UP); // Remote Kill Switch
    GPIO_Digital_Input(&GPIOA_BASE, _GPIO_PINMASK_12); // RF Module - /READY

    //Outputs
    GPIO_Digital_Output(&GPIOA_BASE, _GPIO_PINMASK_11); // RF Module - CMD_DATA_TYPE
    GPIOA_ODR.B11 = 0; // Set RF module to command data
    
    GPIO_Digital_Output(&GPIOC_BASE, _GPIO_PINMASK_8); // Overall Kill Status LED
    GPIOC_ODR.B8 = 0; // Overall Kill Status LED Off
    
    GPIO_Digital_Output(&GPIOC_BASE, _GPIO_PINMASK_9); // Remote Kill Status LED
    GPIOC_ODR.B9 = 0; // Remote Kill Status LED Off
}

unsigned short cdi_read_byte(unsigned short parameter) {
    unsigned short value = 0;

    GPIOA_ODR.B11 = 0;
    
    UART1_Write(0x01);
    UART1_Write(parameter);
    UART1_Write(0x00);
    UART1_Write(0xFF);

    UART1_Read();
    UART1_Read();
    value = UART1_Read();
    UART1_Read();
    
    GPIOA_ODR.B11 = 1;

    return value;
}

void cdi_write_byte(unsigned short parameter, unsigned short value) {
    GPIOA_ODR.B11 = 0;
    
    UART1_Write(0x02);
    UART1_Write(parameter);
    UART1_Write(value);
    UART1_Write(0xFF);

    UART1_Read();
    UART1_Read();
    UART1_Read();
    UART1_Read();
    
    GPIOA_ODR.B11 = 1;
}

void cdi_program_byte(unsigned short parameter, unsigned short value) {
    GPIOA_ODR.B11 = 0;

    UART1_Write(0x03);
    UART1_Write(parameter);
    UART1_Write(value);
    UART1_Write(0xFF);

    UART1_Read();
    UART1_Read();
    UART1_Read();
    UART1_Read();
    
    GPIOA_ODR.B11 = 1;
}

// Read RF Channel
unsigned short read_channel() {
    return cdi_read_byte(0x10);
}

// Write RF Channel
void write_channel(unsigned short channel) {
    cdi_write_byte(0x10, channel);
}

// Read RF Baud
unsigned short read_baud() {
    return cdi_read_byte(0x11);
}

// Write RF Baud
void write_baud(unsigned short baud) {
    cdi_write_byte(0x11, baud);
}

// Read RF Power
unsigned short read_power() {
    return cdi_read_byte(0x13);
}

// Write RF Power
void write_power(unsigned short power) {
    cdi_write_byte(0x13, power);
}

// Read Mask
unsigned short read_mask() {
    return cdi_read_byte(0x14);
}

// Write Mask
void write_mask(unsigned short mask) {
    cdi_write_byte(0x14, mask);
}

// Read RF Module Device Name
// TODO (2 bytes)

// Read RF Module FW Version
// TODO (2 bytes)

// Read RF Module Serial Number
// TODO (4 bytes)

// Read RSSI
unsigned short read_module_rssi() {
    return cdi_read_byte(0x1F);
}

// Reset Module
void reset_module() {
    cdi_program_byte(0x80, 0x00);
}

void main() {
    unsigned short lastState = 0;
    unsigned short currentState = 0;
    
    unsigned short rxData = 0;

    uart_setup();
    pin_setup();
    
    currentState = GPIOC_IDR.B0;
    lastState = currentState;

    // Set pin override mask
    write_mask(0x0F); // All settings software controlled

    // Set RF channel
    write_channel(0x45); // 902.62 MHz (channel 0)

    // Set RF baud rate
    write_baud(0x01); // 19.2 kbps (slowest, longest range)

    // Set RF power
    write_power(0x39); // 12.22 dBm

    while (1) {
        currentState = GPIOC_IDR.B0;
        
        if (currentState != lastState) {
            lastState = currentState;
            if (currentState) {
                UART1_Write(0xF1); // Pressed
                GPIOC_ODR.B9 = 1;
            } else {
                UART1_Write(0xF2); // Released
                GPIOC_ODR.B9 = 0;
            }
        }
        
        if (UART1_Data_Ready()) {
            rxData = UART1_Read();
            if (rxData == 0xE1) { // Kill activation
                GPIOC_ODR.B8 = 1;
            } else if (rxData == 0xE2) { // Kill deactivation
                GPIOC_ODR.B8 = 0;
            }
        }
    }
}
