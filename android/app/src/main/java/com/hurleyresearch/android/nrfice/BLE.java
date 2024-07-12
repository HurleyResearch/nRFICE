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

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.Context;

import com.hurleyresearch.android.nrfice.ui.home.HomeFragment;

import java.util.Arrays;
import java.util.List;

public class BLE implements Runnable {
    public static String DEVICE_NAME = "NRFICE";
    public static int COMMAND_READ_FLASH_BT = 110;
    public static int COMMAND_WRITE_FLASH = 112;
    public static int COMMAND_READ_FLASH_STATUS = 111;
    public static int COMMAND_ERASE_FLASH = 113;
    public static int COMMAND_TEST_FLASH = 114;


    public static int COMMAND_HARDRESETICE = 115;
    public static int COMMAND_SOFTRESTEICE = 116;
    public static int COMMAND_CRESETBLOW = 117;
    public static int COMMAND_CRESETBHIGH = 118;
    public static int COMMAND_TESTREADFLASH = 119;
    public static int COMMAND_TESTWRITEFLASH = 127;
    public static int COMMAND_ERASEZERO = 131;
    public static int COMMAND_TESTBTSEND = 132;


    public static int COMMAND_READ_FLASH_SERIAL = 120;
    public static int COMMAND_RAMTEST = 121;
    public static int COMMAND_RAMTEST_LONG = 122;
    public static int COMMAND_SETRGB = 210;


    public static int WRITESTATE_NOTWRITING = 0;
    public static int WRITESTATE_PREWRITE = 1;
    public static int WRITESTATE_DONEVERIFIED = 2;
    public static int WRITESTATE_DONEERROR = 3;
    public static int writeState = WRITESTATE_NOTWRITING;
    public byte[] writeCommand = null;

    public static int[] zeroes = {0, 0, 0, 0, 0, 0, 0, 0};

    public static String bleFPGAServiceUUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
    //    public static String ringoCharacteristicUUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
    public static String bleFPGARXCharacteristicUUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
    public static String bleFPGATXCharacteristicUUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";
    public static BLE instance_ = null;
    public BluetoothAdapter mBluetoothAdapter;
    public BluetoothDevice mBluetoothDevice;
    public BluetoothGatt mBluetoothGatt;
    public BluetoothLeScanner mBluetoothLeScanner;
    public static BluetoothGattService bleFPGAService;
    public static BluetoothGattCharacteristic bleFPGARXCharacteristic;
    public static BluetoothGattCharacteristic bleFPGATXCharacteristic;
    public static BluetoothGattDescriptor bleFPGADescriptor;

    public static Thread thread;
    public HomeFragment homeFragment = null;
    public MainActivity mainActivity = null;

    public static boolean isConnected = false;

    public static boolean doneReadingFlash = false;

    public static synchronized BLE instance() {

        if (instance_ == null) {
            Log.log("ble instance null");

        }
        return instance_;
    }

    public static synchronized BLE instance(MainActivity ma) {
        ma.setBluetoothIndicator(false);
        isConnected = false;
        if (instance_ == null) {
            instance_ = new BLE(ma);
            thread = new Thread(instance_);
            thread.start();
        }
        return instance_;
    }

    public BLE(MainActivity ma) {
        this.mainActivity = ma;
        this.homeFragment = ma.homeFragment;
    }

    public synchronized void restart() {

        mBluetoothGatt.disconnect();
    }


    public void setRGB(int[] rgbValues) {
        if (isConnected) {
            Log.log("setting rgb to " + Arrays.toString(rgbValues));
            byte[] value = new byte[20];
            value[0] = (byte) (COMMAND_SETRGB & 0xff);
            for (int i = 0; i < 3; i++) {
                value[i + 1] = (byte) rgbValues[i];
            }

            bleFPGARXCharacteristic.setValue(value);
            boolean rv = this.mBluetoothGatt.writeCharacteristic(bleFPGARXCharacteristic);
            if (!rv) {
                Log.log("BLUETOOTH: false back from writeCharacteristic in setRGB: " + rv);
            }
        } else {
            Log.log("not setting rgb cuz not connected");
        }

    }

    public boolean backpackIsCharged() {
        return true;
    }

