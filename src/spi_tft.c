#include "stm32f0xx.h"
#include <stdint.h>
#include "spi_tft.h"

typedef uint16_t u16;

void internal_clock();

// Uncomment only one of the following to test each step
// #define STEP1
// #define STEP2
// #define STEP3
// #define STEP4
#define SHELL

void init_usart5() {
    // TODO
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIODEN;

    GPIOC->MODER &= 0xFCFFFFFF;
    GPIOC->MODER |= 0x02000000;
    GPIOC->AFR[1] |= (0x2 << GPIO_AFRH_AFSEL12_Pos);

    GPIOD->MODER &= 0xFFFFFFCF;
    GPIOD->MODER |= 0x00000020;
    GPIOD->AFR[0] |= (0x2 << GPIO_AFRL_AFSEL2_Pos);

    RCC->APB1ENR |= RCC_APB1ENR_USART5EN;
    USART5->CR1 &= ~USART_CR1_UE;
    USART5->CR1 &= ~(USART_CR1_M1 | USART_CR1_M0);
    USART5->CR2 &= ~(USART_CR2_STOP);
    USART5->CR1 &= ~(USART_CR1_PCE);
    USART5->CR1 &= ~(USART_CR1_OVER8);
    USART5->BRR = 0x1A1;
    USART5->CR1 |= USART_CR1_TE | USART_CR1_RE;
    USART5->CR1 |= USART_CR1_UE;

    while (((USART5->ISR & USART_ISR_TEACK) == 0) || ((USART5->ISR & USART_ISR_REACK) == 0)){
    }
}

#ifdef STEP1
int main(void){
    internal_clock();
    init_usart5();
    for(;;) {
        while (!(USART5->ISR & USART_ISR_RXNE)) { }
        char c = USART5->RDR;
        while(!(USART5->ISR & USART_ISR_TXE)) { }
        USART5->TDR = c;
    }
}
#endif

#ifdef STEP2
#include <stdio.h>

// TODO Resolve the echo and carriage-return problem

int __io_putchar(int c) {
    // TODO
    if (c == '\n') {
        while (!(USART5->ISR & USART_ISR_TXE));
        USART5->TDR = '\r';
    }
    while(!(USART5->ISR & USART_ISR_TXE));
    USART5->TDR = c;
    return c;
}

int __io_getchar(void) {
    while (!(USART5->ISR & USART_ISR_RXNE));
    char c = USART5->RDR;
    // TODO
    if(c == '\r'){
        c = '\n';
    }
    __io_putchar(c);
    return c;
}

int main() {
    internal_clock();
    init_usart5();
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    printf("Enter your name: ");
    char name[80];
    fgets(name, 80, stdin);
    printf("Your name is %s", name);
    printf("Type any characters.\n");
    for(;;) {
        char c = getchar();
        putchar(c);
    }
}
#endif

#ifdef STEP3
#include <stdio.h>
#include "fifo.h"
#include "tty.h"
int __io_putchar(int c) {
    // TODO Copy from your STEP2
    if (c == '\n') {
        while (!(USART5->ISR & USART_ISR_TXE));
        USART5->TDR = '\r';
    }
    while(!(USART5->ISR & USART_ISR_TXE));
    USART5->TDR = c;
    return c;
}

int __io_getchar(void) {
    // TODO
    return line_buffer_getchar();
}

int main() {
    internal_clock();
    init_usart5();
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    printf("Enter your name: ");
    char name[80];
    fgets(name, 80, stdin);
    printf("Your name is %s", name);
    printf("Type any characters.\n");
    for(;;) {
        char c = getchar();
        putchar(c);
    }
}
#endif

#ifdef STEP4

#include <stdio.h>
#include "fifo.h"
#include "tty.h"

// TODO DMA data structures
#define FIFOSIZE 16
char serfifo[FIFOSIZE];
int seroffset = 0;

