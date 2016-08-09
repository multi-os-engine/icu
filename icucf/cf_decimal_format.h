#pragma once

#import <CoreFoundation/CoreFoundation.h>
#include <cf_string.h>

class cf_DecimalFormat {
    UnicodeString patternString;
public:
    cf_DecimalFormat(const cf_Locale& desiredLocale,
                     UNumberFormatStyle style,
                     UErrorCode& errorCode) {
        cf_AutoRef<CFNumberFormatterRef> numFormatter = CFNumberFormatterCreate(
            nullptr, desiredLocale, UNumberFormatStyleToCFNumberFormatStyle(style, errorCode));
        
        if (U_FAILURE(errorCode)) {
            return;
        }

        patternString = cf_String::fromCFGetFunction(CFNumberFormatterGetFormat(numFormatter));
    }
    
    //------------------------------------------------------------------------------
    // Emits the pattern of this DecimalFormat instance.
    
    UnicodeString& toPattern(UnicodeString& result) const
    {
        result = patternString;
        return result;
    }
private:
    CFNumberFormatterStyle UNumberFormatStyleToCFNumberFormatStyle(UNumberFormatStyle style, UErrorCode& errorCode) {
        switch(style) {
            case UNUM_DECIMAL :
                return kCFNumberFormatterDecimalStyle;
            case UNUM_CURRENCY :
                return kCFNumberFormatterCurrencyStyle;
            case UNUM_PERCENT :
                return kCFNumberFormatterPercentStyle;
            case UNUM_SCIENTIFIC :
                return kCFNumberFormatterScientificStyle;
            case UNUM_SPELLOUT :
                return kCFNumberFormatterSpellOutStyle;
 
            case UNUM_ORDINAL :
            case UNUM_DURATION :
            case UNUM_NUMBERING_SYSTEM :
            case UNUM_PATTERN_RULEBASED :
            case UNUM_CURRENCY_ISO :
            case UNUM_CURRENCY_PLURAL :
            default:
                errorCode = U_INVALID_FORMAT_ERROR;
                //no corresponding CoreFoundation Style
                return kCFNumberFormatterNoStyle;
        }

    }
    
};
