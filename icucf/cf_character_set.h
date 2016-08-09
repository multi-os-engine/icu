#pragma once

#import  <CoreFoundation/CoreFoundation.h>
#include <string>
#include <cf_string.h>
#include <algorithm>
#include <locale>

//#define LOG_ENCODINGS

class cf_CharacterSet {
    std::string _name;
public :
    
    static cf_CharacterSet fromICUCanonicalName(const std::string & name) {
        return cf_CharacterSet(name);
    }
    
    cf_CharacterSet(const std::string & name)
        : _name(name) {
    }

    static const std::vector<std::string> & getAvailableEncodings() {
        return encodingsList();
    }
    
    std::string getICUCanonicalName () {
        cf_String ianaCharsetName(_name);
        //two ways conversion to get closest mapping to mac-encoding name
        //came case is different from ICU, so we have
        CFStringEncoding closestEncoding = CFStringConvertIANACharSetNameToEncoding(ianaCharsetName);
        
        //for some predefined encodings we have to provide java compatible names, since they use in  base classes like String.getBytes()
        switch (closestEncoding) {
            case kCFStringEncodingUTF8:
            case kCFStringEncodingUTF16:
            case kCFStringEncodingUTF16BE:
            case kCFStringEncodingUTF16LE:
            case kCFStringEncodingUTF32:
            case kCFStringEncodingUTF32BE:
            case kCFStringEncodingUTF32LE:
            case kCFStringEncodingASCII:
            case kCFStringEncodingISOLatin1: return getEncodingName(closestEncoding);
            default: {
                cf_String canonicalCharSetName = cf_String::fromCFGetFunction(CFStringConvertEncodingToIANACharSetName(closestEncoding));
                return canonicalCharSetName;
            }
        };
    }
    std::string getJavaCanonicalName () {
        return getICUCanonicalName();
    }
    
    bool isEncodingSupported() {
        cf_String ianaCharsetName(_name);
        auto encodingName = CFStringConvertIANACharSetNameToEncoding(ianaCharsetName);
        return TRUE == CFStringIsEncodingAvailable(encodingName);
    }
    std::string getAlias(const std::string & str ) {
        if (str == "IANA") {
            return getICUCanonicalName();
        } else if (str == "MIME") {
            return getJavaCanonicalName();
        } else if (str == "JAVA") {
            return getJavaCanonicalName();
        } else if (str == "WINDOWS") {
            return getJavaCanonicalName();
        }
        return "";
    }
    operator CFStringEncoding() const {
        cf_String ianaCharsetName(_name);
        return CFStringConvertIANACharSetNameToEncoding(ianaCharsetName);
    }

private:
     static std::string getEncodingName(CFStringEncoding encoding) {
        switch (encoding) {
            case kCFStringEncodingUTF8: return "UTF-8";
            case kCFStringEncodingUTF16: return "UTF-16";
            case kCFStringEncodingUTF16BE: return "UTF-16BE";
            case kCFStringEncodingUTF16LE: return "UTF-16LE";
            case kCFStringEncodingUTF32: return "UTF-32";
            case kCFStringEncodingUTF32BE: return "UTF-32BE";
            case kCFStringEncodingUTF32LE: return "UTF-32LE";
            case kCFStringEncodingASCII: return "US-ASCII";
            case kCFStringEncodingISOLatin1: return "ISO-8859-1";
        }
        return "";
    }
    
    static const std::vector<std::string>& encodingsList() {
        auto & encodings = encodingsListRef();
        if (!encodings.empty()) {
            return encodings;
        }
        cacheEncodings();
        return encodings;
    }
    
    static std::vector<std::string>& encodingsListRef() {
        static std::vector<std::string> encodings;
        return encodings;
    }
    
    static void cacheEncodings()
    {
        auto & encodingsList = encodingsListRef();
        const CFStringEncoding * encodings = CFStringGetListOfAvailableEncodings();
    
        for (;kCFStringEncodingInvalidId != *encodings; encodings++) {
            //for some reason apple provides encodings that are not supported in terms of CFStringIsEncodingAvailable
            //we had to double check it
            if ( FALSE == CFStringIsEncodingAvailable(*encodings)) {
                continue;
            }
            
            auto encodingIANAName = CFStringConvertEncodingToIANACharSetName(*encodings);
            auto encodingIANANameStr = cf_String::fromCFGetFunction(encodingIANAName);
            
#ifdef LOG_ENCODINGS
            auto encodingName = CFStringGetNameOfEncoding(*encodings);
            auto encodingNameStr = cf_String::fromCFGetFunction(encodingName);
            
            auto macencoding = CFStringGetMostCompatibleMacStringEncoding(*encodings);
            
            auto encodingIANAName1 = CFStringConvertEncodingToIANACharSetName(macencoding);
            auto encodingIANANameStr1 = cf_String::fromCFGetFunction(encodingIANAName1);
            
            auto encodingName1 = CFStringGetNameOfEncoding(macencoding);
            auto encodingNameStr1 = cf_String::fromCFGetFunction(encodingName1);
            
            printf("IANA=%s/%d, Name=%s, mac_IANA=%s, mac_name=%s\n", encodingIANANameStr.c_str(),encodingIANANameStr.length(), encodingNameStr.c_str(), encodingIANANameStr1.c_str(), encodingNameStr1.c_str());
#endif
        
            //if we put empty IANA name what nex can we do with that encoding
            //currently macos reported number of encodings as available but without IANA utf-8 names:
            //IANA=, Name=Traditional Chinese (Big 5-E), 	mac_IANA=x-mac-trad-chinese, mac_name=Traditional Chinese (Mac OS)
            //IANA=/12, Name=Keyboard Symbols (Mac OS), 		mac_IANA=, mac_name=Keyboard Symbols (Mac OS)
            //IANA=,/10 Name=Non-lossy ASCII, 				mac_IANA=, 	mac_name=
            //IANA=,/10 Name=Western (EBCDIC Latin Core), 	mac_IANA=macintosh, mac_name=Western (Mac OS Roman)
            
            if (((std::string)encodingIANANameStr).length() == 0 ) {
                continue;
            }
            

            encodingsList.push_back(encodingIANANameStr);
        }
    }

};