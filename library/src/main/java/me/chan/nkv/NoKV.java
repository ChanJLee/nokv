package me.chan.nkv;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

import androidx.annotation.Nullable;

import java.io.File;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public class NoKV implements SharedPreferences {
	static {
		System.loadLibrary("nokv");
	}

	@SuppressLint("StaticFieldLeak")
	private static Context sContext;

	public static Context init(Context baseContext) {
		sContext = baseContext;
		e("context: "  + baseContext);
		File ws = baseContext.getDir("nokv", Context.MODE_PRIVATE);
		if (nativeInit(ws.getAbsolutePath()) != 0) {
			e("init nokv failed");
			return baseContext;
		}
		return new ProxyContext(baseContext);
	}

	public static SharedPreferences create(String name, int mode) {
		long ptr = nativeCreate(name);
		if (ptr == 0) {
			return sContext.getSharedPreferences(name, mode);
		}
		return new NoKV(ptr);
	}

	private final long mPtr;
	private Set<OnSharedPreferenceChangeListener> mListeners;

	private NoKV(long ptr) {
		mPtr = ptr;
	}

	@Override
	public Map<String, ?> getAll() {
		return nativeGetAll(mPtr);
	}

	@Nullable
	@Override
	public String getString(String key, @Nullable String defValue) {
		return nativeGetString(mPtr, key, defValue);
	}

	@Nullable
	@Override
	public Set<String> getStringSet(String key, @Nullable Set<String> defValues) {
		return nativeGetStringSet(mPtr, key, defValues);
	}

	@Override
	public int getInt(String key, int defValue) {
		return nativeGetInt(mPtr, key, defValue);
	}

	@Override
	public long getLong(String key, long defValue) {
		return nativeGetLong(mPtr, key, defValue);
	}

	@Override
	public float getFloat(String key, float defValue) {
		return nativeGetFloat(mPtr, key, defValue);
	}

	@Override
	public boolean getBoolean(String key, boolean defValue) {
		return nativeGetBoolean(mPtr, key, defValue);
	}

	@Override
	public boolean contains(String key) {
		return nativeContains(mPtr, key);
	}

	@Override
	public Editor edit() {
		return new NoKvEditor(mPtr);
	}

	@Override
	public synchronized void registerOnSharedPreferenceChangeListener(OnSharedPreferenceChangeListener listener) {
		if (mListeners == null) {
			mListeners = new HashSet<>();
		}
		mListeners.add(listener);
	}

	@Override
	public synchronized void unregisterOnSharedPreferenceChangeListener(OnSharedPreferenceChangeListener listener) {
		// todo
		if (mListeners == null) {
			mListeners = new HashSet<>();
		}
		mListeners.remove(listener);
	}

	@Override
	protected void finalize() throws Throwable {
		nativeRelease(mPtr);
		super.finalize();
	}

	private static native long nativeCreate(String kv);

	private static native int nativeInit(String ws);

	private static native void nativeRelease(long ptr);

	private static native int nativeGetInt(long ptr, String key, int defValue);

	private static native boolean nativeGetBoolean(long ptr, String key, boolean defValue);

	private static native long nativeGetLong(long ptr, String key, long defValue);

	private static native float nativeGetFloat(long ptr, String key, float defValue);

	private static native String nativeGetString(long ptr, String key, String defValue);

	private static native boolean nativeContains(long ptr, String key);

	private static native Map<String, ?> nativeGetAll(long ptr);

	private static native Set<String> nativeGetStringSet(long ptr, String key, Set<String> defValues);

	private static void e(String msg) {
		Log.e("NoKV", msg);
	}
}
