package com.hurleyresearch.android.nrfice.ui.home;

import android.app.Activity;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.Spinner;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
//import androidx.lifecycle.ViewModelProvider;

import com.hurleyresearch.android.nrfice.BLE;
import com.hurleyresearch.android.nrfice.BinDownloader;
import com.hurleyresearch.android.nrfice.Log;
import com.hurleyresearch.android.nrfice.MainActivity;
import com.hurleyresearch.android.nrfice.Programmer;
import com.hurleyresearch.android.nrfice.R;
import com.hurleyresearch.android.nrfice.Verifier;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
//import com.hurleyresearch.android.nrfice.databinding.FragmentHomeBinding;

public class HomeFragment extends Fragment implements AdapterView.OnItemSelectedListener {

    public TextView logTextView = null;
    public EditText sumText = null;
    public EditText ramDataText = null;
    public Spinner projectSpinner = null;
    public  ArrayAdapter<String> spinnerAdapter;
    //    private FragmentHomeBinding binding;
    public List<String> fudges = null;
    public List<String> statusLines = new ArrayList<String>();



    public byte[] flashReadBytes;
    public byte[] binFile;


    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {


        super.onCreate(savedInstanceState);
        if (fudges == null) {
            fudges = new ArrayList<String>();
        }
        fudges.add("creatd again:  " + System.currentTimeMillis());
        Log.log("HomeFragment onCreateView");

        MainActivity.instance().homeFragment = this;

        View view = inflater.inflate(R.layout.fragment_home, container, false);
        this.logTextView = view.findViewById(R.id.logTextView);
        this.sumText = view.findViewById(R.id.sumText);
        this.ramDataText = view.findViewById(R.id.ramDataText);



        StringBuilder sb = new StringBuilder();
        for (String line : statusLines) {
            sb.append(line).append("\n");
        }
        this.logTextView.setText(sb.toString());


        String[] fuckyou = new String[]{"NRFICE04","NRFICE01","NRFICE02","NRFICETesting","VSMTest"};
        ArrayAdapter<String> adapter = new ArrayAdapter(  this.getActivity(),  android.R.layout.simple_spinner_item,fuckyou);
        this.projectSpinner = view.findViewById(R.id.spinner2);
        this.projectSpinner.setAdapter(adapter);



//        this.projectSpinner = view.findViewById(R.id.projectSpinner);
//        this.projectSpinner.setOnItemSelectedListener(this);


//        this.spinnerAdapter.add("RGB");
//        this.spinnerAdapter.add("WaveSynth");
//        this.spinnerAdapter.add("PWM");
        // activityInstance = (MainActivity) getActivity();
        //  CameraFragment.fragmentInstance = this;


//        this.labInfoTextView = view.findViewById(R.id.labInfoTextView);
//        labInfoTextView.setTextSize(18f);
//        labInfoTextView.setTextColor(getResources().getColor(R.color.white));


//        return view;


//        HomeViewModel homeViewModel =
//                new ViewModelProvider(this).get(HomeViewModel.class);

//        binding = FragmentHomeBinding.inflate(inflater, container, false);
//        View root = binding.getRoot();

//        final TextView textView = binding.textHome;
//        textView.setText("fudge this home!");
//        homeViewModel.getText().observe(getViewLifecycleOwner(), textView::setText);

        //ImageView iv = (ImageView) MainActivity.instance().findViewById(R.id.imageViewtest);
//        Activity a = view.getActivity();
//        ImageView iv = view.findViewById(R.id.imageViewtest);
//        iv.setColorFilter(getResources().getColor(R.color.red));


        return view;
    }

