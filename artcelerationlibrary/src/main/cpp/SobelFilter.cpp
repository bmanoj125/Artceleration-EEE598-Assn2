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
    double result = (1/sqrt(2*3.14*std_dev*std_dev))*(exp(-((k*k)/(2*std_dev*std_dev))));
    return  result;
}

int Sx_mask[3][3] = {
            {-1,0,1},
            {-2,0,2},
            {-1,0,1}
};


int Sy_mask[3][3] = {
        {-1,-2,-1},
        {0,0,0},
        {+1,+2,+1}
};

void Sobel_Filter(AndroidBitmapInfo* bmp_info, void *pixels, int integer_array[]) {


    int red, green, blue, x_iterator, y_iterator;
    uint32_t *input_line;
    int a0;
    //Dereference here to avoid overhead in loops
    int total_width = bmp_info->width;
    int total_height = bmp_info->height;
    int stride = bmp_info->stride;
    //variable for storing sum for the row
    double red_q, green_q, blue_q;
    double *G_vector, *GVec_iterator;
    uint32_t *q_vector, *q_vec_iterator;
    double gray_value;
    int sum_x, sum_y;



    a0 = integer_array[0];
    //Get the pointer to the row of pixel values
    input_line = (uint32_t *) pixels;

    //Create a grayscale brightness image of the input

    for (y_iterator = 0; y_iterator < total_height; y_iterator++) {


        int x_index =y_iterator*total_width;
        for (x_iterator = 0; x_iterator < total_width; x_iterator++) {


            //Extract each channel

            red_q = (((input_line[x_index+x_iterator] & 0x00FF0000) >> 16));
            green_q =(((input_line[x_index+x_iterator] & 0x0000FF00) >> 8));
            blue_q = ((input_line[x_index+x_iterator] & 0x000000FF));




            //Scale each channel and compute the gray scale value
            gray_value= red_q*0.2989+green_q*0.5870+blue_q*0.1140;

            //Assign the gray scale value to the input
            // Fuse the three channel assign it to the pixel value;
            input_line[y_iterator*total_width+ x_iterator] =
                    (((int)gray_value << 16) & 0x00FF0000) |
                    (((int)gray_value << 8) & 0x0000FF00) |
                    ((int)gray_value & 0x000000FF);


        }
    }

    //Using the switch statements here instead of inside the loop to avoid the overhead of branches inside the loop
    // This would make the execution relatively faster
    // Also instead of modularizing the updating in functions, we wrote inline to improve speed of execution
    // Modularizing it would reduce the space, but using inline makes it faster, and this was the aim of the project for us.
    switch(a0){
        //Get the pointer to the row of pixel values
        input_line = (uint32_t *) pixels;

        case 0:
            for (y_iterator = 0; y_iterator < total_height; y_iterator++) {
                for (x_iterator = 0; x_iterator < total_width; x_iterator++){
                    //Iterate through the input_line
                   sum_x =0;
                   for(int i= -1; i<2;i++){
                       for(int j=-1; j<2;j++){

                           //Skip the pixels that go out of range
                           if((y_iterator+j) <0 || (y_iterator+j>=total_height)|| (x_iterator+i)<0 || (x_iterator+i)>=total_width){
                               continue;
                           }
                           //Compute the sum of the pixels with the Y mask
                           sum_x +=input_line[((y_iterator+j)*total_width)+x_iterator+i]*Sx_mask[i][j];
                       }
                   }

                    //Insert the updated value into the pixel
                    input_line[y_iterator*total_width+ x_iterator] = sum_x;
                           /* (((int)sum_x << 16) & 0x00FF0000) |
                            (((int)sum_x << 8) & 0x0000FF00) |
                            ((int)sum_x & 0x000000FF);*/

                }
            }
            break;
        /*
        case 1:
            for (y_iterator = 0; y_iterator < total_height; y_iterator++) {

                for (x_iterator = 0; x_iterator < total_width; x_iterator++){
                    //Iterate through the input_line
                    sum_y =0;
                    for(int i= -1; i<2;i++){
                        for(int j=-1; j<2;j++){
                            //Skip the pixels that go out of range
                            if((y_iterator+j) <0 || (y_iterator+j>total_height)|| (x_iterator+i)<0 || (x_iterator+i)>total_width){
                                continue;
                            }
                            //Compute the sum of the pixels with the Y mask
                            sum_y +=input_line[((y_iterator+j)*total_width)+x_iterator+i]*Sy_mask[i][j];
                        }
                    }
                    //Insert the updated value into the pixel
                    input_line[y_iterator*total_width+x_iterator] = sum_y;
                }
            }

            break;

        case 2:

            for (y_iterator = 0; y_iterator < total_height; y_iterator++) {

                for (x_iterator = 0; x_iterator < total_width; x_iterator++){
                    //Iterate through the input_line
                    sum_x=0;
                    sum_y =0;
                    for(int i= -1; i<2;i++){
                        for(int j=-1; j<2;j++){
                            //Skip the pixels that go out of range
                            if((y_iterator+i) <0 || (y_iterator+i>total_height)|| (x_iterator+j)<0 || (x_iterator+j)>total_width){
                                continue;
                            }
                            //Compute the sum of the pixels with the X mask
                            sum_x +=input_line[((y_iterator+i)*total_width)+x_iterator+j]*Sx_mask[i][j];

                            //Compute the sum of the pixels with the Y mask
                            sum_y +=input_line[((y_iterator+i)*total_width)+x_iterator+j]*Sy_mask[i][j];
                        }
                    }
                    //Insert the updated value into the pixel
                    input_line[y_iterator*total_width+x_iterator] = (int)sqrt((sum_x*sum_x)+(sum_y*sum_y));

                }
            }
            break;

        */
    }
    return ;

}



