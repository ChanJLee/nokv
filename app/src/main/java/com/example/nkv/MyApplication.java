package com.example.nkv;

import android.app.Application;

import me.chan.nkv.NoKV;

public class MyApplication extends Application {
	@Override
	public void onCreate() {
		super.onCreate();
		NoKV.init(this);
	}
}
