#include "stm32f0xx.h"
#include <stdint.h>
#include <stdio.h>
#include "fifo.h"
#include "tty.h"
#include "lcd.h"
#include "commands.h"
#include "spi_tft.h"
#include "buttons.h"

void internal_clock();

Point piece_J_rotation[4] = {
    { -1,  0 },
    {  0,  0 },  // pivot cell
    {  1,  0 },
    {  -1,  -1 }
};

int main() {
    internal_clock();
    setup_TIM2();
    init_usart5();

    enable_tty_interrupt();
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    LCD_Setup();
    LCD_Clear(0000);
    //draw_J(50, 50, 0x0ff0);
    //draw_piece(piece_J_rotation, 50, 50, 0x0ff0);
    rotate_point_anticlockwise(piece_J_rotation);
    draw_piece(piece_J_rotation, 50, 50, 0x0ff0);
    //command_shell();
}
