#pragma once

#include <vector>
#include <string>
#import  <CoreFoundation/CoreFoundation.h>
#include <cf_auto_ref.h>
#include <cf_string_api.h>

//workaround agains bug submitted to apple
#define HAS_APPLE_CF_BUG_22743116 (1)

class cf_String : public ::cf::StringAPI {
    //if autoptr not used will keep reference only here
    CFStringRef actualReference = nullptr;
    bool _hasBOM = false;
    cf_AutoRef<CFStringRef> _cfString;
    std::string cString; // vital for const char * operator cache
    
public :
    template <class __CFReferenceType>
    static cf_String fromCFGetFunction(__CFReferenceType cfString) {
        cf_String str;
        str.updateCString(reinterpret_cast<CFStringRef>(cfString));
        return str;
    }
    
    cf_String(){}
    cf_String(std::nullptr_t){}

    cf_String(CFStringRef cfString)
        : _cfString(cfString) {
        updateCString(_cfString.get());
    }
    
    cf_String(const UnicodeString & ustr)
        : _hasBOM(detectBOM((uint8_t*)ustr.getBuffer(),  ustr.length() * sizeof(UChar)))
        , _cfString(CFStringCreateWithBytes( nullptr
                                           , (UInt8*)ustr.getBuffer()
                                           , ustr.length() * sizeof(UChar)
                                           , kCFStringEncodingUTF16
                                           , _hasBOM)) {
        updateCString(_cfString.get());
    }
    
    cf_String(const char* str, int strLen, CFStringEncoding encoding, bool parseBOM)
        : _hasBOM (parseBOM) {
        //roll back max 10 chars, since maximum character len in utf8 is 6, 10 is more than enough
        int strlenMin = std::max(strLen - 10, 0);
        
        while (_cfString == nullptr && strLen > strlenMin) {
            _cfString = CFStringCreateWithBytes(nullptr
                    , (UInt8*)str
                    , strLen
                    , encoding
                    , _hasBOM);
            strLen--;
        }
        updateCString(_cfString.get());
    }

    cf_String (const UniChar* str, int strLen)
        : _cfString(CFStringCreateWithCharacters(nullptr, str, strLen)) {
        updateCString(_cfString.get());
    }
    
    cf_String(const char * str)
        : _cfString(CFStringCreateWithCString(nullptr, str, kCFStringEncodingUTF8)) {
        updateCString(_cfString.get());
    }

    cf_String(const std::string & str)
        : _cfString(CFStringCreateWithCString(nullptr, str.c_str(), kCFStringEncodingUTF8)) {
        updateCString(_cfString.get());
    }
    
    cf_String(const cf_String & that) {
        assign(that);
    }
    
    cf_String & operator = (const cf_String& that ) {
        assign(that);
        return *this;
    }
    
    operator CFStringRef () const {
        return actualReference;
    }
    const char * c_str() const {
        return cString.c_str();
    }
    operator std::string() const {
        return cString;
    }
    
    //TODO: use CFStringGetBytes here in UTF16
    operator UnicodeString () const {
        return UnicodeString (cString.c_str(), cString.length());
    }
    size_t length() const {
        if (actualReference == nullptr)
            return 0;
        
        return CFStringGetLength(actualReference);
    }
    //returns actually bytes converted
    CFIndex getBytes(CFStringEncoding encoding, char replacementSymbol, char * buffer, CFIndex bufferSize, CFIndex *usedSize, bool putBOM) const {
        if (nullptr == actualReference) {
            *usedSize = 0;
            return 0;
        }
        //we dont want partial BOM to happen on output
        if (putBOM && bufferSize < 2) {
            *usedSize = 0;
            return 0;
        }
#if HAS_APPLE_CF_BUG_22743116
        //happened for 1 symbols in test, however there possibility that problems occure with arrays as well
        if (CFStringGetMaximumSizeForEncoding(1, encoding) > bufferSize && length() == 1) {
            *usedSize = 0;
            return 0;
        }
#endif
        
        auto range = CFRangeMake(0, length());

        //check that last unichar isnot a surrogate pair
        if (replacementSymbol != 0 && range.length != 0){
            UniChar ch = CFStringGetCharacterAtIndex ( actualReference, range.length - 1 );
            //if replacement specified it will clear partial surogate pair, that is unwanted
            auto isSurrogate = CFStringIsSurrogateHighCharacter(ch);
            if (isSurrogate) {
                //roll back by 1 character in source array
                range.length --;
            }
        }
    
        auto sourceCharsConverted = CFStringGetBytes(actualReference, range, encoding, replacementSymbol, putBOM, (UInt8*)buffer,  bufferSize, usedSize);
        
        //if last source char is a high-surrogate than we cannot split that operation even if it succeed
        //possible CoreFoundation lack
        if (sourceCharsConverted != 0 && sourceCharsConverted < length()) {
            UniChar ch = CFStringGetCharacterAtIndex ( actualReference, sourceCharsConverted - 1 );
            auto isSurrogate = CFStringIsSurrogateHighCharacter(ch);
            //TODO: possibly check wether we have valid surrogate pair in buffer, but i think such cfstring object cannot be constructed
            //we say that buffer not enough
            if (isSurrogate) {
                sourceCharsConverted = 0;
                *usedSize = 0;
            }
        }
        return sourceCharsConverted;
    }
    
