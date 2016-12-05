//
// Created by Amit on 12/4/2016.
//

#include <jni.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>
#include <android/bitmap.h>
#include "FilterHeaders.h"


//Motion blur takes an average of the nearby pixels in the horizontal direction or vertical direction
void Motion_Blur(AndroidBitmapInfo* bmp_info, void *pixels, int integer_array[]){

    int red, green, blue,x_iterator, y_iterator;
    uint32_t* input_line;
    int radius, blur_direction;
    //Dereference here to avoid overhead in loops
    int total_width = bmp_info->width;
    int total_height = bmp_info->height;
    int stride = bmp_info->stride;

    //variable for storing sum for the row
    int sum_r,sum_g,sum_b;




    blur_direction = integer_array[0];
    radius = integer_array[1];


    //Get the pointer to the row of pixel values
    input_line = (uint32_t*)pixels;

    //Check direction
    if(blur_direction == 0){

        //Horizontal Motion blur
        for(y_iterator=0;y_iterator < total_height; y_iterator++){

            for(x_iterator=0;x_iterator<total_width; x_iterator++) {
                sum_r=0;
                sum_g=0;
                sum_b=0;

                for (int i = -radius; i <= radius; ++i) {
                    //Check whether index is out of range
                    if (x_iterator+i <0 || x_iterator+i >=total_width) {
                        //Add zero to the result
                        continue;
                    }

                    //Extract each channel
                    //Keep adding to the sum for each channel

                    sum_r += (int) ((input_line[y_iterator*total_width+ x_iterator+i] & 0x00FF0000) >> 16);
                    sum_g += (int) ((input_line[y_iterator*total_width+ x_iterator+i] & 0x0000FF00) >> 8);
                    sum_b += (int) ((input_line[y_iterator*total_width+ x_iterator+i] & 0x000000FF));

                }
                //Take average for each channel
                red = sum_r/(2*radius +1);
                green = sum_g/(2*radius +1);
                blue = sum_b/(2*radius +1);

                // Fuse the three channel assign it to the pixel value;
                input_line[y_iterator*total_width+ x_iterator] =
                        ((red << 16) & 0x00FF0000) |
                        ((green << 8) & 0x0000FF00) |
                        (blue & 0x000000FF);
            }

        }
    }else {
        //Blur Direction is Vertical
        //Get the pointer to the row of start of the matrix
        input_line = (uint32_t*)pixels;
        for(y_iterator=0;y_iterator < total_height; y_iterator++) {

            for (x_iterator = 0; x_iterator < total_width; x_iterator++) {
                //Initialize sums to 0
                sum_r=0;sum_g=0; sum_b=0;

                for (int i = -radius; i <= radius; ++i) {
                    //Check whether index is out of range
                    if (y_iterator+i < 0 || y_iterator+i >= total_height){
                        //Add zero to the result
                        continue;
                    }
                    //Otherwise Extract each channel keep adding to the sum of each channel

                    sum_r +=  (int) ((input_line[(y_iterator+i)*total_width+ x_iterator] & 0x00FF0000) >> 16);
                    sum_g +=  (int) ((input_line[(y_iterator+i)*total_width+ x_iterator] & 0x0000FF00) >> 8);
                    sum_b +=  (int) (input_line[(y_iterator+i)*total_width+ x_iterator] & 0x000000FF);
                }

                //Take average for each channel
                red = sum_r/(2*radius +1);
                green = sum_g/(2*radius +1);
                blue = sum_b/(2*radius +1);

                // Fuse the three channel assign it to the pixel value;
                input_line[y_iterator*total_width+x_iterator] =
                        ((red << 16) & 0x00FF0000) |
                        ((green << 8) & 0x0000FF00) |
                        (blue & 0x000000FF);
            }
        }
    }

    return ;
}