void enable_tty_interrupt(void) {
    // TODO
    USART5->CR1 |= USART_CR1_RXNEIE;
    USART5->CR3 |= USART_CR3_DMAR;
    NVIC_EnableIRQ(USART3_8_IRQn);
    RCC->AHBENR |= RCC_AHBENR_DMA2EN;
    DMA2->CSELR |= DMA2_CSELR_CH2_USART5_RX;
    DMA2_Channel2->CCR &= ~DMA_CCR_EN;
    DMA2_Channel2->CMAR = (uint32_t) serfifo;
    DMA2_Channel2->CPAR = (uint32_t) &(USART5->RDR);
    DMA2_Channel2->CNDTR = FIFOSIZE;
    DMA2_Channel2->CCR &= ~DMA_CCR_DIR;
    DMA2_Channel2->CCR &= ~(DMA_CCR_TCIE | DMA_CCR_HTIE);
    DMA2_Channel2->CCR &= ~(DMA_CCR_MSIZE_Msk); //clear MSIZE for 8bit memory size
    DMA2_Channel2->CCR &= ~(DMA_CCR_PSIZE_Msk); //clear PSIZE for 8bit peripheral size
    DMA2_Channel2->CCR |= DMA_CCR_MINC;
    DMA2_Channel2->CCR &= ~DMA_CCR_PINC;
    DMA2_Channel2->CCR |= DMA_CCR_CIRC;
    DMA2_Channel2->CCR &= ~DMA_CCR_MEM2MEM;
    DMA2_Channel2->CCR &= ~DMA_CCR_PL;  //clear the priority
    DMA2_Channel2->CCR |= (0x3 << DMA_CCR_PL_Pos);   //set priority to 3 (11) for highest priority
    DMA2_Channel2->CCR |= DMA_CCR_EN;
    
}

// Works like line_buffer_getchar(), but does not check or clear ORE nor wait on new characters in USART
char interrupt_getchar() {
    // TODO
    // Wait for a newline to complete the buffer.
    while(fifo_newline(&input_fifo) == 0) {
        asm volatile ("wfi"); // wait for an interrupt
    }
    // Return a character from the line buffer.
    char ch = fifo_remove(&input_fifo);
    return ch;
}

int __io_putchar(int c) {
    // TODO copy from STEP2
    if (c == '\n') {
        while (!(USART5->ISR & USART_ISR_TXE));
        USART5->TDR = '\r';
    }
    while(!(USART5->ISR & USART_ISR_TXE)){
    }
    USART5->TDR = c;
    return c;
}

int __io_getchar(void) {
    // TODO Use interrupt_getchar() instead of line_buffer_getchar()
    return interrupt_getchar();
}

// TODO Copy the content for the USART5 ISR here
void USART3_8_IRQHandler(void) {
    while(DMA2_Channel2->CNDTR != sizeof serfifo - seroffset) {
        if (!fifo_full(&input_fifo))
            insert_echo_char(serfifo[seroffset]);
        seroffset = (seroffset + 1) % sizeof serfifo;
    }
}
// TODO Remember to look up for the proper name of the ISR function

int main() {
    internal_clock();
    init_usart5();
    enable_tty_interrupt();

    setbuf(stdin,0); // These turn off buffering; more efficient, but makes it hard to explain why first 1023 characters not dispalyed
    setbuf(stdout,0);
    setbuf(stderr,0);
    printf("Enter your name: "); // Types name but shouldn't echo the characters; USE CTRL-J to finish
    char name[80];
    fgets(name, 80, stdin);
    printf("Your name is %s", name);
    printf("Type any characters.\n"); // After, will type TWO instead of ONE
    for(;;) {
        char c = getchar();
        putchar(c);
    }
}
#endif

#include "fifo.h"
#include "tty.h"
#include <stdio.h>
#include "lcd.h"
#include "commands.h"
#include "buttons.h"
#include <time.h>


