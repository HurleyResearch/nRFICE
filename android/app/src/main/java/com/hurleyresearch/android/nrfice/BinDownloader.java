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

import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;

public class BinDownloader  implements Runnable {
    public boolean done = false;
    public byte[] fileBytes = null;
    public int sum = 0;
    public String folder;
    public BinDownloader(String folder){
        this.folder = folder;
    }
    public void run(){
        try {


            // String binurl = "https://oids.co/FPGAFile?fn=DevBoardDemo1/top_bitmap.bin";
         //   String binurl = "https://oids.co/FPGAFile?fn=BLEFPGA01/top_bitmap.bin";
         //   String binurl = "https://jayhurley.com/BLEFPGA01/top_bitmap.bin";
//            String binurl = "https://jayhurley.com/NRFICETesting/top_bitmap.bin";
       //     String binurl = "https://jayhurley.com/NRFICE01/top_bitmap.bin";
            String binurl = "https://jayhurley.com/" + folder + "/" + folder + "_impl_1.bin";

            Log.log("binurl: " + binurl);
            URL url = new URL(binurl);
            URLConnection conexion = url.openConnection();
            conexion.connect();

            int lenghtOfFile = conexion.getContentLength();
            //  Log.d("download", "Lenght of file: " + lenghtOfFile);

            InputStream input = new BufferedInputStream(url.openStream());
            ByteArrayOutputStream baos = new ByteArrayOutputStream();

            byte data[] = new byte[1024];


            int count= -1;

            while ((count = input.read(data)) != -1) {
                baos.write(data, 0, count);
            }

            baos.flush();
            baos.close();
            input.close();


            fileBytes = baos.toByteArray();
            sum = 0;
            for( int i = 0; i < fileBytes.length; i ++ ){
                sum += (int) (fileBytes[i] & 0xff);
            }





            Log.log("bin file length: " + fileBytes.length + "   and sum " + sum);
        } catch (Exception e) {
            Log.logException(e);
            // e.printStackTrace();
        }


        done = true;
    }
}
