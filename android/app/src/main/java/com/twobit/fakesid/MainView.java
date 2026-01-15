package com.twobit.fakesid;

import android.content.Context;
import android.hardware.display.DisplayManager;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.util.Log;
import android.view.Display;
import android.view.MotionEvent;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

class MainView extends GLSurfaceView {
    static String TAG = "FOOBAR";

    public MainView(Context context) {
        super(context);
        Log.i(TAG, "View constructor");

        float refreshRate;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O_MR1) {
            DisplayManager displayManager = (DisplayManager) context.getSystemService(Context.DISPLAY_SERVICE);
            Display display = displayManager.getDisplay(Display.DEFAULT_DISPLAY);
            refreshRate = display.getRefreshRate();
        }
        else {
            refreshRate = 60.0f;
        }

        String storageDir = context.getExternalFilesDir(null).getAbsolutePath();

        setPreserveEGLContextOnPause(true);
        setEGLContextClientVersion(2);
        setEGLConfigChooser(8, 8, 8, 8, 0, 0);
        setRenderer(new GLSurfaceView.Renderer() {
            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                Lib.init(getResources().getAssets(), storageDir, refreshRate);
            }
            @Override
            public void onSurfaceChanged(GL10 gl, int width, int height) {
                Lib.resize(width, height);
            }
            @Override
            public void onDrawFrame(GL10 gl) {
                Lib.draw();
            }
        });
    }

    int     mTouchId;
    boolean mTouchPressed;

    @Override
    public boolean onTouchEvent(MotionEvent e) {
        int index  = e.getActionIndex();
        int id     = e.getPointerId(index);
        int action = -1;

        switch (e.getActionMasked()) {
        case MotionEvent.ACTION_DOWN:
        case MotionEvent.ACTION_POINTER_DOWN:
            mTouchId = id;
            if (mTouchPressed) action = MotionEvent.ACTION_MOVE;
            else {
                action = MotionEvent.ACTION_DOWN;
                mTouchPressed = true;
            }
            break;
        case MotionEvent.ACTION_UP:
        case MotionEvent.ACTION_POINTER_UP:
            if (mTouchPressed && id == mTouchId) {
                action = MotionEvent.ACTION_UP;
                mTouchPressed = false;
            }
            break;
        case MotionEvent.ACTION_MOVE:
            if (!mTouchPressed) break;
            int count = e.getPointerCount();
            for (int p = 0; p < count; ++p) {
                if (e.getPointerId(p) == mTouchId) {
                    index = p;
                    action = MotionEvent.ACTION_MOVE;
                    break;
                }
            }
            break;
        }

        if (action >= 0) {
            final int a = action;
            final int x = (int) e.getX(index);
            final int y = (int) e.getY(index);
            queueEvent(new Runnable() {
                public void run() {
                    Lib.touch(x, y, a);
                }
            });
            return true;
        }

        return false;
    }
}

