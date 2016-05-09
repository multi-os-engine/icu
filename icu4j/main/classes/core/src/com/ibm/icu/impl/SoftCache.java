/*
*******************************************************************************
*   Copyright (C) 2010-2011, International Business Machines
*   Corporation and others.  All Rights Reserved.
*******************************************************************************
*/
package com.ibm.icu.impl;

import java.lang.ref.Reference;
import java.lang.ref.SoftReference;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Generic, thread-safe cache implementation, storing References to cached instances.
 * To use, instantiate a subclass which implements the createInstance() method,
 * and call get() with the key and the data. The get() call will use the data
 * only if it needs to call createInstance(), otherwise the data is ignored.
 *
 * For example, by using SoftReferences to instances, the Java runtime can release instances
 * once they are not used any more at all. If such an instance is then requested again,
 * the get() method will call createInstance() again and also create a new Reference.
 * The cache holds on to its map of keys to Referenced instances forever.
 *
 * @param <K> Cache lookup key type
 * @param <V> Cache instance value type
 * @param <D> Data type for creating a new instance value
 *
 * @author Markus Scherer, Mark Davis
 */
public abstract class SoftCache<K, V, D> extends CacheBase<K, V, D> {
    @Override
    public final V getInstance(K key, D data) {
        // We synchronize twice, once on the map and once on valueRef,
        // because we prefer the fine-granularity locking of the ConcurrentHashMap
        // over coarser locking on the whole cache instance.
        // We use a SettableReference (a second level of indirection) because
        // ConcurrentHashMap.putIfAbsent() never replaces the key's value, and if it were
        // a simple Reference we would not be able to reset its value after it has been cleared.
        // (And ConcurrentHashMap.put() always replaces the value, which we don't want either.)
        SettableReference<V> valueRef = map.get(key);
        V value;
        if(valueRef != null) {
            synchronized(valueRef) {
                if (valueRef.ref == null) {
                    // The cached value is a null.
                    return null;
                }

                value = valueRef.ref.get();
                if(value != null) {
                    // The cached value is non-null and has not been collected.
                    return value;
                } else {
                    // The cached value was non-null but has been evicted, its Reference
                    // cleared. Create and set a new value.
                    value = createInstance(key, data);
                    valueRef.setValue(value);
                    return value;
                }
            }
        } else /* valueRef == null */ {
            // We had never cached an instance for this key.
            value = createInstance(key, data);
            if (value == null) {
                return null;
            }
            valueRef = map.putIfAbsent(key, new SettableReference<V>(value));
            if(valueRef == null) {
                // Normal "put": Our new value is now cached.
                return value;
            } else {
                // Race condition: Another thread beat us to putting a SettableReference
                // into the map. Return its value, but just in case the garbage collector
                // was aggressive, we also offer our new instance for caching.
                return valueRef.setIfAbsent(value);
            }
        }
    }
    /**
     * Value type for cache items: Has a Reference which can be set
     * to a new value when the Reference has been cleared.
     * The SoftCache class sometimes accesses the ref field directly.
     *
     * @param <V> Cache instance value type
     */
    private static final class SettableReference<V> {
        SettableReference(V value) {
            setValue(value);
        }

        /**
         * Store the value. The value may be {@code null}.
         */
        void setValue(V value) {
            if (value == null) {
                ref = null;
            } else {
                ref = CachesReferenceFactory.createReference(value);
            }
        }

        /**
         * Set a new value iff the existing value has been cleared due to garbage collection.
         *
         * <p>If the Reference has been cleared, then this replaces it with a new Reference
         * for the new value and returns the new value; otherwise returns the current
         * Reference's value. If the old value was explicitly {@code null} then
         * {@code null} is returned and the new value is ignored.
         *
         * @param value Replacement value, for when the current reference has been cleared
         * @return The value that is held by the Reference, old or new
         */
        synchronized V setIfAbsent(V value) {
            if (ref == null) {
                // The old value is explicitly null.
                return null;
            }

            V oldValue = ref.get();
            if(oldValue == null) {
                // The old value was non-null, but has been cleared. The new value is taken.
                setValue(value);
                return value;
            } else {
                // The old value was and is non-null. The new value is ignored.
                return oldValue;
            }
        }

        // ref == null means the cached value is actually null
        // ref == a Reference means there is (or was) a cached, non-null, value.
        private Reference<V> ref;  // never null
    }
    private ConcurrentHashMap<K, SettableReference<V>> map =
        new ConcurrentHashMap<K, SettableReference<V>>();
}
