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