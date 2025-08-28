package com.example.nkv;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.SystemClock;
import android.util.Log;
import android.view.View;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import java.util.ArrayList;
import java.util.List;

import me.chan.nkv.NoKV;

public class MainActivity extends AppCompatActivity {
	@Override
	protected void onCreate(@Nullable Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		findViewById(R.id.btn).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				new Thread(new Runnable() {
					@Override
					public void run() {
						List<String> intKeys = new ArrayList<>(10240);
						List<String> stringKeys = new ArrayList<>(10240);
						for (int i = 0; i < 10000; ++i) {
							intKeys.add("key_i" + i);
							stringKeys.add("key_s" + i);
						}

						long ts = SystemClock.elapsedRealtime();
						SharedPreferences sp = getSharedPreferences("fuck1", MODE_PRIVATE);
						SharedPreferences.Editor editor = sp.edit();
						for (int i = 0; i < intKeys.size(); ++i) {
							editor.putInt(intKeys.get(i), i);
							editor.putString(stringKeys.get(i), stringKeys.get(i));
						}
						editor.commit();
						long ts2 = SystemClock.elapsedRealtime();
						Log.d("NoKV", "sp write: " + (ts2 - ts));

						ts = ts2;
						for (int i = 0; i < intKeys.size(); ++i) {
							sp.getInt(intKeys.get(i), 0);
							sp.getString(stringKeys.get(i), null);
						}
						ts2 = SystemClock.elapsedRealtime();
						Log.d("NoKV", "sp read: " + (ts2 - ts));

						ts = ts2;
						sp = NoKV.create("fuck1", MODE_PRIVATE);
						editor = sp.edit();
						for (int i = 0; i < intKeys.size(); ++i) {
							editor.putInt(intKeys.get(i), i);
							editor.putString(stringKeys.get(i), stringKeys.get(i));
						}
						editor.commit();
						ts2 = SystemClock.elapsedRealtime();
						Log.d("NoKV", "nokv write: " + (ts2 - ts));

						ts = ts2;
						for (int i = 0; i < intKeys.size(); ++i) {
							sp.getInt(intKeys.get(i), 0);
							sp.getString(stringKeys.get(i), null);
						}
						ts2 = SystemClock.elapsedRealtime();
						Log.d("NoKV", "nokv read: " + (ts2 - ts));
					}
				}, "FuckOff").start();
			}
		});

		findViewById(R.id.btn2).setOnClickListener(v -> {
			SharedPreferences sharedPreferences = NoKV.create("demo", MODE_PRIVATE);
			SharedPreferences.Editor editor = sharedPreferences.edit();
			editor.putString("f1", "xxx");
			editor.putString("f2", "yyy");
			editor.putInt("f3", 1234);
			editor.commit();
		});

		findViewById(R.id.btn3).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				NoKV.nativeTestV2();
				startService(new Intent(MainActivity.this, Writer1Service.class));
				startService(new Intent(MainActivity.this, Writer2Service.class));
			}
		});
	}
}