package me.chan.nkv;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

import androidx.annotation.Nullable;

import java.io.File;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public class NoKV implements SharedPreferences {
	@SuppressLint("StaticFieldLeak")
	private static Context sContext;
	private static boolean sInitSuccess = false;
	private static final Map<String, NoKV> LRU = new HashMap<>();

	public static void init(Context context) {
		init(context, new LibraryLoader() {
			@Override
			public boolean load(Context context, String... so) {
				try {
					System.loadLibrary(so[0]);
					return true;
				} catch (Throwable throwable) {
					return false;
				}
			}
		});
	}

	public static void init(Context context, LibraryLoader loader) {
		if (sInitSuccess) {
			throw new IllegalStateException("nokv init twice");
		}

		sContext = context.getApplicationContext();
		File ws = context.getDir("nokv", Context.MODE_PRIVATE);
		if (!loader.load(context, "nokv") || nativeInit(ws.getAbsolutePath()) != 0) {
			e("init nokv failed");
			return;
		}

		sInitSuccess = true;
	}

	public static synchronized SharedPreferences create(String name, int mode) {
		NoKV kv = LRU.get(name);
		if (kv != null) {
			return kv;
		}

		long ptr = sInitSuccess ? nativeCreate(name) : 0;
		if (ptr == 0) {
			return sContext.getSharedPreferences(name, mode);
		}

		kv = new NoKV(ptr);
		LRU.put(name, kv);
		return kv;
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
		return new NoKvEditor(mPtr, mListeners);
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

	private static native long nativeCreate(String ws);

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

	/**
	 * @param sharedPreferences sp
	 * @return 是否迁移成功
	 */
	public boolean migrate(SharedPreferences sharedPreferences) {
		if (sharedPreferences == null) {
			return false;
		}

		Map<String, Object> values = (Map<String, Object>) sharedPreferences.getAll();
		if (values == null) {
			return false;
		}

		Editor editor = new NoKvEditor(values, mPtr, mListeners);
		return editor.commit();
	}

	public interface LibraryLoader {
		boolean load(Context context, String... so);
	}
}
