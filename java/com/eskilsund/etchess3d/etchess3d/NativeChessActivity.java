package com.eskilsund.etchess3d.etchess3d;

import android.os.Bundle;
import android.util.Log;

import com.google.android.gms.games.Game;

/**
 * Created by Eskil on 06.11.2016.
 */

public class NativeChessActivity extends android.app.NativeActivity {
    static {
        System.loadLibrary("native-lib");
    }
    void finishMe()
    {
        finish();
    }
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        initNative();
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native void initNative();
    public native void whiteIsCpu();
    public native void blackIsCpu();
    public native void whiteIsPlayer();
    public native void blackIsPlayer();
    public native void whiteIsExternalPlayer();
    public native void blackIsExternalPlayer();
    public native void setSearchDepth(int depth);
    public native void setNativeGameActivity(GameActivity ga);
    public native void nativePerformMove(int from_x, int from_y, int to_x, int to_y);
}
