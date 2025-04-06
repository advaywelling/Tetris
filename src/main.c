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

#define COLUMNS 10
#define ROWS 20
#define TOP_EDGE 80
#define LEFT_EDGE 60
#define BOT_EDGE 320
#define RIGHT_EDGE 180

int display[ROWS][COLUMNS];


void init_game(){
    leftButton = 0;
    rightButton = 0;
    downButton = 0;
    rotRightButton = 0;
    rotLeftButton = 0;
    holdButton = 0;
    for(int i = 0; i < ROWS; i++){
        for(int j = 0; j < COLUMNS; j++){
            display[i][j] = BLACK;
        }
    }
}

void draw_display(){
    for(int i = 0; i < ROWS; i++){
        for(int j = 0; j < COLUMNS; j++){
            LCD_DrawFillRectangle(LEFT_EDGE, TOP_EDGE, LEFT_EDGE + 12*(j+1), TOP_EDGE + 12*(i+1), display[i][j]);
        }
    }
}

void draw_borders(int color){
    LCD_DrawFillRectangle(0, TOP_EDGE-6, 240, TOP_EDGE-1, color);
    LCD_DrawFillRectangle(LEFT_EDGE-6, TOP_EDGE, LEFT_EDGE-1, 320, color);
    LCD_DrawFillRectangle(RIGHT_EDGE+1, TOP_EDGE, RIGHT_EDGE+7, BOT_EDGE, color);
}

void draw_title(int color){
    int ygap = 17;
    int xgap = 24;
    int len = 8;
    LCD_DrawFillRectangle(xgap, ygap, xgap+len*3, ygap+len, color);
    LCD_DrawFillRectangle(xgap+len, ygap+len,xgap+len*2, ygap+len*5, color);
    xgap = xgap + len*4;
    LCD_DrawFillRectangle(xgap, ygap, xgap+len*3, ygap+len, color);
    LCD_DrawFillRectangle(xgap, ygap+len, xgap+len, ygap+len*5, color);
    LCD_DrawFillRectangle(xgap+len, ygap+len*2, xgap+len*2, ygap+len*3, color);
    LCD_DrawFillRectangle(xgap+len, ygap+len*4, xgap+len*3, ygap+len*5, color);
    xgap = xgap + len*4;
    LCD_DrawFillRectangle(xgap, ygap, xgap+len*3, ygap+len, color);
    LCD_DrawFillRectangle(xgap+len, ygap+len,xgap+len*2, ygap+len*5, color);
    xgap = xgap + len*4;
    LCD_DrawFillRectangle(xgap, ygap, xgap+len*4, ygap+len, color);
    LCD_DrawFillRectangle(xgap, ygap+len, xgap+len, ygap+len*5, color);
    LCD_DrawFillRectangle(xgap, ygap+len*2, xgap+len*4, ygap+len*3, color);
    LCD_DrawFillRectangle(xgap+len*2, ygap+len*3, xgap+len*3, ygap+len*4, color);
    LCD_DrawFillRectangle(xgap+len*3, ygap+len*4, xgap+len*4, ygap+len*5, color);
    LCD_DrawFillRectangle(xgap+len*3, ygap, xgap+len*4, ygap+len*3, color);
    xgap = xgap + len*5;
    LCD_DrawFillRectangle(xgap, ygap, xgap+len*3, ygap+len, color);
    LCD_DrawFillRectangle(xgap+len, ygap+len, xgap+len*2, ygap+len*5, color);
    LCD_DrawFillRectangle(xgap, ygap+len*4, xgap+len*3, ygap+len*5, color);
    xgap = xgap + len*4;
    LCD_DrawFillRectangle(xgap, ygap, xgap+len*3, ygap+len, color);
    LCD_DrawFillRectangle(xgap, ygap, xgap+len, ygap+len*3, color);
    LCD_DrawFillRectangle(xgap+len, ygap+len*2, xgap+len*2, ygap+len*3, color);
    LCD_DrawFillRectangle(xgap+len*2, ygap+len*3, xgap+len*3, ygap+len*5, color);
    LCD_DrawFillRectangle(xgap, ygap+len*4, xgap+len*2, ygap+len*5, color);


    xgap = 24;
    LCD_DrawRectangle(xgap, ygap, xgap+len*3, ygap+len, ~color);
    LCD_DrawRectangle(xgap+len, ygap+len,xgap+len*2, ygap+len*5, ~color);
    xgap = xgap + len*4;
    LCD_DrawRectangle(xgap, ygap, xgap+len*3, ygap+len, ~color);
    LCD_DrawRectangle(xgap, ygap+len, xgap+len, ygap+len*5, ~color);
    LCD_DrawRectangle(xgap+len, ygap+len*2, xgap+len*2, ygap+len*3, ~color);
    LCD_DrawRectangle(xgap+len, ygap+len*4, xgap+len*3, ygap+len*5, ~color);
    xgap = xgap + len*4;
    LCD_DrawRectangle(xgap, ygap, xgap+len*3, ygap+len, ~color);
    LCD_DrawRectangle(xgap+len, ygap+len,xgap+len*2, ygap+len*5, ~color);
    xgap = xgap + len*4;
    LCD_DrawRectangle(xgap, ygap, xgap+len*4, ygap+len, ~color);
    LCD_DrawRectangle(xgap, ygap+len, xgap+len, ygap+len*5, ~color);
    LCD_DrawRectangle(xgap, ygap+len*2, xgap+len*4, ygap+len*3, ~color);
    LCD_DrawRectangle(xgap+len*2, ygap+len*3, xgap+len*3, ygap+len*4, ~color);
    LCD_DrawRectangle(xgap+len*3, ygap+len*4, xgap+len*4, ygap+len*5, ~color);
    LCD_DrawRectangle(xgap+len*3, ygap, xgap+len*4, ygap+len*3, ~color);
    xgap = xgap + len*5;
    LCD_DrawRectangle(xgap, ygap, xgap+len*3, ygap+len, ~color);
    LCD_DrawRectangle(xgap+len, ygap+len, xgap+len*2, ygap+len*5, ~color);
    LCD_DrawRectangle(xgap, ygap+len*4, xgap+len*3, ygap+len*5, ~color);
    xgap = xgap + len*4;
    LCD_DrawRectangle(xgap, ygap, xgap+len*3, ygap+len, ~color);
    LCD_DrawRectangle(xgap, ygap, xgap+len, ygap+len*3, ~color);
    LCD_DrawRectangle(xgap+len, ygap+len*2, xgap+len*2, ygap+len*3, ~color);
    LCD_DrawRectangle(xgap+len*2, ygap+len*3, xgap+len*3, ygap+len*5, ~color);
    LCD_DrawRectangle(xgap, ygap+len*4, xgap+len*2, ygap+len*5, ~color);
}


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
    //buttons setup
    setup_TIM2();
    //lcd setup
    init_usart5();
    enable_tty_interrupt();
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    LCD_Setup();
    //game logic
    init_game();
    LCD_Clear(BLACK);
    draw_title(ORANGE);
    draw_borders(WHITE);
    while(1){
        draw_display();
    }
    
    //draw_piece(current_piece.blocks, current_piece.start_x, current_piece.start_y, current_piece.color);
    
    //command_shell();
}
