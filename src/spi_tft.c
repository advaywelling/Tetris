/**
  ******************************************************************************
  * @file    main.c
  * @author  Weili An, Niraj Menon
  * @date    Feb 3, 2024
  * @brief   ECE 362 Lab 6 Student template
  ******************************************************************************
*/

/*******************************************************************************/

// Fill out your username, otherwise your completion code will have the 
// wrong username!
const char* username = "anamdev";

/*******************************************************************************/ 

#include "stm32f0xx.h"

void set_char_msg(int, char);
void nano_wait(unsigned int);
void game(void);
void internal_clock();
void check_wiring();
void autotest();

//===========================================================================
// Configure GPIOC
//===========================================================================
void enable_ports(void) {
    // Only enable port C for the keypad
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC->MODER &= ~0xffff;
    GPIOC->MODER |= 0x55 << (4*2);
    GPIOC->OTYPER &= ~0xff;
    GPIOC->OTYPER |= 0xf0;
    GPIOC->PUPDR &= ~0xff;
    GPIOC->PUPDR |= 0x55;
}


uint8_t col; // the column being scanned

void drive_column(int);   // energize one of the column outputs
int  read_rows();         // read the four row inputs
void update_history(int col, int rows); // record the buttons of the driven column
char get_key_event(void); // wait for a button event (press or release)
char get_keypress(void);  // wait for only a button press event.
float getfloat(void);     // read a floating-point number from keypad
void show_keys(void);     // demonstrate get_key_event()

//===========================================================================
// Bit Bang SPI LED Array
//===========================================================================
int msg_index = 0;
uint16_t msg[8] = { 0x0000,0x0100,0x0200,0x0300,0x0400,0x0500,0x0600,0x0700 };
extern const char font[];

//===========================================================================
// Configure PB12 (CS), PB13 (SCK), and PB15 (SDI) for outputs
//===========================================================================
void setup_bb(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->MODER |= 0x45000000;
    GPIOB->BSRR |= 1 << 12;
    GPIOB->BRR |= 1 << 13; 
}

void small_delay(void) {
    nano_wait(5000);
}

//===========================================================================
// Set the MOSI bit, then set the clock high and low.
// Pause between doing these steps with small_delay().
//===========================================================================
void bb_write_bit(int val) {
    // CS (PB12)
    // SCK (PB13)
    // SDI (PB15)
    if(val)
        GPIOB->BSRR = 1 << 15;
    else
        GPIOB->BRR = 1 << 15;
    small_delay();
    GPIOB->BSRR = 1 << 13;
    small_delay();
    GPIOB->BRR = (1 << 13);

}

//===========================================================================
// Set CS (PB12) low,
// write 16 bits using bb_write_bit,
// then set CS high.
//===========================================================================
void bb_write_halfword(int halfword) {
    GPIOB->BRR = 1 << 12;
    for(int i = 15; i >= 0; i--){
        bb_write_bit((halfword >> i) & 1);
    }
    GPIOB->BSRR = 1 << 12;
}

//===========================================================================
// Continually bitbang the msg[] array.
//===========================================================================
void drive_bb(void) {
    for(;;)
        for(int d=0; d<8; d++) {
            bb_write_halfword(msg[d]);
            nano_wait(1000000); // wait 1 ms between digits
        }
}

//============================================================================
// Configure Timer 15 for an update rate of 1 kHz.
// Trigger the DMA channel on each update.
// Copy this from lab 4 or lab 5.
//============================================================================
void init_tim15(void) {
    RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;
    TIM15->DIER |= TIM_DIER_UDE;
    TIM15->PSC = 480-1;
    TIM15->ARR = 100-1;
    TIM15->CR1 |= TIM_CR1_CEN;
}


//===========================================================================
// Configure timer 7 to invoke the update interrupt at 1kHz
// Copy from lab 4 or 5.
//===========================================================================
void init_tim7(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
    TIM7->PSC = 480 - 1;
    TIM7->ARR = 100 - 1;
    TIM7->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM7_IRQn);
    TIM7->CR1 |= TIM_CR1_CEN;
}


//===========================================================================
// Copy the Timer 7 ISR from lab 5
//===========================================================================
// TODO To be copied
void TIM7_IRQHandler(){
    TIM7->SR &= ~TIM_SR_UIF;
    int rows = read_rows();
    update_history(col, rows);
    col = (col + 1) & 3;
    drive_column(col);
}


