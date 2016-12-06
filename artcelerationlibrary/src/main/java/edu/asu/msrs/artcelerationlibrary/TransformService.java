package edu.asu.msrs.artcelerationlibrary;

import android.app.Service;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.MemoryFile;
import android.os.Message;
import android.os.Messenger;
import android.os.ParcelFileDescriptor;
import android.os.RemoteException;
import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;

public class TransformService extends Service {

    //Load the NDK libraries
    static {
        System.loadLibrary("AllFilters-lib");
    }

    public TransformService() {
    }
    String TAG = "Service_TAG";
    int ticket_to_execute = 0, ticket_allotted = 0;
    private Object Lock1 = new Object();

    //Define Finals for Image Transform indices
    static final int MOTION_BLUR = 0;
    static final int GAUSSIAN_BLUR = 1;
    static final int SOBEL_FILTER =2;
    static final int UNSHARP_MASK =3;
    static final int NEON_EDGES = 4;

    // Messenger to receive the message from the Library.
    final Messenger ServiceMessenger = new Messenger (new TransformHandler());
    // Messenger to send the message to the Library.
    private Messenger ClientMessenger ;

    //Declare transforms with the keyword Native to link it to respective JNI call
    public native void Motion_Blur(Bitmap img, int[] intArgs);
    public native void Gaussian_Blur(Bitmap img, int[] intArgs, float[] floatArgs);
    public native void Sobel_Filter(Bitmap img, int[] intArgs);
    public native void Unsharp_Mask(Bitmap img, float[] floatArgs);
    // Handler to choose the transform to apply based on value sent from library.
    // This can spawn off threads to do the actual work.
    class TransformHandler extends Handler {
        @Override
        public void handleMessage(Message mesg) {


            // Obtain the messenger handler from the message.
            ClientMessenger = mesg.replyTo;
            // Get the data bundle as received from the service.

            Bundle dataBundle = mesg.getData();
            ParcelFileDescriptor pfd = (ParcelFileDescriptor)dataBundle.get("pfd");
            // Convert the bundle into a byteArray.
            InputStream is = new FileInputStream(pfd.getFileDescriptor());
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            int bytesRead;
            // ByteArray size hardcoded for now. Will make it more dynamic.
            byte[] out = new byte[16384];
            try {
                while((bytesRead = is.read(out,0,out.length)) != -1){
                    baos.write(out,0,bytesRead);
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
            try {
                baos.flush();
            } catch (IOException e) {
                e.printStackTrace();
            }

            Bitmap bitmap = BitmapFactory.decodeByteArray(baos.toByteArray(), 0, baos.size());


            ThreadTask t1;
            synchronized (Lock1) {
                //Pass the new thread with the parameters of the allotted ticket number, the byte Array and the transform number
                t1 = new ThreadTask(ticket_allotted,bitmap, mesg.what,dataBundle.getIntArray("Int_Args"), dataBundle.getFloatArray("Float_Args"));
                ticket_allotted++;
            }


            //Start thread!
            t1.run();
            return;


        }
    }


    @Override
    public IBinder onBind(Intent intent){
        return ServiceMessenger.getBinder();
    }

    class ThreadTask extends Thread{
        private int my_ticket;
        private Bitmap bmp;
        private int transformation;
        private int intArgs[];
        private float floatArgs[];

        private Message outmsg;
        ThreadTask(int a, Bitmap bmp, int transformation, int intArgs[], float floatArgs[]){
            this.my_ticket = a;
            this.bmp = bmp;
            this.transformation = transformation;
            this.intArgs = intArgs;
            this.floatArgs = floatArgs;
        }
        @Override
        public void run() {

            // Using a Switch-case to select the right transformation
            switch(transformation){
                case MOTION_BLUR :
                    // Call the Motion Blur transform
                    Log.i(TAG, "Perform Motion Blur");
                    Motion_Blur(bmp, intArgs);

                    break;
                case GAUSSIAN_BLUR :
                    // Call the Gaussian Blur transform
                    Log.i(TAG, "Perform Gaussian Blur");
                    if(bmp==null|| intArgs==null || floatArgs==null){
                        bmp = change_black(bmp);
                        return;
                    }
                    Gaussian_Blur(bmp, intArgs, floatArgs);
                    break;
                case SOBEL_FILTER :
                    // Call the Sobel Filter transform
                    Log.d(TAG, "Perform Sobel Filter");
                    Sobel_Filter(bmp,intArgs);
                    break;
                case UNSHARP_MASK :
                    // Call the Unsharp Mask transform
                    Log.d(TAG, "Perform Unsharp Mask");
                    Unsharp_Mask(bmp,floatArgs);
                    break;
                case NEON_EDGES:
                    // Call the Neon Edges transform
                    Log.d(TAG, "Perform Neon Edges");
                    break;
                default:
                    Log.d(TAG, "Default");
                    return;
            }

            // Converting to byteArray.
            ByteArrayOutputStream outbAos = new ByteArrayOutputStream();
            bmp.compress(Bitmap.CompressFormat.PNG,100,outbAos);
            byte[] outbA = outbAos.toByteArray();
            // Prepare a memoryFile to send the message back to the Library.
            MemoryFile memFile = null;
            Bundle outdataBundle = new Bundle();
            ParcelFileDescriptor Outpfd;
            try {
                memFile = new MemoryFile("imgOutKey", outbA.length);
                memFile.writeBytes(outbA,0,0,outbA.length);
                Outpfd = MemoryFileUtil.getParcelFileDescriptor(memFile);
                outdataBundle.putParcelable("Outpfd", Outpfd);
                //Create the output message
                outmsg = Message.obtain(null, transformation);
                outmsg.setData(outdataBundle);
            } catch (IOException e) {
                e.printStackTrace();
            }


            while (true) {
                synchronized (Lock1) {
                    if (my_ticket == ticket_to_execute)
                        //Break if its my turn to execute
                        break;
                }
                //Sleep for 10 milliseconds if its not my turn to execute
                try {
                    Thread.sleep(10);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }

            // Use the messenger created just for send.

            try {
                ClientMessenger.send(outmsg);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            //Increment the ticket_to_execute so that the next thread gets a chance to execute

            synchronized (Lock1){
                ticket_to_execute++;
            }

            //Close the memory File
            memFile.close();


            }
        }

    Bitmap change_black(Bitmap bmp){

        int [] allpixels = new int [bmp.getHeight()*bmp.getWidth()];
        bmp.getPixels(allpixels,0,bmp.getWidth(),0,0,bmp.getWidth(),bmp.getHeight());

        //Manipulate all the background black pixels in the image to transparent by making the A value in ARGB to 0

        for(int i = 0; i<allpixels.length;i++){
            //if(allpixels[i] == Color.BLACK)
                //For all black pixels, convert their opacity to 0
                allpixels[i] =Color.RED;
        }
        bmp = Bitmap.createBitmap(allpixels,bmp.getWidth(),bmp.getHeight(), Bitmap.Config.ARGB_8888 );

        return  bmp;
    }


}



