#include "stm32f0xx.h"
#include "buttons.h"
//PC0 rotate right
//PC1 hold
//PC2 rotate left
//PC3 move right
//PC4 move down
//PC5 move left


//for debouncing
uint16_t buttonBounce[6];


volatile int leftButton;
volatile int rightButton;
volatile int downButton;
volatile int rotLeftButton;
volatile int rotRightButton;
volatile int holdButton;

#define YELLOW    0xFFE0  // (255, 255, 0)
#define CYAN      0x07FF  // (0, 255, 255)
#define RED       0xF800  // (255, 0, 0)
#define GREEN     0x07E0  // (0, 255, 0)
#define ORANGE    0xFC00  // (255, 165, 0)
#define BLUE 0x0017  // (0, 0, 139)
#define PURPLE    0x8010  // (128, 0, 128)



void setup_gpio(){
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    
    //pc0-5
    GPIOC->MODER &= ~(0xFFF);
    GPIOC->PUPDR |= (0xAAA);
    GPIOC->PUPDR &= ~(0x555);
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

#define LCD_WIDTH  240
#define LCD_HEIGHT 320
#define NUM_WIDTH  30
#define NUM_HEIGHT 50
#define START_X   ((LCD_WIDTH - NUM_WIDTH) / 2)
#define START_Y   ((LCD_HEIGHT - NUM_HEIGHT) / 2)

void draw1() {
    LCD_Clear(0);
    LCD_DrawRectangle(START_X + 10, START_Y, START_X + 20, START_Y + 50, 0xFFFF);
}

void draw2() {
    LCD_Clear(0);
    LCD_DrawRectangle(START_X, START_Y, START_X + 30, START_Y+10, 0xFFFF);
    LCD_DrawRectangle(START_X, START_Y+30, START_X + 30, START_Y + 30+10, 0xFFFF);
    LCD_DrawRectangle(START_X, START_Y+60, START_X + 30, START_Y + 60+10, 0xFFFF);
    LCD_DrawRectangle(START_X+20, START_Y, START_X + 30, START_Y + 30+10, 0xFFFF);
    LCD_DrawRectangle(START_X, START_Y+30, START_X + 10, START_Y + 60+10, 0xFFFF);
    
}

void draw3() {
    LCD_Clear(0x0);
    LCD_DrawRectangle(START_X, START_Y, START_X + 30, START_Y+10, 0xFFFF);
    LCD_DrawRectangle(START_X, START_Y+30, START_X + 30, START_Y + 30+10, 0xFFFF);
    LCD_DrawRectangle(START_X, START_Y+60, START_X + 30, START_Y + 60+10, 0xFFFF);
    LCD_DrawRectangle(START_X, START_Y, START_X + 10, START_Y + 60+10, 0xFFFF);
}

void draw4() {
    LCD_Clear(0x0);
    LCD_DrawRectangle(START_X, START_Y, START_X + 10, START_Y + 60+10, 0xFFFF);
    LCD_DrawRectangle(START_X+20, START_Y+30+10, START_X + 30, START_Y + 60+10, 0xFFFF);
    LCD_DrawRectangle(START_X, START_Y+30, START_X + 30, START_Y + 30+10, 0xFFFF);
  
}

void draw5() {
    LCD_Clear(0x0);
    LCD_DrawRectangle(START_X, START_Y, START_X + 30, START_Y+10, 0xFFFF);
    LCD_DrawRectangle(START_X, START_Y+30, START_X + 30, START_Y + 30+10, 0xFFFF);
    LCD_DrawRectangle(START_X, START_Y+60, START_X + 30, START_Y + 60+10, 0xFFFF);
    LCD_DrawRectangle(START_X, START_Y, START_X + 10, START_Y + 30+10, 0xFFFF);
    LCD_DrawRectangle(START_X+20, START_Y+30, START_X + 30, START_Y + 60+10, 0xFFFF);
}

void draw6() {
    LCD_Clear(0x0);
    LCD_DrawRectangle(START_X, START_Y, START_X + 10, START_Y + 30+10, 0xFFFF); //short
    LCD_DrawRectangle(START_X, START_Y, START_X+30, START_Y+10, 0xFFFF); //bot hori
    LCD_DrawRectangle(START_X, START_Y+60, START_X + 30, START_Y + 60+10, 0xFFFF); //top hori
    LCD_DrawRectangle(START_X, START_Y+30, START_X + 30, START_Y + 30+10, 0xFFFF); //hori mid
    LCD_DrawRectangle(START_X+20, START_Y, START_X + 30, START_Y + 60+10, 0xFFFF); //tall left
}

void TIM2_IRQHandler(){
    TIM2->SR &= ~TIM_SR_UIF;

    //pc0-5
    int inputs = GPIOC->IDR & 0x3F;

    for(int i = 0; i < 6; i++){
        buttonBounce[i] = (buttonBounce[i] << 1);
        buttonBounce[i] |= 1 & (inputs >> i);

        if(((buttonBounce[i] & 0xF) == 0xF) && (buttonBounce[i] & 0xF000) == 0){
            buttonBounce[i] = 0xFFFF;
            
            if(i == 0){
                //rotate right
                draw1();
            }
            else if(i==1){
                //hold
                draw2();
            }
            else if(i==2){
                //rotate left
                draw3();
            }
            else if(i==3){
                //move right
                draw4();
            }
            else if(i==4){
                //move down
                draw5();
            }
            else if(i==5){
                draw6();
            }
        }
    }
}