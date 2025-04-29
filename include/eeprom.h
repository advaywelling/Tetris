#ifndef __EEPROM_H__
#define __EEPROM_H__

// #include "stm32f0xx.h"
#include <stdint.h>

// Function Declarations

// I2C Setup Functions
void enable_ports(void);
void init_i2c(void);

// I2C Communication Functions
void i2c_start(uint32_t targadr, uint8_t size, uint8_t dir);
void i2c_stop(void);
void i2c_waitidle(void);
void i2c_clearnack(void);
int i2c_checknack(void);
int8_t i2c_senddata(uint8_t targadr, uint8_t data[], uint8_t size);
int i2c_recvdata(uint8_t targadr, void *data, uint8_t size);

// EEPROM Functions
void eeprom_write(uint16_t loc, const char* data, uint8_t len);
void eeprom_read(uint16_t loc, char data[], uint8_t len);

// Command Functions for the Shell
void write(int argc, char* argv[]);
char* read(int argc, char* argv[]);

// Helper Functions
void execute(int argc, char *argv[]);
void parse_commands(char *c);

// High Score Functions
char* read_high_score(void);
void write_high_score(int score);
void intToStr(int N, char *str);

#endif // I2C_LAB_H
