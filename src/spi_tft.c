#include "stm32f0xx.h"
#include <stdint.h>
#include "spi_tft.h"

void internal_clock();

// Uncomment only one of the following to test each step
// #define STEP1
// #define STEP2
// #define STEP3
// #define STEP4
#define SHELL

void init_usart5() {
    // TODO
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIODEN;

    GPIOC->MODER &= 0xFCFFFFFF;
    GPIOC->MODER |= 0x02000000;
    GPIOC->AFR[1] |= (0x2 << GPIO_AFRH_AFSEL12_Pos);

    GPIOD->MODER &= 0xFFFFFFCF;
    GPIOD->MODER |= 0x00000020;
    GPIOD->AFR[0] |= (0x2 << GPIO_AFRL_AFSEL2_Pos);

    RCC->APB1ENR |= RCC_APB1ENR_USART5EN;
    USART5->CR1 &= ~USART_CR1_UE;
    USART5->CR1 &= ~(USART_CR1_M1 | USART_CR1_M0);
    USART5->CR2 &= ~(USART_CR2_STOP);
    USART5->CR1 &= ~(USART_CR1_PCE);
    USART5->CR1 &= ~(USART_CR1_OVER8);
    USART5->BRR = 0x1A1;
    USART5->CR1 |= USART_CR1_TE | USART_CR1_RE;
    USART5->CR1 |= USART_CR1_UE;

    while (((USART5->ISR & USART_ISR_TEACK) == 0) || ((USART5->ISR & USART_ISR_REACK) == 0)){
    }
}

#ifdef STEP1
int main(void){
    internal_clock();
    init_usart5();
    for(;;) {
        while (!(USART5->ISR & USART_ISR_RXNE)) { }
        char c = USART5->RDR;
        while(!(USART5->ISR & USART_ISR_TXE)) { }
        USART5->TDR = c;
    }
}
#endif

#ifdef STEP2
#include <stdio.h>

// TODO Resolve the echo and carriage-return problem

int __io_putchar(int c) {
    // TODO
    if (c == '\n') {
        while (!(USART5->ISR & USART_ISR_TXE));
        USART5->TDR = '\r';
    }
    while(!(USART5->ISR & USART_ISR_TXE));
    USART5->TDR = c;
    return c;
}

int __io_getchar(void) {
    while (!(USART5->ISR & USART_ISR_RXNE));
    char c = USART5->RDR;
    // TODO
    if(c == '\r'){
        c = '\n';
    }
    __io_putchar(c);
    return c;
}

int main() {
    internal_clock();
    init_usart5();
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    printf("Enter your name: ");
    char name[80];
    fgets(name, 80, stdin);
    printf("Your name is %s", name);
    printf("Type any characters.\n");
    for(;;) {
        char c = getchar();
        putchar(c);
    }
}
#endif

#ifdef STEP3
#include <stdio.h>
#include "fifo.h"
#include "tty.h"
int __io_putchar(int c) {
    // TODO Copy from your STEP2
    if (c == '\n') {
        while (!(USART5->ISR & USART_ISR_TXE));
        USART5->TDR = '\r';
    }
    while(!(USART5->ISR & USART_ISR_TXE));
    USART5->TDR = c;
    return c;
}

int __io_getchar(void) {
    // TODO
    return line_buffer_getchar();
}

int main() {
    internal_clock();
    init_usart5();
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    printf("Enter your name: ");
    char name[80];
    fgets(name, 80, stdin);
    printf("Your name is %s", name);
    printf("Type any characters.\n");
    for(;;) {
        char c = getchar();
        putchar(c);
    }
}
#endif

#ifdef STEP4

#include <stdio.h>
#include "fifo.h"
#include "tty.h"

// TODO DMA data structures
#define FIFOSIZE 16
char serfifo[FIFOSIZE];
int seroffset = 0;