    UniChar getChar(int idx) const {
        return CFStringGetCharacterAtIndex(actualReference, idx);
    }

    
    //return number of bytes required to convert string part to certain encoding
    CFIndex getBytesUsedForConversion(CFIndex len, CFStringEncoding encoding) const {
        CFIndex usedBufLen = 0;
        auto range = CFRangeMake(0, len);
        if (actualReference == nullptr) {
            return 0;
        }

        CFStringGetBytes (actualReference , range, encoding, 0, _hasBOM, nullptr, 0, &usedBufLen);
        
        return usedBufLen;
    }

    static CFIndex getMaximumSizeForEncoding(CFIndex len, CFStringEncoding encoding, bool putBOM = false) {
        auto maxSize = CFStringGetMaximumSizeForEncoding(len, encoding);
        return maxSize + (putBOM ? 2 : 0);
    }

    CFIndex getMaximumSizeForEncoding(CFStringEncoding encoding, bool putBOM = false) const {
        auto maxSize = CFStringGetMaximumSizeForEncoding(length(), encoding);
        return maxSize + (putBOM ? 2 : 0);
    }
    
    friend bool operator == (const cf_String & thiz, const cf_String & that )  {
        return thiz.cString == that.cString;
    }
    friend bool operator != (const cf_String & thiz, const cf_String & that )  {
        return thiz.cString != that.cString;
    }
    void clear () {
        _cfString.reset(0);
        actualReference = nullptr;
        cString = "";
    }
   
    //non locale specific comparison
    int compare(const CFStringRef & that, CFStringCompareFlags flags) const {
        return CFStringCompare(actualReference, that, flags);
    }
    
    //non locale specific comparison
    int compare(const CFStringRef & that, CFStringCompareFlags flags, CFLocaleRef locale) const {
        auto wholeString = CFRangeMake(0, length());
        
        return CFStringCompareWithOptionsAndLocale ( actualReference, that,  wholeString, flags, locale );
    }
    
    cf_String substring(int start, int end) {
        cf_String str (CFStringCreateWithSubstring ( nullptr, actualReference, CFRangeMake(start, end)));
        return str;
    }

    
private:
    
    void assign (const cf_String & that) {
        this->_cfString = that._cfString;
        if (_cfString.get() == nullptr) {
            //no addref need
            actualReference = that.actualReference;
            cString = that.cString;
            return;
        }
        updateCString(_cfString.get());
    }

    void updateCString(CFStringRef cfStringDirect, CFStringEncoding encoding = kCFStringEncodingUTF8)  {
        
        if (nullptr == cfStringDirect) {
            cString = "";
            return;
        }
        actualReference = cfStringDirect;
        
        auto cStringPtr = CFStringGetCStringPtr(cfStringDirect, encoding);
        auto len = CFStringGetLength(cfStringDirect);
        
        if (nullptr != cStringPtr) {
            cString = cStringPtr;
            return ;
        }
        
        CFIndex bufferSize = CFStringGetMaximumSizeForEncoding (len, encoding) + 1;
        std::vector<char> str(bufferSize);
        if (CFStringGetCString(cfStringDirect, &*str.begin(), bufferSize, encoding)) {
            cString = &*(str.begin());
        }
    }
    
public:
    static bool detectBOM(const uint8_t *source, int32_t len) {
        return kCFStringEncodingInvalidId != getUTF16Endianess(source, len);
    }
    //TODO:code copied from ICU4c BOM presense detection routine (_UTF16ToUnicodeWithOffsets
    //suggest that CoreFoundation have such functionality
    static CFIndex getUTF16Endianess(const uint8_t *source, int32_t len) {
        if (len < 2) {
            return  kCFStringEncodingInvalidId;
        }
        
        uint8_t b0 = source[0];
        uint8_t b1 = source[1];

        //TODO:
        if(b0 == 0xfe && b1 == 0xff) {
            return kCFStringEncodingUTF16BE; /* detect UTF-16BE */
        } else if(b0==0xff && b1 == 0xfe) {
            return kCFStringEncodingUTF16LE; /* detect UTF-16LE */
        }

        return kCFStringEncodingInvalidId; /* missing BOM for Java "Unicode" */
    }
};