//===========================================================================
// Initialize the SPI2 peripheral.
//===========================================================================
void init_spi2(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->MODER &= 0x30ffffff;
    GPIOB->MODER |= 0x8A000000;
    GPIOB->AFR[1] |= (0x0 << GPIO_AFRH_AFSEL12_Pos) | (0x0 << GPIO_AFRH_AFSEL13_Pos) | (0x0 << GPIO_AFRH_AFSEL15_Pos);
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
    SPI2->CR1 &= ~SPI_CR1_SPE;
    SPI2->CR1 |= (0x7 << SPI_CR1_BR_Pos);
    SPI2->CR2 |= (0xf << SPI_CR2_DS_Pos);
    SPI2->CR1 |= SPI_CR1_MSTR;
    SPI2->CR2 |= SPI_CR2_SSOE | SPI_CR2_NSSP;
    SPI2->CR2 |= SPI_CR2_TXDMAEN;
    SPI2->CR1 |= SPI_CR1_SPE;
}

//===========================================================================
// Configure the SPI2 peripheral to trigger the DMA channel when the
// transmitter is empty.  Use the code from setup_dma from lab 5.
//===========================================================================
void spi2_setup_dma(void) {
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    DMA1_Channel5->CCR &= ~DMA_CCR_EN;
    DMA1_Channel5->CMAR = (uint32_t) msg;
    DMA1_Channel5->CPAR = (uint32_t) &(SPI2->DR);
    DMA1_Channel5->CNDTR = 8;
    DMA1_Channel5->CCR |= DMA_CCR_DIR;
    DMA1_Channel5->CCR |= DMA_CCR_MINC;
    DMA1_Channel5->CCR |= DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0;
    DMA1_Channel5->CCR |= DMA_CCR_CIRC;
    SPI2->CR2 |= SPI_CR2_TXDMAEN;
}

//===========================================================================
// Enable the DMA channel.
//===========================================================================
void spi2_enable_dma(void) {
    DMA1_Channel5->CCR |= DMA_CCR_EN;
}

