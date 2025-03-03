package com.tfe.game;

import com.tfe.core.AndroidActivity;


public class TfeActivity extends AndroidActivity {
    private static final String TAG = "TFE";

    /**
     * This method returns the name of the application entry point
     * It can be overridden by derived classes.
     */
    @Override
    protected String getMainFunction() {
        return "main";
    }

    /**
     * This method is called by SDL before loading the native shared libraries.
     * It can be overridden to provide names of shared libraries to be loaded.
     * The default implementation returns the defaults. It never returns null.
     * An array returned by a new implementation must at least contain "SDL2".
     * Also keep in mind that the order the libraries are loaded may matter.
     * @return names of shared libraries to be loaded (e.g. "SDL2", "main").
     */
    @Override
    protected String[] getLibraries() {
//        if (BuildConfig.IS_VR) {
//            return new String[]{"TheForceEngineVR"};
//        }
//        else {
            return new String[]{"TheForceEngine"};
//        }
    }
}
