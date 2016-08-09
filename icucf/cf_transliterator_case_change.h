#pragma once

#include <cf_itransliterator.h>
#include <cf_mutable_string.h>
namespace U_ICU_NAMESPACE {
    namespace cf{
        class TransliteratorCaseChange : public ITransliterator {
            bool toUpper;
        public:
            TransliteratorCaseChange (bool toUpper)
                : toUpper(toUpper) {
            }
            void transliterate(Replaceable& text) {
                UnicodeString ustr;
                text.extractBetween(0, text.length(), ustr);
                ::cf::MutableString str(ustr);
                if (!toUpper) {
                    str.lowercase();
                } else {
                    str.uppercase();
                }
                text.handleReplaceBetween(0, text.length(), str);
            }
        };
    }
}