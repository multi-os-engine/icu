//
//  cf_Calendar.h
//
//  Created by user on 30/04/15.
//  Copyright (c) 2015 МАК. All rights reserved.
//

#pragma once

#import <CoreFoundation/CoreFoundation.h>

#include <cf_locale.h>
#include <cf_date_time_patterns.h>
#include <cf_timezone.h>
#include <unicode/utypes.h>
namespace U_ICU_NAMESPACE {
    class cf_Calendar {
        
        /**
         * Field IDs for date and time. Used to specify date/time fields. ERA is calendar
         * specific. Example ranges given are for illustration only; see specific Calendar
         * subclasses for actual ranges.
         * @deprecated ICU 2.6. Use C enum UCalendarDateFields defined in ucal.h
         */
        enum EDateFields {
            /*
             * ERA may be defined on other platforms. To avoid any potential problems undefined it here.
             */
    #ifdef ERA
    #undef ERA
    #endif
            ERA,                  // Example: 0..1
            YEAR,                 // Example: 1..big number
            MONTH,                // Example: 0..11
            WEEK_OF_YEAR,         // Example: 1..53
            WEEK_OF_MONTH,        // Example: 1..4
            DATE,                 // Example: 1..31
            DAY_OF_YEAR,          // Example: 1..365
            DAY_OF_WEEK,          // Example: 1..7
            DAY_OF_WEEK_IN_MONTH, // Example: 1..4, may be specified as -1
            AM_PM,                // Example: 0..1
            HOUR,                 // Example: 0..11
            HOUR_OF_DAY,          // Example: 0..23
            MINUTE,               // Example: 0..59
            SECOND,               // Example: 0..59
            MILLISECOND,          // Example: 0..999
            ZONE_OFFSET,          // Example: -12*U_MILLIS_PER_HOUR..12*U_MILLIS_PER_HOUR
            DST_OFFSET,           // Example: 0 or U_MILLIS_PER_HOUR
            YEAR_WOY,             // 'Y' Example: 1..big number - Year of Week of Year
            DOW_LOCAL,            // 'e' Example: 1..7 - Day of Week / Localized
        };
        
        /**
         * Useful constant for days of week. Note: Calendar day-of-week is 1-based. Clients
         * who create locale resources for the field of first-day-of-week should be aware of
         * this. For instance, in US locale, first-day-of-week is set to 1, i.e., SUNDAY.
         * @deprecated ICU 2.6. Use C enum UCalendarDaysOfWeek defined in ucal.h
         */
        enum EDaysOfWeek {
            SUNDAY = 1,
            MONDAY,
            TUESDAY,
            WEDNESDAY,
            THURSDAY,
            FRIDAY,
            SATURDAY
        };

        cf_AutoRef<CFCalendarRef> _cal;
        mutable CFIndex _firstDayIdx;
        mutable CFIndex _dayInFirstWeek;

        //calendars time
        UDate _date;

        
        cf_Calendar (const cf_Locale& aLocale, UErrorCode& success)
            : _firstDayIdx(-1)
            , _dayInFirstWeek(-1)
            , zone((CFTimeZoneRef)nullptr) {
                _cal = CFCalendarCreateWithIdentifier(nullptr, kCFGregorianCalendar);//calendar id is to be taken from locale
        }

        cf_Calendar (CFStringRef id, UErrorCode& success)
            : _firstDayIdx(-1)
            , _dayInFirstWeek(-1)
            , zone((CFTimeZoneRef)nullptr) {
                _cal = CFCalendarCreateWithIdentifier(nullptr, id);
        }
        
        mutable cf_TimeZone zone;

    public:
        
        operator CFTypeRef () const {
            return static_cast<CFTypeRef>(_cal);
        }
        operator  CFCalendarRef () const {
            return _cal;
        }
        
        /*create specific calendar*/
        static cf_Calendar* createGregorian(UErrorCode &success) {
            return new cf_Calendar(kCFGregorianCalendar, success);
        }
        
        /**
         * Creates a Calendar using the default timezone and the given locale.
         *
         * @param aLocale  The given locale.
         * @param success  Indicates the success/failure of Calendar creation. Filled in
         *                 with U_ZERO_ERROR if created successfully, set to a failure result
         *                 otherwise.
         * @return         A Calendar if created successfully. NULL otherwise.
         * @stable ICU 2.0
         */
        static cf_Calendar* createInstance(const cf_Locale& aLocale, UErrorCode& success) {
            return new cf_Calendar(aLocale, success);
        }
        
