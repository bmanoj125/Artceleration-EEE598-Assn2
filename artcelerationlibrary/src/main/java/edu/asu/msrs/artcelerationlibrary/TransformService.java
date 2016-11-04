package edu.asu.msrs.artcelerationlibrary;

import android.app.Service;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.ParcelFileDescriptor;
import android.util.Log;

import java.io.FileInputStream;

public class TransformService extends Service {
    public TransformService() {
    }
    String TAG = "Service_TAG";
    static final int Transform_ONE = 1;
    static final int Transform_TWO = 2;
    class TransformHandler extends Handler {
        @Override
        public void handleMessage(Message mesg) {
            Log.d(TAG, "handleMessage(msg)" + mesg.what);
            switch(mesg.what){
                case Transform_ONE:
                    Log.d(TAG, "Transform_ONE");
                    break;
                case Transform_TWO:
                    Bundle dataBundle = mesg.getData();
                    ParcelFileDescriptor pfd = (ParcelFileDescriptor)dataBundle.get("pfd");
                    FileInputStream fios = new FileInputStream(pfd);
                    int result = mesg.arg1 * mesg.arg2;
                    Log.d(TAG, "Transform_TWO");
                    break;
                default:
                    Log.d(TAG, "Default");
                    break;
            }
        }
    }

    final Messenger mMessenger = new Messenger (new TransformHandler());
    @Override
    public IBinder onBind(Intent intent){
        return mMessenger.getBinder();
    }
}