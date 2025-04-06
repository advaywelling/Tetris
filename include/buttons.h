#ifndef __BUTTONS_H__
#define __BUTTONS_H__

void setup_gpio();
void setup_TIM2();
void draw1();
void draw2();
void draw3();
void draw4();
void draw5();
void draw6();
void TIM2_IRQHandler();

#define YELLOW    0xFFE0  // (255, 255, 0)
#define CYAN      0x07FF  // (0, 255, 255)
#define RED       0xF800  // (255, 0, 0)
#define GREEN     0x07E0  // (0, 255, 0)
#define ORANGE    0xFC00  // (255, 165, 0)
#define BLUE 0x0017  // (0, 0, 139)
#define PURPLE    0x8010  // (128, 0, 128)

volatile int leftButton;
volatile int rightButton;
volatile int downButton;
volatile int rotLeftButton;
volatile int rotRightButton;
volatile int holdButton;

#endif