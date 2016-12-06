/*
** This file contains the native code to perform Unsharp Masking of an image. It makes the image sharper but less clearer.
** Arguments:
        bmp_info - The metadata about the bitmap of the image that needs to be transformed. Contains height and width and format information.
        pixels   - The actual pixel data
        float_array - arguments for the unsharp masking, the standard deviation of gaussian filter is stored in index 0 and scaling factor of mask is stored in index 1.

   Return:
        void
   The function transforms the bitmap image in place.

**
*/

#include <android/bitmap.h>
#include <stdlib.h>
#include "FilterHeaders.h"
#include <math.h>
extern void Gaussian_Blur(AndroidBitmapInfo* bmp_info, void *pixels, int integer_array[], float float_array[]);

void Unsharp_Mask(AndroidBitmapInfo* bmp_info, void* pixels, float float_array[]){

    uint32_t *input_line = (uint32_t*) pixels;
    float std_dev, scaling_factor;
    //Dereference here to avoid overhead in loops
    int total_width = bmp_info->width;
    int total_height = bmp_info->height;
    int red, green, blue, x_iterator, y_iterator;

    //Get the two float arguments
    std_dev = float_array[0];
    scaling_factor = float_array[1];

    int radius[1];
    //Set the radius as 6 timers the standard deviation
    radius[0] = 6*std_dev;

    float sigma[1];
    sigma[0]=std_dev;

    //Generate a copy of the original image

    uint32_t* original_img = (uint32_t*)malloc(sizeof(uint32_t)*total_height*total_width);
    memcpy(original_img, input_line,sizeof(uint32_t)*total_height*total_width);

    //Gaussian Blur the image with 6 sigma radius and sigma standard deviation
    Gaussian_Blur(bmp_info,pixels,radius,sigma);
    input_line = (uint32_t*)pixels;



    //Scale the pixels by the difference.
    for(y_iterator=0;y_iterator < total_height; y_iterator++){

        for(x_iterator=0;x_iterator<total_width; x_iterator++) {
            //Calculate some of the indices beforehand to get speed up

            int index =y_iterator*total_width+x_iterator;

            //Take the difference and Scale by the factor chosen. Add this value to the original image
            //We're using the intensity_limit function to limit it within 0 and 255
            red = ((original_img[index] & 0x00FF0000) >> 16)+scaling_factor*intensity_limit((((original_img[index] & 0x00FF0000) >> 16)-((input_line[index] & 0x00FF0000) >> 16)));
            green = ((original_img[index] & 0x0000FF00) >> 8)+ scaling_factor*intensity_limit((((original_img[index] & 0x0000FF00) >> 8)-((input_line[index] & 0x0000FF00) >> 8)));
            blue = (original_img[index] & 0x000000FF)+scaling_factor*intensity_limit((((original_img[index] & 0x000000FF)) - ((input_line[index] & 0x000000FF))));

            //Limit the intensities to within 0 and 255
            red = intensity_limit(red);
            green = intensity_limit(green);
            blue = intensity_limit(blue);

            // Fuse the three channel assign it to the pixel value in q_vector;
            input_line[y_iterator*total_width+x_iterator] = ((red << 16) & 0x00FF0000) |
                                                                ((green << 8) & 0x0000FF00) |
                                                                (blue & 0x000000FF);

        }
    }


    //Free the allocated memory for the image
    free(original_img);


}