#pragma once

#include <cf_string.h>
#include <cf_auto_ref.h>
#include <cf_date.h>
#include <cf_field_position.h>

class cf_DateFormat {
    cf_AutoRef<CFDateFormatterRef> _dateFormatter;
public:
    /**
     * Create a simple date/time formatter from skeleton, given locale,
     * and date time pattern generator.
     *
     * @param skeleton  the skeleton on which date format based.
     * @param locale    the given locale.
     * @param dtpng     the date time pattern generator.
     * @param status    Output param to be set to success/failure code.
     *                  If it is failure, the returned date formatter will
     *                  be NULL.
     */
     cf_DateFormat(
          const UnicodeString& skeleton,
          const cf_Locale& locale,
          //DateTimePatternGenerator* dtpng,
          UErrorCode& status) {
         _dateFormatter = CFDateFormatterCreate(nullptr, locale, kCFDateFormatterFullStyle, kCFDateFormatterFullStyle);
         cf_String format = CFDateFormatterCreateDateFormatFromTemplate(nullptr, cf_String(skeleton), 0, locale);
         CFDateFormatterSetFormat(_dateFormatter, format);
     }
    
    /**
     * Format a date or time, which is the standard millis since 24:00 GMT, Jan
     * 1, 1970. Overrides DateFormat pure virtual method.
     * <P>
     * Example: using the US locale: "yyyy.MM.dd e 'at' HH:mm:ss zzz" ->>
     * 1996.07.10 AD at 15:08:56 PDT
     *
     * @param cal       Calendar set to the date and time to be formatted
     *                  into a date/time string.
     * @param appendTo  Output parameter to receive result.
     *                  Result is appended to existing contents.
     * @param pos       The formatting position. On input: an alignment field,
     *                  if desired. On output: the offsets of the alignment field.
     * @return          Reference to 'appendTo' parameter.
     * @stable ICU 2.1
     */
    virtual UnicodeString& format(cf_Calendar& cal,
                                  UnicodeString& appendTo,
                                  cf_FieldPosition& pos) const {
        UErrorCode status;
        cf_Date time = cal.getTime(status);
        CFDateFormatterSetProperty(_dateFormatter, kCFDateFormatterTimeZone, cal.getTimeZone());
        cf_String formattedTime = CFDateFormatterCreateStringWithDate(nullptr, _dateFormatter, time);
        appendTo.append(formattedTime);
        return appendTo;
    }
    
    UnicodeString getFormat() {
        cf_String format = cf_String::fromCFGetFunction(CFDateFormatterGetFormat(_dateFormatter));
        return format;
    }
};