//===========================================================================
// 4.4 SPI OLED Display
//===========================================================================
/*void init_spi1() {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= 0x3fc333ff;
    GPIOA->MODER |= 0x80148800;
    //GPIOA->AFR[1] |= (0x0 << GPIO_AFRH_AFSEL15_Pos);
    GPIOA->AFR[0] |= (0x0 << GPIO_AFRL_AFSEL5_Pos) | (0x0 << GPIO_AFRL_AFSEL7_Pos);
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    SPI1->CR1 &= ~SPI_CR1_SPE;
    SPI1->CR1 |= (0x7 << SPI_CR1_BR_Pos);
    SPI1->CR1 |= SPI_CR1_MSTR;
    SPI1->CR2 |= SPI_CR2_SSOE | SPI_CR2_NSSP | SPI_CR2_DS_3 | SPI_CR2_DS_0;
    SPI1->CR2 |= (SPI_CR2_DS_1);
    SPI1->CR2 |= (SPI_CR2_DS_2);
    SPI1->CR2 |= SPI_CR2_TXDMAEN;
    SPI1->CR1 |= SPI_CR1_SPE;
    SPI1->CR1 &= ~SPI_CR1_SPE;                 // Disable SPI before configuration
    SPI1->CR1 |= (0x7 << SPI_CR1_BR_Pos);      // Baud rate (fPCLK / 256)
    SPI1->CR1 |= SPI_CR1_MSTR;                 // Master mode
    SPI1->CR2 |= (0xf << SPI_CR2_DS_Pos);      // 8-bit data format
    //SPI1->CR2 |= SPI_CR2_SSOE | SPI_CR2_NSSP;
    SPI1->CR2 &= ~(SPI_CR2_SSOE | SPI_CR2_NSSP);
    SPI1->CR1 |= SPI_CR1_CPOL | SPI_CR1_CPHA;  // Enable NSS management
    SPI1->CR2 &= ~SPI_CR2_TXDMAEN;             // Disable DMA for now

    // Enable SPI1
    SPI1->CR1 |= SPI_CR1_SPE;
}
void spi_cmd(unsigned int data) {
    GPIOA->BSRR = (1 << (15 + 16));
    GPIOA->BSRR = (1 << (10 + 16));  // PA10 (DC) LOW for command
    while (!(SPI1->SR & SPI_SR_TXE)) {}  // Wait until TXE (Transmit Empty)
    SPI1->DR = data;
    GPIOA->BSRR = (1 << 15); // CS high
}

void spi_data(unsigned int data) {
    GPIOA->BSRR = (1 << (15 + 16));
    GPIOA->BSRR = (1 << 10);  // PA10 (DC) HIGH for data
    while (!(SPI1->SR & SPI_SR_TXE)) {}  // Wait until TXE (Transmit Empty)
    SPI1->DR = data;
    GPIOA->BSRR = (1 << 15); // CS high
}
void spi1_init_oled() {
    nano_wait(2000000);
    spi_cmd(0x01);
    nano_wait(5000000);
    // Exit Sleep mode (0x11)
    spi_cmd(0x11);
    nano_wait(12000000);
    // Set pixel format to 16 bits per pixel (0x3A)
    spi_cmd(0x3A);
    spi_data(0x55);                // 0x55 corresponds to 16-bit/pixel

    // Memory Access Control (0x36)
    // This command sets the orientation; 0x48 is one example (portrait/landscape)
    spi_cmd(0x36);
    spi_data(0x48);

    // Finally, turn the display on (0x29)
    spi_cmd(0x29);
    nano_wait(2000000); 
}

void tft_reset(void) {
    GPIOA->BSRR = (1 << (9 + 16)); // Reset LOW (PA9 connected to RESET)
    nano_wait(200000);
    GPIOA->BSRR = (1 << 9); // Reset HIGH
    nano_wait(200000);
}

void init_tft(void) {
    tft_reset(); // Perform Reset
    
    spi_cmd(0x01); // Software Reset
    nano_wait(500000); // Delay

    spi_cmd(0x28); // Display OFF

    spi_cmd(0xCF); // Power Control B
    spi_data(0x00);
    spi_data(0x81);
    spi_data(0x30);

    spi_cmd(0xED); // Power on Sequence Control
    spi_data(0x64);
    spi_data(0x03);
    spi_data(0x12);
    spi_data(0x81);

    spi_cmd(0xE8); // Driver timing control A
    spi_data(0x85);
    spi_data(0x01);
    spi_data(0x79);

    spi_cmd(0xCB); // Power Control A
    spi_data(0x39);
    spi_data(0x2C);
    spi_data(0x00);
    spi_data(0x34);
    spi_data(0x02);

    spi_cmd(0xF7); // Pump Ratio Control
    spi_data(0x20);

    spi_cmd(0xB1); // Frame Rate Control
    spi_data(0x00);
    spi_data(0x1B);

    spi_cmd(0xB6); // Display Function Control
    spi_data(0x0A);
    spi_data(0xA2);

    spi_cmd(0x36); // Memory Access Control
    spi_data(0x08); // Adjust orientation if needed

    spi_cmd(0x3A); // Pixel Format
    spi_data(0x55); // 16-bit per pixel

    spi_cmd(0x11); // Exit Sleep Mode
    nano_wait(500000);

    spi_cmd(0x29); // Display ON
}

void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    spi_cmd(0x2A); // Column Address Set
    spi_data(x0 >> 8); spi_data(x0 & 0xFF);
    spi_data(x1 >> 8); spi_data(x1 & 0xFF);
    
    spi_cmd(0x2B); // Page Address Set
    spi_data(y0 >> 8); spi_data(y0 & 0xFF);
    spi_data(y1 >> 8); spi_data(y1 & 0xFF);

    spi_cmd(0x2C); // Memory Write
}

void fill_rect(uint16_t color) {
    for (uint32_t i = 0; i < 320 * 240; i++) { // Assuming 320x240 display
        spi_data(color >> 8);
        spi_data(color & 0xFF);
    }
}

void test_display(void) {
    set_window(0, 0, 319, 239); // Full screen
    fill_rect(0xF800); // Red (5-bit R, 6-bit G, 5-bit B)
}

void spi1_display1(const char *string) {
    spi_cmd(0x02);
    while(*string != '\0'){
        spi_data(*string);
        string++;
    }
}
void spi1_display2(const char *string) {
    spi_cmd(0xc0);
    while(*string != '\0'){
        spi_data(*string);
        string++;
    }
}

//===========================================================================
// This is the 34-entry buffer to be copied into SPI1.
// Each element is a 16-bit value that is either character data or a command.
// Element 0 is the command to set the cursor to the first position of line 1.
// The next 16 elements are 16 characters.
// Element 17 is the command to set the cursor to the first position of line 2.
//===========================================================================
uint16_t display[34] = {
        0x002, // Command to set the cursor at the first position line 1
        0x200+'E', 0x200+'C', 0x200+'E', 0x200+'3', 0x200+'6', + 0x200+'2', 0x200+' ', 0x200+'i',
        0x200+'s', 0x200+' ', 0x200+'t', 0x200+'h', + 0x200+'e', 0x200+' ', 0x200+' ', 0x200+' ',
        0x0c0, // Command to set the cursor at the first position line 2
        0x200+'c', 0x200+'l', 0x200+'a', 0x200+'s', 0x200+'s', + 0x200+' ', 0x200+'f', 0x200+'o',
        0x200+'r', 0x200+' ', 0x200+'y', 0x200+'o', + 0x200+'u', 0x200+'!', 0x200+' ', 0x200+' ',
};

//===========================================================================
// Configure the proper DMA channel to be triggered by SPI1_TX.
// Set the SPI1 peripheral to trigger a DMA when the transmitter is empty.
//===========================================================================
void spi1_setup_dma(void) {
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    DMA1_Channel3->CCR &= ~DMA_CCR_EN;
    DMA1_Channel3->CMAR = (uint32_t) display;
    DMA1_Channel3->CPAR = (uint32_t) &(SPI1->DR);
    DMA1_Channel3->CNDTR = 34;
    DMA1_Channel3->CCR |= DMA_CCR_DIR;
    DMA1_Channel3->CCR |= DMA_CCR_MINC;
    DMA1_Channel3->CCR |= DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0;
    DMA1_Channel3->CCR |= DMA_CCR_CIRC;
    SPI1->CR2 |= SPI_CR2_TXDMAEN;
}

//===========================================================================
// Enable the DMA channel triggered by SPI1_TX.
//===========================================================================
void spi1_enable_dma(void) {
    DMA1_Channel3->CCR |= DMA_CCR_EN;
}

//===========================================================================
// Main function
//===========================================================================

int main(void) {
    internal_clock();

    msg[0] |= font['E'];
    msg[1] |= font['C'];
    msg[2] |= font['E'];
    msg[3] |= font[' '];
    msg[4] |= font['3'];
    msg[5] |= font['6'];
    msg[6] |= font['2'];
    msg[7] |= font[' '];

    // GPIO enable
    enable_ports();
    // setup keyboard
    //init_tim7();

    // LED array Bit Bang
// #define BIT_BANG
#if defined(BIT_BANG)
    setup_bb();
    drive_bb();
#endif

    // Direct SPI peripheral to drive LED display
// #define SPI_LEDS
#if defined(SPI_LEDS)
    init_spi2();
    spi2_setup_dma();
    spi2_enable_dma();
    init_tim15();
    show_keys();
#endif

    // LED array SPI
// #define SPI_LEDS_DMA
#if defined(SPI_LEDS_DMA)
    init_spi2();
    spi2_setup_dma();
    spi2_enable_dma();
    show_keys();
#endif

    // SPI OLED direct drive
// #define SPI_OLED
#if defined(SPI_OLED)
    init_spi1();
    spi1_init_oled();
    spi1_display1("Hello again,");
    spi1_display2(username);
#endif

    // SPI
// #define SPI_OLED_DMA
#if defined(SPI_OLED_DMA)
    init_spi1();
    spi1_init_oled();
    spi1_setup_dma();
    spi1_enable_dma();
#endif
    init_spi1();
    init_tft();
    test_display();

    // Uncomment when you are ready to generate a code.
    //autotest();

    // Game on!  The goal is to score 100 points.
    //game();
}*/

