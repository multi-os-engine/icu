#pragma once

#include <cf_locale.h>
#include <cf_string.h>

class cf_DateTimePatterns {
    const cf_Locale *_tmplocale;

    std::string fullTimeFormat;
    std::string longTimeFormat;
    std::string medTimeFormat;
    std::string shortTimeFormat;
    std::string fullDateFormat;
    std::string longDateFormat;
    std::string medDateFormat;
    std::string shortDateFormat;
    
public:
    cf_DateTimePatterns(const cf_Locale &locale, UErrorCode& success)
        : _tmplocale(&locale) {

        fullTimeFormat  = getTimeFormatString(kCFDateFormatterFullStyle);
        longTimeFormat  = getTimeFormatString(kCFDateFormatterLongStyle);
        medTimeFormat   = getTimeFormatString(kCFDateFormatterMediumStyle);
        shortTimeFormat = getTimeFormatString(kCFDateFormatterShortStyle);

        fullDateFormat  = getDateFormatString(kCFDateFormatterFullStyle);
        longDateFormat  = getDateFormatString(kCFDateFormatterLongStyle);
        medDateFormat   = getDateFormatString(kCFDateFormatterMediumStyle);
        shortDateFormat = getDateFormatString(kCFDateFormatterShortStyle);
            
    }
    std::string getFullTimeFormat() {
        return fullTimeFormat;
    }
    std::string getLongTimeFormat() {
        return longTimeFormat;
    }
    std::string getMediumTimeFormat() {
        return medTimeFormat;
    }
    std::string getShortTimeFormat() {
        return shortTimeFormat;
    }
    std::string getFullDateFormat() {
        return fullDateFormat;
    }
    std::string getLongDateFormat() {
        return longDateFormat;
    }
    std::string getMediumDateFormat() {
        return medDateFormat;
    }
    std::string getShortDateFormat() {
        return shortDateFormat;
    }
    
private:
    std::string getDateFormatString(CFDateFormatterStyle dateStyle) {
        return getFormatString(dateStyle, kCFDateFormatterNoStyle);
    }
    std::string getTimeFormatString(CFDateFormatterStyle timeStyle) {
        return getFormatString(kCFDateFormatterNoStyle, timeStyle);
    }
    std::string getFormatString(CFDateFormatterStyle dateStyle, CFDateFormatterStyle timeStyle) {
        
        cf_AutoRef<CFDateFormatterRef> formatter (CFDateFormatterCreate(nullptr, *_tmplocale, dateStyle, timeStyle));
        cf_String format = cf_String::fromCFGetFunction(CFDateFormatterGetFormat(formatter));
        return format;
    }
};