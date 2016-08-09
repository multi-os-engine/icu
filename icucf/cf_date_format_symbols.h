
#pragma once
#include <cf_locale.h>
#include <vector>

namespace U_ICU_NAMESPACE {
    
    class cf_DateFormatSymbols {
    public:
        
        /**
         * Selector for date formatting context
         * @stable ICU 3.6
         */
        enum DtContextType {
            FORMAT,
            STANDALONE,
            DT_CONTEXT_COUNT
        };
        
        /**
         * Selector for date formatting width
         * @stable ICU 3.6
         */
        enum DtWidthType {
            ABBREVIATED,
            WIDE,
            NARROW,
            /**
             * Short width is currently only supported for weekday names.
             * @stable ICU 51
             */
            SHORT,
            /**
             */
            DT_WIDTH_COUNT = 4
        };
        
        std::vector<UnicodeString>  amPmStrings;
        std::vector<UnicodeString>  eras;

        std::vector<UnicodeString>  months;
        std::vector<UnicodeString>  shortMonths;
        std::vector<UnicodeString>  veryShortMonths;

        std::vector<UnicodeString>  standAloneMonths;
        std::vector<UnicodeString>  standAloneShortMonths;
        std::vector<UnicodeString>  standAloneVeryShortMonths;
        
        std::vector<UnicodeString>  weekdays;
        std::vector<UnicodeString>  shortWeekdays;
        std::vector<UnicodeString>  veryShortWeekdays;
        
        std::vector<UnicodeString>  standAloneWeekdays;
        std::vector<UnicodeString>  standAloneShortWeekdays;
        std::vector<UnicodeString>  standAloneVeryShortWeekdays;

        CFDateFormatterRef _formatter;
    public:
        /**
         * Construct a DateFormatSymbols object by loading format data from
         * resources for the given locale, in the default calendar (Gregorian).
         *
         * @param locale    Locale to load format data from.
         * @param status    Status code.  Failure
         *                  results if the resources for the locale cannot be
         *                  found or cannot be loaded
         * @stable ICU 2.0
         */
        cf_DateFormatSymbols(const cf_Locale& locale, UErrorCode& status) {
            cf_AutoRef<CFDateFormatterRef> formatter (_formatter = CFDateFormatterCreate(nullptr, locale, kCFDateFormatterFullStyle, kCFDateFormatterFullStyle));

            amPmStrings.push_back(propertyToUnicodeString(kCFDateFormatterAMSymbol));
            amPmStrings.push_back(propertyToUnicodeString(kCFDateFormatterPMSymbol));
            eras = propertyToUnicodeStrings(kCFDateFormatterEraSymbols);

            months = propertyToUnicodeStrings(kCFDateFormatterMonthSymbols);
            shortMonths = propertyToUnicodeStrings(kCFDateFormatterShortMonthSymbols);
            veryShortMonths = propertyToUnicodeStrings(kCFDateFormatterVeryShortMonthSymbols);

            standAloneMonths = propertyToUnicodeStrings(kCFDateFormatterStandaloneMonthSymbols);
            standAloneShortMonths = propertyToUnicodeStrings(kCFDateFormatterShortStandaloneMonthSymbols);
            standAloneVeryShortMonths = propertyToUnicodeStrings(kCFDateFormatterVeryShortStandaloneMonthSymbols);
            
            weekdays  = propertyToUnicodeStrings(kCFDateFormatterWeekdaySymbols);
            shortWeekdays = propertyToUnicodeStrings(kCFDateFormatterShortWeekdaySymbols);
            veryShortWeekdays = propertyToUnicodeStrings(kCFDateFormatterVeryShortWeekdaySymbols);
            
            standAloneWeekdays = propertyToUnicodeStrings(kCFDateFormatterStandaloneWeekdaySymbols);
            standAloneShortWeekdays = propertyToUnicodeStrings(kCFDateFormatterShortStandaloneWeekdaySymbols);
            standAloneVeryShortWeekdays = propertyToUnicodeStrings(kCFDateFormatterVeryShortStandaloneWeekdaySymbols);

            //ICU uses 1-based weekdays, so it has nothing to do with first day in a week
            //look at dtfmtsym.cpp ln: 1620 (DateFormatSymbols::initializeData)
            // {sfb} fixed to handle 1-based weekdays
            weekdays.insert(weekdays.begin(), UnicodeString());
            shortWeekdays.insert(shortWeekdays.begin(), UnicodeString());
            veryShortWeekdays.insert(veryShortWeekdays.begin(), UnicodeString());
            standAloneWeekdays.insert(standAloneWeekdays.begin(), UnicodeString());
            standAloneShortWeekdays.insert(standAloneShortWeekdays.begin(), UnicodeString());
            standAloneVeryShortWeekdays.insert(standAloneVeryShortWeekdays.begin(), UnicodeString());
           
        }
        
