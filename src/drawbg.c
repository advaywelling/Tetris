#define COLUMNS 10
#define ROWS 20
#define TOP_EDGE 80
#define LEFT_EDGE 60
#define BOT_EDGE 320
#define RIGHT_EDGE 180
void draw_borders(int color){
    LCD_DrawFillRectangle(0, TOP_EDGE-6, 240, TOP_EDGE-1, color);
    LCD_DrawFillRectangle(LEFT_EDGE-6, TOP_EDGE, LEFT_EDGE-2, 320, color);
    LCD_DrawFillRectangle(RIGHT_EDGE-1, TOP_EDGE, RIGHT_EDGE+4, BOT_EDGE, color);
}

void draw_title(int color){
    int ygap = 17;
    int xgap = 24;
    int len = 8;
    //T
    LCD_DrawFillRectangle(xgap, ygap, xgap+len*3, ygap+len, color);
    LCD_DrawFillRectangle(xgap+len, ygap+len,xgap+len*2, ygap+len*5, color);
    xgap = xgap + len*4;
    //E
    LCD_DrawFillRectangle(xgap, ygap, xgap+len*3, ygap+len, color);
    LCD_DrawFillRectangle(xgap, ygap+len, xgap+len, ygap+len*5, color);
    LCD_DrawFillRectangle(xgap+len, ygap+len*2, xgap+len*2, ygap+len*3, color);
    LCD_DrawFillRectangle(xgap+len, ygap+len*4, xgap+len*3, ygap+len*5, color);
    xgap = xgap + len*4;
    //T
    LCD_DrawFillRectangle(xgap, ygap, xgap+len*3, ygap+len, color);
    LCD_DrawFillRectangle(xgap+len, ygap+len,xgap+len*2, ygap+len*5, color);
    xgap = xgap + len*4;
    //R
    LCD_DrawFillRectangle(xgap, ygap, xgap+len*4, ygap+len, color);
    LCD_DrawFillRectangle(xgap, ygap+len, xgap+len, ygap+len*5, color);
    LCD_DrawFillRectangle(xgap, ygap+len*2, xgap+len*4, ygap+len*3, color);
    LCD_DrawFillRectangle(xgap+len*2, ygap+len*3, xgap+len*3, ygap+len*4, color);
    LCD_DrawFillRectangle(xgap+len*3, ygap+len*4, xgap+len*4, ygap+len*5, color);
    LCD_DrawFillRectangle(xgap+len*3, ygap, xgap+len*4, ygap+len*3, color);
    xgap = xgap + len*5;
    //I
    LCD_DrawFillRectangle(xgap, ygap, xgap+len*3, ygap+len, color);
    LCD_DrawFillRectangle(xgap+len, ygap+len, xgap+len*2, ygap+len*5, color);
    LCD_DrawFillRectangle(xgap, ygap+len*4, xgap+len*3, ygap+len*5, color);
    xgap = xgap + len*4;
    //S
    LCD_DrawFillRectangle(xgap, ygap, xgap+len*3, ygap+len, color);
    LCD_DrawFillRectangle(xgap, ygap, xgap+len, ygap+len*3, color);
    LCD_DrawFillRectangle(xgap+len, ygap+len*2, xgap+len*2, ygap+len*3, color);
    LCD_DrawFillRectangle(xgap+len*2, ygap+len*3, xgap+len*3, ygap+len*5, color);
    LCD_DrawFillRectangle(xgap, ygap+len*4, xgap+len*2, ygap+len*5, color);

    //border
    xgap = 24;
    LCD_DrawRectangle(xgap, ygap, xgap+len*3, ygap+len, ~color);
    LCD_DrawRectangle(xgap+len, ygap+len,xgap+len*2, ygap+len*5, ~color);
    xgap = xgap + len*4;
    LCD_DrawRectangle(xgap, ygap, xgap+len*3, ygap+len, ~color);
    LCD_DrawRectangle(xgap, ygap, xgap+len, ygap+len*5, ~color);
    LCD_DrawRectangle(xgap+len, ygap+len*2, xgap+len*2, ygap+len*3, ~color);
    LCD_DrawRectangle(xgap+len, ygap+len*4, xgap+len*3, ygap+len*5, ~color);
    xgap = xgap + len*4;
    LCD_DrawRectangle(xgap, ygap, xgap+len*3, ygap+len, ~color);
    LCD_DrawRectangle(xgap+len, ygap+len,xgap+len*2, ygap+len*5, ~color);
    xgap = xgap + len*4;
    LCD_DrawRectangle(xgap, ygap, xgap+len*4, ygap+len, ~color);
    LCD_DrawRectangle(xgap, ygap+len, xgap+len, ygap+len*5, ~color);
    LCD_DrawRectangle(xgap, ygap+len*2, xgap+len*4, ygap+len*3, ~color);
    LCD_DrawRectangle(xgap+len*2, ygap+len*3, xgap+len*3, ygap+len*4, ~color);
    LCD_DrawRectangle(xgap+len*3, ygap+len*4, xgap+len*4, ygap+len*5, ~color);
    LCD_DrawRectangle(xgap+len*3, ygap, xgap+len*4, ygap+len*3, ~color);
    xgap = xgap + len*5;
    LCD_DrawRectangle(xgap, ygap, xgap+len*3, ygap+len, ~color);
    LCD_DrawRectangle(xgap+len, ygap, xgap+len*2, ygap+len*5, ~color);
    LCD_DrawRectangle(xgap, ygap+len*4, xgap+len*3, ygap+len*5, ~color);
    xgap = xgap + len*4;
    LCD_DrawRectangle(xgap, ygap, xgap+len*3, ygap+len, ~color);
    LCD_DrawRectangle(xgap, ygap, xgap+len, ygap+len*3, ~color);
    LCD_DrawRectangle(xgap+len, ygap+len*2, xgap+len*2, ygap+len*3, ~color);
    LCD_DrawRectangle(xgap+len*2, ygap+len*3, xgap+len*3, ygap+len*5, ~color);
    LCD_DrawRectangle(xgap, ygap+len*4, xgap+len*3, ygap+len*5, ~color);
}

