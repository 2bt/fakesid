package com.twobit.fakesid;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Window;
import android.view.inputmethod.InputMethodManager;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.core.view.WindowInsetsControllerCompat;
import androidx.core.view.WindowCompat;

import java.io.File;

public class MainActivity extends Activity {
    static String TAG = "FOOBAR";
    static MainActivity             sInstance;
    static SharedPreferences        sPrefs;
    static SharedPreferences.Editor sPrefEdit;
    MainView mView;

    private static final int SETTING_FULLSCREEN_ENABLED = 0;
    private static final int REQUEST_CODE_IMPORT_FILE = 1;


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

        // Check if app was opened from another app (WhatsApp, etc.)
        handleIntent(getIntent());
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        handleIntent(intent);
    }

    private void handleIntent(Intent intent) {
        String action = intent.getAction();
        if (action == null) return;

        if (Intent.ACTION_VIEW.equals(action) || Intent.ACTION_SEND.equals(action)) {
            Uri uri = intent.getData();
            if (uri == null) {
                // Handle SEND intent with clip data
                uri = intent.getParcelableExtra(Intent.EXTRA_STREAM);
            }
            if (uri != null) {
                String fileName = FileUtils.getFileName(this, uri);
                if (fileName == null) fileName = "song.sng";
                String path = new File(getCacheDir(), fileName).getAbsolutePath();
                FileUtils.copyUriToFile(this, uri, path);
                Lib.importSong(path);
                Log.i(TAG, "Imported file from intent: " + fileName);
            }
        }
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

    // song import
    // called from C++
    static public void startSongImport() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("application/octet-stream");
        sInstance.startActivityForResult(intent, REQUEST_CODE_IMPORT_FILE);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, android.content.Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        Log.i(TAG, "onActivityResult " + requestCode + " " + resultCode);
        if (resultCode != Activity.RESULT_OK || data == null) return;
        Uri fileUri = data.getData();
        if (requestCode == REQUEST_CODE_IMPORT_FILE) {
            String fileName = FileUtils.getFileName(this, fileUri);
            if (fileName == null) fileName = "song.sng";
            String path = new File(getCacheDir(), fileName).getAbsolutePath();
            FileUtils.copyUriToFile(this, fileUri, path);
            Lib.importSong(path);
        }
    }

    // song export
    // called from C++
    static public void exportSong(String path, String title) {
        File file = new File(path);
        Uri uri = androidx.core.content.FileProvider.getUriForFile(
                sInstance,
                sInstance.getPackageName() + ".fileprovider",
                file);
        Intent share = new Intent(Intent.ACTION_SEND);
        share.setType(getMimeFromName(file.getName()));
        share.putExtra(Intent.EXTRA_STREAM, uri);
        share.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        share.putExtra(Intent.EXTRA_SUBJECT, title);
        share.putExtra(Intent.EXTRA_TEXT, title);
        sInstance.startActivity(Intent.createChooser(share, "Export song"));
    }

    private static String getMimeFromName(String name) {
        if (name.endsWith(".sng")) return "application/octet-stream";
        if (name.endsWith(".ogg")) return "audio/ogg";
        if (name.endsWith(".wav")) return "audio/wav";
        return "application/octet-stream";
    }

}
