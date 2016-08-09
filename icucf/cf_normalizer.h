#pragma once

#include <cf_normalization_mode.h>
#include <cf_mutable_string.h>
namespace U_ICU_NAMESPACE {
    namespace cf {
        class Normalizer {
        public:
            
            //-------------------------------------------------------------------------
            // Static utility methods
            //-------------------------------------------------------------------------
            
            /**
             * Normalizes a <code>UnicodeString</code> according to the specified normalization mode.
             * This is a wrapper for unorm_normalize(), using UnicodeString's.
             *
             * The <code>options</code> parameter specifies which optional
             * <code>Normalizer</code> features are to be enabled for this operation.
             *
             * @param source    the input string to be normalized.
             * @param mode      the normalization mode
             * @param options   the optional features to be enabled (0 for no options)
             * @param result    The normalized string (on output).
             * @param status    The error code.
             * @stable ICU 2.0
             */
            static void U_EXPORT2 normalize(const UnicodeString& source,
                                            UNormalizationMode mode,
                                            int32_t options,
                                            UnicodeString& result,
                                            UErrorCode &status) {

                CFStringNormalizationForm normalizationForm;
                if (!convertNormalizerMode(mode, normalizationForm)) {
                    result = source;
                    return ;
                }

                ::cf::MutableString str (source);
                result = str.normalize(normalizationForm);
            }
            
            
            /**
             * Test if a string is in a given normalization form.
             * This is semantically equivalent to source.equals(normalize(source, mode)) .
             *
             * Unlike unorm_quickCheck(), this function returns a definitive result,
             * never a "maybe".
             * For NFD, NFKD, and FCD, both functions work exactly the same.
             * For NFC and NFKC where quickCheck may return "maybe", this function will
             * perform further tests to arrive at a TRUE/FALSE result.
             *
             * @param src        String that is to be tested if it is in a normalization format.
             * @param mode       Which normalization form to test for.
             * @param errorCode  ICU error code in/out parameter.
             *                   Must fulfill U_SUCCESS before the function call.
             * @return Boolean value indicating whether the source string is in the
             *         "mode" normalization form.
             *
             * @see quickCheck
             * @stable ICU 2.2
             */
            static inline UBool
                isNormalized(const UnicodeString &source, UNormalizationMode mode, UErrorCode &errorCode) {
                    CFStringNormalizationForm normalizationForm;
                    if (!convertNormalizerMode(mode, normalizationForm)) {
                        return TRUE;
                    }
                    
                    ::cf::MutableString str (source);
                    return str.isNormalized(normalizationForm) ? TRUE : FALSE;
            }
        private:
            //if no normalization required returned false
            static bool convertNormalizerMode ( UNormalizationMode mode
                                       , CFStringNormalizationForm &normalizationForm) {
                
                switch (mode) {
                        
                    case UNORM_NFD :
                        normalizationForm = kCFStringNormalizationFormD;
                        break;
                    case UNORM_NFKD :
                        normalizationForm = kCFStringNormalizationFormKD;
                        break;
                        /** Canonical decomposition followed by canonical composition. @stable ICU 2.0 */
                    case UNORM_NFC :
                        normalizationForm = kCFStringNormalizationFormC;
                        break;
                        /** Compatibility decomposition followed by canonical composition. @stable ICU 2.0 */
                    case UNORM_NFKC:
                        normalizationForm = kCFStringNormalizationFormKC;
                        break;
                        
                        /** "Fast C or D" form. @stable ICU 2.0 */ // not supported by CF
                    case UNORM_FCD:
                    case UNORM_NONE:
                    default:
                        return false;
                }
                return true;

            }
        };
    }
}

#define Normalizer cf::Normalizer