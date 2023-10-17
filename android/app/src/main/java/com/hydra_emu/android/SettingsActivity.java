package com.hydra_emu.android;

import android.os.Bundle;
import android.os.PersistableBundle;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.constraintlayout.widget.ConstraintLayout;

import com.google.android.material.tabs.TabLayout;

public class SettingsActivity extends AppCompatActivity {

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        FrameLayout frameLayout = new FrameLayout(this);
        frameLayout.setLayoutParams(new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
        TabLayout tabLayout = new TabLayout(this);
        FrameLayout.LayoutParams tabParams = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        tabLayout.setLayoutParams(tabParams);
        tabLayout.setTabGravity(TabLayout.GRAVITY_FILL);
        frameLayout.addView(tabLayout);
        TabLayout.Tab roms_tab = tabLayout.newTab().setText("Roms");
        TabLayout.Tab cores_tab = tabLayout.newTab().setText("Cores");
        TabLayout.Tab settings_tab = tabLayout.newTab().setText("Settings");
        tabLayout.addTab(roms_tab);
        tabLayout.addTab(cores_tab);
        tabLayout.addTab(settings_tab);
        setContentView(frameLayout);
    }
}
