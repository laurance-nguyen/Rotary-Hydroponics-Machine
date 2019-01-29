package com.example.ross.hashiramaproject;


import android.graphics.Color;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;


import android.view.View;
import android.widget.Button;

import com.gelitenight.waveview.library.WaveView;


public class TestAnimation extends AppCompatActivity {
    private WaveHelper mWaveHelper;
    private int mBorderColor = Color.parseColor("#000000");
    private int mBorderWidth = 10;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        setTheme(R.style.AppTheme);
        super.onCreate(savedInstanceState);
        setContentView(R.layout.test_animation);

        final WaveView waveView = findViewById(R.id.wave);

        final Button button = findViewById(R.id.button);
        waveView.setBorder(mBorderWidth,mBorderColor);
        waveView.setShapeType(WaveView.ShapeType.SQUARE);
        waveView.setWaveColor(Color.parseColor("#882196F3"),Color.parseColor("#2196F3"));
        mWaveHelper = new WaveHelper(waveView);

        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

            }
        });

    }
    @Override
    protected void onPause() {
        super.onPause();
        mWaveHelper.cancel();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mWaveHelper.start();
    }


}

