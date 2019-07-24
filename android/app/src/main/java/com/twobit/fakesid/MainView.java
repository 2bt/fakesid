package com.twobit.fakesid;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.MotionEvent;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

class MainView extends GLSurfaceView {
    static String TAG = "FOOBAR";
    Renderer mRenderer;

    public MainView(Context context) {
        super(context);
        mRenderer = new Renderer();
        setPreserveEGLContextOnPause(true);
        setEGLContextClientVersion(2);
        setEGLConfigChooser(8, 8, 8, 8, 0, 0);
        setRenderer(mRenderer);
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

    class Renderer implements GLSurfaceView.Renderer {
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            Lib.init(getResources().getAssets());
        }
        public void onSurfaceChanged(GL10 gl, int width, int height) {
            Lib.resize(width, height);
        }
        public void onDrawFrame(GL10 gl) {
            Lib.draw();
        }
    }
}

