#pragma once
#include <cf_string.h>

/**
 * This indicates an error has occured during processing or if no more CEs is
 * to be returned.
 * @stable ICU 2.0
 */
#ifndef UCOL_NULLORDER
    #define UCOL_NULLORDER        ((int32_t)0xFFFFFFFF)
#endif
namespace U_ICU_NAMESPACE {
    namespace cf {
        class CollationElements {
            cf_String text;
            int32_t offset;
        public:
            CollationElements(const UChar *text, int32_t textLength, UErrorCode *status)
                : text((UniChar*)text, textLength)
                , offset(0) {
            }
            void setText(const UChar *text, int32_t textLength, UErrorCode *status) {
            }
            void close() {
                
            }
            void reset () {
                
            }
            int32_t getMaxExpansion(int32_t order) const {
                return 0;
            }
            int32_t getOffset() const {
                return offset;
            }
            void setOffset(int32_t offset, UErrorCode *status) {
                
            }
            int32_t next(UErrorCode *status) {
                if (offset == text.length() ) {
                    return UCOL_NULLORDER;
                }
                return ++offset;
            }
            int32_t previous(UErrorCode *status) {
                if (offset == 0) {
                    return UCOL_NULLORDER;
                }
                return --offset;
            }
        };
    }
}