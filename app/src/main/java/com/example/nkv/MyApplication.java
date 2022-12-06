package com.example.nkv;

import android.app.Application;
import android.content.Context;

import me.chan.nkv.NoKV;

public class MyApplication extends Application {
	@Override
	protected void attachBaseContext(Context base) {
		super.attachBaseContext(NoKV.init(base));
	}
}
