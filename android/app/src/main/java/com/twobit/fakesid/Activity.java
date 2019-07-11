package com.twobit.fakesid;

import android.content.res.AssetManager;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;

public class Activity extends android.app.Activity {

    @Override protected void onCreate(Bundle b) {
        super.onCreate(b);
        mView = new View(getApplication());
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

    View mView;
}
