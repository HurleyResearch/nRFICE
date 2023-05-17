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
