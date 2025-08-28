package com.example.nkv;

import android.app.Application;
import android.content.Context;

import java.io.File;

import me.chan.nkv.NoKV;

public class MyApplication extends Application {
	@Override
	public void onCreate() {
		super.onCreate();
		NoKV.init(this);
		File ws = getDir("nokv", Context.MODE_PRIVATE);
		File f = new File(ws, "testdb");
		NoKV.nativeInitV2(f.getAbsolutePath());
	}
}