    public void appendStatus(String line) {
        this.statusLines.add(line);
        if (this.logTextView != null) {
            StringBuilder sb = new StringBuilder();
            if( statusLines.size() >= 3){
                sb.append(this.statusLines.get(statusLines.size() - 3)).append("\n");
            }
            if( statusLines.size() >= 2){
                sb.append(this.statusLines.get(statusLines.size() - 2)).append("\n");
            }

            sb.append(this.statusLines.get(statusLines.size() - 1)).append("\n");

            //  this.logTextView.setText(this.logTextView.getText() + "\n" + line);
            this.logTextView.setText(sb.toString());
        }
    }
public int downloadBin() throws Exception {
    BinDownloader downloader = new BinDownloader(this.projectSpinner.getSelectedItem().toString());
    (new Thread(downloader)).start();
    while (!downloader.done) {
        Thread.sleep(1000);
        Log.log("waiting for bin file downloader");
    }
    this.binFile = Arrays.copyOf(downloader.fileBytes, downloader.fileBytes.length);
    Log.log("done with fw download, this.binFile.length: " + this.binFile.length);
    return downloader.sum;
}
public void eraseFlash() throws Exception {

}
    public void   resetICEButton(View view){
        Log.log("resetICEButton in HomeFragment");
        try {
            BLE.instance().oneByteCommand(BLE.COMMAND_HARDRESETICE);
        } catch (Exception e) {
            e.printStackTrace();
        }

    }
    public void   softResetICEButton(View view){
        Log.log("softResetICEButton in HomeFragment");
        try {
            BLE.instance().oneByteCommand(BLE.COMMAND_SOFTRESTEICE);
        } catch (Exception e) {
            e.printStackTrace();
        }

    }
    public void   ramTestButton(View view){
        Log.log("ramTestButton in HomeFragment");
        try {
            BLE.instance().ramTest(Integer.parseInt(this.ramDataText.getText().toString()));

        } catch (Exception e) {
            e.printStackTrace();
        }

    }

    public void  longTestButton(View view){
        Log.log("ramTestButton in HomeFragment");
        try {
            BLE.instance().oneByteCommand(BLE.COMMAND_RAMTEST_LONG);
        } catch (Exception e) {
            e.printStackTrace();
        }

    }

    public void addProjectButton(View view){
        Log.log("addProjectButton in HomeFragment");
//        try {
//          int sum = downloadBin();
//
//        } catch (Exception e) {
//            e.printStackTrace();
//        }

    }
    public void setBinSum(int sum ){
        final EditText fu = this.sumText;
        MainActivity.instance().runOnUiThread(
                new Runnable(){
                    public void run(){
                        fu.setText("" + sum);
                    }});

    }
    public void programFPGAFlashButton(View view){
        Log.log("programFPGAFlashButton in HomeFragment");
        // fire manage connection, make sure ble knows if we are connected or not, or just write it.
        try {
            Programmer programmer = new Programmer(this);
            (new Thread(programmer)).start();
        // Thread.sleep(1000);
//         while( !programmer.done){
//             Thread.sleep(1000);
//         }
        } catch (Exception e) {
//            e.printStackTrace();
            Log.logException(e);
        }


    }
    public void readVerifyFlashButton(View view){
        Log.log("readVerifyFlashButton in HomeFragment");
        // fire manage connection, make sure ble knows if we are connected or not, or just write it.
        try {
            Verifier verifier = new Verifier(this);
            (new Thread(verifier)).start();

        } catch (Exception e) {
//            e.printStackTrace();
            Log.logException(e);
        }

    }


//
//
//    @Override
//    public void onStart() {
//        super.onStart();
//        Log.log("HomeFragment onStart");
//
////        String[] fuckyou = new String[]{"RGB","WaveSynth","PWM"};
////        this.spinnerAdapter  = new ArrayAdapter(  this.getActivity(),  android.R.layout.simple_spinner_item,fuckyou);
//    }


    @Override
    public void onDestroyView() {
        super.onDestroyView();
        Log.log("HomeFragment onDestroyView");
        fudges.add("destroyed again:  " + System.currentTimeMillis());
        //  binding = null;
    }

    @Override
    public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
        Log.log("onItemSelected, i: " + i + ", l: " + l + ", view: " + view.toString());
    }

    @Override
    public void onNothingSelected(AdapterView<?> adapterView) {
Log.log("onNothingSelected");
    }
}