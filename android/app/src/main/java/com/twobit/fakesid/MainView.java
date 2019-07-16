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

    @Override
    public boolean onTouchEvent(final MotionEvent e) {
        switch (e.getAction()) {
        case MotionEvent.ACTION_DOWN:
        case MotionEvent.ACTION_UP:
        case MotionEvent.ACTION_MOVE:
            queueEvent(new Runnable() { public void run() {
                Lib.touch((int)e.getX(), (int)e.getY(), e.getAction());
            }});
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

