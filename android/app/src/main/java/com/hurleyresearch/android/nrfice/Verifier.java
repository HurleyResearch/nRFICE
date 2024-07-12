/*
MIT License

Copyright (c) 2024 Hurley Research LLC

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

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
