/**
  ******************************************************************************
  * @file    main.c
  * @author  Niraj Menon
  * @date    Sep 23, 2024
  * @brief   ECE 362 I2C Lab Student template
  ******************************************************************************
*/

#include "stm32f0xx.h"
#include <stdio.h>
#include "eeprom.h"

void internal_clock();
void enable_ports();
void init_i2c();
void i2c_start(uint32_t targadr, uint8_t size, uint8_t dir);
void i2c_stop();
void i2c_waitidle();
void i2c_clearnack();
int i2c_checknack();

//===========================================================================
// Configure SDA and SCL.
//===========================================================================
void enable_ports(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN; 
    GPIOB->MODER &= ~(0xF << (6 * 2)); 
    GPIOB->MODER |= (0xA << (6 * 2));  
    GPIOB->AFR[0] |= (1 << (6 * 4)) | (1 << (7 * 4)); 
}

//===========================================================================
// Configure I2C1.
//===========================================================================
void init_i2c(void) {
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;   
    I2C1->CR1 &= ~I2C_CR1_PE;      
    I2C1->CR1 |= I2C_CR1_ANFOFF;
    I2C1->CR1 |= I2C_CR1_ERRIE;
    I2C1->CR1 |= I2C_CR1_NOSTRETCH; 
    I2C1->TIMINGR = (0 << 28) | (3 << 20) | (1 << 16) | (3 << 8) | (9 << 0);
    //PRESC=0, SCLDEL=3, SDADEL=1, SCLH=3, SCLL=9 for 400kHz

             
        
    I2C1->CR2 &= ~I2C_CR2_ADD10;     
    I2C1->CR1 |= I2C_CR1_PE;   
}

//===========================================================================
// Send a START bit.
//===========================================================================
void i2c_start(uint32_t targadr, uint8_t size, uint8_t dir) {
    uint32_t tmpreg = I2C1->CR2;
    tmpreg &= ~(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RD_WRN |
                I2C_CR2_START | I2C_CR2_STOP);
    if (dir) {
        tmpreg |= I2C_CR2_RD_WRN;
    }
    else {
        tmpreg &= ~I2C_CR2_RD_WRN;
    }
    tmpreg |= ((targadr<<1) & I2C_CR2_SADD) | ((size << 16) & I2C_CR2_NBYTES);

    tmpreg |= I2C_CR2_START;
    I2C1->CR2 = tmpreg;
}

//===========================================================================
// Send a STOP bit.
//===========================================================================
void i2c_stop(void) {
    //if (I2C1->CR2 & I2C_CR2_STOP) return;
    if ((I2C1->ISR & I2C_ISR_STOPF)) return;
    I2C1->CR2 |= I2C_CR2_STOP;
    while (!(I2C1->ISR & I2C_ISR_STOPF));
    I2C1->ICR |= I2C_ICR_STOPCF;
}

//===========================================================================
// Wait until the I2C bus is not busy. (One-liner!)
//===========================================================================
void i2c_waitidle(void) {
    while (I2C1->ISR & I2C_ISR_BUSY);
    return;
}

//===========================================================================
// Send each char in data[size] to the I2C bus at targadr.
//===========================================================================
int8_t i2c_senddata(uint8_t targadr, uint8_t data[], uint8_t size) {
    i2c_waitidle();
    i2c_start(targadr, size, 0); // Write

    for (int i = 0; i < size; i++) {
        int count = 0;
        while ((I2C1->ISR & I2C_ISR_TXIS) == 0) {
            count += 1;
            if (count > 1000000)
                return -1;
            if (i2c_checknack()) {
                i2c_clearnack();
                i2c_stop();
                return -1;
            }
        }
        I2C1->TXDR = data[i] & I2C_TXDR_TXDATA;
    }

    while (!(I2C1->ISR & I2C_ISR_TC) && !(I2C1->ISR & I2C_ISR_NACKF));
    if (I2C1->ISR & I2C_ISR_NACKF) {
        //i2c_clearnack();
        //i2c_stop();
        return -1;
    }
    i2c_stop();
    return 0;
}

//===========================================================================
// Receive size chars from the I2C bus at targadr and store in data[size].
//===========================================================================
int i2c_recvdata(uint8_t targadr, uint8_t data[], uint8_t size) {
    i2c_waitidle();
    i2c_start(targadr, size, 1); // Read

    // Cast the void pointer to uint8_t pointer
    //uint8_t *recv_data = (uint8_t *)data;

    for (int i = 0; i < size; i++) {
        int count = 0;
        while ((I2C1->ISR & I2C_ISR_RXNE) == 0) {
            count += 1;
            if (count > 1000000)
                return -1;
            if (i2c_checknack()) {
                i2c_clearnack();
                i2c_stop();
                return -1;
            }
        }
        data[i] = I2C1->RXDR & I2C_RXDR_RXDATA;
        if (data[i] == 0)
            return 0;
    }
    //data[size] = '\0';

    return 0;
}

//===========================================================================
// Clear the NACK bit. (One-liner!)
//===========================================================================
void i2c_clearnack(void) {
    I2C1->ICR |= I2C_ICR_NACKCF;
}