void enable_tty_interrupt(void) {
    // TODO
    USART5->CR1 |= USART_CR1_RXNEIE;
    USART5->CR3 |= USART_CR3_DMAR;
    NVIC_EnableIRQ(USART3_8_IRQn);
    RCC->AHBENR |= RCC_AHBENR_DMA2EN;
    DMA2->CSELR |= DMA2_CSELR_CH2_USART5_RX;
    DMA2_Channel2->CCR &= ~DMA_CCR_EN;
    DMA2_Channel2->CMAR = (uint32_t) serfifo;
    DMA2_Channel2->CPAR = (uint32_t) &(USART5->RDR);
    DMA2_Channel2->CNDTR = FIFOSIZE;
    DMA2_Channel2->CCR &= ~DMA_CCR_DIR;
    DMA2_Channel2->CCR &= ~(DMA_CCR_TCIE | DMA_CCR_HTIE);
    DMA2_Channel2->CCR &= ~(DMA_CCR_MSIZE_Msk); //clear MSIZE for 8bit memory size
    DMA2_Channel2->CCR &= ~(DMA_CCR_PSIZE_Msk); //clear PSIZE for 8bit peripheral size
    DMA2_Channel2->CCR |= DMA_CCR_MINC;
    DMA2_Channel2->CCR &= ~DMA_CCR_PINC;
    DMA2_Channel2->CCR |= DMA_CCR_CIRC;
    DMA2_Channel2->CCR &= ~DMA_CCR_MEM2MEM;
    DMA2_Channel2->CCR &= ~DMA_CCR_PL;  //clear the priority
    DMA2_Channel2->CCR |= (0x3 << DMA_CCR_PL_Pos);   //set priority to 3 (11) for highest priority
    DMA2_Channel2->CCR |= DMA_CCR_EN;
    
}

// Works like line_buffer_getchar(), but does not check or clear ORE nor wait on new characters in USART
char interrupt_getchar() {
    // TODO
    // Wait for a newline to complete the buffer.
    while(fifo_newline(&input_fifo) == 0) {
        asm volatile ("wfi"); // wait for an interrupt
    }
    // Return a character from the line buffer.
    char ch = fifo_remove(&input_fifo);
    return ch;
}

int __io_putchar(int c) {
    // TODO copy from STEP2
    if (c == '\n') {
        while (!(USART5->ISR & USART_ISR_TXE));
        USART5->TDR = '\r';
    }
    while(!(USART5->ISR & USART_ISR_TXE)){
    }
    USART5->TDR = c;
    return c;
}

int __io_getchar(void) {
    // TODO Use interrupt_getchar() instead of line_buffer_getchar()
    return interrupt_getchar();
}

// TODO Copy the content for the USART5 ISR here
void USART3_8_IRQHandler(void) {
    while(DMA2_Channel2->CNDTR != sizeof serfifo - seroffset) {
        if (!fifo_full(&input_fifo))
            insert_echo_char(serfifo[seroffset]);
        seroffset = (seroffset + 1) % sizeof serfifo;
    }
}
// TODO Remember to look up for the proper name of the ISR function

int main() {
    internal_clock();
    init_usart5();
    enable_tty_interrupt();

    setbuf(stdin,0); // These turn off buffering; more efficient, but makes it hard to explain why first 1023 characters not dispalyed
    setbuf(stdout,0);
    setbuf(stderr,0);
    printf("Enter your name: "); // Types name but shouldn't echo the characters; USE CTRL-J to finish
    char name[80];
    fgets(name, 80, stdin);
    printf("Your name is %s", name);
    printf("Type any characters.\n"); // After, will type TWO instead of ONE
    for(;;) {
        char c = getchar();
        putchar(c);
    }
}
#endif

#include "fifo.h"
#include "tty.h"
#include <stdio.h>
#include "lcd.h"
#include "commands.h"

#define FIFOSIZE 16
char serfifo[FIFOSIZE];
int seroffset = 0;

