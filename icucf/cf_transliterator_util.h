#pragma once
#include <cf_mutable_string.h>

namespace U_ICU_NAMESPACE {
    namespace cf {
        struct TransliteratorUtil {
            static void transliterate(Replaceable& text, CFStringRef ID, bool direction) {
                UnicodeString ustr;
                text.extractBetween(0, text.length(), ustr);
                ::cf::MutableString str(ustr);
                str.transform(ID, direction);
                text.handleReplaceBetween(0, text.length(), str);
            }
        };
    }
}