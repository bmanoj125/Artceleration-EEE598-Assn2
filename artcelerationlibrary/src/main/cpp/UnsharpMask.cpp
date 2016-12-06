//
// Created by Amit on 12/5/2016.
//

#include <android/bitmap.h>
#include <stdlib.h>
#include "FilterHeaders.h"

extern void void Gaussian_Blur(AndroidBitmapInfo* bmp_info, void *pixels, int integer_array[], float float_array[]);

void Unsharp_Mask(AndroidBitmapInfo* bmp_info, void* pixels, float floatArgs[]){

    uint32_t *input_line = (uint32_t*) pixels;
    float std_dev, scaling_factor;
    //Dereference here to avoid overhead in loops
    int total_width = bmp_info->width;
    int total_height = bmp_info->height;

    //Get the two float arguments
    std_dev = floatArgs[0];
    scaling_factor = floatArgs[1];


    //Generate a copy of the original image

    uint32_t* original_img = (uint32_t*)malloc(sizeof(uint32_t)*total_height*total_width);
    memcpy(original_img, input_line,sizeof(uint32_t)*total_height*total_width );

}