    public void readFlashStatus() {
        try {
// we want to download the firmware right?

            byte[] value = new byte[20];
            value[0] = (byte) (COMMAND_READ_FLASH_STATUS & 0xff);// 96 is erase app area

            bleFPGARXCharacteristic.setValue(value);
            boolean rv = mBluetoothGatt.writeCharacteristic(bleFPGARXCharacteristic);
            Log.log("back from writeCharacteristic[rstEraseAppFlash]: " + rv);

            // return "{\"name\":\"succeeded to stopMotors: " + rv + "\",\"age\":22}";


        } catch (Exception e) {
            Log.logException(e);
        }
    }

    public void eraseFlash(int startAddress, int len) {

        try {
            for (int loc = startAddress; loc < startAddress + len; loc += 0x10000) {
                byte[] cmd = new byte[20];
                //(byte) (COMMAND_READ_FLASH_STATUS & 0xff);
                cmd[0] = (byte) (COMMAND_ERASE_FLASH & 0xff);
                pack24(cmd, 1, loc);


                bleFPGARXCharacteristic.setValue(cmd);
                Log.log("before erase sector at addr: " + Integer.toHexString(loc));
                boolean rv = mBluetoothGatt.writeCharacteristic(bleFPGARXCharacteristic);
                Log.log("after erase sector at addr: " + Integer.toHexString(loc) + "  and rv: " + rv);
                Thread.sleep(500);
                int retries = 0;
                while (!rv) {
                    retries++;
                    if (retries > 10) {
                        Log.log("aborting erasing bin file area at loc: " + loc);
                        return;
                    }
                    Log.log("waiting while erasiing bin area: " + retries + "   at loc: " + loc);
                    Thread.sleep(500);
                    rv = mBluetoothGatt.writeCharacteristic(bleFPGARXCharacteristic);

                }
            }
        } catch (Exception e) {
            Log.logException(e);
        }


    }

    public void readFlash(int sessionid, int startAddress, int length) {
        try {
// we want to download the firmware right?
// whats the deal? its different from bt. we may as well read the whole thing at once
            doneReadingFlash = false;
            byte[] value = new byte[20];
            value[0] = (byte) (COMMAND_READ_FLASH_BT & 0xff);
            pack32(value, 4, startAddress);
            pack32(value, 8, length);
            pack32(value, 12, sessionid);
            bleFPGARXCharacteristic.setValue(value);
            boolean rv = mBluetoothGatt.writeCharacteristic(bleFPGARXCharacteristic);
            Log.log("back from writeCharacteristic[readFlash]: " + rv);

            // return "{\"name\":\"succeeded to stopMotors: " + rv + "\",\"age\":22}";


        } catch (Exception e) {
            Log.logException(e);
        }
    }

    public void run() {
        try {
            Thread.sleep(2000);
            final BluetoothManager bluetoothManager =
                    (BluetoothManager) MainActivity.instance().getSystemService(Context.BLUETOOTH_SERVICE);
            mBluetoothAdapter = bluetoothManager.getAdapter();
            mBluetoothLeScanner = mBluetoothAdapter.getBluetoothLeScanner();
            mBluetoothLeScanner.startScan(scanCallback);
            Log.log("BLUETOOTH: got ble started scanning");
        } catch (Exception e) {
            Log.logException(e);

        }
    }


