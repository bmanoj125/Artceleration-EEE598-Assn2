//
// Created by Amit on 12/4/2016.
//

#include <android/log.h>
#include <android/bitmap.h>

#include <stdio.h>
#include <jni.h>
#include <math.h>

#include "FilterHeaders.h"



extern "C" {
//Call corresponding to motion blur
JNIEXPORT void JNICALL
Java_edu_asu_msrs_artcelerationlibrary_TransformService_Motion_1Blur(JNIEnv * env, jobject  obj, jobject inp_bitmap, jintArray integer_array)
{

    AndroidBitmapInfo  info;
    int ret;
    void *pixels;


    //Get info from the bitmap to the info struct
    ret = AndroidBitmap_getInfo(env, inp_bitmap, &info);
    if (ret < 0) {
        LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
        return;
    }

    //Lock the memory for the pixels
    ret = AndroidBitmap_lockPixels(env, inp_bitmap, &pixels);
    if (ret < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return;
    }

    //Lock memory of the arguments
    jint *int_args = env->GetIntArrayElements(integer_array,NULL);

    LOGE("Performing Motion Blur");
    //Perform Motion blur
    Motion_Blur(&info,pixels,int_args);

    //LOGI("Motion Blur complete!");


    //Unlock the memory after completing the transform
    AndroidBitmap_unlockPixels(env, inp_bitmap);
    env->ReleaseIntArrayElements(integer_array, int_args,0);
}


//Call mapping to Gaussian Blur
JNIEXPORT void JNICALL Java_edu_asu_msrs_artcelerationlibrary_TransformService_Gaussian_1Blur(JNIEnv * env, jobject  obj, jobject inp_bitmap, jintArray integer_array,jfloatArray float_array)
{
    AndroidBitmapInfo  info;
    int ret;
    void *pixels;


    //Get info from the bitmap to the info struct
    ret = AndroidBitmap_getInfo(env, inp_bitmap, &info);
    if (ret < 0) {
        LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
        return;
    }

    //Lock the memory for the pixels
    ret = AndroidBitmap_lockPixels(env, inp_bitmap, &pixels);
    if (ret < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return;
    }

    //Lock memory of the arguments
    jint *int_args = env->GetIntArrayElements(integer_array,NULL);
    jfloat *float_args = env->GetFloatArrayElements(float_array,NULL);

    //Perform Gaussian Blur
    Gaussian_Blur(&info,pixels,int_args,float_args);
    //Unlock the memory after completing the transform
    AndroidBitmap_unlockPixels(env, inp_bitmap);
    env->ReleaseIntArrayElements(integer_array, int_args,0);
    env->ReleaseFloatArrayElements(float_array,float_args,0);
}


//Call mapping to Sobel Filter
JNIEXPORT void JNICALL Java_edu_asu_msrs_artcelerationlibrary_TransformService_Sobel_1Filter(JNIEnv * env, jobject  obj, jobject inp_bitmap, jintArray integer_array,jfloatArray float_array)
{
    AndroidBitmapInfo  info;
    int ret;
    void *pixels;


    //Get info from the bitmap to the info struct
    ret = AndroidBitmap_getInfo(env, inp_bitmap, &info);
    if (ret < 0) {
        LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
        return;
    }

    //Lock the memory for the pixels
    ret = AndroidBitmap_lockPixels(env, inp_bitmap, &pixels);
    if (ret < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return;
    }

    //Lock memory of the arguments
    jint *int_args = env->GetIntArrayElements(integer_array,NULL);

    //Perform Sobel Filter
    Sobel_Filter(&info, pixels, int_args);
    //Unlock the memory after completing the transform
    AndroidBitmap_unlockPixels(env, inp_bitmap);
    env->ReleaseIntArrayElements(integer_array, int_args,0);
}



//Call mapping to Unsharp Mask
JNIEXPORT void JNICALL Java_edu_asu_msrs_artcelerationlibrary_TransformService_Unsharp_1Mask(JNIEnv * env, jobject  obj, jobject inp_bitmap,jfloatArray float_array)
{
    AndroidBitmapInfo  info;
    int ret;
    void *pixels;


    //Get info from the bitmap to the info struct
    ret = AndroidBitmap_getInfo(env, inp_bitmap, &info);
    if (ret < 0) {
        LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
        return;
    }

    //Lock the memory for the pixels
    ret = AndroidBitmap_lockPixels(env, inp_bitmap, &pixels);
    if (ret < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return;
    }

    //Lock memory of the arguments
    jfloat *float_args = env->GetFloatArrayElements(float_array,NULL);

    //Perform Unsharp Mask
    Unsharp_Mask(&info, pixels, float_args);
    //Unlock the memory after completing the transform

    AndroidBitmap_unlockPixels(env, inp_bitmap);
    env->ReleaseFloatArrayElements(float_array,float_args,0);
}
}