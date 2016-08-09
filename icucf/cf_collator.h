#pragma once

#include <unicode/parseerr.h>
#include <map>
#include <cf_mutable_string.h>

namespace U_ICU_NAMESPACE {
    namespace cf {
        typedef enum {
            /** string a == string b */
            UCOL_EQUAL    = 0,
            /** string a > string b */
            UCOL_GREATER    = 1,
            /** string a < string b */
            UCOL_LESS    = -1
        } CollationResult ;
        
        
        /** Enum containing attribute values for controling collation behavior.
         * Here are all the allowable values. Not every attribute can take every value. The only
         * universal value is UCOL_DEFAULT, which resets the attribute value to the predefined
         * value for that locale
         * @stable ICU 2.0
         */
        typedef enum {
            /** accepted by most attributes */
            UCOL_DEFAULT = -1,
            
            /** Primary collation strength */
            UCOL_PRIMARY = 0,
            /** Secondary collation strength */
            UCOL_SECONDARY = 1,
            /** Tertiary collation strength */
            UCOL_TERTIARY = 2,
            /** Default collation strength */
            UCOL_DEFAULT_STRENGTH = UCOL_TERTIARY,
            UCOL_CE_STRENGTH_LIMIT,
            /** Quaternary collation strength */
            UCOL_QUATERNARY=3,
            /** Identical collation strength */
            UCOL_IDENTICAL=15,
            UCOL_STRENGTH_LIMIT,
            
            /** Turn the feature off - works for UCOL_FRENCH_COLLATION,
             UCOL_CASE_LEVEL, UCOL_HIRAGANA_QUATERNARY_MODE
             & UCOL_DECOMPOSITION_MODE*/
            UCOL_OFF = 16,
            /** Turn the feature on - works for UCOL_FRENCH_COLLATION,
             UCOL_CASE_LEVEL, UCOL_HIRAGANA_QUATERNARY_MODE
             & UCOL_DECOMPOSITION_MODE*/
            UCOL_ON = 17,
            
            /** Valid for UCOL_ALTERNATE_HANDLING. Alternate handling will be shifted */
            UCOL_SHIFTED = 20,
            /** Valid for UCOL_ALTERNATE_HANDLING. Alternate handling will be non ignorable */
            UCOL_NON_IGNORABLE = 21,
            
            /** Valid for UCOL_CASE_FIRST -
             lower case sorts before upper case */
            UCOL_LOWER_FIRST = 24,
            /** upper case sorts before lower case */
            UCOL_UPPER_FIRST = 25,
            
            UCOL_ATTRIBUTE_VALUE_COUNT
            
        } ColAttributeValue;
        
        
        
        typedef enum {
            /** Attribute for direction of secondary weights - used in Canadian French.
             * Acceptable values are UCOL_ON, which results in secondary weights
             * being considered backwards and UCOL_OFF which treats secondary
             * weights in the order they appear.
             * @stable ICU 2.0
             */
            UCOL_FRENCH_COLLATION,
            /** Attribute for handling variable elements.
             * Acceptable values are UCOL_NON_IGNORABLE (default)
             * which treats all the codepoints with non-ignorable
             * primary weights in the same way,
             * and UCOL_SHIFTED which causes codepoints with primary
             * weights that are equal or below the variable top value
             * to be ignored on primary level and moved to the quaternary
             * level.
             * @stable ICU 2.0
             */
            UCOL_ALTERNATE_HANDLING,
            /** Controls the ordering of upper and lower case letters.
             * Acceptable values are UCOL_OFF (default), which orders
             * upper and lower case letters in accordance to their tertiary
             * weights, UCOL_UPPER_FIRST which forces upper case letters to
             * sort before lower case letters, and UCOL_LOWER_FIRST which does
             * the opposite.
             * @stable ICU 2.0
             */
            UCOL_CASE_FIRST,
            /** Controls whether an extra case level (positioned before the third
             * level) is generated or not. Acceptable values are UCOL_OFF (default),
             * when case level is not generated, and UCOL_ON which causes the case
             * level to be generated. Contents of the case level are affected by
             * the value of UCOL_CASE_FIRST attribute. A simple way to ignore
             * accent differences in a string is to set the strength to UCOL_PRIMARY
             * and enable case level.
             * @stable ICU 2.0
             */
            UCOL_CASE_LEVEL,
            /** Controls whether the normalization check and necessary normalizations
             * are performed. When set to UCOL_OFF (default) no normalization check
             * is performed. The correctness of the result is guaranteed only if the
             * input data is in so-called FCD form (see users manual for more info).
             * When set to UCOL_ON, an incremental check is performed to see whether
             * the input data is in the FCD form. If the data is not in the FCD form,
             * incremental NFD normalization is performed.
             * @stable ICU 2.0
             */
            UCOL_NORMALIZATION_MODE,
            /** An alias for UCOL_NORMALIZATION_MODE attribute.
             * @stable ICU 2.0
             */
            UCOL_DECOMPOSITION_MODE = UCOL_NORMALIZATION_MODE,
            /** The strength attribute. Can be either UCOL_PRIMARY, UCOL_SECONDARY,
             * UCOL_TERTIARY, UCOL_QUATERNARY or UCOL_IDENTICAL. The usual strength
             * for most locales (except Japanese) is tertiary.
             *
             * Quaternary strength
             * is useful when combined with shifted setting for alternate handling
             * attribute and for JIS X 4061 collation, when it is used to distinguish
             * between Katakana and Hiragana.
             * Otherwise, quaternary level
             * is affected only by the number of non-ignorable code points in
             * the string.
             *
             * Identical strength is rarely useful, as it amounts
             * to codepoints of the NFD form of the string.
             * @stable ICU 2.0
             */
            UCOL_STRENGTH,
    #ifndef U_HIDE_DEPRECATED_API
            /** When turned on, this attribute positions Hiragana before all
             * non-ignorables on quaternary level This is a sneaky way to produce JIS
             * sort order.
             *
             * This attribute was an implementation detail of the CLDR Japanese tailoring.
             * Since ICU 50, this attribute is not settable any more via API functions.
             * Since CLDR 25/ICU 53, explicit quaternary relations are used
             * to achieve the same Japanese sort order.
             *
             * @deprecated ICU 50 Implementation detail, cannot be set via API, was removed from implementation.
             */
            UCOL_HIRAGANA_QUATERNARY_MODE = UCOL_STRENGTH + 1,
    #endif  /* U_HIDE_DEPRECATED_API */
            /**
             * When turned on, this attribute makes
             * substrings of digits sort according to their numeric values.
             *
             * This is a way to get '100' to sort AFTER '2'. Note that the longest
             * digit substring that can be treated as a single unit is
             * 254 digits (not counting leading zeros). If a digit substring is
             * longer than that, the digits beyond the limit will be treated as a
             * separate digit substring.
             *
             * A "digit" in this sense is a code point with General_Category=Nd,
             * which does not include circled numbers, roman numerals, etc.
             * Only a contiguous digit substring is considered, that is,
             * non-negative integers without separators.
             * There is no support for plus/minus signs, decimals, exponents, etc.
             *
             * @stable ICU 2.8
             */
            UCOL_NUMERIC_COLLATION = UCOL_STRENGTH + 2, 
            /**
             * The number of UColAttribute constants.
             * @stable ICU 2.0
             */
            UCOL_ATTRIBUTE_COUNT
        } ColAttribute;


        
        class Collator {
            std::map <ColAttribute, ColAttributeValue> attributes;
            cf_Locale locale;
        public:
            Collator(const char *loc, UErrorCode *status)
                : locale(loc) {
                
            }
            Collator(const UChar        *rules,
                     int32_t             rulesLength,
                     ColAttributeValue   normalizationMode,
                     ColAttributeValue   strength,
                     UParseError         *parseError,
                     UErrorCode          *status) {

                attributes[UCOL_NORMALIZATION_MODE] = normalizationMode;
                attributes[UCOL_STRENGTH] = strength;
                
            }

