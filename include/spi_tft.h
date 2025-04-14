#ifndef __SPI_TFT_H__
#define __SPI_TFT_H__

#define NUM_BLOCKS 4
typedef struct {
    int x;
    int y;
} Point;

extern const Point blocks_T[4];
extern const Point blocks_L[4];
extern const Point blocks_J[4];
extern const Point blocks_O[4];
extern const Point blocks_S[4];
extern const Point blocks_Z[4];
extern const Point blocks_I[4];

typedef struct {
    Point* blocks;
    int start_x;
    int start_y;
    uint16_t color;
} Piece;

extern Piece piece_J;
extern Piece piece_L;
extern Piece piece_I;
extern Piece piece_O;
extern Piece piece_S;
extern Piece piece_Z;
extern Piece piece_T;


void init_usart5();
void enable_tty_interrupt(void);
char interrupt_getchar();
int __io_putchar(int c);
int __io_getchar(void);
void USART3_8_IRQHandler(void);
void init_spi1_slow();
void enable_sdcard(void);
void disable_sdcard(void);
void init_sdcard_io();
void sdcard_io_high_speed();
void init_lcd_spi();
void draw_I(uint16_t x, uint16_t y, uint16_t c);
void draw_O(uint16_t x, uint16_t y, uint16_t c);
void draw_J(uint16_t x, uint16_t y, uint16_t c);
void draw_L(uint16_t x, uint16_t y, uint16_t c);
void draw_T(uint16_t x, uint16_t y, uint16_t c);
void draw_S(uint16_t x, uint16_t y, uint16_t c);
void draw_Z(uint16_t x, uint16_t y, uint16_t c);
void draw_cell(uint16_t base_x, uint16_t base_y, uint16_t color);
int is_piece_in_bounds(const Piece* p);
void rotate_piece_clockwise(Piece* p);
void rotate_piece_counterclockwise(Piece* p);
void shift_piece_right(Piece *p);
void shift_piece_left(Piece *p);
void draw_piece(const Point* cells, uint16_t start_x, uint16_t start_y, uint16_t color);
void create_first_piece(Piece* p);
Piece* copyPiece(const Piece* src);
Piece* generate_piece(void);



#endif /* __SPI_TFT_H__ */