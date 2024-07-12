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

package com.hurleyresearch.android.nrfice.ui.gallery;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;

import com.hurleyresearch.android.nrfice.Log;
import com.hurleyresearch.android.nrfice.MainActivity;
import com.hurleyresearch.android.nrfice.R;
//import androidx.lifecycle.ViewModelProvider;

//import com.hurleyresearch.android.nrfice.databinding.FragmentGalleryBinding;

public class GalleryFragment extends Fragment {

//    private FragmentGalleryBinding binding;

    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {
//        GalleryViewModel galleryViewModel =
//                new ViewModelProvider(this).get(GalleryViewModel.class);

//        binding = FragmentGalleryBinding.inflate(inflater, container, false);
//        View root = binding.getRoot();

//        final TextView textView = binding.textGallery;
//        textView.setText("gallery you mofo");
//        galleryViewModel.getText().observe(getViewLifecycleOwner(), textView::setText);

        super.onCreate(savedInstanceState);
        Log.log("GalleryFragment onCreateView");
        View view = inflater.inflate(R.layout.fragment_gallery, container, false);

        if( MainActivity.instance().homeFragment == null ){
            Log.log("mains home frag null in gallery create");
        } else {
            for( String s : MainActivity.instance().homeFragment.fudges ){
                Log.log("   linsting home's fudges: " + s);
            }
        }


        return view;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        Log.log("GalleryFragment onDestroyView");
//        binding = null;
    }
}