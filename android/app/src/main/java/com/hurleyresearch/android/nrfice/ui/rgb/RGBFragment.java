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

package com.hurleyresearch.android.nrfice.ui.rgb;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.SeekBar;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
//import androidx.lifecycle.ViewModelProvider;

//import com.hurleyresearch.android.nrfice.databinding.FragmentGalleryBinding;
import com.hurleyresearch.android.nrfice.Log;
import com.hurleyresearch.android.nrfice.R;
//import com.hurleyresearch.android.nrfice.databinding.FragmentRgbBinding;
//import com.hurleyresearch.android.nrfice.ui.gallery.GalleryViewModel;

public class RGBFragment extends Fragment implements SeekBar.OnSeekBarChangeListener {

    //    private FragmentRgbBinding binding;
    public SeekBar seekBarRed = null;
    public SeekBar seekBarGreen = null;
    public SeekBar seekBarBlue = null;
    public static int[] seekBarValues = new int[3];
    public static RGBRunner rgbRunner = null;
    public EditText editTextRGBR = null;
    public EditText editTextRGBB = null;
    public EditText editTextRGBG = null;

    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {
//        RGBViewModel rgbViewModel =
//                new ViewModelProvider(this).get(RGBViewModel.class);

//        binding = FragmentRgbBinding.inflate(inflater, container, false);
//        View root = binding.getRoot();

//        final TextView textView = binding.rgbText;
//        textView.setText("finally some sanity in rgb");
//        rgbViewModel.getText().observe(getViewLifecycleOwner(), textView::setText);

        super.onCreate(savedInstanceState);
        Log.log("RGBFragment onCreateView");
        View view = inflater.inflate(R.layout.fragment_rgb, container, false);
        this.seekBarRed = (SeekBar) view.findViewById(R.id.seekBarRed);
        this.seekBarGreen = (SeekBar) view.findViewById(R.id.seekBarGreen);
        this.seekBarBlue = (SeekBar) view.findViewById(R.id.seekBarBlue);
        this.editTextRGBR = (EditText) view.findViewById(R.id.editTextRGBR);
        this.editTextRGBG = (EditText) view.findViewById(R.id.editTextRGBG);
        this.editTextRGBB = (EditText) view.findViewById(R.id.editTextRGBB);
        this.seekBarRed.setOnSeekBarChangeListener(this);
        this.seekBarGreen.setOnSeekBarChangeListener(this);
        this.seekBarBlue.setOnSeekBarChangeListener(this);


        (new Thread(new RGBRunner())).start();
        return view;
    }


    @Override
    public void onDestroyView() {
        super.onDestroyView();
        Log.log("RGBFragment onDestroyView");
//        binding = null;
    }
// do we need a thread to slowly send these guys? probably.
    public void onProgressChanged(SeekBar seekBar,
                                  int progress,
                                  boolean fromUser) {

        if (seekBar.getId() == R.id.seekBarRed) {
            seekBarValues[0] = progress;
            this.editTextRGBR.setText("" + progress);
         //   Log.log("onProgressChanged, progress seekBarRed: " + progress);
        } else if (seekBar.getId() == R.id.seekBarGreen) {
            seekBarValues[1] = progress;
            this.editTextRGBG.setText("" + progress);
          //  Log.log("onProgressChanged, progress seekBarGreen: " + progress);
        } else if (seekBar.getId() == R.id.seekBarBlue) {
            seekBarValues[2] = progress;
            this.editTextRGBB.setText("" + progress);
           // Log.log("onProgressChanged, progress seekBarBlue: " + progress);
        }
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {

    }
}