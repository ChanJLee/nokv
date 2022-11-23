package me.chan.nkv;

import android.content.Context;
import android.content.ContextWrapper;
import android.content.SharedPreferences;

class ProxyContext extends ContextWrapper {

	public ProxyContext(Context base) {
		super(base);
	}

	@Override
	public SharedPreferences getSharedPreferences(String name, int mode) {
		NoKV kv = NoKV.create(name, mode);
		if (kv != null) {
			return kv;
		}

		return super.getSharedPreferences(name, mode);
	}
}
