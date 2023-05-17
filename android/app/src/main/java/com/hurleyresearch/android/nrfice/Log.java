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
