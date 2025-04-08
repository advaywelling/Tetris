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
#include <time.h>

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
    srand(53);
    Piece* current_piece = generate_piece(); 

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
    create_first_piece(current_piece);
    nano_wait(100000000);
    nano_wait(100000000);
    nano_wait(100000000);
    nano_wait(100000000);
    rotate_piece_counterclockwise(current_piece);
    free(current_piece->blocks);
    free(current_piece);
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
