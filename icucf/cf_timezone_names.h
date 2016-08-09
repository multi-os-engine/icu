//
//  cf_timezone_names.h
//
//  Created by user on 30/04/15.
//  Copyright (c) 2015 МАК. All rights reserved.
//

#pragma once

#include <cf_locale.h>
#include <cf_timezone_name_type.h>
#include <cf_timezone.h>

namespace U_ICU_NAMESPACE {

    class cf_TimeZoneNames {
        cf_Locale _locale;
    private:
        cf_TimeZoneNames (const cf_Locale& locale, UErrorCode& status)
            : _locale(locale) {
        }

    public:
        /**
         * Returns an instance of <code>TimeZoneDisplayNames</code> for the specified locale.
         *
         * @param locale The locale.
         * @param status Receives the status.
         * @return An instance of <code>TimeZoneDisplayNames</code>
         * @stable ICU 50
         */
        static cf_TimeZoneNames* U_EXPORT2 createInstance(const cf_Locale& locale, UErrorCode& status){
            return new cf_TimeZoneNames(locale, status);
        }

        /**
         * Returns the display name of the time zone at the given date.
         * <p>
         * <b>Note:</b> This method calls the subclass's {@link #getTimeZoneDisplayName} first. When the
         * result is bogus, this method calls {@link #getMetaZoneID} to get the meta zone ID mapped from the
         * time zone, then calls {@link #getMetaZoneDisplayName}.
         *
         * @param tzID The canonical time zone ID.
         * @param type The display name type. See {@link #UTimeZoneNameType}.
         * @param date The date.
         * @param name Receives the display name for the time zone at the given date. When this object does not have a localized display
         *          name for the time zone with the specified type and date, "bogus" state is set.
         * @return A reference to the result.
         * @stable ICU 50
         */
        virtual UnicodeString& getDisplayName(const UnicodeString& tzID, cf_UTimeZoneNameType type, UDate date, UnicodeString& name) const {
            cf_TimeZone zone(tzID);
            if (type == UTZNM_EXEMPLAR_LOCATION) {
                //TODO: not supported by CoreFoundation
                name = UnicodeString();
            } else {
                cf_String localizedName = CFTimeZoneCopyLocalizedName (zone, convertToTimezoneStyle(type), _locale );
                name = localizedName;
                //outer code expect null string pointer rather then empty string, so we are setting is bogus to let that check pass
                //think that that behavior should be in operator UnicodeString since that isnot a generic case
                //printf("%s\n", localizedName.c_str());
                if (localizedName.length() == 0) {
                    name.setToBogus();
                }

            }
            return name;
        }
    private:
        CFTimeZoneNameStyle convertToTimezoneStyle(cf_UTimeZoneNameType type) const {

            switch (type) {
                    
                case UTZNM_LONG_GENERIC :
                    return  kCFTimeZoneNameStyleGeneric;
                case UTZNM_LONG_STANDARD :
                    return kCFTimeZoneNameStyleStandard;
                case UTZNM_LONG_DAYLIGHT :
                    return kCFTimeZoneNameStyleDaylightSaving;

                case UTZNM_SHORT_GENERIC :
                    return kCFTimeZoneNameStyleShortGeneric;
                case UTZNM_SHORT_STANDARD :
                    return kCFTimeZoneNameStyleShortStandard;
                case UTZNM_SHORT_DAYLIGHT :
                    return kCFTimeZoneNameStyleShortDaylightSaving;
                    
                default :
                    return kCFTimeZoneNameStyleStandard;
            }
        }
    };
}

#define TimeZoneNames cf_TimeZoneNames