#include "stm32f0xx.h"
#include "dac.h"
#define M_PI 3.14159
#define RATE 8000
/*
#define SAMPLE_RATE   8000U
#define WAVE_SAMPLES  100U      // size of our wavetable
static uint8_t wave[WAVE_SAMPLES];

// generate a simple square wave (change to sine if you like)
void fill_wave(void) {
    for(uint32_t i = 0; i < WAVE_SAMPLES; i++) {
        wave[i] = (i < WAVE_SAMPLES/2) ? 0xFF : 0x00;
    }
}
//setting up DAC for sound output
const int freqs[] = {650, 480, 590, 780}; // 4 notes

void setup_dac(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER |= 0x0300; // PA4 as analog mode
    RCC->APB1ENR |= RCC_APB1ENR_DACEN;
    DAC->CR &= ~DAC_CR_TSEL1_Msk;
    DAC->CR |= 4 << DAC_CR_TSEL1_Pos;
    DAC->CR |= DAC_CR_TEN1 | DAC_CR_DMAEN1;
    DAC->CR |= DAC_CR_EN1;
}

void init_tim6(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    TIM6->PSC = 48 - 1;
    TIM6->ARR = 124 - 1; // approx 8kHz 
    //'TIM6->DIER |= TIM_DIER_UIE;
    //TIM6->CR2 |= TIM_CR2_MMS_1;
    //TIM6->CR1 |= TIM_CR1_CEN;
    TIM6->CR2 &= ~TIM_CR2_MMS_Msk; 
    TIM6->CR2 |=  (2 << TIM_CR2_MMS_Pos); 
    TIM6->EGR = TIM_EGR_UG;
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
    DMA1_Channel3->CCR |= ~DMA_CCR_EN;
}
*/
