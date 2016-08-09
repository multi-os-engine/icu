#pragma once

#include <cf_string.h>
#include <cf_locale.h>

namespace cf {
    class MutableString  : public StringAPI{
        cf_AutoRef<CFMutableStringRef> mstr;
    public:
        MutableString(const cf_String & that)
            : mstr(CFStringCreateMutableCopy ( nullptr, 0, that )) {
        }
        
        MutableString & uppercase(const cf_Locale & loc) {
            CFStringUppercase(mstr, loc);
            return *this;
        }

        MutableString & lowercase(const cf_Locale & loc) {
            CFStringLowercase(mstr, loc);
            return *this;
        }
        
        MutableString & uppercase() {
            CFStringUppercase(mstr, cf_Locale::getDefault());
            return *this;
        }
        
        MutableString & lowercase() {
            CFStringLowercase(mstr, cf_Locale::getDefault());
            return *this;
        }
        
        MutableString & normalize(CFStringNormalizationForm form) {
            CFStringNormalize(mstr, form);
            return *this;
        }
        
        MutableString & transform(const CFStringRef translitID, bool bReverse, bool &result) {
            auto range = CFRangeMake(0, length());
            if (FALSE == CFStringTransform ( mstr, &range, translitID, bReverse ? TRUE : FALSE)){
                result = false;
            }
            return *this;
        }
        MutableString & transform(const CFStringRef translitID, bool bReverse) {
            bool result;
            return transform(translitID, bReverse, result);
        }
        
        bool isNormalized(CFStringNormalizationForm form) {
            cf_AutoRef<CFMutableStringRef> normalized (CFStringCreateMutableCopy ( nullptr, 0, mstr)) ;
            CFStringNormalize(normalized, form);
            return kCFCompareEqualTo == CFStringCompare(normalized, mstr, 0);
        }
        
       
        operator UnicodeString () const {
            auto len = CFStringGetLength(mstr);
            auto bufferSize = CFStringGetMaximumSizeForEncoding (len, kCFStringEncodingUTF16) + 1;
            std::vector<char> str(bufferSize);
            if (CFStringGetCString(mstr, &*str.begin(), bufferSize, kCFStringEncodingUTF16)) {
                return UnicodeString ((UChar*)&*(str.begin()), len);
            }
            return UnicodeString();
        }
        operator CFMutableStringRef () {
            return mstr;
        }

        ////////////////////////
        //CFStringAPI implementation
        virtual size_t length() const {
            return CFStringGetLength(mstr);
        }
        
        virtual int compare(const CFStringRef & that, CFStringCompareFlags flags) const {
            return CFStringCompare(mstr, that, flags);
        }
        //non locale specific comparison

        int compare(const CFStringRef & that, CFStringCompareFlags flags, CFLocaleRef locale) const {
            auto wholeString = CFRangeMake(0, length());
            return CFStringCompareWithOptionsAndLocale ( mstr, that,  wholeString, flags, locale );
        }
    };
}