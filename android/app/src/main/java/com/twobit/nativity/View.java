package com.twobit.nativity;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;


class View extends GLSurfaceView {
    private static String TAG = "libgl2";

    Renderer mRenderer = new Renderer();

    public View(Context context) {
        super(context);
        setPreserveEGLContextOnPause(true);
        setEGLContextClientVersion(2);
        setEGLConfigChooser(8, 8, 8, 8, 0, 0);
        setRenderer(mRenderer);
    }
    public boolean onTouchEvent(final MotionEvent event) {
        queueEvent(new Runnable() { public void run() { Lib.touch((int)event.getX(), (int)event.getY()); }});
        return true;
    }

    public void onDestroy() {
        //queueEvent(new Runnable() { public void run() { Lib.exit(); }});
    }


    class Renderer implements GLSurfaceView.Renderer {
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            Lib.init();
        }
        public void onSurfaceChanged(GL10 gl, int width, int height) {
            Lib.resize(width, height);
        }
        public void onDrawFrame(GL10 gl) {
            Lib.draw();
        }
    }
}