#define FIFOSIZE 16
#define CELL_SIZE 12
#define LEFT_BOUNDARY   60
#define RIGHT_BOUNDARY  180
#define TOP_BOUNDARY    80
#define BOTTOM_BOUNDARY 320

char serfifo[FIFOSIZE];
int seroffset = 0;

void enable_tty_interrupt(void) {
    // TODO
    USART5->CR1 |= USART_CR1_RXNEIE;
    USART5->CR3 |= USART_CR3_DMAR;
    NVIC_EnableIRQ(USART3_8_IRQn);
    RCC->AHBENR |= RCC_AHBENR_DMA2EN;
    DMA2->CSELR |= DMA2_CSELR_CH2_USART5_RX;
    DMA2_Channel2->CCR &= ~DMA_CCR_EN;
    DMA2_Channel2->CMAR = (uint32_t) serfifo;
    DMA2_Channel2->CPAR = (uint32_t) &(USART5->RDR);
    DMA2_Channel2->CNDTR = FIFOSIZE;
    DMA2_Channel2->CCR &= ~DMA_CCR_DIR;
    DMA2_Channel2->CCR &= ~(DMA_CCR_TCIE | DMA_CCR_HTIE);
    DMA2_Channel2->CCR &= ~(DMA_CCR_MSIZE_Msk); //clear MSIZE for 8bit memory size
    DMA2_Channel2->CCR &= ~(DMA_CCR_PSIZE_Msk); //clear PSIZE for 8bit peripheral size
    DMA2_Channel2->CCR |= DMA_CCR_MINC;
    DMA2_Channel2->CCR &= ~DMA_CCR_PINC;
    DMA2_Channel2->CCR |= DMA_CCR_CIRC;
    DMA2_Channel2->CCR &= ~DMA_CCR_MEM2MEM;
    DMA2_Channel2->CCR &= ~DMA_CCR_PL;  //clear the priority
    DMA2_Channel2->CCR |= (0x3 << DMA_CCR_PL_Pos);   //set priority to 3 (11) for highest priority
    DMA2_Channel2->CCR |= DMA_CCR_EN;
    
}

char interrupt_getchar() {
    // TODO
    // Wait for a newline to complete the buffer.
    while(fifo_newline(&input_fifo) == 0) {
        asm volatile ("wfi"); // wait for an interrupt
    }
    // Return a character from the line buffer.
    char ch = fifo_remove(&input_fifo);
    return ch;
}

int __io_putchar(int c) {
    // TODO copy from STEP2
    if (c == '\n') {
        while (!(USART5->ISR & USART_ISR_TXE));
        USART5->TDR = '\r';
    }
    while(!(USART5->ISR & USART_ISR_TXE)){
    }
    USART5->TDR = c;
    return c;
}

int __io_getchar(void) {
    // TODO Use interrupt_getchar() instead of line_buffer_getchar()
    return interrupt_getchar();
}

void USART3_8_IRQHandler(void) {
    while(DMA2_Channel2->CNDTR != sizeof serfifo - seroffset) {
        if (!fifo_full(&input_fifo))
            insert_echo_char(serfifo[seroffset]);
        seroffset = (seroffset + 1) % sizeof serfifo;
    }
}


void init_spi1_slow(){
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    
    GPIOB->MODER &= 0xfffff03f;
    GPIOB->MODER |= 0x00000a80;
    GPIOB->AFR[0] |= (0x0 << GPIO_AFRL_AFSEL3_Pos) | (0x0 << GPIO_AFRL_AFSEL4_Pos) | (0x0 << GPIO_AFRL_AFSEL5_Pos);

    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    // Configure SPI1
    SPI1->CR1 &= ~SPI_CR1_SPE;
    SPI1->CR1 |= (0x7 << SPI_CR1_BR_Pos);
    SPI1->CR1 |= SPI_CR1_MSTR;           // Master mode
    SPI1->CR2 |= SPI_CR2_FRXTH;
    SPI1->CR2 |= (0x7 << SPI_CR2_DS_Pos); // 8-bit data size
    SPI1->CR1 |= SPI_CR1_SSI | SPI_CR1_SSM;

    SPI1->CR1 |= SPI_CR1_SPE;  
}

