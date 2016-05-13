/* GENERATED SOURCE. DO NOT MODIFY. */
/*
 ****************************************************************************
 * Copyright (c) 2007-2016 International Business Machines Corporation and  *
 * others.  All rights reserved.                                            *
 ****************************************************************************
 */

package android.icu.impl;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * An implementaton of ICUCache that stores entry values using CacheValue.
 * @hide Only a subset of ICU is exposed in Android
 */
public class CacheValueICUCache<K, V> implements ICUCache<K, V> {
    private final Map<K, CacheValue<V>> cache = createMap();

    public CacheValueICUCache() {
    }

    protected Map<K, CacheValue<V>> createMap() {
        return new ConcurrentHashMap<K, CacheValue<V>>();
    }

    public V get(Object key) {
        CacheValue<V> value = cache.get(key);
        if (value == null) {
            return null;
        }
        return value.get();
    }

    public void put(K key, V value) {
        cache.put(key, CacheValue.getInstance(value));
    }

    public void clear() {
        cache.clear();
    }
}
