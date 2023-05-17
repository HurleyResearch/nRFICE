package com.hurleyresearch.android.nrfice.ui.slideshow;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;

import com.hurleyresearch.android.nrfice.Log;
import com.hurleyresearch.android.nrfice.R;
//import androidx.lifecycle.ViewModelProvider;

//import com.hurleyresearch.android.nrfice.databinding.FragmentSlideshowBinding;

public class SlideshowFragment extends Fragment {

//    private FragmentSlideshowBinding binding;

    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {
//        SlideshowViewModel slideshowViewModel =
//                new ViewModelProvider(this).get(SlideshowViewModel.class);

//        binding = FragmentSlideshowBinding.inflate(inflater, container, false);
//        View root = binding.getRoot();

//        final TextView textView = binding.textSlideshow;
//        textView.setText("fudge all this bs");
//        slideshowViewModel.getText().observe(getViewLifecycleOwner(), textView::setText);
        super.onCreate(savedInstanceState);
        Log.log("SlideshowFragment onCreateView");
        View view = inflater.inflate(R.layout.fragment_slideshow, container, false);
        return view;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        Log.log("SlideshowFragment onDestroyView");
//        binding = null;
    }
}