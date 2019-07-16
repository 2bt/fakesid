package com.twobit.fakesid;

import android.content.Context;
import android.content.pm.PackageManager;
import android.Manifest;
import android.os.Bundle;
import android.app.Activity;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.Log;
import android.view.inputmethod.InputMethodManager;
import android.view.KeyEvent;

public class MainActivity extends Activity {
    static String TAG = "FOOBAR";
    static MainActivity sInstance;
    MainView mView;

    static public void showKeyboard() {
        sInstance.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                InputMethodManager imm = (InputMethodManager) sInstance.getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.showSoftInput(sInstance.getWindow().getDecorView(), InputMethodManager.SHOW_FORCED);
            }
        });
    }
    static public void hideKeyboard() {
        sInstance.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                InputMethodManager imm = (InputMethodManager) sInstance.getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(sInstance.getWindow().getDecorView().getWindowToken(), 0);
            }
        });
    }

    @Override
    public boolean onKeyDown(final int code, final KeyEvent e) {
        mView.queueEvent(new Runnable() { public void run() {
            Lib.key(code, e.getUnicodeChar());
        }});
        return false;
    }

    @Override protected void onCreate(Bundle b) {
        super.onCreate(b);
        sInstance = this;
        mView = new MainView(getApplication());
        setContentView(mView);
        getWritePermission();
    }

    @Override protected void onResume() {
        super.onResume();
        mView.onResume();
        Lib.startAudio();
    }
    @Override protected void onPause() {
        super.onPause();
        mView.onPause();
        Lib.stopAudio();
    }
//    @Override protected void onDestroy() {
//        mView.queueEvent(new Runnable() { public void run() { Lib.exit(); }});
//        super.onDestroy();
//    }

    static final int PERMISSION_REQUEST_WRITE_EXTERNAL_STORAGE = 42;
    boolean mWritePermission = false;

    public void getWritePermission() {
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED) {
            mWritePermission = true;
        }
        else {
            ActivityCompat.requestPermissions(this,
                    new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},
                    PERMISSION_REQUEST_WRITE_EXTERNAL_STORAGE);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        if (requestCode != PERMISSION_REQUEST_WRITE_EXTERNAL_STORAGE) return;
        if (grantResults.length < 1) return;
        if (grantResults[0] == PackageManager.PERMISSION_GRANTED) mWritePermission = true;
    }

}
