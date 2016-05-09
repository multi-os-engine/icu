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
 * An internal class used by ICU to construct Reference objects for general
 * caching.
 * @hide Only a subset of ICU is exposed in Android
 */
public class CachesReferenceFactory {

    /**
     * The current {@link ReferenceFactory}. Defaults to
     * {@link ReferenceFactory.SOFT_REFERENCE_FACTORY}.
     */
    private static volatile ReferenceFactory referenceFactory =
            ReferenceFactory.SOFT_REFERENCE_FACTORY;

    /**
     * Replaces the {@link ReferenceFactory} used to create {@link Reference} for
     * caches.
     */
    public static void setReferenceFactory(ReferenceFactory referenceFactory) {
        if (referenceFactory == null) {
          throw new NullPointerException("referenceFactory == null");
        }
        CachesReferenceFactory.referenceFactory = referenceFactory;
    }

    /**
     * Creates a {@link Reference} to the supplied value using the installed
     * {@link ReferenceFactory}.
     */
    public static <X> Reference<X> createReference(X value) {
        return referenceFactory.create(value);
    }

    // All methods are static.
    private CachesReferenceFactory() {}
}

