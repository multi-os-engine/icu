#pragma once

namespace U_ICU_NAMESPACE {
    namespace cf {
        struct ITransliterator {
            virtual void transliterate(Replaceable& text) = 0;
        };
    }
}