        static UDate getNow() {
            return UDate();
        }
        
        /**
         * Gets what the first day of the week is; e.g., Sunday in US, Monday in France.
         *
         * @return   The first day of the week.
         * @deprecated ICU 2.6 use the overload with error code
         */
        EDaysOfWeek getFirstDayOfWeek(void) const {
            if ( _firstDayIdx == -1) {
                _firstDayIdx = CFCalendarGetFirstWeekday(_cal);
            }

            return (EDaysOfWeek)(SUNDAY + (int)_firstDayIdx);
        }
        
        /**
         * Gets what the minimal days required in the first week of the year are; e.g., if
         * the first week is defined as one that contains the first day of the first month
         * of a year, getMinimalDaysInFirstWeek returns 1. If the minimal days required must
         * be a full week, getMinimalDaysInFirstWeek returns 7.
         *
         * @return   The minimal days required in the first week of the year.
         * @stable ICU 2.0
         */
        uint8_t getMinimalDaysInFirstWeek(void) const {
            if (-1 == _dayInFirstWeek) {
                _dayInFirstWeek = CFCalendarGetMinimumDaysInFirstWeek(_cal);
            }

            return _dayInFirstWeek;
        }
        
        /**
         * Gets the value for a given time field. Recalculate the current time field values
         * if the time value has been changed by a call to setTime(). Return zero for unset
         * fields if any fields have been explicitly set by a call to set(). To force a
         * recomputation of all fields regardless of the previous state, call complete().
         * This method is semantically const, but may alter the object in memory.
         *
         * @param field  The given time field.
         * @param status Fill-in parameter which receives the status of the operation.
         * @return       The value for the given time field, or zero if the field is unset,
         *               and set() has been called for any other field.
         * @stable ICU 2.6.
         
        int32_t get(UCalendarDateFields field, UErrorCode& status) const{
                CFCalendarDecomposeAbsoluteTime(gregorian, CFAbsoluteTimeGetCurrent(), "yMd",  &year, &month, &day);
            
                UCAL_ERA,
                UCAL_YEAR,
                UCAL_MONTH,
                UCAL_DATE,
                UCAL_AM_PM,
                UCAL_HOUR,
                UCAL_MINUTE,
                UCAL_SECOND,
                UCAL_MILLISECOND,
                

        }
        */
        
        /**
         * Sets the calendar's time zone to be the same as the one passed in. The TimeZone
         * passed in is _not_ adopted; the client is still responsible for deleting it.
         *
         * @param zone  The given time zone.
         * @stable ICU 2.0
         */
        void setTimeZone(const cf_TimeZone& zone) {
            CFCalendarSetTimeZone(_cal, zone);
        }
        
        /**
         * Returns a reference to the time zone owned by this calendar. The returned reference
         * is only valid until clients make another call to adoptTimeZone or setTimeZone,
         * or this Calendar is destroyed.
         *
         * @return   The time zone object associated with this calendar.
         * @stable ICU 2.0
         */
        const cf_TimeZone& getTimeZone(void) const {
            zone = cf_TimeZone(CFCalendarCopyTimeZone(_cal));
            return zone;
        }
        
        /**
         * Gets this Calendar's time as milliseconds. May involve recalculation of time due
         * to previous calls to set time field values. The time specified is non-local UTC
         * (GMT) time. Although this method is const, this object may actually be changed
         * (semantically const).
         *
         * @param status  Output param set to success/failure code on exit. If any value
         *                previously set in the time field is invalid or restricted by
         *                leniency, this will be set to an error status.
         * @return        The current time in UTC (GMT) time, or zero if the operation
         *                failed.
         * @stable ICU 2.0
         */
        UDate getTime(UErrorCode& status) const {
            return _date;
        }
        /**
         * Sets this Calendar's current time with the given UDate. The time specified should
         * be in non-local UTC (GMT) time.
         *
         * @param date  The given UDate in UTC (GMT) time.
         * @param status  Output param set to success/failure code on exit. If any value
         *                set in the time field is invalid or restricted by
         *                leniency, this will be set to an error status.
         * @stable ICU 2.0
         */
        void setTime(UDate date, UErrorCode& status) {
            _date = date;
            //setTimeInMillis(date, status);
        }
    };
}
#define Calendar cf_Calendar
