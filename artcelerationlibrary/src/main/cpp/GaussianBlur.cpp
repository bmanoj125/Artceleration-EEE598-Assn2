//
// Created by Amit on 12/4/2016.
//

#include <jni.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>
#include <android/bitmap.h>
#include <math.h>
#include "FilterHeaders.h"


inline double Gaussian_factor(int k, float std_dev){
    return   (1/sqrt(2*3.14*std_dev*std_dev))*(exp(-((k*k)/(2*std_dev*std_dev))));
}


void Gaussian_Blur(AndroidBitmapInfo* bmp_info, void *pixels, int integer_array[], float float_array[]){


    int red, green, blue,x_iterator, y_iterator;
    uint32_t* input_line;
    int radius, std_dev;
    //Dereference here to avoid overhead in loops
    int total_width = bmp_info->width;
    int total_height = bmp_info->height;
    int stride = bmp_info->stride;
    //variable for storing sum for the row
    double sum_r,sum_g,sum_b;
    double *G_vector, *GVec_iterator;
    int* q_vector, *q_vec_iterator;

    //Get Standard Deviation and radius from input arrays
    std_dev = float_array[0];
    radius = integer_array[0];


    //Generate the Gaussian vector
    G_vector = (double*) malloc(sizeof(double)*(2*radius+1));

    GVec_iterator =G_vector;

    for(int i =0; i<=(2*radius); i++){
        GVec_iterator[i] = Gaussian_factor(i-radius, std_dev);
    }

    //Allocate memory for intermediate q vector
    q_vector = (int*) malloc(total_width*total_height* sizeof(int));
    q_vec_iterator= q_vector;


    //Get the pointer to the row of pixel values
    input_line = (uint32_t*)pixels;

    //Compute the q_vector first
    for(y_iterator=0;y_iterator < total_height; y_iterator++){

        for(x_iterator=0;x_iterator<total_width; x_iterator++) {
            sum_r=0;
            sum_g=0;
            sum_b=0;
            //Calculate some of the indices beforehand to get speed up

            int x_index= y_iterator*total_width+x_iterator;
            for (int i = -radius; i <= radius; ++i) {
                //Check whether index is out of range
                if (x_iterator+i <0 || x_iterator+i >=total_width) {
                    //Add zero to the result
                    continue;
                }

                //Extract each channel
                //Keep adding to the sum for each channel

                sum_r += ((int) ((input_line[x_index+i] & 0x00FF0000) >> 16))* GVec_iterator[radius +i];
                sum_g += ((int) ((input_line[x_index+i] & 0x0000FF00) >> 8))* GVec_iterator[radius +i];
                sum_b += ((int) ((input_line[x_index+i] & 0x000000FF)))* GVec_iterator[radius +i];

            }
            //Convert the double to ints by casting to a different variable.
            red = (int)sum_r;
            green = (int)sum_g;
            blue = (int)sum_b;

            // Fuse the three channel assign it to the pixel value in q_vector;
            q_vec_iterator[y_iterator*total_width+x_iterator] = ((red << 16) & 0x00FF0000) |
                                                     ((green << 8) & 0x0000FF00) |
                                                     (blue & 0x000000FF);

        }
    }



    //Compute the values for the Output by applying Gaussian vector on q_vector



    //Get the pointer to the row of pixel values
    input_line = (uint32_t*)pixels;

    for(y_iterator=0;y_iterator < total_height; y_iterator++){

        for(x_iterator=0;x_iterator<total_width; x_iterator++) {
            sum_r=0;
            sum_g=0;
            sum_b=0;
            for (int i = -radius; i <= radius; ++i) {
                //Check whether index is out of range
                if (y_iterator+i < 0 || y_iterator+i >=total_height){
                    //Add zero to the result
                    continue;
                }
                //Extract each channel
                //Keep adding to the sum for each channel

                sum_r += ((int) ((q_vec_iterator[(y_iterator+i)*total_width+x_iterator] & 0x00FF0000) >> 16))* GVec_iterator[radius +i];
                sum_g += ((int) ((q_vec_iterator[(y_iterator+i)*total_width+x_iterator] & 0x0000FF00) >> 8))* GVec_iterator[radius +i];
                sum_b += ((int) ((q_vec_iterator[(y_iterator+i)*total_width+x_iterator] & 0x000000FF)))* GVec_iterator[radius +i];

            }
            //Convert the double to ints by casting to a different variable.
            red = (int)sum_r;
            green = (int)sum_g;
            blue = (int)sum_b;

            // Fuse the three channel assign it to the pixel value in q_vector;
            input_line[y_iterator*total_width+x_iterator] = ((red << 16) & 0x00FF0000) |
                                                           ((green << 8) & 0x0000FF00) |
                                                           (blue & 0x000000FF);

        }
    }



    //Free the allocated memory
    free(q_vector);
    free(G_vector);

    return ;
}