void draw_hold(int color){
    // right edge = 54;
    int xgap = 2;
    int len = 3;
    int ygap = 85;
    //H
    LCD_DrawFillRectangle(xgap, ygap, xgap+len, ygap+len*5, color);
    LCD_DrawFillRectangle(xgap, ygap+len*2, xgap+len*3, ygap+len*3, color);
    LCD_DrawFillRectangle(xgap+len*2, ygap, xgap+len*3, ygap+len*5, color);
    xgap = xgap + len*4;
    //O
    LCD_DrawFillRectangle(xgap+len, ygap, xgap+len*3, ygap+len, color);
    LCD_DrawFillRectangle(xgap, ygap+len, xgap+len, ygap+len*4, color);
    LCD_DrawFillRectangle(xgap+len*3, ygap+len, xgap+len*4, ygap+len*4, color);
    LCD_DrawFillRectangle(xgap+len, ygap+len*4, xgap+len*3, ygap+len*5, color);
    xgap = xgap + len*5;
    //L
    LCD_DrawFillRectangle(xgap, ygap, xgap+len, ygap+len*5, color);
    LCD_DrawFillRectangle(xgap, ygap+len*4, xgap+len*3, ygap+len*5, color);
    xgap = xgap + len*4;
    //D
    LCD_DrawFillRectangle(xgap, ygap, xgap+len*2, ygap+len, color);
    LCD_DrawFillRectangle(xgap, ygap, xgap+len, ygap+len*5, color);
    LCD_DrawFillRectangle(xgap, ygap+len*4, xgap+len*2, ygap+len*5, color);
    LCD_DrawFillRectangle(xgap+len*2, ygap+len, xgap+len*3, ygap+len*4, color);
    
}

