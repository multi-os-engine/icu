#pragma once
#include <cf_break_iterator.h>

namespace U_ICU_NAMESPACE {
    namespace cf {
        class BreakIteratorCoreFoundation  : public BreakIterator {
            cf_Locale where;
            CFOptionFlags options;
            //cf_String str;
            cf_AutoRef<CFStringTokenizerRef> tokenizer;
            CFRange range;
            int32_t currentPosition;
            bool currentIsDirty;
            UText text;

        public:
            
            BreakIteratorCoreFoundation (const cf_Locale& where, CFOptionFlags options)
                : where(where), options(options), currentPosition(), currentIsDirty() {
            }
            
            virtual BreakIteratorCoreFoundation* clone(void) const {

                auto bi = new BreakIteratorCoreFoundation(where, options);
                bi->initialize(text);
                if (currentIsDirty) {
                    bi->last();
                } else {
                    bi->preceding(currentPosition);
                }
                return bi;
            }
            
            virtual void  setText(UText *text, UErrorCode &status) {
                if (nullptr == text) {
                    return;
                }
                initialize(*text);
            }
            
            virtual BreakIteratorCoreFoundation &refreshInputText(UText *input, UErrorCode &status) {
                // TODO: this used in ICU4C that never copies input text, while CF works differently, we are creating copy of the text
                // thus this notifications not necessary untill we start using external data for CFStringRef
                refresh(*input);
                return *this;
            }
            
            virtual int32_t first(void) {
                return preceding(0);
            }
            
            virtual int32_t last(void)  {
                currentIsDirty = true;
                currentPosition = range.length;
                return current();
            }
            
            virtual int32_t previous(void)  {
                
                if (currentIsDirty) {
                    return preceding(currentPosition - 1);
                }
                
                if (current() == 0 ) {
                    return cf::DONE;
                }
                
                return preceding(current() - 1);
            }
            
            
            virtual int32_t next(void)  {
                if (currentIsDirty) {
                    return cf::DONE;
                }
                if (kCFStringTokenizerTokenNone == CFStringTokenizerAdvanceToNextToken(tokenizer)) {
                    //CF do not return last to end of text
                    return last();
                }
                
                updateCurrentPos();
                return current();
            }
            
            virtual int32_t current(void) const   {
                return currentPosition;
            }
            
            virtual int32_t preceding(int32_t offset) {
                int32_t orig;
                if (!currentIsDirty) {
                    orig = current();
                }
                
                if (!gotoIndex(offset)) {
                    if (!currentIsDirty) {
                        gotoIndex(orig);
                    }
                    return cf::DONE;
                }
                updateCurrentPos();
                return current();
            }
            virtual int32_t following(int32_t offset)   {
                
                if (cf::DONE == preceding(offset)) {
                    return cf::DONE;
                }
                
                return next();
            }
            virtual UBool isBoundary(int32_t offset)   {
                int32_t orig;
                auto dirty = currentIsDirty;
                
                if (!currentIsDirty) {
                    orig = current();
                }
                
                if (!gotoIndex(offset)) {
                    return FALSE;
                }
                
                updateCurrentPos();
                
                UBool result = current() == offset ? TRUE : FALSE;
                
                if (!dirty) {
                    gotoIndex(orig);
                } else {
                    last();
                }
                
                return result;
            };
        private:
            void initialize(const UText & input) {
                text = input;
                auto str = cf_String(UnicodeString((UChar*)input.context, input.chunkLength));
                range = CFRangeMake(0, str.length());
                tokenizer = CFStringTokenizerCreate(nullptr, str, range, options, where);
                currentPosition = 0;
                currentIsDirty = false;
            }
            
            void refresh(UText & input) {
                text = input;
               /* str = cf_String(UnicodeString((UChar*)input.context, input.chunkLength));
    /*          range = CFRangeMake(0, str.length());
                CFStringTokenizerSetString(tokenizer, str, range);
                currentPosition = 0;
                currentIsDirty = false;*/
            }
            
            bool gotoIndex(int32_t offset) {
                return kCFStringTokenizerTokenNone != CFStringTokenizerGoToTokenAtIndex ( tokenizer, (CFIndex) offset );
            }
                
            void updateCurrentPos() {
                auto range = CFStringTokenizerGetCurrentTokenRange ( tokenizer );
                currentPosition = range.location;
                currentIsDirty = false;
            }
        };
    }
}