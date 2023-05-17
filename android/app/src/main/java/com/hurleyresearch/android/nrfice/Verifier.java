package com.hurleyresearch.android.nrfice;

import com.hurleyresearch.android.nrfice.ui.home.HomeFragment;

public class Verifier  implements Runnable {
    public static int readSessionID = 0;
    public boolean done = false;
    public HomeFragment homeFragment;
    public Verifier(HomeFragment homeFragment){
        this.homeFragment = homeFragment;
    }
    public void run(){
        try {
            done = false;
            BLE.doneReadingFlash = false;
            MainActivity.instance().ble.readFlash(readSessionID,0,homeFragment.binFile.length);
            while( !BLE.doneReadingFlash ){
                  Thread.sleep(1000);
                  Log.log("reading flash...");
            }

            readSessionID ++;


            int errorCount = 0;
            for (int i = 0; i < homeFragment.binFile.length; i++) {
                if (homeFragment.flashReadBytes[i] != homeFragment.binFile[i]) {
                    Log.log("flash mismatch at " + i + "  flash: " + homeFragment.flashReadBytes[i] + "   binFile: " + homeFragment.binFile[i]);
                    errorCount ++;
                }
            }
            if( errorCount > 0 ){
                Log.log("Flash Verify Failed, errorCount: " + errorCount);
            } else {
                Log.log("Flash Verify Success, errorCount: " + errorCount);
            }
        } catch (Exception e) {
//            e.printStackTrace();
            Log.logException(e);
        }
        done = true;
    }
}
