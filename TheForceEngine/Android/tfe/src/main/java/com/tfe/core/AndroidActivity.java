package com.tfe.core;

import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;

import org.libsdl.app.SDLActivity;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;


public class AndroidActivity extends SDLActivity {
    private static final String TAG = "TFE";
    private List<String> assetList;
    protected static Context context;
    protected static AndroidActivity activity;

    private void createListAssetFiles(String dir) {
        String[] list;
        try {
            list = getAssets().list(dir);
        } catch (IOException e) {
            //assetList.add(dir);
            return;
        }

        if (list.length > 0) {
            for (String item : list) {
                String tested = dir.isEmpty() ?  dir + item : dir + "/" + item;
                createListAssetFiles(tested);
            }
        }
        else {
            assetList.add(dir);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        context = getApplicationContext();
        activity = this;

        if (assetList == null) {
            assetList = new ArrayList<String>();
        }

        if (assetList.isEmpty()) {
            long startTime = System.currentTimeMillis();
            createListAssetFiles("");
            long difference = System.currentTimeMillis() - startTime;
            Log.d(TAG, "creating asset list in java took " + difference + " ms");
//            for(String dir : assetList) {
//                Log.d("dir ", dir);
//            }
        }

        File externalDir = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOCUMENTS), "TheForceEngine");
        String externalPublicDir = externalDir.getAbsolutePath();
        AndroidActivity.onCreateActivity((Activity)this, getAssets(), assetList, externalPublicDir);
    }

    @Override
    protected void onDestroy() {
        AndroidActivity.onDestroyActivity();
        super.onDestroy();
        //super.nativeSendQuit();
    }

    /**
     * This method is called by SDL before starting the native application thread.
     * It can be overridden to provide the arguments after the application name.
     * The default implementation returns an empty array. It never returns null.
     * @return arguments for the native application.
     */
    public static Context GetApplicationContext() {
        return context;
    }

    public static native void onCreateActivity(Activity activity, AssetManager assetManager, List<String> assetList, String externalPublicDir);
    public static native void onDestroyActivity();

    public static int doGetVersionCode() {
        try {
            return context.getPackageManager().getPackageInfo(context.getPackageName(), 0).versionCode;
        } catch (PackageManager.NameNotFoundException e) {
            return 0;
        }
    }

    public static String doGetVersionName() {
        try {
            return context.getPackageManager().getPackageInfo(context.getPackageName(), 0).versionName;
        } catch (PackageManager.NameNotFoundException e) {
            return null;
        }
    }
}
