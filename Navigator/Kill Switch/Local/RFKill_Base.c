void uart_setup() {
    UART1_Init_Advanced(115200, _UART_8_BIT_DATA, _UART_NOPARITY, _UART_ONE_STOPBIT, &_GPIO_MODULE_USART1_PA9_10); // RF module
    UART2_Init_Advanced(19200, _UART_8_BIT_DATA, _UART_NOPARITY, _UART_ONE_STOPBIT, &_GPIO_MODULE_USART2_PA23); // FTDI
}

void pin_setup() {
    // Inputs
    GPIO_Config(&GPIOC_BASE, _GPIO_PINMASK_0 | _GPIO_PINMASK_1 | _GPIO_PINMASK_2 | _GPIO_PINMASK_3, _GPIO_CFG_MODE_INPUT | _GPIO_CFG_PULL_UP); // Local Kill Switches
    GPIO_Digital_Input(&GPIOA_BASE, _GPIO_PINMASK_12); // RF Module - /READY
    
    //Outputs
    GPIO_Digital_Output(&GPIOA_BASE, _GPIO_PINMASK_6); // Kill Relay
    GPIOA_ODR.B6 = 0; // Kill Relay Off
    
    GPIO_Digital_Output(&GPIOA_BASE, _GPIO_PINMASK_7); // Horn Relay
    GPIOA_ODR.B7 = 0; // Horn Relay Off
    
    GPIO_Digital_Output(&GPIOA_BASE, _GPIO_PINMASK_11); // RF Module - CMD_DATA_TYPE
    GPIOA_ODR.B11 = 0; // Set RF module to command data
    
    GPIO_Digital_Output(&GPIOB_BASE, _GPIO_PINMASK_10); // Red Stack Light
    GPIOB_ODR.B10 = 0; // Red LED Off
    
    GPIO_Digital_Output(&GPIOB_BASE, _GPIO_PINMASK_11); // Yellow Stack Light
    GPIOB_ODR.B11 = 0; // Green LED Off
    
    GPIO_Digital_Output(&GPIOB_BASE, _GPIO_PINMASK_12); // Green Stack Light
    GPIOB_ODR.B12 = 0; // Green LED Off
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
    unsigned short lastStatePF = 0;
    unsigned short currentStatePF = 0;
    unsigned short lastStatePA = 0;
    unsigned short currentStatePA = 0;
    unsigned short lastStateSF = 0;
    unsigned short currentStateSF = 0;
    unsigned short lastStateSA = 0;
    unsigned short currentStateSA = 0;
    
    unsigned short rxData = 0;
    unsigned short overallKillStatus = 0;
    unsigned short remoteKillStatus = 0;
    unsigned short computerKillStatus = 0;
    
    unsigned short killNotified = 0;
    unsigned short unkillNotified = 0;
    
    unsigned short remoteNotified = 0;

    uart_setup();
    pin_setup();

    // Initialize local kill switch states
    currentStatePF = GPIOC_IDR.B0;
    lastStatePF = currentStatePF;
    currentStatePA = GPIOC_IDR.B1;
    lastStatePA = currentStatePA;
    currentStateSF = GPIOC_IDR.B2;
    lastStateSF = currentStateSF;
    currentStateSA = GPIOC_IDR.B3;
    lastStateSA = currentStateSA;
    
    // Set pin override mask
    write_mask(0x0F); // All settings software controlled

    // Set RF channel
    write_channel(0x45);

    // Set RF baud rate
    write_baud(0x01); // 19.2 kbps (slowest, longest range)

    // Set RF power
    write_power(0x39); // 12.22 dBm
            
    while (1) {
        // Read local kill switch states
        currentStatePF = GPIOC_IDR.B0;
        currentStatePA = GPIOC_IDR.B1;
        currentStateSF = GPIOC_IDR.B2;
        currentStateSA = GPIOC_IDR.B3;
        
        // Check if PF kill switch has changed states
        if (currentStatePF != lastStatePF) {
            lastStatePF = currentStatePF;
            if (currentStatePF) {
                UART2_Write(0x12); // PF Pressed
            } else {
                UART2_Write(0x13); // PF Released
            }
        }

        // Check if PA kill switch has changed states
        if (currentStatePA != lastStatePA) {
            lastStatePA = currentStatePA;
            if (currentStatePA) {
                UART2_Write(0x14); // PA Pressed
            } else {
                UART2_Write(0x15); // PF Released
            }
        }

        // Check if SF kill switch has changed states
        if (currentStateSF != lastStateSF) {
            lastStateSF = currentStateSF;
            if (currentStateSF) {
                UART2_Write(0x16); // SF Pressed
            } else {
                UART2_Write(0x17); // SF Released
            }
        }

        // Check if SA kill switch has changed states
        if (currentStateSA != lastStateSA) {
            lastStateSA = currentStateSA;
            if (currentStateSA) {
                UART2_Write(0x18); // SA Pressed
            } else {
                UART2_Write(0x19); // SA Released
            }
        }
        
        if (UART1_Data_Ready()) {
            rxData = UART1_Read();
            if (rxData == 0xF0) {
                UART1_Write(0xE0);
            } else if (rxData == 0xF1) {
                remoteKillStatus = 1; // Killed
                UART2_Write(0x1A); // Killed
            } else if (rxData == 0xF2) {
                remoteKillStatus = 0; // Unkilled
                UART2_Write(0x1B); // Unkilled
            }
        }
        
        if (UART2_Data_Ready()) {
            rxData = UART2_Read();
            if (rxData == 0x20) { // Ping
                UART2_Write(0x30); // Send Acknowledge
            } else if (rxData == 0x21) { // Kill Status
                if (overallKillStatus) {
                    UART2_Write(0x01); // Killed
                } else {
                    UART2_Write(0x00); // Unkilled
                }
            } else if (rxData == 0x22) { // PF Status
                if (currentStatePF) {
                    UART2_Write(0x01); // Pressed
                } else {
                    UART2_Write(0x00); // Released
                }
            } else if (rxData == 0x23) { // PA Status
                if (currentStatePA) {
                    UART2_Write(0x01); // Pressed
                } else {
                    UART2_Write(0x00); // Released
                }
            } else if (rxData == 0x24) { // SF Status
                if (currentStateSF) {
                    UART2_Write(0x01); // Pressed
                } else {
                    UART2_Write(0x00); // Released
                }
            } else if (rxData == 0x25) { // SA Status
                if (currentStateSA) {
                    UART2_Write(0x01); // Pressed
                } else {
                    UART2_Write(0x00); // Released
                }
            } else if (rxData == 0x26) { // Remote Kill Status
                if (remoteKillStatus) {
                    UART2_Write(0x01); // Killed
                } else {
                    UART2_Write(0x00); // Unkilled
                }
            } else if (rxData == 0x27) { // Computer Kill Status
                if (computerKillStatus) {
                    UART2_Write(0x01); // Killed
                } else {
                    UART2_Write(0x00); // Unkilled
                }
            } else if (rxData == 0x40) { // Set No Operation
                GPIOB_ODR.B11 = 0; // Yellow LED Off
                GPIOB_ODR.B12 = 0; // Green LED Off
                UART2_Write(0x50); // Send Acknowledge
            } else if (rxData == 0x41) { // Set Manual Operation
                GPIOB_ODR.B11 = 1; // Yellow LED On
                GPIOB_ODR.B12 = 0; // Green LED Off
                UART2_Write(0x51); // Send Acknowledge
            } else if (rxData == 0x42) { // Set Autonomous Operation
                GPIOB_ODR.B11 = 0; // Yellow LED Off
                GPIOB_ODR.B12 = 1; // Green LED On
                UART2_Write(0x52); // Send Acknowledge
            } else if (rxData == 0x43) { // Set Horn Off
                GPIOA_ODR.B7 = 0; // Horn Relay Off
                UART2_Write(0x53); // Send Acknowledge
            } else if (rxData == 0x44) { // Set Horn On
                GPIOA_ODR.B7 = 1; // Horn Relay On
                UART2_Write(0x54); // Send Acknowledge
            } else if (rxData == 0x45) { // Set Computer Kill State
                computerKillStatus = 1; // Computer Killed
                UART2_Write(0x55); // Send Acknowledge
                UART2_Write(0x1C); // Send kill notification to computer
            } else if (rxData == 0x46) { // Set Computer Unkill State
                computerKillStatus = 0; // Computer Unkilled
                UART2_Write(0x56); // Send Acknowledge
                UART2_Write(0x1D); // Send unkill notification to computer
            }
        }
        
        // Check if any kill switches are activated
        if (currentStatePF || currentStatePA || currentStateSF || currentStateSA || remoteKillStatus || computerKillStatus) {
            overallKillStatus = 1;
        } else {
            overallKillStatus = 0;
        }
        
        // Run once on overall kill activation or deactivation
        if (!killNotified && overallKillStatus) {
            UART1_Write(0xE1); // Send kill notification to remote
            UART2_Write(0x10); // Send kill notification to computer
            GPIOB_ODR.B10 = 1; // Set kill status LED to on
            GPIOA_ODR.B6 = 1; // Set kill relay to on
            killNotified = 1;
            unkillNotified = 0;
        } else if (!unkillNotified && !overallKillStatus) {
            UART1_Write(0xE2); // Send unkill notification to remote
            UART2_Write(0x11); // Send unkill notification to computer
            GPIOB_ODR.B10 = 0; // Set kill status LED to off
            GPIOA_ODR.B6 = 0; // Set kill relay to off
            unkillNotified = 1;
            killNotified = 0;
        }
    }
}
