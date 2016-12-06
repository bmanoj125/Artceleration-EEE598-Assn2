/*
** This file contains the native code to perform Sobel Filtering of an image. It showcases the lines detected in the direction chosen.
** Arguments:
        bmp_info - The metadata about the bitmap of the image that needs to be transformed. Contains height and width and format information.
        pixels   - The actual pixel data
        integer_array - arguments for the sobel filter, the type of filtering is stored in index 0.
                        0 is for row filtering.
                        1 is for column filtering.
                        2 is for row and column filtering.

   Return:
        void
   The function transforms the bitmap image in place. The output is a grayscale image.
*/

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
    int sum_xr, sum_xg, sum_xb;


    a0 = integer_array[0];
    //Get the pointer to the row of pixel values
    input_line = (uint32_t *) pixels;

    //Create a grayscale brightness image of the input

    int *gray = (int *) malloc(sizeof(int) * total_height * total_width);

    for (y_iterator = 0; y_iterator < total_height; y_iterator++) {


        int x_index = y_iterator * total_width;
        for (x_iterator = 0; x_iterator < total_width; x_iterator++) {


            //Extract each channel

            red_q = (((input_line[x_index + x_iterator] & 0x00FF0000) >> 16));
            green_q = (((input_line[x_index + x_iterator] & 0x0000FF00) >> 8));
            blue_q = ((input_line[x_index + x_iterator] & 0x000000FF));


            //Scale each channel and compute the gray scale value
            gray_value = red_q * 0.2989 + green_q * 0.5870 + blue_q * 0.1140;


            //Store the gray value into the malloced array after attenuating it
            gray[y_iterator * total_width + x_iterator] = intensity_limit((int) gray_value);
        }
    }

    //Using the switch statements here instead of inside the loop to avoid the overhead of branches inside the loop
    // This would make the execution relatively faster
    // Also instead of modularizing the updating in functions, we wrote inline to improve speed of execution
    // Modularizing it would reduce the space, but using inline makes it faster, and this was the aim of the project for us.


    switch (a0) {
        //Get the pointer to the row of pixel values
        int out;

        case 0:
            for (y_iterator = 0; y_iterator < total_height; y_iterator++) {
                for (x_iterator = 0; x_iterator < total_width; x_iterator++) {

                    //Iterate through the input_line
                    sum_x = 0;
                    uint32_t final_value;
                    for (int i = -1; i < 2; i++) {
                        for (int j = -1; j < 2; j++) {
                            //Skip the pixels that go out of range or where the mask is 0
                            if (j == 0 || (y_iterator + i) < 0 ||
                                (y_iterator + i >= total_height) || (x_iterator + j) < 0 ||
                                (x_iterator + j) >= total_width) {
                                continue;
                            }
                            sum_x += gray[((y_iterator + i) * total_width) + x_iterator + j] *
                                     Sx_mask[1 + i][1 + j];
                        }
                    }
                    //Limit the values between 0 and 255
                    sum_x = intensity_limit(sum_x);

                    //Insert the updated value into the pixel
                    input_line[y_iterator * total_width + x_iterator] =
                            ((sum_x << 16) & 0x00FF0000) |
                            ((sum_x << 8) & 0x0000FF00) |
                            (sum_x & 0x000000FF);
                }
            }
            break;

        case 1:
            for (y_iterator = 0; y_iterator < total_height; y_iterator++) {

                for (x_iterator = 0; x_iterator < total_width; x_iterator++) {
                    //Iterate through the input_line
                    sum_y = 0;
                    uint32_t final_value;
                    for (int i = -1; i < 2; i++) {
                        for (int j = -1; j < 2; j++) {
                            //Skip the pixels that go out of range or where the mask is 0
                            if (i == 0 || (y_iterator + i) < 0 ||
                                (y_iterator + i >= total_height) || (x_iterator + j) < 0 ||
                                (x_iterator + j) >= total_width) {
                                continue;
                            }
                            sum_y += gray[((y_iterator + i) * total_width) + x_iterator + j] *
                                     Sy_mask[1 + i][1 + j];
                        }
                    }
                    //Limit the values between 0 and 255
                    sum_y = intensity_limit(sum_y);

                    //Insert the updated value into the pixel
                    input_line[y_iterator * total_width + x_iterator] =
                            (((int) sum_y << 16) & 0x00FF0000) |
                            (((int) sum_y << 8) & 0x0000FF00) |
                            ((int) sum_y & 0x000000FF);
                }
            }
            break;

        case 2:

            for (y_iterator = 0; y_iterator < total_height; y_iterator++) {

                for (x_iterator = 0; x_iterator < total_width; x_iterator++) {
                    //Iterate through the input_line
                    sum_x = 0;
                    sum_y = 0;
                    uint32_t final_value;
                    for (int i = -1; i < 2; i++) {
                        for (int j = -1; j < 2; j++) {
                            //Skip the pixels that go out of range or where the mask is 0
                            if ((y_iterator + i) < 0 || (y_iterator + i >= total_height) ||
                                (x_iterator + j) < 0 || (x_iterator + j) >= total_width) {
                                continue;
                            }
                            sum_y += gray[((y_iterator + i) * total_width) + x_iterator + j] *
                                     Sy_mask[1 + i][1 + j];
                            sum_x += gray[((y_iterator + i) * total_width) + x_iterator + j] *
                                     Sx_mask[1 + i][1 + j];
                        }
                    }
                    //Limit the values between 0 and 255
                    sum_y = intensity_limit(sum_y);
                    sum_x = intensity_limit(sum_x);

                    //Get the output of Gx and Gy to generate the edges in both directions
                    out = intensity_limit(sqrt((sum_x * sum_x) + (sum_y * sum_y)));

                    //Insert the updated value into the pixel
                    input_line[y_iterator * total_width + x_iterator] =
                            (((int) out << 16) & 0x00FF0000) |
                            (((int) out << 8) & 0x0000FF00) |
                            ((int) out & 0x000000FF);


                }
            }
            break;

    }

    free(gray);


    return;

}