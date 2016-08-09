#pragma once

#include <cf_decimal_format.h>

class cf_NumberFormat {
public:
    /**
     * Creates the specified decimal format style of the desired locale.
     * @param desiredLocale    the given locale.
     * @param style            the given style.
     * @param errorCode        Output param filled with success/failure status.
     * @return                 A new NumberFormat instance.
     * @stable ICU 4.8
     */
    static cf_DecimalFormat * createInstance(const cf_Locale& desiredLocale,
                                             UNumberFormatStyle style,
                                             UErrorCode& errorCode) {
        return new cf_DecimalFormat(desiredLocale, style, errorCode);
    }
};