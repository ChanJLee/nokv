package me.chan.nkv;

import android.content.Context;
import android.content.SharedPreferences;

import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.platform.app.InstrumentationRegistry;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import java.util.HashSet;
import java.util.Map;
import java.util.Set;

@RunWith(AndroidJUnit4.class)
public class NoKvUnitTest {
	private static boolean INIT = false;

	@Before
	public void setup() {
		if (INIT) {
			return;
		}

		Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
		NoKV.init(appContext);
		INIT = true;
	}

	@Test
	public void testBaseWrite() {
		SharedPreferences sp = NoKV.create("w1", Context.MODE_PRIVATE);
		SharedPreferences.Editor editor = sp.edit();
		editor.putString("string", "fuck");
		Set<String> set = new HashSet<>();
		set.add("fuck");
		set.add(null);
		set.add("");
		set.add("fuck2");
		editor.putStringSet("set", set);
		editor.putInt("int", 1);
		editor.putLong("long", 2);
		editor.putFloat("float", 1.5f);
		editor.putBoolean("bool", true);
		editor.commit();

		Assert.assertEquals(sp.getString("string", null), "fuck");
		Assert.assertEquals(sp.getStringSet("set", null), set);
		Assert.assertEquals(sp.getInt("int", -1), 1);
		Assert.assertEquals(sp.getLong("long", -1), 2);
		Assert.assertEquals(sp.getFloat("float", -1), 1.5f, 0.1);
		Assert.assertTrue(sp.getBoolean("bool", false));

		Map<String, Object> keys = (Map<String, Object>) sp.getAll();
		Assert.assertEquals(keys.size(), 6);

		editor = sp.edit();
		editor.clear();
		Assert.assertTrue(editor.commit());
		Assert.assertNull(sp.getString("string", null));
		Assert.assertNull(sp.getStringSet("set", null));
		Assert.assertEquals(sp.getInt("int", -1), -1);
		Assert.assertEquals(sp.getLong("long", -1), -1);
		Assert.assertEquals(sp.getFloat("float", -1), -1f, 0.1);
		Assert.assertFalse(sp.getBoolean("bool", false));
	}
}
