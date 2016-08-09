#pragma once

#include <cf_collation_elements.h>
#include <cf_collator.h>
#include <unicode/parseerr.h>

//remap cf namespace to icu4c functions

#define  CF_UCOL(a) icu::cf::UCol::a

#undef  ucol_open
#define ucol_open CF_UCOL(open)

#undef  ucol_openElements
#define ucol_openElements CF_UCOL(openElements)

#undef  ucol_setText
#define ucol_setText CF_UCOL(setText)

#undef  ucol_strcoll
#define ucol_strcoll CF_UCOL(strcoll)

#undef  ucol_close
#define ucol_close CF_UCOL(close)

#undef  ucol_closeElements
#define ucol_closeElements CF_UCOL(closeElements)

#undef  ucol_getMaxExpansion
#define ucol_getMaxExpansion CF_UCOL(getMaxExpansion)

#undef  ucol_getOffset
#define ucol_getOffset CF_UCOL(getOffset)

#undef  ucol_setOffset
#define ucol_setOffset CF_UCOL(setOffset)

#undef  ucol_safeClone
#define ucol_safeClone CF_UCOL(safeClone)

#undef  ucol_reset
#define ucol_reset CF_UCOL(reset)

#undef  ucol_getRules
#define ucol_getRules CF_UCOL(getRules)

#undef  ucol_getSortKey
#define ucol_getSortKey CF_UCOL(getSortKey)

#undef  ucol_next
#define ucol_next CF_UCOL(next)

#undef  ucol_previous
#define ucol_previous CF_UCOL(previous)

#undef  ucol_getAttribute
#define ucol_getAttribute CF_UCOL(getAttribute)

#undef  ucol_setAttribute
#define ucol_setAttribute CF_UCOL(setAttribute)

#undef  ucol_openRules
#define ucol_openRules CF_UCOL(openRules)


//types
#define UCollator icu::cf::Collator
#define UCollationElements icu::cf::CollationElements
#define UCollationResult icu::cf::CollationResult
#define UColAttributeValue icu::cf::ColAttributeValue
#define UColAttribute icu::cf::ColAttribute
#define UCollationStrength icu::cf::ColAttributeValue

namespace U_ICU_NAMESPACE {
    namespace cf {
        class UCol {
        public:
            
            static void  setAttribute(Collator *coll, ColAttribute attr, ColAttributeValue value, UErrorCode *status) {
                coll->setAttribute(attr, value, status);
            }
            
            static ColAttributeValue getAttribute(const Collator *coll, ColAttribute attr, UErrorCode *status){
                return coll->getAttribute(attr, status);
            }
            
            static int32_t previous(CollationElements *elems, UErrorCode *status) {
                return elems->previous(status);
            }
            
            static int32_t next(CollationElements *elems, UErrorCode *status) {
                return elems->next(status);
            }
            static int32_t getSortKey(const  Collator *coll,
                                      const  UChar *source,
                                      int32_t  sourceLength,
                                      uint8_t  *result,
                                      int32_t  resultLength) {
                return coll->getSortKey(source, sourceLength, result, resultLength);
            }
            
            static const UChar* getRules(const Collator    *coll,
                                         int32_t *length) {
                return coll->getRules(length);
                
            }
            static Collator* safeClone(const Collator *coll,
                                       void           *stackBuffer,
                                       int32_t        *pBufferSize,
                                       UErrorCode     *status) {
                return coll->safeClone(stackBuffer, pBufferSize, status);
            }
            
            static void setOffset(CollationElements *elems,
                                  int32_t             offset,
                                  UErrorCode         *status){
                elems->setOffset(offset, status);
            }
            
            static  int32_t getOffset(const CollationElements *elems) {
                return elems->getOffset();
            }
            
            static int32_t getMaxExpansion(const CollationElements *elems, int32_t order) {
                return elems->getMaxExpansion(order);
            }
            static CollationResult strcoll(const    Collator    *coll,
                                           const    UChar        *source,
                                           int32_t            sourceLength,
                                           const    UChar        *target,
                                           int32_t            targetLength) {
                return coll->strcoll(source, sourceLength, target, targetLength);
            }
            static void setText(CollationElements *elems,
                                const UChar        *text,
                                int32_t            textLength,
                                UErrorCode         *status) {
                return elems->setText(text, textLength, status);
            }
            static Collator* open(const char *loc, UErrorCode *status) {
                return new Collator(loc, status);
            }
            static  Collator* openRules(const UChar        *rules,
                           int32_t            rulesLength,
                           UColAttributeValue normalizationMode,
                           UCollationStrength strength,
                           UParseError        *parseError,
                           UErrorCode         *status) {

                return new Collator(rules, rulesLength, normalizationMode, strength, parseError, status);
            }
            
            static void close(Collator *coll) {
                coll->close();
            }
            static CollationElements* openElements(const Collator  *coll,
                                                   const UChar      *text,
                                                   int32_t    textLength,
                                                   UErrorCode *status) {
                return new CollationElements(text, textLength, status);
            }
            static void closeElements(CollationElements *elems) {
                elems->close();
            }
            static void reset(CollationElements *elems) {
                elems->reset();
            }
        };
    }

}