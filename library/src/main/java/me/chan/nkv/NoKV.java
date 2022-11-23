package me.chan.nkv;

import android.content.Context;
import android.content.SharedPreferences;

import androidx.annotation.Nullable;

import java.io.File;
import java.util.Map;
import java.util.Set;

public class NoKV implements SharedPreferences {
	static {
		System.loadLibrary("nokv");
	}

	private static Context sContext;
	private static File sWorkspace;

	public static Context init(Context baseContext) {
		sContext = baseContext;
		sWorkspace = baseContext.getDir("nokv", Context.MODE_PRIVATE);
		if (nativeInit(new File(sWorkspace, "nkv_meta").toString()) != 0) {
			return baseContext;
		}
		return new ProxyContext(baseContext);
	}

	public static SharedPreferences create(String name, int mode) {
		long ptr = nativeCreate(new File(sWorkspace, "/kv/" + name).getAbsolutePath());
		if (ptr == 0) {
			return sContext.getSharedPreferences(name, mode);
		}
		return new NoKV(ptr);
	}

	private final long mPtr;

	private NoKV(long ptr) {
		mPtr = ptr;
	}

	@Override
	public Map<String, ?> getAll() {
		return null;
	}

	@Nullable
	@Override
	public String getString(String key, @Nullable String defValue) {
		return null;
	}

	@Nullable
	@Override
	public Set<String> getStringSet(String key, @Nullable Set<String> defValues) {
		return null;
	}

	@Override
	public int getInt(String key, int defValue) {
		return 0;
	}

	@Override
	public long getLong(String key, long defValue) {
		return 0;
	}

	@Override
	public float getFloat(String key, float defValue) {
		return 0;
	}

	@Override
	public boolean getBoolean(String key, boolean defValue) {
		return false;
	}

	@Override
	public boolean contains(String key) {
		return false;
	}

	@Override
	public Editor edit() {
		return null;
	}

	@Override
	public void registerOnSharedPreferenceChangeListener(OnSharedPreferenceChangeListener listener) {
		// todo
	}

	@Override
	public void unregisterOnSharedPreferenceChangeListener(OnSharedPreferenceChangeListener listener) {
		// todo
	}

	@Override
	protected void finalize() throws Throwable {
		super.finalize();
		nativeRelease(mPtr);
	}

	private static native long nativeCreate(String kv);

	private static native int nativeInit(String metaFile);

	private static native void nativeRelease(long ptr);
}