            Collator* safeClone(void            *stackBuffer,
                               int32_t         *pBufferSize,
                               UErrorCode      *status) const {
                return new Collator(nullptr, nullptr);
            }
            
            void close() {
                delete this;
            }
            
             const UChar* getRules(int32_t            *length) const {
                 return nullptr;
             }

            int32_t getSortKey(const  UChar *source,
                               int32_t  sourceLength,
                               uint8_t  *result,
                               int32_t  resultLength) const {
                return 0;
            }
            void  setAttribute(ColAttribute attr, ColAttributeValue value, UErrorCode *status) {
                attributes[attr] = value;
            }

            ColAttributeValue getAttribute(ColAttribute attr, UErrorCode *status) const {
                try {
                    return attributes.at(attr);
                } catch (...) {
                    *status = U_MISSING_RESOURCE_ERROR;
                    return UCOL_DEFAULT;
                }
            }

            /*
             TODO: APPLE has failed to pass lcale compsrison for peach fr-fr locale push them via bug tracker
             auto a = CFSTR("p\u00E9ch\u00E9");
             auto b = CFSTR("p\u00EAche");
             auto wholeString = CFRangeMake(0, CFStringGetLength(a));
             auto locfr = CFLocaleCreate ( nullptr, CFSTR("fr-FR"));
             auto locen = CFLocaleCreate ( nullptr, CFSTR("en-US"));
             
             printf("result=%d", CFStringCompareWithOptionsAndLocale ( a, b,  wholeString, kCFCompareCaseInsensitive, locfr ));
             printf("result=%d", CFStringCompareWithOptionsAndLocale ( a, b,  wholeString, kCFCompareCaseInsensitive, locen ));
             */
            
            CollationResult strcoll(const    UChar        *source,
                                   int32_t            sourceLength,
                                   const    UChar        *target,
                                   int32_t            targetLength) const {

                cf_String cfSource(source, sourceLength);
                cf_String cfTarget(target, targetLength);

                bool isNormalizedComparison = false;
                
                try {
                    isNormalizedComparison = attributes.at(UCOL_NORMALIZATION_MODE) == UCOL_ON;
                } catch (...) {}
                
                if (isNormalizedComparison)
                {
                    ::cf::MutableString mutSource(cfSource);
                    ::cf::MutableString mutTarget(cfTarget);
                    mutSource.normalize(kCFStringNormalizationFormD);
                    mutTarget.normalize(kCFStringNormalizationFormD);
                    
                    return (CollationResult)mutSource.compare(mutTarget, kCFCompareCaseInsensitive, locale);
                }

                return (CollationResult)cfSource.compare(cfTarget, kCFCompareCaseInsensitive, locale);
            }
        };
    }
}