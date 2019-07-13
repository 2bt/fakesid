package com.twobit.fakesid;

import android.content.res.AssetManager;

public class Lib {
    static {
        System.loadLibrary("main");
    }
    public static native void setAssetManager(AssetManager am);
    public static native void init();
    public static native void free();
    public static native void exit();
    public static native void resize(int width, int height);
    public static native void draw();
    public static native void touch(int x, int y, int action);
    public static native void key(int key, int unicode);

}
