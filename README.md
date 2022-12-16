# Android Key-Value Library

## desc

A sp-like key-value library. Compatible with android native SharedPreferences.

## feature

1. sp-like api, compatible with android native SharedPreferences.
2. thread-safe & process-safe. 
3. based on mmap, don't worry about data loss.
4. type-safe, type information is retained during persistence.
5. runtime data verification, more secure.

## usage

1. init
```java
public class MyApplication extends Application {
	@Override
	public void onCreate() {
		super.onCreate();
		NoKV.init(this);
	}
}
```

2. create
```java
SharedPreferences sp = NoKV.create("fuck1", MODE_PRIVATE);
SharedPreferences.Editor editor = sp.edit();
editor.putInt("foo", 1);
editor.commit();
sp.getInt("foo", 0);
```