void init_spi1() {
    // Enable GPIOA and SPI1 clocks
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    
    GPIOA->MODER &= 0x3fc333ff;
    GPIOA->MODER |= 0x40148800;
    //GPIOA->AFR[1] |= (0x0 << GPIO_AFRH_AFSEL15_Pos);
    GPIOA->AFR[0] |= (0x0 << GPIO_AFRL_AFSEL5_Pos) | (0x0 << GPIO_AFRL_AFSEL7_Pos);

    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    // Configure SPI1
    SPI1->CR1 &= ~SPI_CR1_SPE; 
    SPI1->CR1 |= SPI_CR1_MSTR;           // Master mode
    SPI1->CR1 |= SPI_CR1_BR_2;           // Baud rate: fPCLK/64 (adjust as needed)
    SPI1->CR1 &= ~SPI_CR1_CPOL;          // CPOL = 0 (SPI Mode 0)
    SPI1->CR1 &= ~SPI_CR1_CPHA;          // CPHA = 0

    SPI1->CR2 = 0;
    SPI1->CR2 |= (0x7 << SPI_CR2_DS_Pos); // 8-bit data size
    SPI1->CR2 |= SPI_CR2_SSOE;            // Enable NSS output (optional, but keep CS software-controlled)

    SPI1->CR1 |= SPI_CR1_SPE;            // Enable SPI1
}

