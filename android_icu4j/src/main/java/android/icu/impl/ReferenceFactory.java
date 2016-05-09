/* GENERATED SOURCE. DO NOT MODIFY. */
/*
 *******************************************************************************
 * Copyright (C) 2004-2016, International Business Machines Corporation and
 * others. All Rights Reserved.
 *******************************************************************************
 */
package android.icu.impl;

import java.lang.ref.Reference;
import java.lang.ref.SoftReference;

/**
 * Constructs {@link Reference} objects. Allows any GC-based caching strategy
 * to be pluggable. e.g. Soft Vs Weak Vs strong.
 * @hide Only a subset of ICU is exposed in Android
 */
public interface ReferenceFactory {
    /** An instance that creates {@link SoftReference} instances. */
    final ReferenceFactory SOFT_REFERENCE_FACTORY = new ReferenceFactory() {
        @Override
        public <X> SoftReference<X> create(X referent) {
            return new SoftReference<X>(referent);
        }
    };

    /** Constructs a {@link Reference} to the {@code referent}. */
    <X> Reference<X> create(X referent);
}