        /**
         * Gets abbreviated era strings. For example: "AD" and "BC".
         *
         * @param count    Filled in with length of the array.
         * @return         the era strings.
         * @stable ICU 2.0
         */
        const UnicodeString* getEras(int32_t& count) const {
            return getArrayAndSize(eras, count);
        }
        
        
        /**
         * Gets AM/PM strings. For example: "AM" and "PM".
         * @param count        Filled in with length of the array.
         * @return             the weekday strings. (DateFormatSymbols retains ownership.)
         * @stable ICU 2.0
         */
        const UnicodeString* getAmPmStrings(int32_t& count) const {
            return getArrayAndSize(amPmStrings, count);
        }
        
        /**
         * Gets month strings by width and context. For example: "January", "February", etc.
         * @param count Filled in with length of the array.
         * @param context The formatting context, either FORMAT or STANDALONE
         * @param width   The width of returned strings, either WIDE, ABBREVIATED, or NARROW.
         * @return the month strings. (DateFormatSymbols retains ownership.)
         * @stable ICU 3.4
         */
        const UnicodeString* getMonths(int32_t& count, DtContextType context, DtWidthType width) const {
            if (context == STANDALONE) {
                if (width == WIDE)   return getArrayAndSize(standAloneMonths, count);
                if (width == ABBREVIATED) return getArrayAndSize(standAloneShortMonths, count);
                if (width == NARROW)  return getArrayAndSize(standAloneVeryShortMonths, count);
            } else {
                if (width == WIDE)   return getArrayAndSize(months, count);
                if (width == ABBREVIATED) return getArrayAndSize(shortMonths, count);
                if (width == NARROW)  return getArrayAndSize(veryShortMonths, count);
            }
            return nullptr;
        }
        
        /**
         * Gets weekday strings by width and context. For example: "Sunday", "Monday", etc.
         * @param count   Filled in with length of the array.
         * @param context The formatting context, either FORMAT or STANDALONE
         * @param width   The width of returned strings, either WIDE, ABBREVIATED, SHORT, or NARROW
         * @return the month strings. (DateFormatSymbols retains ownership.)
         * @stable ICU 3.4
         */
        const UnicodeString* getWeekdays(int32_t& count, DtContextType context, DtWidthType width) const {
            if (context == STANDALONE) {
                if (width == WIDE)   return getArrayAndSize(standAloneWeekdays, count);
                if (width == ABBREVIATED) return getArrayAndSize(standAloneShortWeekdays, count);
                if (width == NARROW)  return getArrayAndSize(standAloneVeryShortWeekdays, count);
            } else {
                if (width == WIDE)   return getArrayAndSize(weekdays, count);
                if (width == ABBREVIATED) return getArrayAndSize(shortWeekdays, count);
                if (width == NARROW)  return getArrayAndSize(veryShortWeekdays, count);
            }
            return nullptr;
        }
        
    private:
        UnicodeString propertyToUnicodeString(CFStringRef key) {
            cf_String property = (CFStringRef)CFDateFormatterCopyProperty ( _formatter, key);
            return property;
        }
        
        std::vector<UnicodeString> propertyToUnicodeStrings(CFStringRef key) {
            return cf_Array<CFStringRef>(CFDateFormatterCopyProperty ( _formatter, key));
        }
        
        const UnicodeString * getArrayAndSize(const std::vector<UnicodeString> & array, int32_t & count) const {
            count = array.size();
            return array.data();
        }
        
    };
}