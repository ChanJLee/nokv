package com.example.nkv;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.SystemClock;
import android.util.Log;

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
		}).start();
	}
}