void enable_tty_interrupt(void) {
    // TODO
    USART5->CR1 |= USART_CR1_RXNEIE;
    USART5->CR3 |= USART_CR3_DMAR;
    NVIC_EnableIRQ(USART3_8_IRQn);
    RCC->AHBENR |= RCC_AHBENR_DMA2EN;
    DMA2->CSELR |= DMA2_CSELR_CH2_USART5_RX;
    DMA2_Channel2->CCR &= ~DMA_CCR_EN;
    DMA2_Channel2->CMAR = (uint32_t) serfifo;
    DMA2_Channel2->CPAR = (uint32_t) &(USART5->RDR);
    DMA2_Channel2->CNDTR = FIFOSIZE;
    DMA2_Channel2->CCR &= ~DMA_CCR_DIR;
    DMA2_Channel2->CCR &= ~(DMA_CCR_TCIE | DMA_CCR_HTIE);
    DMA2_Channel2->CCR &= ~(DMA_CCR_MSIZE_Msk); //clear MSIZE for 8bit memory size
    DMA2_Channel2->CCR &= ~(DMA_CCR_PSIZE_Msk); //clear PSIZE for 8bit peripheral size
    DMA2_Channel2->CCR |= DMA_CCR_MINC;
    DMA2_Channel2->CCR &= ~DMA_CCR_PINC;
    DMA2_Channel2->CCR |= DMA_CCR_CIRC;
    DMA2_Channel2->CCR &= ~DMA_CCR_MEM2MEM;
    DMA2_Channel2->CCR &= ~DMA_CCR_PL;  //clear the priority
    DMA2_Channel2->CCR |= (0x3 << DMA_CCR_PL_Pos);   //set priority to 3 (11) for highest priority
    DMA2_Channel2->CCR |= DMA_CCR_EN;
    
}

char interrupt_getchar() {
    // TODO
    // Wait for a newline to complete the buffer.
    while(fifo_newline(&input_fifo) == 0) {
        asm volatile ("wfi"); // wait for an interrupt
    }
    // Return a character from the line buffer.
    char ch = fifo_remove(&input_fifo);
    return ch;
}

int __io_putchar(int c) {
    // TODO copy from STEP2
    if (c == '\n') {
        while (!(USART5->ISR & USART_ISR_TXE));
        USART5->TDR = '\r';
    }
    while(!(USART5->ISR & USART_ISR_TXE)){
    }
    USART5->TDR = c;
    return c;
}

int __io_getchar(void) {
    // TODO Use interrupt_getchar() instead of line_buffer_getchar()
    return interrupt_getchar();
}

void USART3_8_IRQHandler(void) {
    while(DMA2_Channel2->CNDTR != sizeof serfifo - seroffset) {
        if (!fifo_full(&input_fifo))
            insert_echo_char(serfifo[seroffset]);
        seroffset = (seroffset + 1) % sizeof serfifo;
    }
}


void init_spi1_slow(){
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    
    GPIOB->MODER &= 0xfffff03f;
    GPIOB->MODER |= 0x00000a80;
    GPIOB->AFR[0] |= (0x0 << GPIO_AFRL_AFSEL3_Pos) | (0x0 << GPIO_AFRL_AFSEL4_Pos) | (0x0 << GPIO_AFRL_AFSEL5_Pos);

    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    // Configure SPI1
    SPI1->CR1 &= ~SPI_CR1_SPE;
    SPI1->CR1 |= (0x7 << SPI_CR1_BR_Pos);
    SPI1->CR1 |= SPI_CR1_MSTR;           // Master mode
    SPI1->CR2 |= SPI_CR2_FRXTH;
    SPI1->CR2 |= (0x7 << SPI_CR2_DS_Pos); // 8-bit data size
    SPI1->CR1 |= SPI_CR1_SSI | SPI_CR1_SSM;

    SPI1->CR1 |= SPI_CR1_SPE;  
}

void enable_sdcard(void) {
    // Use the BSRR register: writing to bit (pin number + 16) resets (sets low) the pin.
    GPIOB->BSRR = (1U << (2 + 16)); // Reset PB2
}

// Function to disable the SD card by setting PB2 high.
void disable_sdcard(void) {
    // Use the BSRR register: writing to the pin number sets (sets high) the pin.
    GPIOB->BSRR = (1U << 2); // Set PB2
}

void init_sdcard_io(){
    init_spi1_slow();
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->MODER &= 0xffffffcf;
    GPIOB->MODER |= 0x00000010;
    disable_sdcard();
}

void sdcard_io_high_speed(){
    SPI1->CR1 &= ~SPI_CR1_SPE;
    SPI1->CR1 &= ~SPI_CR1_BR;
    SPI1->CR1 |= SPI_CR1_BR_0;
    SPI1->CR1 |= SPI_CR1_SPE;
}

void init_lcd_spi(){
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->MODER &= 0xcf3cffff;
    GPIOB->MODER |= 0x10410000;
    init_spi1_slow();
    sdcard_io_high_speed();
}