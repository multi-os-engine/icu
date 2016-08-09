//
//  cf_timezone_name_types.h
//
//  Created by user on 30/04/15.
//  Copyright (c) 2015 МАК. All rights reserved.
//

#pragma once
/**
 * Constants for time zone display name types.
 * @stable ICU 50
 */
typedef enum cf_UTimeZoneNameType {
    /**
     * Unknown display name type.
     * @stable ICU 50
     */
    UTZNM_UNKNOWN           = 0x00,
    /**
     * Long display name, such as "Eastern Time".
     * @stable ICU 50
     */
    UTZNM_LONG_GENERIC      = 0x01,
    /**
     * Long display name for standard time, such as "Eastern Standard Time".
     * @stable ICU 50
     */
    UTZNM_LONG_STANDARD     = 0x02,
    /**
     * Long display name for daylight saving time, such as "Eastern Daylight Time".
     * @stable ICU 50
     */
    UTZNM_LONG_DAYLIGHT     = 0x04,
    /**
     * Short display name, such as "ET".
     * @stable ICU 50
     */
    UTZNM_SHORT_GENERIC     = 0x08,
    /**
     * Short display name for standard time, such as "EST".
     * @stable ICU 50
     */
    UTZNM_SHORT_STANDARD    = 0x10,
    /**
     * Short display name for daylight saving time, such as "EDT".
     * @stable ICU 50
     */
    UTZNM_SHORT_DAYLIGHT    = 0x20,
    /**
     * Exemplar location name, such as "Los Angeles".
     * @stable ICU 51
     */
    UTZNM_EXEMPLAR_LOCATION = 0x40
} cf_UTimeZoneNameType;
