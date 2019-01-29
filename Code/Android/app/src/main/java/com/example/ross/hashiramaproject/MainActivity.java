package com.example.ross.hashiramaproject;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.widget.CardView;
import android.view.View;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {
    private CardView monitoringCard, manualCard, cameraCard, settingCard;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        setTheme(R.style.AppTheme);
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        // defining cards
        monitoringCard = findViewById(R.id.card_monitoring);
        manualCard = findViewById(R.id.card_manual);
        cameraCard = findViewById(R.id.card_camera);
        settingCard = findViewById(R.id.card_setting);
        // Add Click Listener to the cards
        monitoringCard.setOnClickListener(this);
        manualCard.setOnClickListener(this);
        cameraCard.setOnClickListener(this);
        settingCard.setOnClickListener(this);

    }

    @Override
    public void onClick(View v) {
        Intent i;

        switch(v.getId()){
            case R.id.card_monitoring : i = new Intent(this, MonitoringActivity.class); startActivity(i); break;
            case R.id.card_manual : i = new Intent(this, ManualActivity.class); startActivity(i); break;
            case R.id.card_camera : i = new Intent(this, CameraActivity.class); startActivity(i); break;
            case R.id.card_setting : i = new Intent(this, SettingActivity.class); startActivity(i); break;
            default : break;
        }

    }
}
