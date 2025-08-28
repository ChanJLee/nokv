package com.example.nkv;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

import androidx.annotation.Nullable;

import me.chan.nkv.NoKV;

public class BaseService extends Service {

	@Nullable
	@Override
	public IBinder onBind(Intent intent) {
		return null;
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		new Thread(() -> {
			for (int i = 0; i < 10000; ++i) {
				NoKV.nativeTestV2();
			}
		}).start();

		return super.onStartCommand(intent, flags, startId);
	}
}