void draw_score(int color){
    int ygap = 185;
    int len = 3;
    int xlen = 2;
    int xgap = 6;
    //S
    LCD_DrawFillRectangle(xgap, ygap, xgap+xlen*3, ygap+len, color);
    LCD_DrawFillRectangle(xgap, ygap, xgap+xlen, ygap+len*3, color);
    LCD_DrawFillRectangle(xgap+xlen, ygap+len*2, xgap+xlen*2, ygap+len*3, color);
    LCD_DrawFillRectangle(xgap+xlen*2, ygap+len*3, xgap+xlen*3, ygap+len*5, color);
    LCD_DrawFillRectangle(xgap, ygap+len*4, xgap+xlen*2, ygap+len*5, color);
    xgap = xgap + xlen*4;

    //C
    LCD_DrawFillRectangle(xgap, ygap, xgap+xlen*3, ygap+len, color);
    LCD_DrawFillRectangle(xgap, ygap, xgap+xlen, ygap+len*5, color);
    LCD_DrawFillRectangle(xgap, ygap+len*4, xgap+xlen*3, ygap+len*5, color);
    LCD_DrawFillRectangle(xgap+xlen*2, ygap, xgap+xlen*3, ygap+len*2, color);
    LCD_DrawFillRectangle(xgap+xlen*2, ygap+len*3, xgap+xlen*3, ygap+len*5, color);

    xgap = xgap + xlen*4;
    //O
    LCD_DrawFillRectangle(xgap+xlen, ygap, xgap+xlen*3, ygap+len, color);
    LCD_DrawFillRectangle(xgap, ygap+len, xgap+xlen, ygap+len*4, color);
    LCD_DrawFillRectangle(xgap+xlen*3, ygap+len, xgap+xlen*4, ygap+len*4, color);
    LCD_DrawFillRectangle(xgap+xlen, ygap+len*4, xgap+xlen*3, ygap+len*5, color);
    xgap = xgap + xlen*5;
    
    //R
    LCD_DrawFillRectangle(xgap, ygap, xgap+xlen*4, ygap+len, color);
    LCD_DrawFillRectangle(xgap, ygap+len, xgap+xlen, ygap+len*5, color);
    LCD_DrawFillRectangle(xgap, ygap+len*2, xgap+xlen*4, ygap+len*3, color);
    LCD_DrawFillRectangle(xgap+xlen*2, ygap+len*3, xgap+xlen*3, ygap+len*4, color);
    LCD_DrawFillRectangle(xgap+xlen*3, ygap+len*4, xgap+xlen*4, ygap+len*5, color);
    LCD_DrawFillRectangle(xgap+xlen*3, ygap, xgap+xlen*4, ygap+len*3, color);
    xgap = xgap + xlen*5;
    //E
    LCD_DrawFillRectangle(xgap, ygap, xgap+xlen*3, ygap+len, color);
    LCD_DrawFillRectangle(xgap, ygap+len, xgap+xlen, ygap+len*5, color);
    LCD_DrawFillRectangle(xgap+xlen, ygap+len*2, xgap+xlen*2, ygap+len*3, color);
    LCD_DrawFillRectangle(xgap+xlen, ygap+len*4, xgap+xlen*3, ygap+len*5, color);
}

void draw_next(int color){
    // left edge = 186, right = 240

    int xgap = 188;
    int len = 3;
    int ygap = 85;

    //N
    LCD_DrawFillRectangle(xgap, ygap, xgap+len, ygap+len*5, color);
    LCD_DrawFillRectangle(xgap+len, ygap+len*2, xgap+len*2, ygap+len*3, color);
    LCD_DrawFillRectangle(xgap+len*2, ygap+len*3, xgap+len*3, ygap+len*4, color);
    LCD_DrawFillRectangle(xgap+len*3, ygap, xgap+len*4, ygap+len*5, color);

    xgap = xgap + len*5;
    //E
    LCD_DrawFillRectangle(xgap, ygap, xgap+len*3, ygap+len, color);
    LCD_DrawFillRectangle(xgap, ygap+len, xgap+len, ygap+len*5, color);
    LCD_DrawFillRectangle(xgap+len, ygap+len*2, xgap+len*2, ygap+len*3, color);
    LCD_DrawFillRectangle(xgap+len, ygap+len*4, xgap+len*3, ygap+len*5, color);
    xgap = xgap + len*4;

    //X
    LCD_DrawFillRectangle(xgap, ygap, xgap+len, ygap+len*2, color);
    LCD_DrawFillRectangle(xgap+len*3, ygap, xgap+len*4, ygap+len*2, color);
    LCD_DrawFillRectangle(xgap+len, ygap+len*2, xgap+len*3, ygap+len*3, color);
    LCD_DrawFillRectangle(xgap, ygap+len*3, xgap+len, ygap+len*5, color);
    LCD_DrawFillRectangle(xgap+len*3, ygap+len*3, xgap+len*4, ygap+len*5, color);

    xgap = xgap + len*5;
    //T
    LCD_DrawFillRectangle(xgap, ygap, xgap+len*3, ygap+len, color);
    LCD_DrawFillRectangle(xgap+len, ygap+len,xgap+len*2, ygap+len*5, color);
    xgap = xgap + len*4;

}