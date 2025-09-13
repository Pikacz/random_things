#ifndef PAWEL_RENDERER_H
#define PAWEL_RENDERER_H

#include <stdint.h>


void renderToBitmap(uint32_t* bitmap, size_t width, size_t height);


#ifndef PAWEL_DO_NOT_IMPLEMENT

#include <math.h>

typedef union {
    uint32_t raw;
    struct {
        
        uint8_t blue;
        uint8_t green;
        uint8_t red;
        uint8_t alpha;
    };
} Color;

void renderToBitmap(uint32_t* bitmap, size_t width, size_t height)
{
    size_t row, column;
    
    Color* pixel = (Color*)bitmap;
    for (row = 0; row < height; row++)
    {    
        for (column = 0; column < width; column++)
        {
            pixel->raw = 0xFF000000;
            pixel->red = (uint8_t)round(((float)row / (float)height) * 255);
            pixel->green = 0;
            pixel->blue = (uint8_t)round(((float)column / (float)width) * 255);;            
            pixel++;
        }
    }

}

#endif
#endif