    private final BluetoothGattCallback mGattCallback = new BluetoothGattCallback() {
        public BluetoothGatt mBluetoothGattInCallback;

        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
//            String intentAction;
            Log.log("BLUETOOTH: mGattCallback, onConnectionStateChange, newState: " + newState);
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                mBluetoothGatt.discoverServices();

//                MainActivity.BLEINIT = true;
//                MainActivity.instance().refreshToolbar();
                Log.log("BLUETOOTH: connectionStateChange connected");

            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
//                MainActivity.BLEINIT = false;
                mBluetoothLeScanner = mBluetoothAdapter.getBluetoothLeScanner();
                mBluetoothLeScanner.startScan(scanCallback);
                Log.log("BLUETOOTH: connectionStateChange disconnected");
                mainActivity.setBluetoothIndicator(false);
                isConnected = false;
//                MainActivity.instance().refreshToolbar();
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            Log.log("BLUETOOTH: mGattCallback, onServicesDiscovered, status: " + status);
            if (status == BluetoothGatt.GATT_SUCCESS) {

                ////////////////////// original
//                this.mBluetoothGattInCallback = gatt;
//                Log.log("services discovered, now we can write the char directly");
//
//                BluetoothGattService rxService = BLE.instance().mBluetoothGatt.getService(UUID.fromString("6e400001-b5a3-f393-e0a9-e50e24dcca9e"));
//                BLE.ringoCharacteristic = rxService.getCharacteristic(UUID.fromString("6e400002-b5a3-f393-e0a9-e50e24dcca9e"));
                //////////////////////// done original


                this.mBluetoothGattInCallback = gatt;
                List<BluetoothGattService> services = gatt.getServices();
                //  BluetoothGattService rstService = null;
                //  BluetoothGattCharacteristic rstCharacteristic = null;
                for (BluetoothGattService service : services) {
                    Log.log("BLUETOOTH: serivce uuid: " + service.getUuid());
                    if (service.getUuid().toString().equals(bleFPGAServiceUUID)) {
                        bleFPGAService = service;
                        Log.log("BLUETOOTH: found rst service");
                    }
                }
                List<BluetoothGattCharacteristic> chars = bleFPGAService.getCharacteristics();
                for (BluetoothGattCharacteristic ch : chars) {
                    Log.log("BLUETOOTH: characteristic uuid: " + ch.getUuid().toString());
                    if (ch.getUuid().toString().equals(bleFPGARXCharacteristicUUID)) {
                        Log.log("BLUETOOTH: found rst characteristic");
                        bleFPGARXCharacteristic = ch;
                    } else if (ch.getUuid().toString().equals(bleFPGATXCharacteristicUUID)) {
                        Log.log("BLUETOOTH: found rst characteristic");
                        bleFPGATXCharacteristic = ch;
                    }
                }
                List<BluetoothGattDescriptor> descriptos = bleFPGATXCharacteristic.getDescriptors();
                for (BluetoothGattDescriptor descriptor : descriptos) {
                    Log.log("BLUETOOTH: descriptor: " + descriptor.getUuid().toString());
                    if (descriptor.getUuid().toString().equals("00002902-0000-1000-8000-00805f9b34fb")) {
                        Log.log("BLUETOOTH: found notification descriptor");
                        boolean backFromSetCharNoti = mBluetoothGatt.setCharacteristicNotification(bleFPGATXCharacteristic, true);

                        Log.log("BLUETOOTH: backFromCharNoti: " + backFromSetCharNoti);
                        bleFPGADescriptor = descriptor;
//                        descriptor.setValue(BluetoothGattDescriptor.ENABLE_INDICATION_VALUE);


                        descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                    //jkh who knowsmaybethisisfuckingup    descriptor.setValue(BluetoothGattDescriptor.ENABLE_INDICATION_VALUE);


                        boolean backFromWriteDescriptor = gatt.writeDescriptor(descriptor);
                        Log.log("BLUETOOTH: should be done setting notifications for our characteristic, backFromWriteDescriptor:  " + backFromWriteDescriptor);
                        mainActivity.setBluetoothIndicator(true);
                        isConnected = true;
                    }
                }


                // BluetoothGattService rxService = BLE.instance().mBluetoothGatt.getService(UUID.fromString("6e400001-b5a3-f393-e0a9-e50e24dcca9e"));
                // BLE.currentRXChar = rxService.getCharacteristic(UUID.fromString("6e400002-b5a3-f393-e0a9-e50e24dcca9e"));
                //  testSendBLEMessage(gatt);
            } else {
                Log.log("BLUETOOTH: onServicesDiscovered no successful, status: " + status);
            }
        }


        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            //   Log.log("BLUETOOTH: mGattCallback, onCharacteristicWrite, status: " + status + ", char: " + characteristic.getUuid().toString());
            if (writeState == WRITESTATE_NOTWRITING) {
                Log.log("onCharacteristicWrite with writeState == WRITESTATE_NOTWRITING, status: " + status);
                return;
            }
            byte[] val = characteristic.getValue();
            boolean verified = true;
            for (int i = 0; i < val.length; i++) {
                if (val[i] != writeCommand[i]) {
                    verified = false;
                }
            }
            if (verified) {
                writeState = WRITESTATE_DONEVERIFIED;
            } else {
                writeState = WRITESTATE_DONEERROR;
            }


        }


        @Override
        public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            Log.log("BLUETOOTH: mGattCallback, onCharacteristicRead, status: " + status + ", char: " + characteristic.getUuid().toString());

            byte[] val = characteristic.getValue();
            int addr = unpack32(val, 0);
            // Log.log("BLUETOOTH: onCharacteristicRead, valLength: " + val.length);
            for (int i = 4; i < val.length; i++) {
                Log.log("BLUETOOTH: onCharacteristicRead val[" + i + "]: " + val[i] + " ");
            }