//===========================================================================
// Check the NACK bit. (One-liner!)
//===========================================================================
int i2c_checknack(void) {
    return (I2C1->ISR & I2C_ISR_NACKF);
}

//===========================================================================
// EEPROM functions
// We'll give these so you don't have to figure out how to write to the EEPROM.
// These can differ by device.

#define EEPROM_ADDR 0x57

void eeprom_write(uint16_t loc, const char* data, uint8_t len) {
    uint8_t bytes[34];
    bytes[0] = loc>>8;
    bytes[1] = loc&0xFF;
    for(int i = 0; i<len; i++){
        bytes[i+2] = data[i];
    }
    i2c_senddata(EEPROM_ADDR, bytes, len+2);
}

void eeprom_read(uint16_t loc, char data[], uint8_t len) {
    // ... your code here
    uint8_t bytes[2];
    bytes[0] = loc>>8;
    bytes[1] = loc&0xFF;
    i2c_senddata(EEPROM_ADDR, bytes, 2);
    i2c_recvdata(EEPROM_ADDR, data, len);
}

//===========================================================================
// Copy in code from Lab 7 - USART
//===========================================================================

#include "fifo.h"
#include "tty.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define FIFOSIZE 16
// char serfifo[FIFOSIZE];
// int seroffset = 0;

// //===========================================================================
// // init_usart5
// //===========================================================================
// void init_usart5() {
//     // TODO
//     RCC->AHBENR |= RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIODEN;
//     GPIOC->MODER &= ~GPIO_MODER_MODER12;
//     GPIOC->MODER |= GPIO_MODER_MODER12_1;
//     GPIOC->AFR[1] &= ~GPIO_AFRH_AFSEL12;
//     GPIOC->AFR[1] |= 2 << GPIO_AFRH_AFSEL12_Pos;
//     GPIOD->MODER &= ~GPIO_MODER_MODER2;
//     GPIOD->MODER |= GPIO_MODER_MODER2_1;
//     GPIOD->AFR[0] &= ~GPIO_AFRL_AFSEL2;
//     GPIOD->AFR[0] |= 2 << GPIO_AFRL_AFSEL2_Pos;
//     RCC->APB1ENR |= RCC_APB1ENR_USART5EN;
//     USART5->CR1 &= ~USART_CR1_UE;
//     USART5->CR1 &= ~USART_CR1_M;
//     USART5->CR2 &= ~USART_CR2_STOP;
//     USART5->CR1 &= ~USART_CR1_PCE; 
//     USART5->CR1 &= ~USART_CR1_OVER8;
//     USART5->BRR = 0x1A1;
//     USART5->CR1 |= USART_CR1_TE | USART_CR1_RE;
//     USART5->CR1 |= USART_CR1_UE;
//     while (!(USART5->ISR & USART_ISR_TEACK));
//     while (!(USART5->ISR & USART_ISR_REACK));

// }

// //===========================================================================
// // enable_tty_interrupt
// //===========================================================================
// void enable_tty_interrupt(void) {
//     // TODO
//     USART5->CR1 |= USART_CR1_RXNEIE;
//     NVIC_EnableIRQ(USART3_8_IRQn);
//     USART5->CR3 |= USART_CR3_DMAR;
//     RCC->AHBENR |= RCC_AHBENR_DMA2EN;
//     DMA2->CSELR |= DMA2_CSELR_CH2_USART5_RX;
//     DMA2_Channel2->CCR &= ~DMA_CCR_EN;
//     DMA2_Channel2->CMAR = (uint32_t)serfifo;
//     DMA2_Channel2->CPAR = (uint32_t)&USART5->RDR;
//     DMA2_Channel2->CNDTR = FIFOSIZE;
//     DMA2_Channel2->CCR &= ~(DMA_CCR_DIR);
//     DMA2_Channel2->CCR |= (DMA_CCR_TCIE);
//     DMA2_Channel2->CCR |= DMA_CCR_CIRC;
//     DMA2_Channel2->CCR &= ~DMA_CCR_PINC;
//     DMA2_Channel2->CCR |= DMA_CCR_MINC;
//     DMA2_Channel2->CCR |= DMA_CCR_PL_1 | DMA_CCR_PL_0;
//     DMA2_Channel2->CCR |= DMA_CCR_EN;

// }

// //===========================================================================
// // interrupt_getchar
// //===========================================================================
// char interrupt_getchar() {
//     // TODO
//     char ch;
//     while(fifo_newline(&input_fifo) == 0) {
//         asm volatile ("wfi");
//     }
//     ch = fifo_remove(&input_fifo);
//     return ch;

// }

// //===========================================================================
// // __io_putchar
// //===========================================================================
// int __io_putchar(int c) {
//     // TODO copy from STEP2
//     if (c == '\n') {
//         while(!(USART5->ISR & USART_ISR_TXE));
//         USART5->TDR = '\r';
//     }
//     while(!(USART5->ISR & USART_ISR_TXE));
//     USART5->TDR = c;
//     return c;

// }

// //===========================================================================
// // __io_getchar
// //===========================================================================
// int __io_getchar(void) {
//     // TODO Use interrupt_getchar() instead of line_buffer_getchar()
//     return interrupt_getchar();

