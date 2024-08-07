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

import android.graphics.Color;
import android.os.Bundle;
import android.view.MenuItem;
import android.view.View;
import android.view.Menu;
import android.widget.ImageView;

import com.google.android.material.snackbar.Snackbar;
import com.google.android.material.navigation.NavigationView;
import com.hurleyresearch.android.nrfice.ui.home.HomeFragment;

import androidx.core.view.GravityCompat;
import androidx.fragment.app.Fragment;
import androidx.navigation.NavController;
import androidx.navigation.Navigation;
import androidx.navigation.ui.AppBarConfiguration;
import androidx.navigation.ui.NavigationUI;
import androidx.drawerlayout.widget.DrawerLayout;
import androidx.appcompat.app.AppCompatActivity;

//import com.hurleyresearch.android.nrfice.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity implements NavigationView.OnNavigationItemSelectedListener{
public DrawerLayout drawer = null;
    public static MainActivity instance_;
    public HomeFragment homeFragment;
    public static MainActivity instance(){
        return instance_;
    }



    public static BLE ble = null;


    private AppBarConfiguration mAppBarConfiguration;
//    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);


//what's next?
     //   setContentView(R.layout.drawer_layout);

//
//        toolbar = findViewById(R.id.toolbar);
//        setSupportActionBar(toolbar);
//
//        drawer = findViewById(R.id.drawer_layout);
//        NavigationView navigationView = findViewById(R.id.nav_view);
//        navigationView.setNavigationItemSelectedListener(this);
//
//        ActionBarDrawerToggle toggle = new ActionBarDrawerToggle(this, drawer, toolbar, R.string.navigation_drawer_open, R.string.navigation_drawer_close);
//        drawer.addDrawerListener(toggle);
//        toggle.syncState();



//
//        View view = this.inflate(R.layout.activity_main_main, container, false);
// this is not hard. just knock it out. but we need to shred, and we need math. but you need money, believe it or not, more than

MainActivity.instance_ = this;

//        binding = ActivityMainBinding.inflate(getLayoutInflater());
//        setContentView(R.layout.app_bar_main);
        setContentView(R.layout.activity_main);
this.drawer = this.findViewById(R.id.drawer_layout);
       // setSupportActionBar(binding.appBarMain.toolbar);
        setSupportActionBar(this.findViewById(R.id.toolbar));
//        binding.appBarMain.fab.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
//                        .setAction("Action", null).show();
//            }
//        });
//        DrawerLayout drawer = binding.drawerLayout;
//        NavigationView navigationView = binding.navView;
       // DrawerLayout drawer = this.findViewById(R.id.drawer_layout);
        NavigationView navigationView = this.findViewById(R.id.nav_view);
        navigationView.setNavigationItemSelectedListener(this);
        // Passing each menu ID as a set of Ids because each
        // menu should be considered as top level destinations.
        mAppBarConfiguration = new AppBarConfiguration.Builder(
                R.id.nav_home, R.id.nav_rgb, R.id.nav_gallery, R.id.nav_slideshow)
                .setOpenableLayout(drawer)
                .build();
        NavController navController = Navigation.findNavController(this, R.id.nav_host_fragment_content_main);
        NavigationUI.setupActionBarWithNavController(this, navController, mAppBarConfiguration);
        NavigationUI.setupWithNavController(navigationView, navController);

        ble = BLE.instance(this);// start connecting automatically
    }


    public void   addProjectButton(View view){

homeFragment.addProjectButton(view);

    }
    public void    programFPGAFlashButton(View view){
        homeFragment.programFPGAFlashButton(view);

    }
    public void   readVerifyFlashButton(View view){
        homeFragment.readVerifyFlashButton(view);

    }

public void setBluetoothIndicator(boolean connected){
        if( connected ){
            ((ImageView) this.findViewById(R.id.imageViewtest)).setColorFilter(Color.WHITE);

        } else {
            ((ImageView) this.findViewById(R.id.imageViewtest)).setColorFilter(Color.RED);
        }
}
    public void   resetICEButton(View view){
     homeFragment.resetICEButton(view);

    }
    public void   softResetICEButton(View view){
        homeFragment.softResetICEButton(view);

    }
    public void   eraseZeroButton(View view){
        homeFragment.eraseZeroButton(view);

    }
    public void   btSendButton(View view){
        homeFragment.btSendButton(view);

    }
    public void   readFlashTestButton(View view){
        homeFragment.readFlashTestButton(view);

    }
    public void   writeFlashTestButton(View view){
        homeFragment.writeFlashTestButton(view);

    }
    public void   eraseFlashButton(View view){
        homeFragment.eraseFlashButton(view);

    }
    public void   flashStatusButton(View view){
        homeFragment.flashStatusButton(view);

    }


    @Override
    public void onBackPressed() {
        if (drawer.isDrawerOpen(GravityCompat.START)) {
            drawer.closeDrawer(GravityCompat.START);
        } else {
            super.onBackPressed();
        }
    }
//
    @Override
    public boolean onNavigationItemSelected(MenuItem item) {
     if( item.getItemId() == R.id.nav_home){
         System.out.println("NRFICE: home chosen");
     }


        switch (item.getItemId()) {
            case R.id.nav_home:
//                returnToCameraFragment = true;
//                switchToSettingsPage();

        }
        drawer.closeDrawer(GravityCompat.START);
        return true;
    }
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onSupportNavigateUp() {
        NavController navController = Navigation.findNavController(this, R.id.nav_host_fragment_content_main);
        return NavigationUI.navigateUp(navController, mAppBarConfiguration)
                || super.onSupportNavigateUp();
    }
}