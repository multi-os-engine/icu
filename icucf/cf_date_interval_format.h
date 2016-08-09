
#pragma once

#include <cf_locale.h>
#include <cf_timezone.h>
#include <cf_date_interval.h>
#include <cf_field_position.h>
#include <cf_calendar.h>
#include <cf_date.h>
#include <cf_date_format.h>
#include <memory>

namespace U_ICU_NAMESPACE {
    
    class  cf_DateIntervalFormat //: public Format
    {
    private:
        //will use single calendar instead of source and dest in icu , since different calendars not supported
        std::unique_ptr<cf_Calendar> calendar;

        cf_DateFormat fDateFormat;
        
        cf_DateIntervalFormat(const UnicodeString& skeleton,
                              const cf_Locale& locale,
                              UErrorCode& status)
            : fDateFormat (skeleton, locale, status) {
            calendar.reset(cf_Calendar::createInstance(locale, status));
            if (U_FAILURE(status)) {
                return ;
            }
        }
        
    public:
        
        /**
         * Construct a DateIntervalFormat from skeleton and a given locale.
         * <P>
         * In this factory method,
         * the date interval pattern information is load from resource files.
         * Users are encouraged to created date interval formatter this way and
         * to use the pre-defined skeleton macros.
         *
         * <P>
         * There are pre-defined skeletons (defined in udat.h) having predefined
         * interval patterns in resource files.
         * Users are encouraged to use those macros.
         * For example:
         * DateIntervalFormat::createInstance(UDAT_MONTH_DAY, status)
         *
         * The given Locale provides the interval patterns.
         * For example, for en_GB, if skeleton is UDAT_YEAR_ABBR_MONTH_WEEKDAY_DAY,
         * which is "yMMMEEEd",
         * the interval patterns defined in resource file to above skeleton are:
         * "EEE, d MMM, yyyy - EEE, d MMM, yyyy" for year differs,
         * "EEE, d MMM - EEE, d MMM, yyyy" for month differs,
         * "EEE, d - EEE, d MMM, yyyy" for day differs,
         * @param skeleton  the skeleton on which the interval format is based.
         * @param locale    the given locale
         * @param status    output param set to success/failure code on exit
         * @return          a date time interval formatter which the caller owns.
         * @stable ICU 4.0
         * <p>
         * <h4>Sample code</h4>
         * \snippet samples/dtitvfmtsample/dtitvfmtsample.cpp dtitvfmtPreDefined1
         * \snippet samples/dtitvfmtsample/dtitvfmtsample.cpp dtitvfmtPreDefined
         * <p>
         */
        
        static cf_DateIntervalFormat* createInstance(const UnicodeString& skeleton,
                                                     const cf_Locale& locale,
                                                     UErrorCode& status)
        {
            return new cf_DateIntervalFormat(skeleton, locale, status);
        }
        
        /**
         * Sets the time zone for the calendar used by this DateIntervalFormat object. The
         * caller no longer owns the TimeZone object and should not delete it after this call.
         * @param zoneToAdopt the TimeZone to be adopted.
         * @stable ICU 4.8
         */
        virtual void adoptTimeZone(cf_TimeZone* zoneToAdopt) {
            std::unique_ptr<cf_TimeZone> zone(zoneToAdopt);
            // The fDateFormat has the master calendar for the DateIntervalFormat and has
            // ownership of any adopted TimeZone; fFromCalendar and fToCalendar are internal
            // work clones of that calendar (and should not also be given ownership of the
            // adopted TimeZone).
            if (nullptr != calendar.get()) {
                calendar->setTimeZone(*zoneToAdopt);
            }
          
        }
        
        
        /**
         * Format a DateInterval to produce a string.
         *
         * @param dtInterval        DateInterval to be formatted.
         * @param appendTo          Output parameter to receive result.
         *                          Result is appended to existing contents.
         * @param fieldPosition     On input: an alignment field, if desired.
         *                          On output: the offsets of the alignment field.
         * @param status            Output param filled with success/failure status.
         * @return                  Reference to 'appendTo' parameter.
         * @stable ICU 4.0
         */
        UnicodeString& format(const cf_DateInterval* dtInterval,
                              UnicodeString& appendTo,
                              cf_FieldPosition& fieldPosition,
                              UErrorCode& status) const {
            if ( U_FAILURE(status) ) {
                return appendTo;
            }
            int days, months, years, eras;
            int ampm, hours, minutes, seconds;
            
            if (FALSE == CFCalendarGetComponentDifference ( *calendar.get()
              , cf_Date(dtInterval->getFromDate())
              , cf_Date(dtInterval->getToDate())
              , 0
              , "GyMdaHm"
              , &eras, &years, &months,  &days, &ampm, &hours, &minutes, &seconds)){
                status = U_ILLEGAL_ARGUMENT_ERROR;
                return appendTo;
            }
            
            //if (!eras && !years && !months && !days &&  ! ampm && !hours && !minutes )
            {
                /* ignore the second/millisecond etc. small fields' difference.
                 * use single date when all the above are the same.
                 */
                cf_FieldPosition pos = 0;
                calendar->setTime(dtInterval->getFromDate(), status);
                if ( U_SUCCESS(status) ) {
                    fDateFormat.format(*calendar.get(), appendTo, pos);
                }
                return appendTo;
            }
            
            /* TODO: adopt that code that scan minimum formatable entity
             
            // following call should not set wrong status,
            // all the pass-in fields are valid till here
            int32_t itvPtnIndex = DateIntervalInfo::calendarFieldToIntervalIndex(field,
                                                                                 status);
            const PatternInfo& intervalPattern = fIntervalPatterns[itvPtnIndex];
            
            if ( intervalPattern.firstPart.isEmpty() &&
                intervalPattern.secondPart.isEmpty() ) {
                if ( fDateFormat->isFieldUnitIgnored(field) ) {
                    /* the largest different calendar field is small than
                     * the smallest calendar field in pattern,
                     * return single date format.
                     */
                   /* return fDateFormat->format(fromCalendar, appendTo, pos);
                }
                return fallbackFormat(fromCalendar, toCalendar, appendTo, pos, status);
            }
            
            
            // If the first part in interval pattern is empty,
            // the 2nd part of it saves the full-pattern used in fall-back.
            // For a 'real' interval pattern, the first part will never be empty.
            if ( intervalPattern.firstPart.isEmpty() ) {
                // fall back
                UnicodeString originalPattern;
                fDateFormat->toPattern(originalPattern);
                fDateFormat->applyPattern(intervalPattern.secondPart);
                appendTo = fallbackFormat(fromCalendar, toCalendar, appendTo, pos, status);
                fDateFormat->applyPattern(originalPattern);
                return appendTo;
            }
            Calendar* firstCal;
            Calendar* secondCal;
            if ( intervalPattern.laterDateFirst ) {
                firstCal = &toCalendar;
                secondCal = &fromCalendar;
            } else {
                firstCal = &fromCalendar;
                secondCal = &toCalendar;
            }
            // break the interval pattern into 2 parts,
            // first part should not be empty,
            UnicodeString originalPattern;
            fDateFormat->toPattern(originalPattern);
            fDateFormat->applyPattern(intervalPattern.firstPart);
            fDateFormat->format(*firstCal, appendTo, pos);
            if ( !intervalPattern.secondPart.isEmpty() ) {
                fDateFormat->applyPattern(intervalPattern.secondPart);
                fDateFormat->format(*secondCal, appendTo, pos);
            }
            fDateFormat->applyPattern(originalPattern);*/
            return appendTo;

        }
    };
}

