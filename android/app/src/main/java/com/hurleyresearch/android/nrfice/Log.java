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

import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.StringTokenizer;

public class Log {
    public static void log(String line) {

        if (MainActivity.instance() != null) {

//            MainActivity.instance().runOnUiThread(new Runnable() {
//                public void run() {

                    System.out.println("NRFICE: " + line);
                    if (MainActivity.instance().homeFragment != null) {
                        MainActivity.instance().homeFragment.appendStatus(line);
                    }

//                }
//            });

        } else {
            System.out.println("NRFICE:(mainactivity null): " + line);
        }


    }


    public static void logException(Exception e) {
        StringWriter sw = new StringWriter();
        PrintWriter pw = new PrintWriter(sw);
        e.printStackTrace(pw);
        String es = sw.toString();
        StringTokenizer st = new StringTokenizer(es, "\r\n");
        while (st.hasMoreTokens()) {
            log(st.nextToken());
        }
        log(sw.toString());
    }
}
