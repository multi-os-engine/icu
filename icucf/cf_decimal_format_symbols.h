#pragma once

class cf_DecimalFormatSymbols {
public:
    /**
     * Constants for specifying a number format symbol.
     * @stable ICU 2.0
     */
    enum ENumberFormatSymbol {
        /** The decimal separator */
        kDecimalSeparatorSymbol,
        /** The grouping separator */
        kGroupingSeparatorSymbol,
        /** The pattern separator */
        kPatternSeparatorSymbol,
        /** The percent sign */
        kPercentSymbol,
        /** Zero*/
        kZeroDigitSymbol,
        /** Character representing a digit in the pattern */
        kDigitSymbol,
        /** The minus sign */
        kMinusSignSymbol,
        /** The plus sign */
        kPlusSignSymbol,
        
        /** The currency symbol */
        kCurrencySymbol,
        
        /** The currency symbol */
        kCurrencyCode,
        
        /** The international currency symbol */
        kIntlCurrencySymbol,
        /** The monetary separator */
        kMonetarySeparatorSymbol,
        /** The exponential symbol */
        kExponentialSymbol,
        /** Per mill symbol - replaces kPermillSymbol */
        kPerMillSymbol,
        /** Escape padding character */
        kPadEscapeSymbol,
        /** Infinity symbol */
        kInfinitySymbol,
        /** Nan symbol */
        kNaNSymbol,
        /** Significant digit symbol
         * @stable ICU 3.0 */
        kSignificantDigitSymbol,
        /** The monetary grouping separator
         * @stable ICU 3.6
         */
        kMonetaryGroupingSeparatorSymbol,
        /** One
         * @stable ICU 4.6
         */
        kOneDigitSymbol,
        /** Two
         * @stable ICU 4.6
         */
        kTwoDigitSymbol,
        /** Three
         * @stable ICU 4.6
         */
        kThreeDigitSymbol,
        /** Four
         * @stable ICU 4.6
         */
        kFourDigitSymbol,
        /** Five
         * @stable ICU 4.6
         */
        kFiveDigitSymbol,
        /** Six
         * @stable ICU 4.6
         */
        kSixDigitSymbol,
        /** Seven
         * @stable ICU 4.6
         */
        kSevenDigitSymbol,
        /** Eight
         * @stable ICU 4.6
         */
        kEightDigitSymbol,
        /** Nine
         * @stable ICU 4.6
         */
        kNineDigitSymbol,
        /** count symbol constants */
        kFormatSymbolCount
    };
    cf_AutoRef<CFNumberFormatterRef> formatter;
    
    cf_DecimalFormatSymbols(const cf_Locale & locale, UErrorCode& status) {
        
         formatter = CFNumberFormatterCreate ( nullptr, locale, kCFNumberFormatterNoStyle);

    }

    inline void setSymbol(ENumberFormatSymbol symbol, cf_String value) {
       CFNumberFormatterSetProperty(formatter, ENumberFormatSymbolToCFNumberFormatterProperty(symbol), value);
    }
    inline UnicodeString getSymbol(ENumberFormatSymbol symbol) const {
        return getSymbolTmpl<UnicodeString>(symbol);
    }
    
    template<class T>
    inline T getSymbolTmpl(ENumberFormatSymbol symbol) const {
        cf_String formatString(nullptr);
        switch(symbol) {
            case kZeroDigitSymbol:
                formatString = getZeroSymbol();
                break;
            case kPatternSeparatorSymbol :
                formatString = getPatternSeparator();
                break;
            default:
                formatString = (CFStringRef)CFNumberFormatterCopyProperty(formatter, ENumberFormatSymbolToCFNumberFormatterProperty(symbol));
                break;
        }
        return formatString;
    }
private:
    //WA since CF always returns null pointer to zero-symbol request
    cf_String getZeroSymbol () const {
        const int nZero = 0;
        cf_String str = CFNumberFormatterCreateStringWithValue(nullptr, formatter, kCFNumberIntType, &nZero);
        return str;
    }
    //TODO: WA Corefoundation doesnot provide API for that, however checked using jdk that only ; separator exists, so we are using it
    cf_String getPatternSeparator () const {
        return CFSTR(";");
    }
    CFStringRef ENumberFormatSymbolToCFNumberFormatterProperty (ENumberFormatSymbol symbol) const {
        switch(symbol) {
            case kPercentSymbol:
                return kCFNumberFormatterPercentSymbol;
            case kZeroDigitSymbol:
                return kCFNumberFormatterZeroSymbol;
            case kMinusSignSymbol:
                return kCFNumberFormatterMinusSign;
            case kPlusSignSymbol:
                return kCFNumberFormatterPlusSign;
            case kCurrencySymbol:
                return kCFNumberFormatterCurrencySymbol;
            case kIntlCurrencySymbol:
                return kCFNumberFormatterInternationalCurrencySymbol;
            case kExponentialSymbol:
                return kCFNumberFormatterExponentSymbol;
            case kPerMillSymbol:
                return kCFNumberFormatterPerMillSymbol;
            case kInfinitySymbol:
                return kCFNumberFormatterInfinitySymbol;
            case kNaNSymbol:
                return kCFNumberFormatterNaNSymbol;


            case kDecimalSeparatorSymbol:
                return kCFNumberFormatterDecimalSeparator;
            case kGroupingSeparatorSymbol :
                return kCFNumberFormatterGroupingSeparator;
            //todo: questionable not exact mapping, monetary instead of currency
            case kMonetaryGroupingSeparatorSymbol:
                return kCFNumberFormatterCurrencyGroupingSeparator;
            case kMonetarySeparatorSymbol :
                return kCFNumberFormatterCurrencyDecimalSeparator;
            case kPatternSeparatorSymbol: ///----- used directly by JNI Localedatainit so have to return something
                return kCFNumberFormatterGroupingSeparator;
                
                /*TODO: looks  missed in COREFOUNDATION
                case kPadEscapeSymbol,
                case kDigitSymbol,
                case kSignificantDigitSymbol,
                case kOneDigitSymbol,
                case kTwoDigitSymbol,
                case kThreeDigitSymbol,
                case kFourDigitSymbol,
                case kFiveDigitSymbol,
                case kSixDigitSymbol,
                case kSevenDigitSymbol,
                case kEightDigitSymbol,
                case kNineDigitSymbol,*/
        }
        return kCFNumberFormatterGroupingSeparator;

    }
};