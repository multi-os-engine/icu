#pragma once

namespace cf {
    struct StringAPI {
        virtual size_t length() const = 0;
        virtual int compare(const CFStringRef & that, CFStringCompareFlags flags) const = 0;
        virtual int compare(const CFStringRef & that, CFStringCompareFlags flags, CFLocaleRef locale) const =0;
    };
}