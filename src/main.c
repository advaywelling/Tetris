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


const Point blocks_T[4] = {
    { -1,  0 },
    {  0,  0 },  // pivot cell
    {  1,  0 },
    {  0,  -1 }
};

const Point blocks_L[4] = {
    { -1,  0 },
    {  0,  0 },  // pivot cell
    {  1,  0 },
    {  1,  -1 }
};

const Point blocks_J[4] = {
    { -1,  0 },
    {  0,  0 },  // pivot cell
    {  1,  0 },
    {  -1,  -1 }
};

const Point blocks_O[4] = {
    { -1,  0 },
    {  0,  0 },  // pivot cell
    { -1,  1 },
    {  0,  1 }
};

const Point blocks_S[4] = {
    { -1,  0 },
    {  0,  0 },  // pivot cell
    {  0, -1 },
    {  1, -1 }
};

const Point blocks_Z[4] = {
    { -1, -1 },
    {  0,  0 },  // pivot cell
    {  0, -1 },
    {  1,  0 }
};

const Point blocks_I[4] = {
    { -1,  0 },
    {  0,  0 },  // pivot cell
    {  1,  0 },
    {  2,  0 }
};

Point test_blocks[4] = {
    { -1,  0 },
    {  0,  0 },  // pivot cell
    {  1,  0 },
    {  -1,  -1 }
}; 

Piece current_piece;
Piece piece_J;
Piece piece_L;
Piece piece_I;
Piece piece_O;
Piece piece_S;
Piece piece_Z;
Piece piece_T;


int main() {

    piece_J.blocks = blocks_J;
    piece_J.start_x = 108;
    piece_J.start_y = 80;
    piece_J.color = BLUE;

    piece_L.blocks = blocks_L;
    piece_L.start_x = 108;
    piece_L.start_y = 80;
    piece_L.color = ORANGE;

    piece_I.blocks = blocks_I;
    piece_I.start_x = 108;
    piece_I.start_y = 80;
    piece_I.color = CYAN;

    piece_O.blocks = blocks_O;
    piece_O.start_x = 108;
    piece_O.start_y = 80;
    piece_O.color = YELLOW;

    piece_S.blocks = blocks_S;
    piece_S.start_x = 108;
    piece_S.start_y = 80;
    piece_S.color = GREEN;

    piece_Z.blocks = blocks_Z;
    piece_Z.start_x = 108;
    piece_Z.start_y = 80;
    piece_Z.color = RED;

    piece_T.blocks = blocks_T;
    piece_T.start_x = 108;
    piece_T.start_y = 80;
    piece_T.color = PURPLE;

    current_piece.blocks = test_blocks;
    current_piece.start_x = 108;
    current_piece.start_y = 80;
    current_piece.color = BLUE;

    internal_clock();
    setup_TIM2();
    init_usart5();

    enable_tty_interrupt();
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    LCD_Setup();
    LCD_Clear(0000);
    draw_piece(current_piece.blocks, 108, 80, BLUE);
    nano_wait(1000000000);
    rotate_piece_clockwise(&current_piece);
    //draw_piece(current_piece.blocks, current_piece.start_x, current_piece.start_y, current_piece.color);
    
    //command_shell();
}