void enable_sdcard(void) {
    // Use the BSRR register: writing to bit (pin number + 16) resets (sets low) the pin.
    GPIOB->BSRR = (1U << (2 + 16)); // Reset PB2
}

// Function to disable the SD card by setting PB2 high.
void disable_sdcard(void) {
    // Use the BSRR register: writing to the pin number sets (sets high) the pin.
    GPIOB->BSRR = (1U << 2); // Set PB2
}

void init_sdcard_io(){
    init_spi1_slow();
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->MODER &= 0xffffffcf;
    GPIOB->MODER |= 0x00000010;
    disable_sdcard();
}

void sdcard_io_high_speed(){
    SPI1->CR1 &= ~SPI_CR1_SPE;
    SPI1->CR1 &= ~SPI_CR1_BR;
    SPI1->CR1 |= SPI_CR1_BR_0;
    SPI1->CR1 |= SPI_CR1_SPE;
}

void init_lcd_spi(){
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->MODER &= 0xcf3cffff;
    GPIOB->MODER |= 0x10410000;
    init_spi1_slow();
    sdcard_io_high_speed();
}


void draw_cell(uint16_t base_x, uint16_t base_y, uint16_t color) {
    LCD_DrawFillRectangle(base_x, base_y, base_x + 12, base_y + 12, color);
}

void draw_piece(const Point* cells, u16 start_x, u16 start_y, u16 color) {
    for (int i = 0; i < 4; i++) {
        int draw_x = start_x + (cells[i].x * 12);
        int draw_y = start_y + (cells[i].y * 12);
        draw_cell(draw_x, draw_y, color);
    }
}

void create_first_piece(Piece* p) {
    for (int i = 0; i < 4; i++) {
        int draw_x = p->start_x + (p->blocks[i].x * 12);
        int draw_y = p->start_y + (p->blocks[i].y * 12);
        draw_cell(draw_x, draw_y, p->color);
    }
}

int is_piece_in_bounds(const Piece* p) {
    for (int i = 0; i < NUM_BLOCKS; i++) {
        int x = p->start_x + (p->blocks[i].x * CELL_SIZE);
        int y = p->start_y + (p->blocks[i].y * CELL_SIZE);
        if (x < LEFT_BOUNDARY || (x + CELL_SIZE) > RIGHT_BOUNDARY)
            return 0;
        if (y < TOP_BOUNDARY || (y + CELL_SIZE) > BOTTOM_BOUNDARY)
            return 0;
    }
    return 1;
}

void rotate_piece_clockwise(Piece* p) {
    if(p->color == YELLOW){
        return;
    }
    Point old_blocks[NUM_BLOCKS];
    for (int i = 0; i < NUM_BLOCKS; i++) {
        old_blocks[i] = p->blocks[i];
    }

    draw_piece(p->blocks, p->start_x, p->start_y, 0x0000);

    for(int i=0; i<4; i++){
        int temp = (p->blocks)[i].x;
        (p->blocks)[i].x = -((p->blocks)[i].y);
        (p->blocks)[i].y = temp;
    }
    if (!is_piece_in_bounds(p)) {
        for (int i = 0; i < NUM_BLOCKS; i++) {
            p->blocks[i] = old_blocks[i];
        }
    }
    draw_piece(p->blocks, p->start_x, p->start_y, p->color);
}

