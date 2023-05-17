package com.hurleyresearch.android.nrfice.ui.rgb;

import com.hurleyresearch.android.nrfice.BLE;

public class RGBRunner implements Runnable {
    public void run() {
        try {
            int[] lastRGB = new int[3];
            while (true) {
                Thread.sleep(500);
                if (BLE.instance_ != null && BLE.isConnected) {
                    if (lastRGB[0] != RGBFragment.seekBarValues[0] || lastRGB[1] != RGBFragment.seekBarValues[1] || lastRGB[2] != RGBFragment.seekBarValues[2]) {

                        lastRGB[0] = RGBFragment.seekBarValues[0];
                        lastRGB[1] = RGBFragment.seekBarValues[1];
                        lastRGB[2] = RGBFragment.seekBarValues[2];

                        BLE.instance().setRGB(lastRGB);
                    }
                }

            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
