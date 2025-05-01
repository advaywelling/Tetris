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
#include <strings.h>
#include <string.h>
#include "eeprom.h"
#include <ctype.h>
#include <math.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// Audio parameters
#define NOTE_COUNT 8
const float tune[NOTE_COUNT] = {
    261.63f,  // C4
    329.63f,  // E4
    392.00f,  // G4
    523.25f,  // C5
    659.26f,  // E5
    783.99f,  // G5
    523.25f,  // C5
    392.00f   // G4
};

#define NOTES_PER_SECOND 4  // Faster tempo for better flow
volatile uint8_t current_note = 0;
volatile uint32_t note_counter = 0;

#define N 1000
#define RATE 20000
short int wavetable[N];
int step0 = 0;
int offset0 = 0;
uint32_t volume = 4095; // Fixed volume (50%)

void init_wavetable(void);
void setup_dac(void);
void init_tim6(void);
void set_freq(float f);

// Audio functions
void init_wavetable(void) {
    for(int i=0; i<N; i++)
        wavetable[i] = 32767 * sin(2 * M_PI * i / N);
}

void setup_dac(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER |= 0x0300;        // Analog
    RCC->APB1ENR |= RCC_APB1ENR_DACEN;
    DAC->CR |= DAC_CR_EN1;         // Enable DAC channel 1
}

void init_tim6(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    TIM6->PSC = 48 - 1;            // 1MHz
    TIM6->ARR = (1000000/RATE) -1; // 20kHz update rate
    TIM6->DIER |= TIM_DIER_UIE;    // Enable update interrupt
    NVIC_EnableIRQ(TIM6_DAC_IRQn);
    TIM6->CR1 |= TIM_CR1_CEN;
}

void set_freq(float f) {
    step0 = (int)((f * N / RATE) * (1 << 16));
}void TIM6_DAC_IRQHandler(void) {
    static uint32_t interrupt_count = 0;
    const uint32_t notes_per_interrupt = RATE / NOTES_PER_SECOND;
    static uint8_t direction = 0; // 0=ascending, 1=descending
    
    TIM6->SR &= ~TIM_SR_UIF;
    
    // Update waveform position
    offset0 += step0;
    if(offset0 >= N << 16) offset0 -= N << 16;
    
    // Generate sample with volume variation
    int samp = wavetable[offset0 >> 16];
    int32_t scaled_samp = samp * (1 + (interrupt_count % 200 < 100)); // Pseudo-pulse width modulation
    DAC->DHR12R1 = (scaled_samp >> 5) + 2048;

    // Change note with dynamic pattern
    if(++interrupt_count >= notes_per_interrupt) {
        interrupt_count = 0;
        
        if(direction == 0) {
            if(++current_note >= NOTE_COUNT-1) direction = 1;
        } else {
            if(--current_note <= 0) direction = 0;
        }
        
        step0 = (int)((tune[current_note] * N / RATE * (1 << 16)));
        
        // Add subtle emphasis on first beat
        if(current_note == 0) {
            DAC->DHR12R1 = (samp >> 3) + 2048; // Temporary volume boost
            nano_wait(10000);
        }
    }
}
int score = 0;
int high_score = 0;

void internal_clock();



void clearButtons(){
    rightButton = 0;
    leftButton = 0;
    downButton = 0;
    holdButton = 0;
    rotRightButton = 0;
    rotLeftButton = 0;
}

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
            draw_cell(LEFT_EDGE - 1 + 12*j, TOP_EDGE + 12*i, display[i][j]);
        }
    }
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


void clear_line(int r){
    for(int i = 0; i < COLUMNS; i++){
        display[r][i] = BLACK; 
    }
    draw_display();
}

void shiftDown(int r){
    for(int row = r; row > 0; row--){
        
        for(int col = 0; col < COLUMNS; col++){
            display[row][col] = display[row-1][col];
        }
    }
}

void check_line_clear(){
    int lineFilled = 1;

    for(int r = ROWS - 1; r >= 0; r--){
        lineFilled = 1;

        for(int c = 0; c < COLUMNS; c++){

            if(display[r][c] == BLACK){
                lineFilled = 0;
                break;
            }
        }
        if(lineFilled == 1){
            score += 10;
            clear_line(r);
            shiftDown(r);
            clear_line(0);
            r++;
        }
    }

}

//returns 1 for touched bottom, 0 if in air 
int check_bottom(Piece* p){
    int x = (p->start_x - 60) / 12;
    int y = (p->start_y - 80) / 12;

    for(int i = 0; i < 4; i++){
        if(y+p->blocks[i].y + 1 > 19 || display[y + p->blocks[i].y + 1][x + p->blocks[i].x] != BLACK){
            for(int j = 0; j < 4; j++){
                display[y+p->blocks[j].y][x+p->blocks[j].x] = p->color;
            }
            check_line_clear();
            return 1;
        }
    }
    return 0;
}

