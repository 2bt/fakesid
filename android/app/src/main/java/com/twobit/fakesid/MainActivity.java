package com.twobit.fakesid;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.Manifest;
import android.os.Bundle;
import android.app.Activity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.core.view.WindowInsetsControllerCompat;
import androidx.core.view.WindowCompat;

import android.util.Log;
import android.view.inputmethod.InputMethodManager;
import android.view.KeyEvent;
import android.view.Window;

public class MainActivity extends Activity {
    static String TAG = "FOOBAR";
    static MainActivity             sInstance;
    static SharedPreferences        sPrefs;
    static SharedPreferences.Editor sPrefEdit;
    MainView mView;

    private static final int SETTING_FULLSCREEN_ENABLED = 0;


    static public void showKeyboard() {
        sInstance.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                InputMethodManager imm = (InputMethodManager) sInstance.getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.showSoftInput(sInstance.getWindow().getDecorView(), InputMethodManager.SHOW_IMPLICIT);
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


    // called from C++
    static public void updateSetting(int i) {
        int v = Lib.getSettingValue(i);
        sInstance.runOnUiThread(() -> {
            switch (i) {
                case SETTING_FULLSCREEN_ENABLED:
                    sInstance.setImmersiveMode(v != 0);
                    break;
                default:
                    break;
            }
        });
    }

    private void setImmersiveMode(boolean enabled) {
        Log.i(TAG, "setImmersiveMode " + enabled);
        Window window = getWindow();
        android.view.View decorView = window.getDecorView();
        WindowInsetsControllerCompat controller = WindowCompat.getInsetsController(window, decorView);
        WindowCompat.setDecorFitsSystemWindows(window, !enabled);
        if (enabled) {
            controller.setSystemBarsBehavior(WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
            controller.hide(WindowInsetsCompat.Type.systemBars());
        } else {
            controller.show(WindowInsetsCompat.Type.systemBars());
        }
    }


    @Override
    public boolean onKeyDown(final int code, final KeyEvent e) {
        mView.queueEvent(new Runnable() { public void run() {
            Lib.key(code, e.getUnicodeChar());
        }});
        return false;
    }

    @Override protected void onCreate(Bundle bundle) {
        super.onCreate(bundle);
        sInstance = this;
        mView = new MainView(getApplication());
        setContentView(mView);
        getWritePermission();

        sPrefs    = getPreferences(MODE_PRIVATE);
        sPrefEdit = sPrefs.edit();

        // Setup edge-to-edge display
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.KITKAT_WATCH) {
            ViewCompat.setOnApplyWindowInsetsListener(mView, (view, insets) -> {
                int topInset = insets.getInsets(WindowInsetsCompat.Type.systemBars()).top;
                int bottomInset = insets.getInsets(WindowInsetsCompat.Type.systemBars()).bottom;
                Lib.setInsets(topInset, bottomInset);
                return insets;
            });
        }

        // Load settings from SharedPreferences to C++
        loadSettings();

        // Apply fullscreen setting
        updateSetting(SETTING_FULLSCREEN_ENABLED);

        // trigger inset listener
        var decorView = getWindow().getDecorView();
        decorView.post(() -> ViewCompat.requestApplyInsets(decorView));
    }

    private void loadSettings() {
        Log.i(TAG, "loadSettings");
        String name;
        for (int i = 0; (name = Lib.getSettingName(i)) != null; ++i) {
            // Get value from SharedPreferences
            int defaultValue = Lib.getSettingValue(i);
            int value = sPrefs.getInt(name, defaultValue);
            // Set in C++
            Log.i(TAG, "loadSettings: " + name + " = " + value);
            Lib.setSettingValue(i, value);
        }
    }

    private void saveSettings() {
        SharedPreferences prefs = getPreferences(MODE_PRIVATE);
        SharedPreferences.Editor edit = prefs.edit();
        String name;
        for (int i = 0; (name = Lib.getSettingName(i)) != null; ++i) {
            int v = Lib.getSettingValue(i);
            edit.putInt(name, v);
        }
        edit.apply();
    }

    @Override protected void onResume() {
        super.onResume();
        mView.onResume();
        Lib.startAudio();
        Log.i(TAG, "onResume");
    }

    @Override protected void onPause() {
        super.onPause();
        mView.onPause();
        Lib.stopAudio();
        saveSettings();
        Log.i(TAG, "onPause");
    }

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
