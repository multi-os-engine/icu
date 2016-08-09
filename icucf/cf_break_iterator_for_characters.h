#pragma once

#include <cf_break_iterator.h>

namespace U_ICU_NAMESPACE {
    namespace cf {
        class BreakIteratorForChars : public BreakIterator {
            int currentPosition;
            CFRange mRange;
            cf_String mStr;
        public:
            BreakIteratorForChars () {
                mStr = nullptr;
                currentPosition = 0;
            }
            
            virtual BreakIteratorForChars* clone(void) const {
                return new BreakIteratorForChars();
            }
            
            virtual void  setText(UText *input, UErrorCode &status) {
                if (nullptr == input) {
                    currentPosition = 0;
                    return;
                }
                initialize(*input);
            }
            
            virtual BreakIteratorForChars &refreshInputText(UText *input, UErrorCode &status) {
                if (nullptr == input) {
                    currentPosition = 0;
                    return *this;
                }
                //do not change internal position
                mStr = cf_String(UnicodeString((UChar*)input->context, input->chunkLength));
                return *this;
            }
            
            virtual int32_t first(void) {
                currentPosition = 0;
                return current();
            }

            virtual int32_t last(void)  {
                currentPosition = mStr.length();
                return mStr.length();
            }
            
            virtual int32_t previous(void)  {
                if (current() == 0)
                    return cf::DONE;
                currentPosition --;
                return current();
            }
            
            virtual int32_t next(void)  {
                if (current() >= mStr.length() - 1 || mStr.length() == 0 || current() < 0)
                    return cf::DONE;
                mRange = CFStringGetRangeOfComposedCharactersAtIndex(mStr, currentPosition);
                currentPosition += mRange.length;
                return current();
            }
            
            virtual int32_t current(void) const   {
                return currentPosition;
            }
            
            virtual int32_t preceding(int32_t offset)   {
                if (offset == 0)
                    return cf::DONE;
                mRange = CFStringGetRangeOfComposedCharactersAtIndex(mStr, offset - 1);
                currentPosition = offset - mRange.length;
                return currentPosition;
            }
            
            virtual int32_t following(int32_t offset)   {
                if (offset == last())
                    return cf::DONE;
                currentPosition = offset + 1;
                return currentPosition;
            }
            
            virtual UBool isBoundary(int32_t offset)   {
                mRange = CFStringGetRangeOfComposedCharactersAtIndex(mStr, offset);
                if(mRange.length > 1 && offset != mRange.location){
                    return false;
                }
                if (offset >= first() && offset <= last())
                    return true;
                return false;
            }
        private:
            void initialize(const UText &input) {
                currentPosition = 0;
                mStr = cf_String(UnicodeString((UChar*)input.context, input.chunkLength));
            }
        };
    }

}