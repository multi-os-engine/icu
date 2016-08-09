//
//  cf_date_time_apptern_generator.h
//
//  Created by user on 30/04/15.
//  Copyright (c) 2015 МАК. All rights reserved.
//

#pragma once
#include <cf_locale.h>
#include <cf_date_format.h>

namespace U_ICU_NAMESPACE {

    class cf_DateTimePatternGenerator {
    private:
        cf_Locale _locale;
        cf_DateTimePatternGenerator(const cf_Locale& uLocale, UErrorCode& status)
            :   _locale(uLocale){
            
        }
    public:
        /**
         * Construct a flexible generator according to data for a given locale.
         * @param uLocale
         * @param status  Output param set to success/failure code on exit,
         *               which must not indicate a failure before the function call.
         * @stable ICU 3.8
         */
        static cf_DateTimePatternGenerator* createInstance(const cf_Locale& uLocale, UErrorCode& status) {
            return new cf_DateTimePatternGenerator(uLocale, status);
        }
        
        /**
         * Return the best pattern matching the input skeleton. It is guaranteed to
         * have all of the fields in the skeleton.
         *
         * @param skeleton
         *            The skeleton is a pattern containing only the variable fields.
         *            For example, "MMMdd" and "mmhh" are skeletons.
         * @param status  Output param set to success/failure code on exit,
         *               which must not indicate a failure before the function call.
         * @return bestPattern
         *            The best pattern found from the given skeleton.
         * @stable ICU 3.8
         * <p>
         * <h4>Sample code</h4>
         * \snippet samples/dtptngsample/dtptngsample.cpp getBestPatternExample1
         * \snippet samples/dtptngsample/dtptngsample.cpp getBestPatternExample
         * <p>
         */
        UnicodeString getBestPattern(const UnicodeString& skeleton, UErrorCode& status) {
            cf_DateFormat formatter(skeleton, _locale, status);
            return formatter.getFormat();
        }
        
    };

}