#include "stm32f0xx.h"
#define RATE 8000

//setting up DAC for sound output
const int freqs[] = {650, 480, 590, 780}; // 4 notes

void setup_dac(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER |= 0x0300; // PA4 as analog mode
    RCC->APB1ENR |= RCC_APB1ENR_DACEN;
    DAC->CR |= 4 << DAC_CR_TSEL1_Pos;
    DAC->CR |= DAC_CR_TEN1 | DAC_CR_DMAEN1;
    DAC->CR |= DAC_CR_EN1;
}

void init_tim6(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    TIM6->PSC = 48 - 1;
    TIM6->ARR = 124 - 1; // approx 8kHz 
    //'TIM6->DIER |= TIM_DIER_UIE;
    TIM6->CR2 |= TIM_CR2_MMS_1;
    TIM6->CR1 |= TIM_CR1_CEN;
}

uint16_t audio[RATE];

void setup_dma(void) {
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    DMA1_Channel3->CCR &= ~DMA_CCR_EN;
    DMA1_Channel3->CMAR = (uint32_t)audio;
    DMA1_Channel3->CPAR = (uint32_t)&(DAC->DHR12R1);
    DMA1_Channel3->CNDTR = RATE; // 1 second sound
    DMA1_Channel3->CCR |= DMA_CCR_MINC;
    DMA1_Channel3->CCR &= ~DMA_CCR_MSIZE;
    DMA1_Channel3->CCR &= ~DMA_CCR_PSIZE;
    DMA1_Channel3->CCR |= DMA_CCR_MSIZE_0;
    DMA1_Channel3->CCR |= DMA_CCR_PSIZE_0;
    DMA1_Channel3->CCR |= DMA_CCR_CIRC;
    DMA1_Channel3->CCR |= DMA_CCR_DIR;
    DMA1_Channel3->CCR |= DMA_CCR_EN;
}

void tetris_sound(void) {
    int duration = RATE / 4;
    int idx = 0;
    for (int i = 0; i <= 3; i++) {
        int freq = freqs[i];
        for (int j = 0; j < duration; j++) {
            if (idx >= RATE) return;
            audio[idx++] = (j % (RATE / freq) < (RATE / (2 * freq))) ? 3000 : 1000;
        }
    }
}
/*
int main(void) {
    setup_dac();
    setup_dma();
    init_tim6();
    tetris_sound();
    while (1) {
    }
    return 0;
}*/