void rotate_piece_counterclockwise(Piece* p) {
    if(p->color == YELLOW){
        return;
    }
    Point old_blocks[NUM_BLOCKS];
    for (int i = 0; i < NUM_BLOCKS; i++) {
        old_blocks[i] = p->blocks[i];
    }

    draw_piece(p->blocks, p->start_x, p->start_y, 0x0000);
    for(int i=0; i<4; i++){
        int temp = (p->blocks)[i].x;
        (p->blocks)[i].x = (p->blocks)[i].y;
        (p->blocks)[i].y = -temp;
    }
    if (!is_piece_in_bounds(p)) {
        for (int i = 0; i < NUM_BLOCKS; i++) {
            p->blocks[i] = old_blocks[i];
        }
    }
    draw_piece(p->blocks, p->start_x, p->start_y, p->color);
}

void shift_piece_right(Piece *p){
    draw_piece(p->blocks, p->start_x, p->start_y, 0x0000);
    int old_start_x = p->start_x;
    p->start_x += CELL_SIZE;
    if(!is_piece_in_bounds(p)){
        p->start_x = old_start_x;
    }
    draw_piece(p->blocks, p->start_x, p->start_y, p->color);
}

void shift_piece_left(Piece *p){
    draw_piece(p->blocks, p->start_x, p->start_y, 0x0000);
    int old_start_x = p->start_x;
    p->start_x -= CELL_SIZE;
    if(!is_piece_in_bounds(p)){
        p->start_x = old_start_x;
    }
    draw_piece(p->blocks, p->start_x, p->start_y, p->color);
}

void shift_piece_down(Piece *p){
    draw_piece(p->blocks, p->start_x, p->start_y, 0x0000);
    int old_start_x = p->start_x;
    p->start_y += CELL_SIZE;
    if(!is_piece_in_bounds(p)){
        p->start_x = old_start_x;
    }
    draw_piece(p->blocks, p->start_x, p->start_y, p->color);
}

Piece* copyPiece(const Piece* src) {
    if (src == NULL) {
        return NULL;
    }
    
    Piece* newPiece = malloc(sizeof(Piece));
    if (newPiece == NULL) {
        return NULL;
    }
    
    newPiece->start_x = src->start_x;
    newPiece->start_y = src->start_y;
    newPiece->color   = src->color;
    
    newPiece->blocks = malloc(NUM_BLOCKS * sizeof(Point));
    if (newPiece->blocks == NULL) {
        free(newPiece);
        return NULL;
    }
    
    memcpy(newPiece->blocks, src->blocks, NUM_BLOCKS * sizeof(Point));
    
    return newPiece;
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

Piece piece_J;
Piece piece_L;
Piece piece_I;
Piece piece_O;
Piece piece_S;
Piece piece_Z;
Piece piece_T;

Piece* generate_piece(void) {
    piece_J.blocks = blocks_J;
    piece_J.start_x = 108;
    piece_J.start_y = 92;
    piece_J.color = BLUE;

    piece_L.blocks = blocks_L;
    piece_L.start_x = 108;
    piece_L.start_y = 92;
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
    piece_S.start_y = 92;
    piece_S.color = GREEN;

    piece_Z.blocks = blocks_Z;
    piece_Z.start_x = 108;
    piece_Z.start_y = 92;
    piece_Z.color = RED;

    piece_T.blocks = blocks_T;
    piece_T.start_x = 108;
    piece_T.start_y = 92;
    piece_T.color = PURPLE;
    // Generate a random integer from 1 to 7.
    int r = rand() % 7 + 1;
    
    Piece* newPiece = NULL;
    
    switch (r) {
        case 1:
            newPiece = copyPiece(&piece_I);
            break;
        case 2:
            newPiece = copyPiece(&piece_J);
            break;
        case 3:
            newPiece = copyPiece(&piece_L);
            break;
        case 4:
            newPiece = copyPiece(&piece_O);
            break;
        case 5:
            newPiece = copyPiece(&piece_S);
            break;
        case 6:
            newPiece = copyPiece(&piece_T);
            break;
        case 7:
            newPiece = copyPiece(&piece_Z);
            break;
        default:
            // Should never happen, but just in case...
            newPiece = NULL;
            break;
    }
    
    return newPiece;
}