//            MainActivity.instance().capVal = ((double) val[0]) / 10;
//            MainActivity.instance().capUpdate = true;


        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            Log.log("BLUETOOTH********: mGattCallback, onCharacteristicChanged, characteristic: " + characteristic.toString());
//jkh never gets called wtf
            byte[] val = characteristic.getValue();
            if (val[0] == COMMAND_READ_FLASH_BT) {
                // this is a read flash response
                int addr = unpack24(val, 1);

                StringBuilder sb = new StringBuilder();
                sb.append("onCharChange COMMAND_READ_FLASH_BT addr: " + Integer.toHexString(addr) + "  ");
                for (int i = 0; i < val.length - 4; i++) {
                    byte v = val[i + 4];
                    int loc = addr + i;
                    if (loc < BLE.instance().homeFragment.flashReadBytes.length) {
                        BLE.instance().homeFragment.flashReadBytes[addr + i] = v;
                    }

                    sb.append(Integer.toHexString((v & 0xff)) + " ");
                }
                if (addr + 20 >= BLE.instance().homeFragment.flashReadBytes.length) {
                    doneReadingFlash = true;
                }
                //  sb.append("\n");
                if (addr % 0x1000 == 0) {
                    Log.log(sb.toString());
                }

            } else {
                Log.log("invalid command in onCharacteristicChanged: " + val[0]);
            }


            // broadcastUpdate(ACTION_DATA_AVAILABLE, characteristic);
        }
    };
