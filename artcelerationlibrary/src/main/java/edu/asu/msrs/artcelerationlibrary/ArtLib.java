package edu.asu.msrs.artcelerationlibrary;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
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
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;

/**
 * Created by rlikamwa on 10/2/2016.
 */

public class ArtLib {
    private TransformHandler artlistener;
    private Activity myActivity;

    public ArtLib(Activity activity) {
        myActivity = activity;
        initialize();
    }

    // Messenger to get replies on from the Service.
    private Messenger ServiceMessenger ;
    // Messenger to send messages to Service
    private Messenger myMessenger;
    private boolean Bound;

    //Define Finals for Image Transform indices
    static final int MOTION_BLUR = 0;
    static final int GAUSSIAN_BLUR = 1;
    static final int SOBEL_FILTER =2;
    static final int UNSHARP_MASK =3;
    static final int NEON_EDGES = 4;
    static final String[] transforms = {"Motion Blur","Gaussian Blur", "Sobel Filter", "Unsharp Mask", "Neon edges"};

    // Handler to start a connection and close a connection between library and service.
    ServiceConnection mServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            myMessenger = new Messenger(service);
            ServiceMessenger = new Messenger(new TransformHandlerLib());
            Bound = true;
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            myMessenger = null;
            ServiceMessenger = null;
            Bound = false;
        }
    };

    // Initialization Method. Binds the activity to the transform service.
    public void initialize() {
        myActivity.bindService(new Intent(myActivity, TransformService.class), mServiceConnection, Context.BIND_AUTO_CREATE);
    }

    // This array contains the list of transforms that the library supports. Also used to populate the drop down on the UI.
    public String[] getTransformsArray() {;
        return transforms;
    }

    //
    public TransformTest[] getTestsArray() {
        TransformTest[] transforms = new TransformTest[5];
        transforms[0] = new TransformTest(0, new int[]{1,10}, new float[]{});
        transforms[1] = new TransformTest(1, new int[]{50}, new float[]{5.0f});
        transforms[2] = new TransformTest(2, new int[]{0}, new float[]{});
        transforms[3] =  new TransformTest(3, new int[]{51, 42, 33}, new float[]{0.5f, 0.6f, 0.3f});
        transforms[4] = new TransformTest(4, new int[]{51, 42, 33}, new float[]{0.5f, 0.6f, 0.3f});

        return transforms;
    }

    public void registerHandler(TransformHandler artlistener) {
        this.artlistener = artlistener;
    }

    // Request transform sends a request to the service to perform an image transform. It sends the relevant data to the Service.
    // It is non blocking, so it returns true if the request was successfully made. A new request can be placed before the old one has completed.
    public boolean requestTransform(Bitmap img, int index, int[] intArgs, float[] floatArgs) {
        try {
            //Do Argument Checking first
            if(!Argument_checks(index,intArgs,floatArgs)){
                return false;
            }

            // Convert the bitmap image to byteArray
            Log.d("Library","Requesting transform index "+index);
            ByteArrayOutputStream bAos = new ByteArrayOutputStream();
            img.compress(Bitmap.CompressFormat.PNG, 100, bAos);
            byte[] bA = bAos.toByteArray();
            // Create a memoryFile of size of the input  or now the byteArray.
            MemoryFile memFile = new MemoryFile("imgKey", bA.length);
            memFile.writeBytes(bA, 0, 0, bA.length);
            ParcelFileDescriptor pfd = MemoryFileUtil.getParcelFileDescriptor(memFile);

            int what = index;
            // Making data parcelable to be sent across to service
            Bundle dataBundle = new Bundle();
            dataBundle.putParcelable("pfd", pfd);

            dataBundle.putIntArray("Int_Args", intArgs);
            dataBundle.putFloatArray("Float_Args", floatArgs);


            // Build the message to send to Service.
            Message msg = Message.obtain(null, what);
            // Payload of the message
            msg.setData(dataBundle);
            // Tell Service to reply on this Messenger.
            msg.replyTo = ServiceMessenger;

            try {
                myMessenger.send(msg);          // Send message to service.myMessenger.send(msg);          // Send message to service.
                memFile.close();                // Close to memory File before finalize().
            } catch (RemoteException e) {
                e.printStackTrace();
            }

        } catch (IOException e) {
            e.printStackTrace();

        }

        return true;                            // Return to user to allow new requests.
    }

    // Handler to receive messages from the Service. This is owned by the messenger dedicated for receive.
    class TransformHandlerLib extends Handler {
        @Override
        public void handleMessage(Message mesg) {
            // Unpack the data. Similar to what we do on the Service side when Library sent it a message.
            Log.d("Library Response", "Completed " + transforms[mesg.what]+" Transform!" );
            Bundle dataBundle = mesg.getData();
            ParcelFileDescriptor pfd = (ParcelFileDescriptor) dataBundle.get("Outpfd");
            //Convert the data bundle obtained into a byte array.
            InputStream is = new FileInputStream(pfd.getFileDescriptor());
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            int bytesRead;

            byte[] out = new byte[16384];

            try {
                while ((bytesRead = is.read(out, 0, out.length)) != -1) {
                    baos.write(out, 0, bytesRead);
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
            try {
                baos.flush();
            } catch (IOException e) {
                e.printStackTrace();
            }

            // Decode the byteArray back into a bitmap to be displayed.
            Bitmap bitmap = BitmapFactory.decodeByteArray(baos.toByteArray(), 0, baos.size());

            //Send Processed bitmap back to the UI thread
            artlistener.onTransformProcessed(bitmap);

        }
    }

    boolean Argument_checks(int index, int int_Args[], float float_Args[]){
        //Check limits of the index
        String TAG = "Invalid input:";
        if(index >4 || index <0){
            Log.e(TAG, "Index of Filter should be between 0 to 4");
                    return false;
        }

        switch (index){

            case MOTION_BLUR :
                //Check Arguments for motion blur
                if(int_Args == null){
                    Log.e(TAG,"Motion Blur - Int Args is null");
                    return false;
                }
                if(int_Args[0] < 0  || int_Args[0] > 1 || int_Args[1] <0) {
                    Log.e(TAG,"Motion Blur - Provide the radius greater than 0  and blur direction as 0 or 1");
                    return false;
                }
                break;
            case GAUSSIAN_BLUR :
                //Check Arguments for Gaussian Blur
                if(int_Args[0]< 0  || float_Args[0] <0) {
                    Log.e(TAG, "Gaussian Blur - The radius and standard deviation should be greater than 0");
                    return false;
                }
                break;
            case SOBEL_FILTER :
                //Check Arguments for Sobel Filter
                if(int_Args[0]< 0 ||int_Args[0]>2){
                    Log.e(TAG, "Sobel Filter - a0 should be within 0 and 2 inclusive.");
                    return false;
                }
                break;
            case UNSHARP_MASK :
                // Call the Unsharp Mask transform
                if(float_Args[0]<0||float_Args[1]<0) {
                    Log.e(TAG, "Unsharp Mask - The scaling factor and standard deviation should be greater than 0");
                    return false;
                }
                break;
            case NEON_EDGES:
                if(float_Args[0]<0|| float_Args[1]<0 || float_Args[2]<0) {
                    Log.e(TAG, "Neon Edges - The scaling factors and standard deviation should be greater than 0");
                    return false;
                }
                break;

        }

        return true;


    }
}
