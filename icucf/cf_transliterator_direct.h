#pragma once

#include <cf_transliterator_patterns.h>
#include <cf_itransliterator.h>
#include <cf_mutable_string.h>

namespace U_ICU_NAMESPACE {
    namespace cf {
        class TransliteratorDirect : public ITransliterator {
            cf_String translitID;
        public:
            TransliteratorDirect(const std::string& ID) {
                ::cf::MutableString m("empty");

                translitID = ID;
                bool result = true;
                m.transform(translitID, false, result);
                 
                if (!result) {
                    throw std::runtime_error("invalid id");
                }
            }
            virtual void transliterate(Replaceable& text) {
                TransliteratorUtil::transliterate(text, translitID, false);
            }
        };
    }
}