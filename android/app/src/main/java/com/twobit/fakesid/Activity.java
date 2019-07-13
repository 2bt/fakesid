package com.twobit.fakesid;

import android.os.Bundle;
import android.util.Log;
import android.content.Context;
import android.view.KeyEvent;
import android.view.inputmethod.InputMethodManager;


public class Activity extends android.app.Activity {
    static String TAG = "FOOBAR";
    static Activity sInstance;
    MainView        mView;

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
        return true;
    }

    @Override protected void onCreate(Bundle b) {
        super.onCreate(b);
        sInstance = this;
        mView = new MainView(getApplication());
        setContentView(mView);

        Lib.setAssetManager(getResources().getAssets());
    }

    @Override protected void onPause() {
        super.onPause();
        mView.onPause();
    }

    @Override protected void onResume() {
        super.onResume();
        mView.onResume();
    }

    @Override protected void onDestroy() {
        super.onDestroy();
        mView.onDestroy();
    }
}
