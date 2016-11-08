package edu.asu.msrs.artcelerationlibrary;

import android.app.Service;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
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
    public TransformService() {
    }
    String TAG = "Service_TAG";
    int ticket_to_execute = 0, ticket_allotted = 0;
    private Object Lock1 = new Object();
    static final int Transform_ONE = 0;
    static final int Transform_TWO = 1;
    static final int Transform_THREE =2;
    // Messenger to receive the message from the Library.
    final Messenger ServiceMessenger = new Messenger (new TransformHandler());
    // Messenger to send the message to the Library.
    private Messenger ClientMessenger ;

    // Handler to choose the transform to apply based on value sent from library.
    // This can spawn off threads to do the actual work.
    class TransformHandler extends Handler {
        @Override
        public void handleMessage(Message mesg) {
            Log.d(TAG, "handleMessage(msg)" + mesg.what);


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
            // Convert the outputStream to String for easy printing/viewing on log.
            String temp = Arrays.toString(baos.toByteArray());
            Log.d("Service Output",temp);
            // Converting to byteArray.
            ByteArrayOutputStream outbAos = new ByteArrayOutputStream();
            bitmap.compress(Bitmap.CompressFormat.PNG,0,outbAos);
            byte[] outbA = outbAos.toByteArray();


            ThreadTask t1;
            synchronized (Lock1) {
                //Pass the new thread with the parameters of the allotted ticket number, the byte Array and the transform number
                t1 = new ThreadTask(ticket_allotted,outbA, mesg.what);
                ticket_allotted++;
            }


            //Start thread!
            t1.run();
            return;

        }
    }

    // A simple function that "transforms" the input image.
    public byte[] transformImg1(byte[] In){
        for( int i=0; i < In.length;i++){
            In[i]--;
        }
        return In;
    }

    public byte[] transformImg2(byte[] In){
        for( int i=0; i < In.length;i++){
            In[i]++;
        }
        return In;
    }

    public byte[] transformImg3(byte[] In){
        for( int i=0; i < In.length;i++){
            In[i]=0;
        }
        return In;
    }


    @Override
    public IBinder onBind(Intent intent){
        return ServiceMessenger.getBinder();
    }

    class ThreadTask extends Thread{
        private int my_ticket;
        private byte[] outbA;
        private int transformation;

        private Message outmsg;
        ThreadTask(int a, byte[] bA, int transformation){
            this.my_ticket = a;
            this.outbA = bA;
            this.transformation = transformation;
        }
        @Override
        public void run() {

            // Using a Switch-case to select the right transform or action to perform. For now hardcoded.
            switch(transformation){
                case Transform_ONE:
                    // Do a garbage transformation. Here the function decrements each element by 1.
                    Log.d(TAG, "Perform Transform_ONE");
                    outbA= transformImg1(outbA);
                    break;
                case Transform_TWO:
                    // Do a garbage transformation. Here the function increments each element by 1.
                    Log.d(TAG, "Perform Transform_TWO");
                    //Sleep to test the in order queueing
                    try {
                        sleep(10000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    outbA= transformImg2(outbA);
                    break;
                case Transform_THREE:
                    // Do a garbage transformation. Here the function sets each element to 0.
                    Log.d(TAG, "Perform Transform_THREE");
                    outbA=transformImg3(outbA);
                    break;
                default:
                    Log.d(TAG, "Default");
                    return;
            }
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
                outmsg = Message.obtain(null, 1);
                outmsg.setData(outdataBundle);
            } catch (IOException e) {
                e.printStackTrace();
            }



            // Print this "Transformed" byteArray.
            String TfrbA = Arrays.toString(outbA);
            Log.d("Service Response",TfrbA);
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

            // Remember to use the messenger created just for send.

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
}



