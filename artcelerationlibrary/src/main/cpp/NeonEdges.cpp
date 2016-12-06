//
// Created by Amit on 12/5/2016.
//

#include <android/bitmap.h>
#include <stdlib.h>

#include "FilterHeaders.h"

extern void Gaussian_Blur(AndroidBitmapInfo* bmp_info, void *pixels, int integer_array[], float float_array[]);
extern void Sobel_Filter(AndroidBitmapInfo* bmp_info, void *pixels, int integer_array[]);


void Neon_Edges(AndroidBitmapInfo* bmp_info, void *pixels, float float_array[]) {

    int red, green, blue,x_iterator, y_iterator;
    uint32_t* input_line;
    int std_dev;
    //Dereference here to avoid overhead in loops
    int total_width = bmp_info->width;
    int total_height = bmp_info->height;


    std_dev = float_array[0];
    float scale_1 = float_array[1];
    float scale_2 = float_array[2];

    int radius[1];
    //Set the radius as 6 timers the standard deviation
    radius[0] = (int) (6*std_dev);

    float sigma[1];
    sigma[0]=std_dev;

    int fitler[1]= {2};

    //Point input_ptr to the row of pixels
    input_line = (uint32_t*)pixels;

    //Generate a copies of the original image
    uint32_t* copy1 = (uint32_t*)malloc(sizeof(uint32_t)*total_height*total_width);
    memcpy(copy1, input_line,sizeof(uint32_t)*total_height*total_width);

    //Compute the Sobel Edge Detection
    Sobel_Filter(bmp_info,copy1,fitler);

    //Apply Gaussian Blur to the Sobel Edge output
    Gaussian_Blur(bmp_info,copy1,radius,sigma);

    for(y_iterator=0;y_iterator < total_height; y_iterator++){

        for(x_iterator=0;x_iterator<total_width; x_iterator++) {
            //Calculate some of the indices beforehand to get speed up
            int index =y_iterator*total_width+x_iterator;

            //Take the difference and Scale by the factor chosen. Add this value to the original image
            //We're using the intensity_limit function to limit it within 0 and 255
            red = (((copy1[index] & 0x00FF0000) >> 16)*scale_1)+((input_line[index] & 0x00FF0000) >> 16)*scale_2;
            green = ((copy1[index] & 0x0000FF00) >> 8)*scale_1+ ((input_line[index] & 0x0000FF00) >> 8)*scale_2;
            blue = (copy1[index] & 0x000000FF)*scale_1+ ((input_line[index] & 0x000000FF))*scale_2;

            //Limit the intensities to within 0 and 255
            red = intensity_limit(red);
            green = intensity_limit(green);
            blue = intensity_limit(blue);

            // Fuse the three channel assign it to the output bmp;
            input_line[y_iterator*total_width+x_iterator] = ((red << 16) & 0x00FF0000) |
                                                            ((green << 8) & 0x0000FF00) |
                                                            (blue & 0x000000FF);

        }
    }

    //Free Allocated Memory
    free(copy1);


}