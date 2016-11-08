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
    public String[] getTransformsArray() {
        String[] transforms = {"Gaussian Blur", "Neon edges", "Color Filter"};
        return transforms;
    }

    //
    public TransformTest[] getTestsArray() {
        TransformTest[] transforms = new TransformTest[3];
        transforms[0] = new TransformTest(0, new int[]{1, 2, 3}, new float[]{0.1f, 0.2f, 0.3f});
        transforms[1] = new TransformTest(1, new int[]{11, 22, 33}, new float[]{0.3f, 0.2f, 0.3f});
        transforms[2] = new TransformTest(2, new int[]{51, 42, 33}, new float[]{0.5f, 0.6f, 0.3f});

        return transforms;
    }

    public void registerHandler(TransformHandler artlistener) {
        this.artlistener = artlistener;
    }

    // Request transform sends a request to the service to perform an image transform. It sends the relevant data to the Service.
    // It is non blocking, so it returns true if the request was successfully made. A new request can be placed before the old one has completed.
    public boolean requestTransform(Bitmap img, int index, int[] intArgs, float[] floatArgs) {
        try {
            // Convert the bitmap image to byteArray
            ByteArrayOutputStream bAos = new ByteArrayOutputStream();
            img.compress(Bitmap.CompressFormat.PNG, 0, bAos);
            byte[] bA = bAos.toByteArray();
            // Create a memoryFile of size of the input  or now the byteArray.
            MemoryFile memFile = new MemoryFile("imgKey", bA.length);
            memFile.writeBytes(bA, 0, 0, bA.length);
            String input = Arrays.toString(bA);
            Log.d("Library Input", input);
            ParcelFileDescriptor pfd = MemoryFileUtil.getParcelFileDescriptor(memFile);

            int what = index;

            // Making data parcelable to be sent across to service
            Bundle dataBundle = new Bundle();
            dataBundle.putParcelable("pfd", pfd);
            // Build the message to send to Service.
            Message msg = Message.obtain(null, what);
            // Payload of the message
            msg.setData(dataBundle);
            // Tell Service to reply on this Messenger.
            msg.replyTo = ServiceMessenger;
            try {
                myMessenger.send(msg);          // Send message to service.
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
            Log.d("Library Response", "handleMessage(msg)" + mesg.what);
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
            // Convert the byteArray to a string for easy viewability.
            String temp = Arrays.toString(baos.toByteArray());
            Log.d("LibraryResponse", temp);
        }
    }
}