public static int nuscount = 0;

    private ScanCallback scanCallback =
            new ScanCallback() {
                public void onBatchScanResults(List<ScanResult> results) {
                    Log.log("BLUETOOTH: onBatchScanResults: " + results.size());
                    for (int i = 0; i < results.size(); i++) {
                        Log.log("BLUETOOTH: batch result " + i + ": " + results.get(i));
                    }
                }

                public void onScanFailed(int errorCode) {
                    Log.log("BLUETOOTH: onScanFailed, errorCode: " + errorCode);
                }

                public void onScanResult(int callbackType, ScanResult result) {
                   // Log.log("BLUETOOTH: onScanResult, callbackType: " + callbackType + ", result: " + result);
                    BluetoothDevice device = result.getDevice();
                    if( device.getName() != null ){
                        if( device.getName().contains("Nordic")){
//Log.log("NUS: " + nuscount);
nuscount ++;
                        }
                    }
                  //  Log.log("BLUETOOTH: Device Name: " + device.getName() + "\t Address: " + device.getAddress());
//                    if (device.getName() != null && device.getName().equals("RBP001") && MainActivity.instance().androidId.equals(MainActivity.instance().androidId)) {
//                    if (device.getName() != null && device.getName().equals("RBP001")) { //jdp
//                    if (device.getName() != null && device.getName().equals("RBP002")) {
//                    if (device.getName() != null && device.getName().equals("RBP003")) {
                    String deviceName = device.getName();
                    if( deviceName != null ){
                        Log.log("devicename: " + deviceName);
                    }
                    if (deviceName != null && deviceName.equals(DEVICE_NAME)) {

                        Log.log("BLUETOOTH: onLeScan, FOUND DEVICE_NAME: " + device.getName() + ", rssi: " + result.getRssi());
                        mBluetoothLeScanner.stopScan(scanCallback);
                        //   mBluetoothAdapter.stopLeScan(mLeScanCallback);
//                        if (mBluetoothDevice == null) { // jdp
                        mBluetoothDevice = device;
                        Log.log("BLUETOOTH: set mBluetoothDevice");
                        // now connect.
                        mBluetoothGatt = mBluetoothDevice.connectGatt(MainActivity.instance(), false, mGattCallback);
//                        mBluetoothGatt.discoverServices(); // doesn't work
//                            mBluetoothGatt.connect(); // doesn't work
//                        }


//                        } else if (device.getName() != null && device.getName().equals("RBP002") && MainActivity.instance().androidId.equals("7b7774ada6cb979a")) {
                    }
//                        else if (device.getName() != null && device.getName().equals("RBP002")) {
//                        Log.log("onLeScan, FOUND BACKPACK: " + device.getName() + ", rssi: " + result.getRssi());
//                        mBluetoothLeScanner.stopScan(scanCallback);
//                        //   mBluetoothAdapter.stopLeScan(mLeScanCallback);
//                        if (mBluetoothDevice == null) {
//                            mBluetoothDevice = device;
//                            Log.log("set mBluetoothDevice");
//                            // now connect.
//                            mBluetoothGatt = mBluetoothDevice.connectGatt(MainActivity.instance(), false, mGattCallback);
//                        }
//
//                    } else {
//
//                    }
                }

            };

    public static void pack16(byte[] msg, int loc, int val) {
        msg[loc] = (byte) (val & 0xff);
        msg[loc + 1] = (byte) ((val >> 8) & 0xff);

        // remember this probably does not work for signed values that are negative ints at first.
        // listen to the radness, then fix it. and name this routine appropriately.
    }

    public static void pack24(byte[] msg, int loc, int val) {
        msg[loc + 0] = (byte) (val & 0xff);
        msg[loc + 1] = (byte) ((val >> 8) & 0xff);
        msg[loc + 2] = (byte) ((val >> 16) & 0xff);
        //  msg[loc + 3] = (byte) ((val >> 24) & 0xff);

        // remember this probably does not work for signed values that are negative ints at first.
        // listen to the radness, then fix it. and name this routine appropriately.
    }

    public static void pack32(byte[] msg, int loc, int val) {
        msg[loc] = (byte) (val & 0xff);
        msg[loc + 1] = (byte) ((val >> 8) & 0xff);
        msg[loc + 2] = (byte) ((val >> 16) & 0xff);
        msg[loc + 3] = (byte) ((val >> 24) & 0xff);

        // remember this probably does not work for signed values that are negative ints at first.
        // listen to the radness, then fix it. and name this routine appropriately.
    }

    public static void pack16(int[] msg, int loc, int val) {
        msg[loc] = val & 0xff;
        msg[loc + 1] = (val >> 8) & 0xff;

        // remember this probably does not work for signed values that are negative ints at first.
        // listen to the radness, then fix it. and name this routine appropriately.
    }

    public static void pack32(int[] msg, int loc, int val) {
        msg[loc] = val & 0xff;
        msg[loc + 1] = (val >> 8) & 0xff;
        msg[loc + 2] = (val >> 16) & 0xff;
        msg[loc + 3] = (val >> 24) & 0xff;

        // remember this probably does not work for signed values that are negative ints at first.
        // listen to the radness, then fix it. and name this routine appropriately.
    }


    public static int unpack32(int[] src, int index) {

        int out = ((src[index + 3] << 24) & 0xff000000) + ((src[index + 2] << 16) & 0x00ff0000) + ((src[index + 1] << 8) & 0x0000ff00) + ((src[index + 0] << 0) & 0x000000ff);
        return out;

    }

    public static int unpack32(byte[] src, int index) {

        int out = (((src[index + 3] & 0xff) << 24) & 0xff000000) + (((src[index + 2] & 0xff) << 16) & 0x00ff0000) + (((src[index + 1] & 0xff) << 8) & 0x0000ff00) + (((src[index + 0] & 0xff) << 0) & 0x000000ff);
        return out;

    }

    public static int unpack24(byte[] src, int index) {

        int out = (((src[index + 2] & 0xff) << 16) & 0x00ff0000) + (((src[index + 1] & 0xff) << 8) & 0x0000ff00) + (((src[index + 0] & 0xff) << 0) & 0x000000ff);
        return out;

    }


    public void testFlash() {
        try {

            byte[] cmd = new byte[20];
            //(byte) (COMMAND_READ_FLASH_STATUS & 0xff);
            cmd[0] = (byte) (COMMAND_TEST_FLASH & 0xff);


            bleFPGARXCharacteristic.setValue(cmd);
            boolean rv = mBluetoothGatt.writeCharacteristic(bleFPGARXCharacteristic);


        } catch (Exception e) {
            Log.logException(e);
        }

    }


    public void ramTest(int ramData) {
        try {

            byte[] cmd = new byte[20];
            //(byte) (COMMAND_READ_FLASH_STATUS & 0xff);
            cmd[0] = (byte) (BLE.COMMAND_RAMTEST & 0xff);
            cmd[1] = 2; // fixed address
            pack16(cmd, 2, ramData);

            bleFPGARXCharacteristic.setValue(cmd);
            boolean rv = mBluetoothGatt.writeCharacteristic(bleFPGARXCharacteristic);


        } catch (Exception e) {
            Log.logException(e);
        }

    }


    public void oneByteCommand(int commandByte) {
        try {

            byte[] cmd = new byte[20];
            //(byte) (COMMAND_READ_FLASH_STATUS & 0xff);
            cmd[0] = (byte) (commandByte & 0xff);


            bleFPGARXCharacteristic.setValue(cmd);
            boolean rv = mBluetoothGatt.writeCharacteristic(bleFPGARXCharacteristic);


        } catch (Exception e) {
            Log.logException(e);
        }

    }


