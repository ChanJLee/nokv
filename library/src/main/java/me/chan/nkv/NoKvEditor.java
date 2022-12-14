package me.chan.nkv;

import android.content.SharedPreferences;
import android.text.TextUtils;

import androidx.annotation.Keep;
import androidx.annotation.Nullable;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

@Keep
class NoKvEditor implements SharedPreferences.Editor {
	private final long mPtr;
	private final Map<String, Object> mModify;
	private final Set<String> mDelete = new HashSet<>();
	private boolean mClear = false;
	private final Set<SharedPreferences.OnSharedPreferenceChangeListener> mListeners;
	private final SharedPreferences mLock;

	NoKvEditor(SharedPreferences lock, long ptr, Set<SharedPreferences.OnSharedPreferenceChangeListener> listeners) {
		mLock = lock;
		mPtr = ptr;
		mListeners = listeners;
		mModify = new HashMap<>();
	}

	NoKvEditor(Map<String, Object> values, SharedPreferences lock, long ptr, Set<SharedPreferences.OnSharedPreferenceChangeListener> listeners) {
		mLock = lock;
		mPtr = ptr;
		mListeners = listeners;
		mModify = values;
	}

	@Override
	public SharedPreferences.Editor putString(String key, @Nullable String value) {
		mModify.put(key, value);
		return this;
	}

	@Override
	public SharedPreferences.Editor putStringSet(String key, @Nullable Set<String> values) {
		mModify.put(key, values);
		return this;
	}

	@Override
	public SharedPreferences.Editor putInt(String key, int value) {
		mModify.put(key, value);
		return this;
	}

	@Override
	public SharedPreferences.Editor putLong(String key, long value) {
		mModify.put(key, value);
		return this;
	}

	@Override
	public SharedPreferences.Editor putFloat(String key, float value) {
		mModify.put(key, value);
		return this;
	}

	@Override
	public SharedPreferences.Editor putBoolean(String key, boolean value) {
		mModify.put(key, value);
		return this;
	}

	@Override
	public SharedPreferences.Editor remove(String key) {
		mDelete.add(key);
		return this;
	}

	@Override
	public SharedPreferences.Editor clear() {
		mClear = true;
		return this;
	}

	@Override
	public boolean commit() {
		if (!nativeBeginTransaction(mPtr)) {
			return false;
		}

		Set<String> changes = mListeners == null || mListeners.isEmpty() ? null : new HashSet<>();

		try {
			if (mClear && !nativeClear(mPtr)) {
				return false;
			}

			for (String key : mDelete) {
				if (TextUtils.isEmpty(key)) {
					continue;
				}

				if (!nativeRemove(mPtr, key)) {
					return false;
				}
			}

			if (changes != null) {
				changes.addAll(mDelete);
			}

			for (Map.Entry<String, Object> entry : mModify.entrySet()) {
				String key = entry.getKey();
				if (TextUtils.isEmpty(key)) {
					continue;
				}

				if (changes != null) {
					changes.add(key);
				}

				Object value = entry.getValue();
				if (value instanceof String) {
					if (!nativePutString(mPtr, key, (String) value)) {
						return false;
					}
				} else if (value instanceof Boolean) {
					if (!nativePutBoolean(mPtr, key, (Boolean) value)) {
						return false;
					}
				} else if (value instanceof Integer) {
					if (!nativePutInteger(mPtr, key, (Integer) value)) {
						return false;
					}
				} else if (value instanceof Long) {
					if (!nativePutLong(mPtr, key, (Long) value)) {
						return false;
					}
				} else if (value instanceof Float) {
					if (!nativePutFloat(mPtr, key, (Float) value)) {
						return false;
					}
				} else if (value instanceof Set) {
					if (!nativePutStringSet(mPtr, key, (Set<String>) value)) {
						return false;
					}
				} else if (value == null) {
					if (!nativePutNull(mPtr, key)) {
						return false;
					}
				}
			}
		} finally {
			nativeEndTransaction(mPtr);
		}

		// reset state
		mClear = false;
		mModify.clear();
		mDelete.clear();

		notifyCommitSubmit(changes);

		return true;
	}

	@Override
	public void apply() {
		commit();
	}

	private static native boolean nativeBeginTransaction(long ptr);

	private static native void nativeEndTransaction(long ptr);

	private static native boolean nativeClear(long ptr);

	private static native boolean nativeRemove(long ptr, String key);

	private static native boolean nativePutString(long ptr, String key, String value);

	private static native boolean nativePutStringSet(long ptr, String key, Set<String> value);

	private static native boolean nativePutInteger(long ptr, String key, int value);

	private static native boolean nativePutLong(long ptr, String key, long value);

	private static native boolean nativePutFloat(long ptr, String key, float value);

	private static native boolean nativePutBoolean(long ptr, String key, boolean value);

	private static native boolean nativePutNull(long ptr, String key);

	private void notifyCommitSubmit(Set<String> changes) {
		if (changes == null || mListeners == null) {
			return;
		}

		synchronized (mLock) {
			for (String key : changes) {
				for (SharedPreferences.OnSharedPreferenceChangeListener listener : mListeners) {
					listener.onSharedPreferenceChanged(mLock, key);
				}
			}
		}
	}
}