// Send command to TFT
void spi_cmd(uint8_t cmd) {
    GPIOA->BSRR = (1 << 15) << 16;        // CS low (PA4)
    GPIOA->BSRR = (1 << 10) << 16;       // DC low (command mode, PA10)

    while (!(SPI1->SR & SPI_SR_TXE)) {}  // Wait for TX buffer empty
    SPI1->DR = cmd;                       // Send command
    while (SPI1->SR & SPI_SR_BSY) {}      // Wait until SPI not busy

    GPIOA->BSRR = (1 << 15);              // CS high
}

// Send data to TFT
void spi_data(uint8_t data) {
    GPIOA->BSRR = (1 << 15) << 16;        // CS low (PA4)
    GPIOA->BSRR = (1 << 10);             // DC high (data mode, PA10)

    while (!(SPI1->SR & SPI_SR_TXE)) {}  // Wait for TX buffer empty
    SPI1->DR = data;                      // Send data
    while (SPI1->SR & SPI_SR_BSY) {}      // Wait until SPI not busy

    GPIOA->BSRR = (1 << 15);              // CS high
}

// Reset TFT
void tft_reset() {
    GPIOA->BSRR = (1 << 9) << 16;        // Reset low (PA9)
    nano_wait(10000000);              
    GPIOA->BSRR = (1 << 9);              // Reset high
    nano_wait(10000000);
}

// Initialize TFT
void init_tft() {
    tft_reset();

    // Initialization sequence (ILI9341)
    spi_cmd(0x01);                       // Software reset
    nano_wait(150000000);                // 150 ms delay

    spi_cmd(0x28);                       // Display OFF

    spi_cmd(0xCF);                       // Power control B
    spi_data(0x00);
    spi_data(0x81);
    spi_data(0x30);

    spi_cmd(0xED);                       // Power sequence
    spi_data(0x64);
    spi_data(0x03);
    spi_data(0x12);
    spi_data(0x81);

    spi_cmd(0xE8);                       // Driver timing control A
    spi_data(0x85);
    spi_data(0x01);
    spi_data(0x78);

    spi_cmd(0xCB);                       // Power control A
    spi_data(0x39);
    spi_data(0x2C);
    spi_data(0x00);
    spi_data(0x34);
    spi_data(0x02);

    spi_cmd(0xF7);                       // Pump ratio control
    spi_data(0x20);

    spi_cmd(0xC0);                       // Power control 1
    spi_data(0x26);

    spi_cmd(0xC1);                       // Power control 2
    spi_data(0x11);

    spi_cmd(0x36);                       // Memory Access Control
    spi_data(0x08);                      // Orientation (adjust as needed)

    spi_cmd(0x3A);                       // Pixel format
    spi_data(0x55);                      // 16-bit RGB

    spi_cmd(0x11);                       // Sleep OUT
    nano_wait(120000000);                // 120 ms delay

    spi_cmd(0x29);                       // Display ON
}

// Set drawing window
void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    spi_cmd(0x2A);                       // Column address set
    spi_data(x0 >> 8); spi_data(x0 & 0xFF);
    spi_data(x1 >> 8); spi_data(x1 & 0xFF);

    spi_cmd(0x2B);                       // Page address set
    spi_data(y0 >> 8); spi_data(y0 & 0xFF);
    spi_data(y1 >> 8); spi_data(y1 & 0xFF);

    spi_cmd(0x2C);                       // Memory write
}

// Fill screen with color
void fill_rect(uint16_t color) {
    for (uint32_t i = 0; i < 320 * 240; i++) {
        spi_data(color >> 8);            // Send high byte
        spi_data(color & 0xFF);          // Send low byte
    }
}

