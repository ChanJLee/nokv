package com.example.nkv;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.content.ContextWrapper;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.util.Log;

public class MainActivity extends AppCompatActivity {
	@Override
	protected void onCreate(@Nullable Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		Context context = getBaseContext();
		while (context instanceof ContextWrapper) {
			Log.d("NoKV", "context: " + context);
			ContextWrapper wrapper = (ContextWrapper) context;
			context = wrapper.getBaseContext();
		}

		Log.d("NoKV", "final context: " + context);

		SharedPreferences sp = getSharedPreferences("fuck", MODE_PRIVATE);
		Log.d("NoKV", "sp -> " + sp);
	}
}