int generate_color(void){
    int r = rand() % 7 + 1;
    int color;
    switch (r) {
        case 1:
            color = CYAN;
            break;
        case 2:
            color = YELLOW;
            break;
        case 3:
            color = PURPLE;
            break;
        case 4:
            color = ORANGE;
            break;
        case 5:
            color = BLUE;
            break;
        case 6:
            color = RED;
            break;
        case 7:
            color = GREEN;
            break;
        default:
            color = WHITE;
            break;
    }
    return color;
}

//TODO: set back to 0 or 1 to test start menu or pure game functionality
int state = 0;
int cnt = 0;
int holdcycle = 1;
int title_color;
int main() {
    enable_ports();
    init_i2c();
    srand(31);
    internal_clock();
    //audio
    init_wavetable();
    setup_dac();
    init_tim6();
    //set_freq(440.0); 
    step0 = (int)((tune[0] * N / RATE * (1 << 16)));
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
    //draw_title(ORANGE);
    draw_borders(WHITE);
    draw_hold(WHITE);
    draw_score(WHITE);
    draw_next(WHITE);
    current_piece = NULL;
    next_piece = NULL;
    hold_piece = NULL;
    draw_display();
    score = 0;
    char* temp_data = read_high_score();
    char high_score_str[32];
    //high_score_str = read_high_score();
    //strcpy(high_score_str, temp_data);
    for (int i = 0; i < 32 && temp_data[i] != '\0'; i++)
    {
        if (isdigit(temp_data[i]))
        {
            high_score_str[i] = temp_data[i];
        }
        else
        {
            high_score_str[i] = '\0';
            break;
        }        
    }
    high_score_str[31] = '\0';
    //high_score_str = temp_data;
    char score_str[32];
    strcpy(score_str, "0");
    if (!strcmp(high_score_str, 0))
    {
        high_score = 0;
        strcpy(high_score_str, "0");
    } else
    {
        high_score = atoi(high_score_str);
    }
    
    
    //high_score = atoi(high_score_str);
    //read_high_score();
    //actual game
    while(1){
        switch(state){
            //title screen
            case 0: 
                //TODO: clear score function (draw black box below score on the mid left)
                title_color = generate_color();
                draw_title(title_color);
                LCD_DrawString(69, 140, WHITE, BLACK, "Press Right", 19, 0);
                LCD_DrawString(82, 160, WHITE, BLACK, "To START", 19, 0);
                LCD_DrawString(62, 190, WHITE, BLACK, "Left: Controls", 17, 0);
                nano_wait(500000000);
                clearButtons();
                while(!(leftButton || rightButton || rotLeftButton)){
                    LCD_DrawString(69, 140, WHITE, BLACK, "Press Right", 19, 0);
                    LCD_DrawString(82, 160, WHITE, BLACK, "To START", 19, 0);
                    LCD_DrawString(62, 190, WHITE, BLACK, "Left: Controls", 17, 0);
                    LCD_DrawString(69, 240, WHITE, BLACK, "High Score:", 19, 0); //START HOGH SCORE
                    LCD_DrawString(69, 270, WHITE, BLACK, high_score_str, 19, 0);
                }
                if(rightButton){
                    score = 0;
                    srand(TIM2->CNT);
                    state = 1;
                    cnt = 0;
                    draw_display();
                    clearButtons();
                }
                if(leftButton || rotLeftButton){
                    state = 4;
                    draw_display();
                    clearButtons();
                    
                }
                break;
            //playing game
            case 1:
                sprintf(score_str, "%d", score);
                LCD_DrawString(2, 205, WHITE, BLACK, score_str, 19, 0);
                if(next_piece == NULL) next_piece = generate_piece();
                if(current_piece == NULL) current_piece = generate_piece();
                draw_piece(current_piece->blocks, current_piece->start_x, current_piece->start_y, current_piece->color);
                draw_next_piece(next_piece, 0);  
                if(check_bottom(current_piece)){
                    draw_display();
                    free(current_piece->blocks);
                    free(current_piece);
                    holdcycle = 1;
                    
                    check_line_clear();
                    if(check_bottom(next_piece)){
                        if(hold_piece != NULL){
                            draw_hold_piece(hold_piece, 1);
                            free(hold_piece->blocks);
                            free(hold_piece);
                        }
                        hold_piece = NULL;
                        clearButtons();
                        state = 2; //CHECK SCORE VS HIGH SCORE
                        if (score > high_score)
                        {
                            write_high_score(score);
                            sprintf(high_score_str, "%d", score);
                        }                        
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
                        nano_wait(70000000);
                    }
                    check_line_clear();
                    current_piece = next_piece;
                    draw_next_piece(next_piece, 1);
                    next_piece = NULL; //gens next cycle of while loop
                    holdcycle = 1;
                    downButton = 0;
                    cnt = 0;
                }

                else if(holdButton){
                    draw_hold_piece(hold_piece, 1); //clears old piece
                    if(hold_piece == NULL && holdcycle != 0){
                        hold_piece = current_piece;
                        draw_piece(current_piece->blocks, current_piece->start_x, current_piece->start_y, BLACK);
                        next_piece->start_x = 108;
                        next_piece->start_y =  (next_piece->color == YELLOW) ? 81 : 93;
                        current_piece = next_piece;
                        draw_next_piece(next_piece, 1);
                        next_piece = generate_piece();
                        draw_next_piece(next_piece, 1);
                        holdcycle = 0;
                    }
                    else if(holdcycle != 0){
                        draw_piece(current_piece->blocks, current_piece->start_x, current_piece->start_y, BLACK);
                        hold_piece->start_x = 108;
                        hold_piece->start_y = (hold_piece->color == YELLOW) ? 81 :  93;
                        temp_piece = current_piece;
                        current_piece = hold_piece;
                        hold_piece = temp_piece;
                        holdcycle = 0;
                    }
                    holdButton = 0;
                    draw_hold_piece(hold_piece, 0); //draws new piece
                }
                break;
            //end screen
            case 2: 
                LCD_DrawString(76, 120, RED, BLACK, "GAME OVER", 19, 0);
                nano_wait(500000000);
                clearButtons();

                while(!(rightButton || leftButton || rotLeftButton)){
                    LCD_DrawString(76, 120, RED, BLACK, "GAME OVER", 19, 0);
                    LCD_DrawString(69, 170, RED, BLACK, "Press Right", 19, 0);
                    LCD_DrawString(82, 190, RED, BLACK, "To  MENU", 17, 0);
                    LCD_DrawString(69, 230, RED, BLACK, "High Score:", 19, 0); //HIGH SCORE END
                    LCD_DrawString(69, 270, RED, BLACK, high_score_str, 19, 0);
                    LCD_DrawString(60, 300, RED, BLACK, "Left: How to Play", 15, 0);
                }
                if(rightButton){
                    state = 0;
                    //erase old score when leaving ending screen
                    LCD_DrawString(2, 205, BLACK, BLACK, score_str, 19, 0);
                    init_game(BLACK);
                    draw_display();
                    clearButtons();
                }
                if(rotLeftButton || leftButton){
                    state = 5;
                    //erase old score when leaving ending screen
                    LCD_DrawString(2, 205, BLACK, BLACK, score_str, 19, 0);
                    init_game(BLACK);
                    draw_display();
                    clearButtons();
                }
                break;
            //error state (shouldn't ever enter)
            case 3: 
                init_game(RED);
                draw_display();
                nano_wait(10000000000);
                state = 0;
                break;

            //controls
            case 4:
                while(!rightButton){
                    LCD_DrawString(60, 120, WHITE, BLACK, "--Left to Right--", 15, 0);
                    LCD_DrawString(69, 140, WHITE, BLACK, "Rotate Left", 19, 0);
                    LCD_DrawString(69, 160, WHITE, BLACK, "Hold", 19, 0);
                    LCD_DrawString(69, 180, WHITE, BLACK, "Rotate Right", 19, 0);
                    LCD_DrawString(69, 200, WHITE, BLACK, "Move Left", 19, 0);
                    LCD_DrawString(69, 220, WHITE, BLACK, "Move Down", 19, 0);
                    LCD_DrawString(69, 240, WHITE, BLACK, "Move Right", 19, 0);
                    LCD_DrawString(69, 280, WHITE, BLACK, "Press Right", 19, 0);
                    LCD_DrawString(82, 300, WHITE, BLACK, "To  MENU", 17, 0);
                }
                if(rightButton){
                    state = 0;
                    init_game(BLACK);
                    draw_display();
                    clearButtons();
                }
                break;
            //every game needs a small easter egg at least
            case 5:
                init_game(BLACK);
                draw_display();
                while(!(holdButton || downButton || rightButton || leftButton || rotLeftButton || rotRightButton)){
                    LCD_DrawString(65, 120, RED, BLACK, "You know how", 18, 0);
                    LCD_DrawString(83, 140, RED, BLACK, "to PLAY.", 18, 0);
                }
                if(holdButton || downButton || rightButton || leftButton || rotLeftButton || rotRightButton){
                    state = 0;
                    init_game(BLACK);
                    draw_display();
                    clearButtons();
                }
                break;
            //shouldn't enter
            default: 
                state = 3;
                break;
        }

    }
}