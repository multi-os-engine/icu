//
//  cf_timezone.h
//  a123
//
//  Created by user on 30/04/15.
//  Copyright (c) 2015 МАК. All rights reserved.
//

#pragma once

#import <CoreFoundation/CoreFoundation.h>

#include <cf_auto_ref.h>
#include <cf_string.h>
namespace U_ICU_NAMESPACE {
    
    class cf_TimeZone {

        cf_AutoRef<CFTimeZoneRef> _timezone;
        
    public:
        operator CFTimeZoneRef() const {
            return _timezone;
        }
        
        cf_TimeZone(const UnicodeString & id)
            : _timezone (CFTimeZoneCreateWithName ( nullptr, cf_String(id), TRUE )) {
        }
        
        cf_TimeZone ( CFTimeZoneRef timezone)
            : _timezone(timezone) {
        }
        /**
         * Creates a <code>TimeZone</code> for the given ID.
         * @param ID the ID for a <code>TimeZone</code>, such as "America/Los_Angeles",
         * or a custom ID such as "GMT-8:00".
         * @return the specified <code>TimeZone</code>, or a mutable clone of getUnknown()
         * if the given ID cannot be understood or if the given ID is "Etc/Unknown".
         * The return result is guaranteed to be non-NULL.
         * If you require that the specific zone asked for be returned,
         * compare the result with getUnknown() or check the ID of the return result.
         * @stable ICU 2.0
         */
        static cf_TimeZone*  createTimeZone(const UnicodeString& ID) {
            return new cf_TimeZone(ID);
        }
    };
}