//    public static void log(String line){
//        MainActivity.instance().log(line);
//    }

    public int writeBinLoc = 0;

    public void writeBinFile(byte[] fileBytes) {
        try {
            Log.log("top of writebinfile");
            long start = System.currentTimeMillis();
            //   int wd = MainActivity.instance().getWriteDelay();
            for (int loc = 0; loc < fileBytes.length; loc += 16) {
                writeState = WRITESTATE_PREWRITE;

                writeCommand = new byte[20];
                //(byte) (COMMAND_READ_FLASH_STATUS & 0xff);
                writeCommand[0] = (byte) (COMMAND_WRITE_FLASH & 0xff);
                pack24(writeCommand, 1, loc);
                for (int i = 0; i < 16; i++) {
                    if (loc + i < fileBytes.length) {
                        writeCommand[i + 4] = fileBytes[loc + i];
                    }

                }
                if (loc % 0x1000 == 0) {
                    Log.log("writing flash at " + Integer.toHexString(loc));
                }
                bleFPGARXCharacteristic.setValue(writeCommand);


                boolean rv = mBluetoothGatt.writeCharacteristic(bleFPGARXCharacteristic);
                int waitCount = 0;
                while (writeState == WRITESTATE_PREWRITE) {
                    Thread.sleep(1);
                    waitCount++;
                    if (waitCount > 1000) {
                        Log.log("abort write sequence with waitcount: " + waitCount + " at addr: " + Integer.toHexString(loc));
                        return;
                    }
                }
                if (writeState == WRITESTATE_DONEVERIFIED) {

                    if (loc % 0x1000 == 0) {
                        Log.log("done writing flash at " + Integer.toHexString(loc) + " with rv: " + rv);
                    }
                } else if (writeState == WRITESTATE_DONEERROR) {
                    Log.log("abort write sequence with writeState: WRITESTATE_DONEERROR  at addr: " + Integer.toHexString(loc));
                    return;
                }


            }
            long end = System.currentTimeMillis();
            long elapsed = end - start;
            Log.log("write bin took " + (elapsed / 1000) + " seconds");
            writeState = WRITESTATE_NOTWRITING;
        } catch (Exception e) {
            Log.logException(e);
        }

    }




    /*public void writeBinFile(byte[] fileBytes) {
        try {
            log("top of writebinfile");
            long start = System.currentTimeMillis();
            int wd = MainActivity.instance().getWriteDelay();
            for (int loc = 0; loc < fileBytes.length; loc += 16) {
                byte[] cmd = new byte[20];
                //(byte) (COMMAND_READ_FLASH_STATUS & 0xff);
                cmd[0] = (byte) (COMMAND_WRITE_FLASH & 0xff);
                pack24(cmd, 1, loc);
                for (int i = 0; i < 16; i++) {
                    if( loc + i < fileBytes.length){
                        cmd[i + 4] = fileBytes[loc + i];
                    }

                }
                if( loc % 0x1000 == 0 ){
                    Log.log("writing flash at " + Integer.toHexString(loc));
                }
                bleFPGARXCharacteristic.setValue(cmd);
                boolean rv = mBluetoothGatt.writeCharacteristic(bleFPGARXCharacteristic);

                if( loc % 0x100 == 0 ){
                    Log.log("done writing flash at " + Integer.toHexString(loc) + " with rv: " + rv);
                }
                Thread.sleep(wd);


                if( !rv ){
                    Thread.sleep(wd);
                    rv = mBluetoothGatt.writeCharacteristic(bleFPGARXCharacteristic);
                }


                int retries = 0;
                while( !rv ){
                    retries ++;
                    if( retries > 10 ){
                        Log.log("aborting writing bin file at loc: " + loc);
                        return;
                    }
                    wd = wd * 2;
                    if( wd >= 100 ){
                        wd = 100;
                    }
                    Log.log("*******  waiting while writing bin: " + retries + "   at loc: " + Integer.toHexString(loc) + " new wd: " + wd);

                    Thread.sleep(150);
                    rv = mBluetoothGatt.writeCharacteristic(bleFPGARXCharacteristic);

                }
            }
            long end = System.currentTimeMillis();
            long elapsed = end - start;
            log("write bin took " + (elapsed / 1000) + " seconds");
        } catch(Exception e){
            Log.logException(e);
        }

    }*/


}
