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


void init_game(int color){
    leftButton = 0;
    rightButton = 0;
    downButton = 0;
    rotRightButton = 0;
    rotLeftButton = 0;
    holdButton = 0;
    for(int i = 0; i < ROWS; i++){
        for(int j = 0; j < COLUMNS; j++){
            display[i][j] = color;
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

void draw_start_screen(int color){
    init_game(BLUE);
    draw_display();
}

Piece* next_piece;
Piece* hold_piece;
Piece* current_piece;
Piece* temp_piece;

//call clear w 1 & prev piece to clear before drawing a new piece
void draw_next_piece(Piece* p, int clear){
    int shift = 0;
    //o check
    if(p->blocks[0].x == blocks_O[0].x && p->blocks[1].x == blocks_O[1].x
      && p->blocks[2].x == blocks_O[2].x && p->blocks[3].x == blocks_O[3].x &&
      p->blocks[0].y == blocks_O[0].y && p->blocks[1].y == blocks_O[1].y
      && p->blocks[2].y == blocks_O[2].y && p->blocks[3].y == blocks_O[3].y){
        shift = 6;
    }
    //i check
    if(p->blocks[0].x == blocks_I[0].x && p->blocks[1].x == blocks_I[1].x
        && p->blocks[2].x == blocks_I[2].x && p->blocks[3].x == blocks_I[3].x &&
        p->blocks[0].y == blocks_I[0].y && p->blocks[1].y == blocks_I[1].y
        && p->blocks[2].y == blocks_I[2].y && p->blocks[3].y == blocks_I[3].y){
        shift = -6;
    }
    if(p->blocks[0].x == -1*blocks_I[0].x && p->blocks[1].x == -1*blocks_I[1].x
        && p->blocks[2].x == -1*blocks_I[2].x && p->blocks[3].x == -1*blocks_I[3].x &&
        p->blocks[0].y == blocks_I[0].y && p->blocks[1].y == blocks_I[1].y
        && p->blocks[2].y == blocks_I[2].y && p->blocks[3].y == blocks_I[3].y){
        shift = 12;
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

//returns 1 for touched bottom, 0 if in air 
int check_bottom(Piece* p){
    int x = (p->start_x - 60) / 12;
    int y = (p->start_y-80) / 12;

    for(int i = 0; i < 4; i++){
        if(y+p->blocks[i].y + 1 > 19 || display[y + p->blocks[i].y + 1][x + p->blocks[i].x] != BLACK){
            for(int j = 0; j < 4; j++){
                display[y+p->blocks[j].y][x+p->blocks[j].x] = p->color;
            }
            return 1;
        }
    }
    return 0;
}

void check_line_clear(){
    for(int i = 0; i < ROWS; i++){
        for(int j = 0; j < COLUMNS; j++){
            if(display[i][j] == BLACK){
                break;
            }
            if(j == COLUMNS - 1){
                //clear line
            }
        }
    }
}

//TODO: set back to 1 to test start menu / full game functionality
int state = 1;
int cnt = 0;
int main() {
    srand(31);

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
    init_game(BLACK);
    LCD_Clear(BLACK);
    draw_title(ORANGE);
    draw_borders(WHITE);
    draw_hold(WHITE);
    draw_score(WHITE);
    draw_next(WHITE);
    current_piece = NULL;
    next_piece = NULL;
    hold_piece = NULL;

    //draw_display()
    //actual game
    while(1){
        switch(state){
            //title screen
            case 0: 
                //TODO: restart timer
                draw_start_screen(WHITE);
                if(holdButton || downButton || rightButton || leftButton || rotLeftButton || rotRightButton){
                    state = 1;
                    init_game(BLACK);
                    draw_display();
                    nano_wait(500000000);
                }
                break;
            //playing game
            case 1:
                //TODO: set srand + stop timer
                
                if(next_piece == NULL) next_piece = generate_piece();
                if(current_piece == NULL) current_piece = generate_piece();
                draw_piece(current_piece->blocks, current_piece->start_x, current_piece->start_y, current_piece->color);
                draw_next_piece(next_piece, 0);  
                
                if(check_bottom(current_piece)){
                    draw_piece(current_piece->blocks, current_piece->start_x, current_piece->start_y, current_piece->color);
                    // free(current_piece->blocks);
                    // free(current_piece);
                    //TODO: save struct somewhere for line clearing, can't just free it.
                    if(check_bottom(next_piece)){
                        state = 2;
                    }
                    current_piece = next_piece;
                    draw_next_piece(next_piece, 1);
                    next_piece = NULL; //gens next cycle of while loop
                }
                cnt++;
                if(cnt > 300){
                    shift_piece_down(current_piece);
                    cnt = 0;
                }
                nano_wait(500000);

                if(rotLeftButton){
                    rotate_piece_counterclockwise(current_piece);
                    rotLeftButton = 0;
                }
                else if(rotRightButton){
                    rotate_piece_clockwise(current_piece);
                    rotRightButton = 0;
                }

                if(leftButton){
                    shift_piece_left(current_piece);
                    leftButton = 0;
                }
                else if(rightButton){
                    shift_piece_right(current_piece);
                    rightButton = 0;
                }

                if(downButton){
                    //this will drop fully down
                    while(!check_bottom(current_piece)){
                        shift_piece_down(current_piece);
                    }
                 
                    downButton = 0;
                }

                else if(holdButton){
                    draw_hold_piece(hold_piece, 1); //clears old piece
                    if(hold_piece == NULL){
                        hold_piece = current_piece;
                        draw_piece(current_piece->blocks, current_piece->start_x, current_piece->start_y, BLACK);
                        next_piece->start_x = current_piece->start_x;
                        next_piece->start_y = current_piece->start_y;
                        current_piece = next_piece;
                        draw_next_piece(next_piece, 1);
                        next_piece = generate_piece();
                        draw_next_piece(next_piece, 1);
                    }
                    else{
                        draw_piece(current_piece->blocks, current_piece->start_x, current_piece->start_y, BLACK);
                        hold_piece->start_x = current_piece->start_x;
                        hold_piece->start_y = current_piece->start_y;
                        temp_piece = current_piece;
                        current_piece = hold_piece;
                        hold_piece = temp_piece;
                    }
                    holdButton = 0;
                    draw_hold_piece(hold_piece, 0); //draws new piece
                }
                break;
            //end screen
            case 2: 
                // if(holdButton || downButton || rightButton || leftButton || rotLeftButton || rotRightButton){
                //     state = 1;
                //     init_game(BLACK);
                //     draw_display();
                // }
                break;
            //error state (shouldn't ever enter)
            case 3: 
                init_game(RED);
                draw_display();
                nano_wait(10000000000);
                state = 0;
                break;
            //shouldn't enter
            default: 
                state = 3;
                break;
        }

    }
}