// }

// //===========================================================================
// // IRQHandler for USART5
// //===========================================================================
// void USART3_8_IRQHandler(void) {
//     while(DMA2_Channel2->CNDTR != sizeof serfifo - seroffset) {
//         if (!fifo_full(&input_fifo))
//             insert_echo_char(serfifo[seroffset]);
//         seroffset = (seroffset + 1) % sizeof serfifo;
//     }
// }

//===========================================================================
// Command functions for the shell
// These are the functions that are called when a command is entered, in 
// order to parse the command being entered, split into argc/argv calling 
// convention, and then execute the command by calling the respective 
// function.  We implement "write" and "read" for you to easily test your 
// EEPROM functions from a terminal.
//===========================================================================

void write(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: write <addr> <data>\n");
        printf("Ensure the address is a hexadecimal number.  No need to include 0x.\n");
        return;
    }
    uint32_t addr = strtol(argv[1], NULL, 16); 
    // concatenate all args from argv[2], until empty string is found, to a string
    char data[32] = "";
    int i = 0;
    int j = 2;
    while (strcmp(argv[j], "") != 0 && i < 32) {
        for (char c = argv[j][0]; c != '\0'; c = *(++argv[j])) {
            data[i++] = c;
        }
        if (strcmp(argv[j+1], "") != 0) {
            data[i++] = ' ';
        }
        // else {
        //      data[i + 1] = '\0';
        // }

        j++;
    }
    
    // ensure addr is a multiple of 32
    if ((addr % 32) != 0) {
        printf("Address 0x%ld is not evenly divisible by 32.  Your address must be a hexadecimal value.\n", addr);
        return;
    }
    int msglen = strlen(data);
    if (msglen > 32) {
        printf("Data is too long. Max length is 32.\n");
        return;
    }
    printf("Writing to address 0x%ld: %s\n", addr, data);
    eeprom_write(addr, data, msglen);
}

char* read(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: read <addr>\n");
        printf("Ensure the address is a hexadecimal number.  No need to include 0x.\n");
        return;
    }
    uint32_t addr = strtol(argv[1], NULL, 16); 
    char data_temp[32];
    // ensure addr is a multiple of 32
    if ((addr % 32) != 0) {
        printf("Address 0x%ld is not evenly divisible by 32.  Your address must be a hexadecimal value.\n", addr);
        return;
    }
    eeprom_read(addr, data_temp, 32);
    //printf("String at address 0x%ld: %s\n", addr, data);
    return data_temp;
}

struct commands_t {
    const char *cmd;
    void      (*fn)(int argc, char *argv[]);
};

struct commands_t cmd_io[] = {
    { "write", write },
    { "read", read }
};

void execute(int argc, char *argv[])
{
    for(int i=0; i<sizeof cmd_io/sizeof cmd_io[0]; i++)
        if (strcmp(cmd_io[i].cmd, argv[0]) == 0) {
            cmd_io[i].fn(argc, argv);
            return;
        }
    printf("%s: No such command.\n", argv[0]);
}

void parse_commands(char *c)
{
    char *argv[20];
    int argc=0;
    int skipspace=1;
    for(; *c; c++) {
        if (skipspace) {
            if (*c != ' ' && *c != '\t') {
                argv[argc++] = c;
                skipspace = 0;
            }
        } else {
            if (*c == ' ' || *c == '\t') {
                *c = '\0';
                skipspace=1;
            }
        }
    }
    if (argc > 0) {
        argv[argc] = "";
        execute(argc, argv);
    }
}

char* read_high_score() {
    //parse_commands("read 0");
    char data_temp[32];
    char* data = "";
    char* data_ptr = data;
    eeprom_read(0, data_temp, 32);
    for (int i = 0; i < 32; i++)
    {
        if (data_temp[i] != '\0')
        {
            data_ptr = data_temp[i];
        }
        else
        {
            data_ptr = '\0';
        }
        data_ptr++;
    }
    return data;
}

void write_high_score(int score) {
    char score_str[32];
    sprintf(score_str, "%d", score);
    eeprom_write(0, score_str, strlen(score_str));
}

//===========================================================================
// main()
//===========================================================================

// int main() {
//     internal_clock();
    
//     // I2C specific
//     enable_ports();
//     init_i2c();

//     // If you don't want to deal with the command shell, you can 
//     // comment out all code below and call 
//     // eeprom_read/eeprom_write directly.
//     init_usart5();
//     enable_tty_interrupt();
//     // These turn off buffering.
//     setbuf(stdin,0); 
//     setbuf(stdout,0);
//     setbuf(stderr,0);

//     printf("I2C Command Shell\n");
//     printf("This is a simple shell that allows you to write to or read from the I2C EEPROM at %d.\n", EEPROM_ADDR);
//     for(;;) {
//         printf("\n> ");
//         char line[100];
//         fgets(line, 99, stdin);
//         line[99] = '\0';
//         int len = strlen(line);
//         if (line[len-1] == '\n')
//             line[len-1] = '\0';
//         parse_command(line);
//     }
// }