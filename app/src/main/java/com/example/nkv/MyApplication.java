package com.example.nkv;

import android.app.Application;
import android.content.Context;

import java.io.File;

import me.chan.nkv.NoKV;

public class MyApplication extends Application {

	public static long PTR = 0;

	@Override
	public void onCreate() {
		super.onCreate();
		NoKV.init(this);
		File ws = getDir("nokv", Context.MODE_PRIVATE);
		File f = new File(ws, "testdb");
		PTR = NoKV.nativeInitV2(f.getAbsolutePath());
	}
}
