//
// Created by Amit on 12/3/2016.
//

//Headers for common data and functions to be used in multiple Filters

//Guard bands
#ifndef ARTCELERATION_EEE598_ASSN2_AMITMANOJ_FILTERHEADERS_H
#define ARTCELERATION_EEE598_ASSN2_AMITMANOJ_FILTERHEADERS_H


#include <android/log.h>
#include <jni.h>

//Define macros for printing to the log
#define LOG_TAG "Color_Filter"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)


//Prototype functions
void Motion_Blur(AndroidBitmapInfo* bmp_info, void *pixels, int integer_array[]);
void Gaussian_Blur(AndroidBitmapInfo* bmp_info, void *pixels, int integer_array[], float float_array[]);
void Sobel_Filter(AndroidBitmapInfo* bmp_info, void *pixels, int integer_array[]);
void Unsharp_Mask(AndroidBitmapInfo* bmp_info, void *pixels, float float_array[]);
//Function to limit the intensity between 0 to 255
static int intensity_limit(int value){
    if(value >255){
        return 255;
    }
    if(value<0){
        return 0;
    }
    return value;
}




#endif //ARTCELERATION_EEE598_ASSN2_AMITMANOJ_FILTERHEADERS_H
