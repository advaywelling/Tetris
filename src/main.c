#include "stm32f0xx.h"
#include <stdint.h>
#include <stdio.h>
#include "fifo.h"
#include "tty.h"
#include "lcd.h"
#include "commands.h"
#include "spi_tft.h"
#include "buttons.h"
#include "drawbg.h"

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

Piece* next_piece;
Piece* hold_piece;

//call clear w 1 & prev piece to clear before drawing a new piece
void draw_next_piece(Piece* p, int clear){
    int shift = 0;
    if(p->blocks[0].x == blocks_O[0].x && p->blocks[1].x == blocks_O[1].x
      && p->blocks[2].x == blocks_O[2].x && p->blocks[3].x == blocks_O[3].x &&
      p->blocks[0].y == blocks_O[0].y && p->blocks[1].y == blocks_O[1].y
      && p->blocks[2].y == blocks_O[2].y && p->blocks[3].y == blocks_O[3].y){
        shift = 6;
    }
    if(p->blocks[0].x == blocks_I[0].x && p->blocks[1].x == blocks_I[1].x
        && p->blocks[2].x == blocks_I[2].x && p->blocks[3].x == blocks_I[3].x &&
        p->blocks[0].y == blocks_I[0].y && p->blocks[1].y == blocks_I[1].y
        && p->blocks[2].y == blocks_I[2].y && p->blocks[3].y == blocks_I[3].y){
        shift = -6;
    }
    int color = clear == 1 ? BLACK : p->color;
    draw_piece(p->blocks, 186+3+18+shift, 130, color);
}

//call clear w 1 & prev piece to clear before drawing a new piece
void draw_hold_piece(Piece* p, int clear){
    if(p != NULL){
        //draw it bc hold can be no piece if game just started
        int shift = 0;
        if(p->blocks[0].x == blocks_O[0].x && p->blocks[1].x == blocks_O[1].x
        && p->blocks[2].x == blocks_O[2].x && p->blocks[3].x == blocks_O[3].x &&
        p->blocks[0].y == blocks_O[0].y && p->blocks[1].y == blocks_O[1].y
        && p->blocks[2].y == blocks_O[2].y && p->blocks[3].y == blocks_O[3].y){
            shift = 6;
        }
        if(p->blocks[0].x == blocks_I[0].x && p->blocks[1].x == blocks_I[1].x
            && p->blocks[2].x == blocks_I[2].x && p->blocks[3].x == blocks_I[3].x &&
            p->blocks[0].y == blocks_I[0].y && p->blocks[1].y == blocks_I[1].y
            && p->blocks[2].y == blocks_I[2].y && p->blocks[3].y == blocks_I[3].y){
            shift = -6;
        }
        int color = clear == 1 ? BLACK : p->color;
        draw_piece(p->blocks, 3+18+shift, 130, color);
    }
}

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
    //game setup
    init_game();
    LCD_Clear(BLACK);
    draw_title(ORANGE);
    draw_borders(WHITE);
    draw_hold(WHITE);
    draw_score(WHITE);
    draw_next(WHITE);
    //game logic
    //draw_display()
    hold_piece = NULL;
    next_piece = &piece_L; //copy_piece(&piece_L)
    hold_piece = &piece_O; //tmp for testing
    draw_hold_piece(hold_piece, 0); //tmp for testing
    while(1){
        //draw_display(); //THIS IS SO SLOW, maybe just redraw the update every update?
        
        draw_next_piece(next_piece, 0); //can make this conditional once we create new piece gen
        
        if(holdButton){
            draw_hold_piece(hold_piece, 1);
            if(hold_piece == NULL){
                //hold_piece = curr_piece
                //curr_piece = next_piece
                //generate new next piece
            }
            else{
                //curr_piece = temp
                //curr_piece = hold_piece
                //hold_piece = temp
            }
            hold_piece = next_piece; //temp logic to test buttons
            holdButton = 0;
            draw_hold_piece(hold_piece, 0);
            draw_next_piece(next_piece, 1);
            next_piece = &piece_T;
        }
    }
    
    //draw_piece(current_piece.blocks, current_piece.start_x, current_piece.start_y, current_piece.color);
    
    //command_shell();
}
