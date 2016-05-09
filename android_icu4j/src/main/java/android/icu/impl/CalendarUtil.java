/* GENERATED SOURCE. DO NOT MODIFY. */
/*
 *******************************************************************************
 * Copyright (C) 2009, International Business Machines Corporation and         *
 * others. All Rights Reserved.                                                *
 *******************************************************************************
 */
package android.icu.impl;

import java.util.MissingResourceException;

import android.icu.util.ULocale;
import android.icu.util.UResourceBundle;

/**
 * Calendar utilities.
 * 
 * Date/time format service classes in android.icu.text packages
 * sometimes need to access calendar internal APIs.  But calendar
 * classes are in android.icu.util package, so the package local
 * cannot be used.  This class is added in android.icu.impl
 * package for sharing some calendar internal code for calendar
 * and date format.
 * @hide Only a subset of ICU is exposed in Android
 */
public class CalendarUtil {

    private static SoftCache<String, String, ULocale> CALTYPE_CACHE =
            new SoftCache<String, String, ULocale>() {
                @Override
                protected String createInstance(String key, ULocale locale) {
                    return lookupCalendarType(locale);
                }
            };

    private static final String CALKEY = "calendar";
    private static final String DEFCAL = "gregorian";

    /**
     * Returns a calendar type for the given locale.
     * When the given locale has calendar keyword, the
     * value of calendar keyword is returned.  Otherwise,
     * the default calendar type for the locale is returned.
     * @param loc The locale
     * @return Calendar type string, such as "gregorian"
     */
    public static String getCalendarType(ULocale loc) {
        String calType = loc.getKeywordValue(CALKEY);
        if (calType != null) {
            return calType;
        }

        String baseLoc = loc.getBaseName();

        // Check the cache
        return CALTYPE_CACHE.getInstance(baseLoc, loc);
    }

    static String lookupCalendarType(ULocale loc) {
        // Canonicalize, so grandfathered variant will be transformed to keywords
        ULocale canonical = ULocale.createCanonical(loc.toString());
        String calType = canonical.getKeywordValue("calendar");

        if (calType == null) {
            // When calendar keyword is not available, use the locale's
            // region to get the default calendar type
            String region = canonical.getCountry();
            if (region.length() == 0) {
                ULocale fullLoc = ULocale.addLikelySubtags(canonical);
                region = fullLoc.getCountry();
            }

            // Read supplementalData to get the default calendar type for
            // the locale's region
            try {
                UResourceBundle rb = UResourceBundle.getBundleInstance(
                                        ICUResourceBundle.ICU_BASE_NAME,
                                        "supplementalData",
                                        ICUResourceBundle.ICU_DATA_CLASS_LOADER);
                UResourceBundle calPref = rb.get("calendarPreferenceData");
                UResourceBundle order;
                try {
                    order = calPref.get(region);
                } catch (MissingResourceException mre) {
                    // use "001" as fallback
                    order = calPref.get("001");
                }
                // the first calendar type is the default for the region
                calType = order.getString(0);
            } catch (MissingResourceException mre) {
                // fall through
            }

            if (calType == null) {
                // Use "gregorian" as the last resort fallback.
                calType = DEFCAL;
            }
        }
        return calType;
    }
}
