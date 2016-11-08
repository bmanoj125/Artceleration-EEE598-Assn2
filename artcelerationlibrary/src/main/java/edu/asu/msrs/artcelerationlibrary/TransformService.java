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
    static final int Transform_ONE = 1;
    static final int Transform_TWO = 2;
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
            // Using a Switch-case to select the right transform or action to perform. For now hardcoded.
            switch(mesg.what){
                case Transform_ONE:
                    Log.d(TAG, "Transform_ONE");
                    break;
                case Transform_TWO:
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
                    // Do a garbage transformation. Here the function decrements each element by 1.
                    outbA = transformImg(outbA);
                    // Print this "Transformed" byteArray.
                    String TfrbA = Arrays.toString(outbA);
                    Log.d("Service Response",TfrbA);

                    // Prepare a memoryFile to send the message back to the Library.
                    MemoryFile memFile = null;
                    Bundle outdataBundle = new Bundle();
                    ParcelFileDescriptor Outpfd;
                    try {
                        memFile = new MemoryFile("imgOutKey", outbA.length);
                        memFile.writeBytes(outbA,0,0,outbA.length);
                        Outpfd = MemoryFileUtil.getParcelFileDescriptor(memFile);
                        outdataBundle.putParcelable("Outpfd", Outpfd);
                        Message outmsg = Message.obtain(null, 1);
                        outmsg.setData(outdataBundle);
                        // Remember to use the messenger created just for send.
                        ClientMessenger.send(outmsg);
                        memFile.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    } catch (RemoteException e) {
                        e.printStackTrace();
                    }
                    break;
                default:
                    Log.d(TAG, "Default");
                    break;
            }
        }
    }

    // A simple function that "transforms" the input image.
    public byte[] transformImg(byte[] In){
        for( int i=0; i < In.length;i++){
            In[i]--;
        }
        return In;
    }


    @Override
    public IBinder onBind(Intent intent){
        return ServiceMessenger.getBinder();
    }

//    class ThreadTask extends Thread{
//        int my_ticket;
//        ThreadTask(int a){
//            this.my_ticket = a;
//        }
//        @Override
//        public void run() {
//                synchronized(barrier) {
                      //while(current_ticket)
//                    Log.d("TTask", "hey before " + a);
//                    try {
//                        Thread.sleep(a * 1000);
//                    } catch (InterruptedException e) {
//                        e.printStackTrace();
//                    }
//                }
//                Log.d("TTask", "hey after " + a);
//                try {
//                    Thread.sleep(20);
//                } catch (InterruptedException e) {
//                    e.printStackTrace();
//                }
//            }
//        }

}



