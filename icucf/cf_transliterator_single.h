#pragma once
#include <cf_transliterator_patterns.h>
#include <cf_itransliterator.h>
#include <cf_transliterator_util.h>

namespace U_ICU_NAMESPACE {
    namespace cf {
        class TransliteratorSingle : public ITransliterator {
            const CFStringRef translitID ;
            bool  bReverse;
        public:
            TransliteratorSingle(const std::string& ID)
                : translitID (getTranslitID(ID)) {
            }
            virtual void transliterate(Replaceable& text) {
                  TransliteratorUtil::transliterate(text, translitID, bReverse);
            }
        protected:
            const CFStringRef getTranslitID(const std::string & ID) {
                //first trying to match id as it registered in the map
                try {
                    auto result = TransliteratorPatterns::getMergedMap().at(ID);
                    bReverse = !result.second;
                    return result.first;
                } catch (...) {
                }
            }
        };
    }
}