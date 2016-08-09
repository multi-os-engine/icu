#pragma once

//apple absolute time started at 1.1.2001 00:00, and unix time of that moment represented by this constant
#define APPLE_TIME_IN_UNIX_TIME 978307200000

class cf_Date {
    UDate date;
    cf_AutoRef<CFDateRef> cfDate;
public:
    cf_Date (UDate date)
        : date(date)
    , cfDate(CFDateCreate(nullptr, *this)) {
    }
    operator CFAbsoluteTime() const {
        return (date - APPLE_TIME_IN_UNIX_TIME) / 1000;
    }
    //no thread safe
    operator CFDateRef() {
        return cfDate;
    }
};