// Test: Fill screen red
void test_display() {
    set_window(0, 0, 319, 239);          // Full screen (320x240)
    fill_rect(0xF800);                   // Red color (RGB565)
}

void spi1_display1(const char *string) {
    spi_cmd(0x02);
    while(*string != '\0'){
        spi_data(*string);
        string++;
    }
}
void spi1_display2(const char *string) {
    spi_cmd(0xc0);
    while(*string != '\0'){
        spi_data(*string);
        string++;
    }
}

//===========================================================================
// This is the 34-entry buffer to be copied into SPI1.
// Each element is a 16-bit value that is either character data or a command.
// Element 0 is the command to set the cursor to the first position of line 1.
// The next 16 elements are 16 characters.
// Element 17 is the command to set the cursor to the first position of line 2.
//===========================================================================
uint16_t display[34] = {
        0x002, // Command to set the cursor at the first position line 1
        0x200+'E', 0x200+'C', 0x200+'E', 0x200+'3', 0x200+'6', + 0x200+'2', 0x200+' ', 0x200+'i',
        0x200+'s', 0x200+' ', 0x200+'t', 0x200+'h', + 0x200+'e', 0x200+' ', 0x200+' ', 0x200+' ',
        0x0c0, // Command to set the cursor at the first position line 2
        0x200+'c', 0x200+'l', 0x200+'a', 0x200+'s', 0x200+'s', + 0x200+' ', 0x200+'f', 0x200+'o',
        0x200+'r', 0x200+' ', 0x200+'y', 0x200+'o', + 0x200+'u', 0x200+'!', 0x200+' ', 0x200+' ',
};

//===========================================================================
// Configure the proper DMA channel to be triggered by SPI1_TX.
// Set the SPI1 peripheral to trigger a DMA when the transmitter is empty.
//===========================================================================
void spi1_setup_dma(void) {
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    DMA1_Channel3->CCR &= ~DMA_CCR_EN;
    DMA1_Channel3->CMAR = (uint32_t) display;
    DMA1_Channel3->CPAR = (uint32_t) &(SPI1->DR);
    DMA1_Channel3->CNDTR = 34;
    DMA1_Channel3->CCR |= DMA_CCR_DIR;
    DMA1_Channel3->CCR |= DMA_CCR_MINC;
    DMA1_Channel3->CCR |= DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0;
    DMA1_Channel3->CCR |= DMA_CCR_CIRC;
    SPI1->CR2 |= SPI_CR2_TXDMAEN;
}

//===========================================================================
// Enable the DMA channel triggered by SPI1_TX.
//===========================================================================
void spi1_enable_dma(void) {
    DMA1_Channel3->CCR |= DMA_CCR_EN;
}

int main(void) {
    internal_clock();

    msg[0] |= font['E'];
    msg[1] |= font['C'];
    msg[2] |= font['E'];
    msg[3] |= font[' '];
    msg[4] |= font['3'];
    msg[5] |= font['6'];
    msg[6] |= font['2'];
    msg[7] |= font[' '];

    // GPIO enable
    enable_ports();
    // setup keyboard
    //init_tim7();

    // LED array Bit Bang
// #define BIT_BANG
#if defined(BIT_BANG)
    setup_bb();
    drive_bb();
#endif

    // Direct SPI peripheral to drive LED display
// #define SPI_LEDS
#if defined(SPI_LEDS)
    init_spi2();
    spi2_setup_dma();
    spi2_enable_dma();
    init_tim15();
    show_keys();
#endif

    // LED array SPI
// #define SPI_LEDS_DMA
#if defined(SPI_LEDS_DMA)
    init_spi2();
    spi2_setup_dma();
    spi2_enable_dma();
    show_keys();
#endif

    // SPI OLED direct drive
// #define SPI_OLED
#if defined(SPI_OLED)
    init_spi1();
    spi1_init_oled();
    spi1_display1("Hello again,");
    spi1_display2(username);
#endif

    // SPI
// #define SPI_OLED_DMA
#if defined(SPI_OLED_DMA)
    init_spi1();
    spi1_init_oled();
    spi1_setup_dma();
    spi1_enable_dma();
#endif
    init_spi1();
    init_tft();
    test_display();
    while(1){}

    // Uncomment when you are ready to generate a code.
    //autotest();

    // Game on!  The goal is to score 100 points.
    //game();
}