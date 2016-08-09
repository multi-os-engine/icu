#pragma once

//converter specific functions
#define  CF_UCNV(a) ::icu::cf::UCnv::a

//general types
#define  ICU_CF(a) icu::cf::a


#define UCNV_UNASSIGNED ICU_CF(UNASSIGNED)
#define UCNV_ILLEGAL ICU_CF(ILLEGAL)
#define UCNV_IRREGULAR ICU_CF(IRREGULAR)
#define UCNV_RESET ICU_CF(RESET)
#define UCNV_CLOSE ICU_CF(CLOSE)
#define UCNV_CLONE ICU_CF(CLONE)
#define UCNV_ROUNDTRIP_SET ICU_CF(ROUNDTRIP_SET)
#define UCNV_ROUNDTRIP_AND_FALLBACK_SET ICU_CF(ROUNDTRIP_AND_FALLBACK_SET)
#define UCNV_SET_COUNT ICU_CF(SET_COUNT)

#define UConverterToUCallback ICU_CF(ConverterToUCallback)
#define UConverterFromUCallback ICU_CF(ConverterFromUCallback)
#define UConverterToUnicodeArgs ICU_CF(ConverterToUnicodeArgs)
#define UConverterCallbackReason ICU_CF(ConverterCallbackReason)
#define UConverterFromUnicodeArgs ICU_CF(ConverterFromUnicodeArgs)

namespace U_ICU_NAMESPACE {
    
    namespace cf {
        
        class UConverter;
        
        /**
         * Selectors for Unicode sets that can be returned by ucnv_getUnicodeSet().
         * @see ucnv_getUnicodeSet
         * @stable ICU 2.6
         */
        typedef enum UConverterUnicodeSet {
            /** Select the set of roundtrippable Unicode code points. @stable ICU 2.6 */
            ROUNDTRIP_SET,
            /** Select the set of Unicode code points with roundtrip or fallback mappings. @stable ICU 4.0 */
            ROUNDTRIP_AND_FALLBACK_SET,
            /** Number of UConverterUnicodeSet selectors. @stable ICU 2.6 */
            SET_COUNT
        } ConverterUnicodeSet;
        
        
        
        typedef enum {
            UNASSIGNED = 0,  /**< The code point is unassigned.
                              The error code U_INVALID_CHAR_FOUND will be set. */
            ILLEGAL = 1,     /**< The code point is illegal. For example,
                              \\x81\\x2E is illegal in SJIS because \\x2E
                              is not a valid trail byte for the \\x81
                              lead byte.
                              Also, starting with Unicode 3.0.1, non-shortest byte sequences
                              in UTF-8 (like \\xC1\\xA1 instead of \\x61 for U+0061)
                              are also illegal, not just irregular.
                              The error code U_ILLEGAL_CHAR_FOUND will be set. */
            IRREGULAR = 2,   /**< The codepoint is not a regular sequence in
                              the encoding. For example, \\xED\\xA0\\x80..\\xED\\xBF\\xBF
                              are irregular UTF-8 byte sequences for single surrogate
                              code points.
                              The error code U_INVALID_CHAR_FOUND will be set. */
            RESET = 3,       /**< The callback is called with this reason when a
                              'reset' has occured. Callback should reset all
                              state. */
            CLOSE = 4,        /**< Called when the converter is closed. The
                               callback should release any allocated memory.*/
            CLONE = 5         /**< Called when ucnv_safeClone() is called on the
                               converter. the pointer available as the
                               'context' is an alias to the original converters'
                               context pointer. If the context must be owned
                               by the new converter, the callback must clone
                               the data and call ucnv_setFromUCallback
                               (or setToUCallback) with the correct pointer.
                               @stable ICU 2.2
                               */
        } ConverterCallbackReason;
        
        
        /**
         * The structure for the fromUnicode callback function parameter.
         * @stable ICU 2.0
         */
        typedef struct {
            uint16_t size;              /**< The size of this struct. @stable ICU 2.0 */
            UBool flush;                /**< The internal state of converter will be reset and data flushed if set to TRUE. @stable ICU 2.0    */
            UConverter *converter;      /**< Pointer to the converter that is opened and to which this struct is passed as an argument. @stable ICU 2.0  */
            const UChar *source;        /**< Pointer to the source source buffer. @stable ICU 2.0    */
            const UChar *sourceLimit;   /**< Pointer to the limit (end + 1) of source buffer. @stable ICU 2.0    */
            char *target;               /**< Pointer to the target buffer. @stable ICU 2.0    */
            const char *targetLimit;    /**< Pointer to the limit (end + 1) of target buffer. @stable ICU 2.0     */
            int32_t *offsets;           /**< Pointer to the buffer that recieves the offsets. *offset = blah ; offset++;. @stable ICU 2.0  */
        } ConverterFromUnicodeArgs;
        
        
        /**
         * The structure for the toUnicode callback function parameter.
         * @stable ICU 2.0
         */
        typedef struct {
            uint16_t size;              /**< The size of this struct   @stable ICU 2.0 */
            UBool flush;                /**< The internal state of converter will be reset and data flushed if set to TRUE. @stable ICU 2.0   */
            UConverter *converter;      /**< Pointer to the converter that is opened and to which this struct is passed as an argument. @stable ICU 2.0 */
            const char *source;         /**< Pointer to the source source buffer. @stable ICU 2.0    */
            const char *sourceLimit;    /**< Pointer to the limit (end + 1) of source buffer. @stable ICU 2.0    */
            UChar *target;              /**< Pointer to the target buffer. @stable ICU 2.0    */
            const UChar *targetLimit;   /**< Pointer to the limit (end + 1) of target buffer. @stable ICU 2.0     */
            int32_t *offsets;           /**< Pointer to the buffer that recieves the offsets. *offset = blah ; offset++;. @stable ICU 2.0  */
        } ConverterToUnicodeArgs;
        
        
        typedef void( * ConverterToUCallback)(const void *context, ConverterToUnicodeArgs *args, const char *codeUnits, int32_t length, ConverterCallbackReason reason, UErrorCode *pErrorCode);
        
        typedef void( * ConverterFromUCallback)(const void *context, ConverterFromUnicodeArgs *args, const UChar *codeUnits, int32_t length, UChar32 codePoint, ConverterCallbackReason reason, UErrorCode *pErrorCode);
    }
}