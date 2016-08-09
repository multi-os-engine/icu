#pragma once

#include <string>

class cf_RelativeDays {
    
    std::string yesterday;
    std::string today;
    std::string tomorrow;
    
public:
 
    cf_RelativeDays(const cf_Locale &locale, UErrorCode& success) {
        
        cf_AutoRef<CFDateFormatterRef> formatter (CFDateFormatterCreate(nullptr, locale, kCFDateFormatterShortStyle, kCFDateFormatterNoStyle));
        CFDateFormatterSetProperty(formatter, kCFDateFormatterDoesRelativeDateFormattingKey, kCFBooleanTrue);

        CFTimeInterval day = 24*60*60;
        CFAbsoluteTime now = CFAbsoluteTimeGetCurrent();

        yesterday = getDayName(formatter, now - day);
        today     = getDayName(formatter, now);
        tomorrow  = getDayName(formatter, now + day);
    }
    
    std::string getToday() {
        return today;
    }
    
    std::string getYesterday() {
        return yesterday;
    }
    
    std::string getTomorrow() {
        return tomorrow;
    }
private:

    std::string getDayName(CFDateFormatterRef formatter, CFAbsoluteTime time) {
        cf_AutoRef<CFDateRef> date = CFDateCreate(nullptr, time);
        cf_String str = CFDateFormatterCreateStringWithDate ( nullptr, formatter, date );
        return str;
    }
};