

#pragma once
#include <unicode/utypes.h>
/**
 * This class represents a date interval.
 * It is a pair of UDate representing from UDate 1 to UDate 2.
 * @stable ICU 4.0
 **/

namespace U_ICU_NAMESPACE {
    class  cf_DateInterval  {
        
        UDate fromDate;
        UDate toDate;
        
    public:
        cf_DateInterval(UDate from, UDate to)
        :   fromDate(from),
            toDate(to)
        {}
        
        UDate getFromDate() const {
            return fromDate;
        }
        
        UDate getToDate () const {
            return toDate;
        }
    };
}