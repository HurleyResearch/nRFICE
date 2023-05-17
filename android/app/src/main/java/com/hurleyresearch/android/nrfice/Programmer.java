package com.hurleyresearch.android.nrfice;

import com.hurleyresearch.android.nrfice.ui.home.HomeFragment;

public class Programmer implements Runnable {
    public boolean done = false;
    public HomeFragment homeFragment;
    public Programmer(HomeFragment homeFragment){
        this.homeFragment = homeFragment;
    }
    public void run(){
        try {
            done = false;
            int sum = homeFragment.downloadBin();
            homeFragment.setBinSum(sum);
            MainActivity.instance().ble.eraseFlash(0,homeFragment.binFile.length);
            MainActivity.instance().ble.writeBinFile(homeFragment.binFile);
            BLE.instance().oneByteCommand(BLE.COMMAND_HARDRESETICE);

        } catch (Exception e) {
//            e.printStackTrace();
            Log.logException(e);
        }
        done = true;
    }
}
