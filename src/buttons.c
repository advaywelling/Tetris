#include "stm32f0xx.h"
#include "buttons.h"
//PA5 rotate left
//PA6 hold
//PA7 rotate right
//PA8 move left
//PA9 move down
//PA10 move right


//for debouncing
uint16_t buttonBounce[6];

void setup_gpio(){
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    
    //pa5-10
    GPIOA->MODER &= ~(0xFFF << 10);
    GPIOA->PUPDR |= (0xAAA << 10);
    GPIOA->PUPDR &= ~(0x555 << 10);
}

void setup_TIM2(){
    setup_gpio();
    for(int i = 0; i < 6; i++){
        buttonBounce[i] = 0;
    }

    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    //2khz
    TIM2->PSC = 240-1;
    TIM2->ARR = 100-1;
    TIM2->DIER |= TIM_DIER_UIE;
    TIM2->CR1 &= ~TIM_CR1_DIR;

    TIM2->CR1 |= TIM_CR1_CEN;
    NVIC->ISER[0] = 1 << TIM2_IRQn;
}

void TIM2_IRQHandler(){
    TIM2->SR &= ~TIM_SR_UIF;

    //pa5-10
    int inputs = (GPIOA->IDR & (0x3F << 5));
    inputs = inputs >> 5; //makes them right aligned

    for(int i = 0; i < 6; i++){
        buttonBounce[i] = (buttonBounce[i] << 1);
        buttonBounce[i] |= 1 & (inputs >> i);

        if(((buttonBounce[i] & 0xF) == 0xF) && (buttonBounce[i] & 0xF000) == 0){
            buttonBounce[i] = 0xFFFF;
            
            if(i == 0){
                //rotate left
                rotLeftButton = 1;
            }
            if(i==1){
                //hold
                holdButton = 1;
            }
            if(i==2){
                //rotate right
                rotRightButton = 1;
            }
            if(i==3){
                //move left
                leftButton = 1;
            }
            if(i==4){
                //move down
                downButton = 1;
            }
            if(i==5){
                //move right
                rightButton = 1;
            }
        }
    }
}

// #define LCD_WIDTH  240
// #define LCD_HEIGHT 320
// #define NUM_WIDTH  30
// #define NUM_HEIGHT 50
// #define START_X   ((LCD_WIDTH - NUM_WIDTH) / 2)
// #define START_Y   ((LCD_HEIGHT - NUM_HEIGHT) / 2)

// void draw1() {
//     LCD_Clear(0);
//     LCD_DrawRectangle(START_X + 10, START_Y, START_X + 20, START_Y + 50, 0xFFFF);
// }

// void draw2() {
//     LCD_Clear(0);
//     LCD_DrawRectangle(START_X, START_Y, START_X + 30, START_Y+10, 0xFFFF);
//     LCD_DrawRectangle(START_X, START_Y+30, START_X + 30, START_Y + 30+10, 0xFFFF);
//     LCD_DrawRectangle(START_X, START_Y+60, START_X + 30, START_Y + 60+10, 0xFFFF);
//     LCD_DrawRectangle(START_X+20, START_Y, START_X + 30, START_Y + 30+10, 0xFFFF);
//     LCD_DrawRectangle(START_X, START_Y+30, START_X + 10, START_Y + 60+10, 0xFFFF);
    
// }

// void draw3() {
//     LCD_Clear(0x0);
//     LCD_DrawRectangle(START_X, START_Y, START_X + 30, START_Y+10, 0xFFFF);
//     LCD_DrawRectangle(START_X, START_Y+30, START_X + 30, START_Y + 30+10, 0xFFFF);
//     LCD_DrawRectangle(START_X, START_Y+60, START_X + 30, START_Y + 60+10, 0xFFFF);
//     LCD_DrawRectangle(START_X, START_Y, START_X + 10, START_Y + 60+10, 0xFFFF);
// }

// void draw4() {
//     LCD_Clear(0x0);
//     LCD_DrawRectangle(START_X, START_Y, START_X + 10, START_Y + 60+10, 0xFFFF);
//     LCD_DrawRectangle(START_X+20, START_Y+30+10, START_X + 30, START_Y + 60+10, 0xFFFF);
//     LCD_DrawRectangle(START_X, START_Y+30, START_X + 30, START_Y + 30+10, 0xFFFF);
  
// }

// void draw5() {
//     LCD_Clear(0x0);
//     LCD_DrawRectangle(START_X, START_Y, START_X + 30, START_Y+10, 0xFFFF);
//     LCD_DrawRectangle(START_X, START_Y+30, START_X + 30, START_Y + 30+10, 0xFFFF);
//     LCD_DrawRectangle(START_X, START_Y+60, START_X + 30, START_Y + 60+10, 0xFFFF);
//     LCD_DrawRectangle(START_X, START_Y, START_X + 10, START_Y + 30+10, 0xFFFF);
//     LCD_DrawRectangle(START_X+20, START_Y+30, START_X + 30, START_Y + 60+10, 0xFFFF);
// }

// void draw6() {
//     LCD_Clear(0x0);
//     LCD_DrawRectangle(START_X, START_Y, START_X + 10, START_Y + 30+10, 0xFFFF); //short
//     LCD_DrawRectangle(START_X, START_Y, START_X+30, START_Y+10, 0xFFFF); //bot hori
//     LCD_DrawRectangle(START_X, START_Y+60, START_X + 30, START_Y + 60+10, 0xFFFF); //top hori
//     LCD_DrawRectangle(START_X, START_Y+30, START_X + 30, START_Y + 30+10, 0xFFFF); //hori mid
//     LCD_DrawRectangle(START_X+20, START_Y, START_X + 30, START_Y + 60+10, 0xFFFF); //tall left
// }