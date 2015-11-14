/*
**********************************************************************
* Copyright (c) 2004-2015, International Business Machines
* Corporation and others.  All Rights Reserved.
**********************************************************************
* Author: Alan Liu
* Created: April 20, 2004
* Since: ICU 3.0
**********************************************************************
*/
#include "utypeinfo.h"  // for 'typeid' to work
#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING

#include "unicode/measfmt.h"
#include "unicode/numfmt.h"
#include "currfmt.h"
#include "unicode/localpointer.h"
#include "simplepatternformatter.h"
#include "quantityformatter.h"
#include "unicode/plurrule.h"
#include "unicode/decimfmt.h"
#include "uresimp.h"
#include "unicode/ures.h"
#include "ureslocs.h"
#include "cstring.h"
#include "mutex.h"
#include "ucln_in.h"
#include "unicode/listformatter.h"
#include "charstr.h"
#include "unicode/putil.h"
#include "unicode/smpdtfmt.h"
#include "uassert.h"
#include "uresource.h"

#include "sharednumberformat.h"
#include "sharedpluralrules.h"
#include "unifiedcache.h"

#define MEAS_UNIT_COUNT 129
#define WIDTH_INDEX_COUNT (UMEASFMT_WIDTH_NARROW + 1)

U_NAMESPACE_BEGIN

UOBJECT_DEFINE_RTTI_IMPLEMENTATION(MeasureFormat)

// Used to format durations like 5:47 or 21:35:42.
class NumericDateFormatters : public UMemory {
public:
    // Formats like H:mm
    SimpleDateFormat hourMinute;

    // formats like M:ss
    SimpleDateFormat minuteSecond;

    // formats like H:mm:ss
    SimpleDateFormat hourMinuteSecond;

    // Constructor that takes the actual patterns for hour-minute,
    // minute-second, and hour-minute-second respectively.
    NumericDateFormatters(
            const UnicodeString &hm,
            const UnicodeString &ms,
            const UnicodeString &hms,
            UErrorCode &status) : 
            hourMinute(hm, status),
            minuteSecond(ms, status), 
            hourMinuteSecond(hms, status) {
        const TimeZone *gmt = TimeZone::getGMT();
        hourMinute.setTimeZone(*gmt);
        minuteSecond.setTimeZone(*gmt);
        hourMinuteSecond.setTimeZone(*gmt);
    }
private:
    NumericDateFormatters(const NumericDateFormatters &other);
    NumericDateFormatters &operator=(const NumericDateFormatters &other);
};

/**
 * Instances contain all MeasureFormat specific data for a particular locale.
 * This data is cached. It is never copied, but is shared via shared pointers.
 *
 * Note: We might change the cache data to have an array[WIDTH_INDEX_COUNT] of
 * complete sets of unit & per patterns,
 * to correspond to the resource data and its aliases.
 *
 * TODO: Maybe store more sparsely in general, with pointers rather than potentially-empty objects.
 */
class MeasureFormatCacheData : public SharedObject {
public:
    /**
     * Redirection data from root-bundle, top-level sideways aliases.
     * - UMEASFMT_WIDTH_COUNT: initial value, just fall back to root
     * - UMEASFMT_WIDTH_WIDE/SHORT/NARROW: sideways alias for missing data
     */
    UMeasureFormatWidth widthFallback[WIDTH_INDEX_COUNT];
    QuantityFormatter formatters[MEAS_UNIT_COUNT][WIDTH_INDEX_COUNT];
    SimplePatternFormatter *perUnitFormatters[MEAS_UNIT_COUNT][WIDTH_INDEX_COUNT];
    SimplePatternFormatter perFormatters[WIDTH_INDEX_COUNT];

    MeasureFormatCacheData();

    UBool hasPerFormatter(int32_t width) const {
        // TODO: Create a more obvious way to test if the per-formatter has been set?
        // Use pointers, check for NULL? Or add an isValid() method?
        return perFormatters[width].getPlaceholderCount() == 2;
    }

    void adoptCurrencyFormat(int32_t widthIndex, NumberFormat *nfToAdopt) {
        delete currencyFormats[widthIndex];
        currencyFormats[widthIndex] = nfToAdopt;
    }
    const NumberFormat *getCurrencyFormat(int32_t widthIndex) const {
        return currencyFormats[widthIndex];
    }
    void adoptIntegerFormat(NumberFormat *nfToAdopt) {
        delete integerFormat;
        integerFormat = nfToAdopt;
    }
    const NumberFormat *getIntegerFormat() const {
        return integerFormat;
    }
    void adoptNumericDateFormatters(NumericDateFormatters *formattersToAdopt) {
        delete numericDateFormatters;
        numericDateFormatters = formattersToAdopt;
    }
    const NumericDateFormatters *getNumericDateFormatters() const {
        return numericDateFormatters;
    }
    void setPerUnitFormatterIfAbsent(int32_t unitIndex, int32_t width, UResourceValue &value,
                                     UErrorCode &errorCode) {
        if (perUnitFormatters[unitIndex][width] == NULL) {
            perUnitFormatters[unitIndex][width] =
                new SimplePatternFormatter(value.getUnicodeString(errorCode));
        }
    }
    const SimplePatternFormatter * const * getPerUnitFormattersByIndex(
            int32_t index) const {
        return perUnitFormatters[index];
    }
    virtual ~MeasureFormatCacheData();
private:
    NumberFormat *currencyFormats[WIDTH_INDEX_COUNT];
    NumberFormat *integerFormat;
    NumericDateFormatters *numericDateFormatters;
    MeasureFormatCacheData(const MeasureFormatCacheData &other);
    MeasureFormatCacheData &operator=(const MeasureFormatCacheData &other);
};

MeasureFormatCacheData::MeasureFormatCacheData() {
    for (int32_t i = 0; i < WIDTH_INDEX_COUNT; ++i) {
        widthFallback[i] = UMEASFMT_WIDTH_COUNT;
    }
    for (int32_t i = 0; i < UPRV_LENGTHOF(currencyFormats); ++i) {
        currencyFormats[i] = NULL;
    }
    for (int32_t i = 0; i < MEAS_UNIT_COUNT; ++i) {
        for (int32_t j = 0; j < WIDTH_INDEX_COUNT; ++j) {
            perUnitFormatters[i][j] = NULL;
        }
    }
    integerFormat = NULL;
    numericDateFormatters = NULL;
}

MeasureFormatCacheData::~MeasureFormatCacheData() {
    for (int32_t i = 0; i < UPRV_LENGTHOF(currencyFormats); ++i) {
        delete currencyFormats[i];
    }
    for (int32_t i = 0; i < MEAS_UNIT_COUNT; ++i) {
        for (int32_t j = 0; j < WIDTH_INDEX_COUNT; ++j) {
            delete perUnitFormatters[i][j];
        }
    }
    delete integerFormat;
    delete numericDateFormatters;
}

static int32_t widthToIndex(UMeasureFormatWidth width) {
    if (width >= WIDTH_INDEX_COUNT) {
        return WIDTH_INDEX_COUNT - 1;
    }
    return width;
}

static UBool isCurrency(const MeasureUnit &unit) {
    return (uprv_strcmp(unit.getType(), "currency") == 0);
}

static UBool getString(
        const UResourceBundle *resource,
        UnicodeString &result,
        UErrorCode &status) {
    int32_t len = 0;
    const UChar *resStr = ures_getString(resource, &len, &status);
    if (U_FAILURE(status)) {
        return FALSE;
    }
    result.setTo(TRUE, resStr, len);
    return TRUE;
}

namespace {

struct UnitDataSink;

/**
 * Common base class for other on-the-path sinks.
 * Holds the reference to the outer sink.
 * Not necessary in Java where we can just use non-static inner sink classes.
 */
struct UnitPathSink : public UResourceTableSink {
    UnitPathSink(UnitDataSink &sink) : dataSink(sink) {}
    ~UnitPathSink();

    UnitDataSink &dataSink;
};

struct UnitPatternSink : public UnitPathSink {
    UnitPatternSink(UnitDataSink &sink) : UnitPathSink(sink) {}
    ~UnitPatternSink();
    virtual void put(const char *key, UResourceValue &value, UErrorCode &errorCode);
};

struct UnitSubtypeSink : public UnitPathSink {
    UnitSubtypeSink(UnitDataSink &sink) : UnitPathSink(sink) {}
    ~UnitSubtypeSink();
    virtual UResourceTableSink *getOrCreateTableSink(
            const char *key, int32_t initialSize, UErrorCode &errorCode);
};

struct UnitCompoundSink : public UnitPathSink {
    UnitCompoundSink(UnitDataSink &sink) : UnitPathSink(sink) {}
    ~UnitCompoundSink();
    virtual void put(const char *key, UResourceValue &value, UErrorCode &errorCode);
};

struct UnitTypeSink : public UnitPathSink {
    UnitTypeSink(UnitDataSink &sink) : UnitPathSink(sink) {}
    ~UnitTypeSink();
    virtual UResourceTableSink *getOrCreateTableSink(
            const char *key, int32_t initialSize, UErrorCode &errorCode);
};

static const UChar g_LOCALE_units[] = {
    0x2F, 0x4C, 0x4F, 0x43, 0x41, 0x4C, 0x45, 0x2F,
    0x75, 0x6E, 0x69, 0x74, 0x73
};
static const UChar gShort[] = { 0x53, 0x68, 0x6F, 0x72, 0x74 };
static const UChar gNarrow[] = { 0x4E, 0x61, 0x72, 0x72, 0x6F, 0x77 };

struct UnitDataSink : public UResourceTableSink {
    UnitDataSink(MeasureFormatCacheData &outputData);
    ~UnitDataSink();
    virtual void put(const char *key, UResourceValue &value, UErrorCode &errorCode);
    virtual UResourceTableSink *getOrCreateTableSink(
            const char *key, int32_t initialSize, UErrorCode &errorCode);

    static UMeasureFormatWidth widthFromKey(const char *key) {
        if (uprv_strncmp(key, "units", 5) == 0) {
            key += 5;
            if (*key == 0) {
                return UMEASFMT_WIDTH_WIDE;
            } else if (uprv_strcmp(key, "Short") == 0) {
                return UMEASFMT_WIDTH_SHORT;
            } else if (uprv_strcmp(key, "Narrow") == 0) {
                return UMEASFMT_WIDTH_NARROW;
            }
        }
        return UMEASFMT_WIDTH_COUNT;
    }

    static UMeasureFormatWidth widthFromAlias(const UResourceValue &value, UErrorCode &errorCode) {
        int32_t length;
        const UChar *s = value.getAliasString(length, errorCode);
        // For example: "/LOCALE/unitsShort"
        if (U_SUCCESS(errorCode) && length >= 13 && u_memcmp(s, g_LOCALE_units, 13) == 0) {
            s += 13;
            length -= 13;
            if (*s == 0) {
                return UMEASFMT_WIDTH_WIDE;
            } else if (u_strCompare(s, length, gShort, 5, FALSE) == 0) {
                return UMEASFMT_WIDTH_SHORT;
            } else if (u_strCompare(s, length, gNarrow, 6, FALSE) == 0) {
                return UMEASFMT_WIDTH_NARROW;
            }
        }
        return UMEASFMT_WIDTH_COUNT;
    }

    // Output data.
    MeasureFormatCacheData &cacheData;

    // Path to current data.
    UMeasureFormatWidth width;
    const char *type;
    int32_t unitIndex;
    UBool hasPatterns;

    UnitTypeSink typeSink;
    UnitSubtypeSink subtypeSink;
    UnitCompoundSink compoundSink;
    UnitPatternSink patternSink;
};

UnitPathSink::~UnitPathSink() {}

UnitPatternSink::~UnitPatternSink() {}

void UnitPatternSink::put(const char *key, UResourceValue &value, UErrorCode &errorCode) {
    if (U_FAILURE(errorCode)) { return; }
    if (uprv_strcmp(key, "dnam") == 0) {
        // Skip display name for now.
    } else if (uprv_strcmp(key, "per") == 0) {
        dataSink.cacheData.
            setPerUnitFormatterIfAbsent(dataSink.unitIndex, dataSink.width, value, errorCode);
    } else {
        // The key must be one of the plural form strings.
        if (!dataSink.hasPatterns) {
            dataSink.cacheData.formatters[dataSink.unitIndex][dataSink.width].add(
                    key, value.getUnicodeString(errorCode), errorCode);
        }
    }
}

UnitSubtypeSink::~UnitSubtypeSink() {}

UResourceTableSink *UnitSubtypeSink::getOrCreateTableSink(
        const char *key, int32_t /* initialSize */, UErrorCode &errorCode) {
    if (U_FAILURE(errorCode)) { return NULL; }
    dataSink.unitIndex = MeasureUnit::internalGetIndexForTypeAndSubtype(dataSink.type, key);
    if (dataSink.unitIndex >= 0) {
        dataSink.hasPatterns =
            dataSink.cacheData.formatters[dataSink.unitIndex][dataSink.width].isValid();
        return &dataSink.patternSink;
    }
    return NULL;
}

UnitCompoundSink::~UnitCompoundSink() {}

void UnitCompoundSink::put(const char *key, UResourceValue &value, UErrorCode &errorCode) {
    if (U_SUCCESS(errorCode) && uprv_strcmp(key, "per") == 0) {
        dataSink.cacheData.perFormatters[dataSink.width].
                compile(value.getUnicodeString(errorCode), errorCode);
    }
}

UnitTypeSink::~UnitTypeSink() {}

UResourceTableSink *UnitTypeSink::getOrCreateTableSink(
        const char *key, int32_t /* initialSize */, UErrorCode &errorCode) {
    if (U_FAILURE(errorCode)) { return NULL; }
    if (uprv_strcmp(key, "currency") == 0) {
    } else if (uprv_strcmp(key, "compound") == 0) {
        if (!dataSink.cacheData.hasPerFormatter(dataSink.width)) {
            return &dataSink.compoundSink;
        }
    } else {
        dataSink.type = key;
        return &dataSink.subtypeSink;
    }
    return NULL;
}

UnitDataSink::UnitDataSink(MeasureFormatCacheData &outputData)
        : cacheData(outputData),
          width(UMEASFMT_WIDTH_COUNT), type(NULL), unitIndex(0), hasPatterns(FALSE),
          typeSink(*this), subtypeSink(*this), compoundSink(*this), patternSink(*this) {}

UnitDataSink::~UnitDataSink() {}

void UnitDataSink::put(const char *key, UResourceValue &value, UErrorCode &errorCode) {
    // Handle aliases like
    // units:alias{"/LOCALE/unitsShort"}
    // which should only occur in the root bundle.
    if (U_FAILURE(errorCode) || value.getType() != URES_ALIAS) { return; }
    UMeasureFormatWidth sourceWidth = widthFromKey(key);
    if (sourceWidth == UMEASFMT_WIDTH_COUNT) {
        // Alias from something we don't care about.
        return;
    }
    UMeasureFormatWidth targetWidth = widthFromAlias(value, errorCode);
    if (targetWidth == UMEASFMT_WIDTH_COUNT) {
        // We do not recognize what to fall back to.
        errorCode = U_INVALID_FORMAT_ERROR;
        return;
    }
    // Check that we do not fall back to another fallback.
    if (cacheData.widthFallback[targetWidth] != UMEASFMT_WIDTH_COUNT) {
        errorCode = U_INVALID_FORMAT_ERROR;
        return;
    }
    cacheData.widthFallback[sourceWidth] = targetWidth;
}

UResourceTableSink *UnitDataSink::getOrCreateTableSink(
        const char *key, int32_t /* initialSize */, UErrorCode &errorCode) {
    if (U_SUCCESS(errorCode) &&
            (width = widthFromKey(key)) != UMEASFMT_WIDTH_COUNT) {
        return &typeSink;
    }
    return NULL;
}

}  // namespace

static UBool loadMeasureUnitData(
        const UResourceBundle *resource,
        MeasureFormatCacheData &cacheData,
        UErrorCode &status) {
    UnitDataSink sink(cacheData);
    ures_getAllTableItemsWithFallback(resource, "", sink, status);
    return U_SUCCESS(status);
}

static UnicodeString loadNumericDateFormatterPattern(
        const UResourceBundle *resource,
        const char *pattern,
        UErrorCode &status) {
    UnicodeString result;
    if (U_FAILURE(status)) {
        return result;
    }
    CharString chs;
    chs.append("durationUnits", status)
            .append("/", status).append(pattern, status);
    LocalUResourceBundlePointer patternBundle(
            ures_getByKeyWithFallback(
                resource,
                chs.data(),
                NULL,
                &status));
    if (U_FAILURE(status)) {
        return result;
    }
    getString(patternBundle.getAlias(), result, status);
    // Replace 'h' with 'H'
    int32_t len = result.length();
    UChar *buffer = result.getBuffer(len);
    for (int32_t i = 0; i < len; ++i) {
        if (buffer[i] == 0x68) { // 'h'
            buffer[i] = 0x48; // 'H'
        }
    }
    result.releaseBuffer(len);
    return result;
}

static NumericDateFormatters *loadNumericDateFormatters(
        const UResourceBundle *resource,
        UErrorCode &status) {
    if (U_FAILURE(status)) {
        return NULL;
    }
    NumericDateFormatters *result = new NumericDateFormatters(
        loadNumericDateFormatterPattern(resource, "hm", status),
        loadNumericDateFormatterPattern(resource, "ms", status),
        loadNumericDateFormatterPattern(resource, "hms", status),
        status);
    if (U_FAILURE(status)) {
        delete result;
        return NULL;
    }
    return result;
}

template<> U_I18N_API
const MeasureFormatCacheData *LocaleCacheKey<MeasureFormatCacheData>::createObject(
        const void * /*unused*/, UErrorCode &status) const {
    const char *localeId = fLoc.getName();
    LocalUResourceBundlePointer unitsBundle(ures_open(U_ICUDATA_UNIT, localeId, &status));
    static UNumberFormatStyle currencyStyles[] = {
            UNUM_CURRENCY_PLURAL, UNUM_CURRENCY_ISO, UNUM_CURRENCY};
    LocalPointer<MeasureFormatCacheData> result(new MeasureFormatCacheData(), status);
    if (U_FAILURE(status)) {
        return NULL;
    }
    if (!loadMeasureUnitData(
            unitsBundle.getAlias(),
            *result,
            status)) {
        return NULL;
    }
    result->adoptNumericDateFormatters(loadNumericDateFormatters(
            unitsBundle.getAlias(), status));
    if (U_FAILURE(status)) {
        return NULL;
    }

    for (int32_t i = 0; i < WIDTH_INDEX_COUNT; ++i) {
        result->adoptCurrencyFormat(i, NumberFormat::createInstance(
                localeId, currencyStyles[i], status));
        if (U_FAILURE(status)) {
            return NULL;
        }
    }
    NumberFormat *inf = NumberFormat::createInstance(
            localeId, UNUM_DECIMAL, status);
    if (U_FAILURE(status)) {
        return NULL;
    }
    inf->setMaximumFractionDigits(0);
    DecimalFormat *decfmt = dynamic_cast<DecimalFormat *>(inf);
    if (decfmt != NULL) {
        decfmt->setRoundingMode(DecimalFormat::kRoundDown);
    }
    result->adoptIntegerFormat(inf);
    result->addRef();
    return result.orphan();
}

static UBool isTimeUnit(const MeasureUnit &mu, const char *tu) {
    return uprv_strcmp(mu.getType(), "duration") == 0 &&
            uprv_strcmp(mu.getSubtype(), tu) == 0;
}

// Converts a composite measure into hours-minutes-seconds and stores at hms
// array. [0] is hours; [1] is minutes; [2] is seconds. Returns a bit map of
// units found: 1=hours, 2=minutes, 4=seconds. For example, if measures
// contains hours-minutes, this function would return 3.
//
// If measures cannot be converted into hours, minutes, seconds or if amounts
// are negative, or if hours, minutes, seconds are out of order, returns 0.
static int32_t toHMS(
        const Measure *measures,
        int32_t measureCount,
        Formattable *hms,
        UErrorCode &status) {
    if (U_FAILURE(status)) {
        return 0;
    }
    int32_t result = 0;
    if (U_FAILURE(status)) {
        return 0;
    }
    // We use copy constructor to ensure that both sides of equality operator
    // are instances of MeasureUnit base class and not a subclass. Otherwise,
    // operator== will immediately return false.
    for (int32_t i = 0; i < measureCount; ++i) {
        if (isTimeUnit(measures[i].getUnit(), "hour")) {
            // hour must come first
            if (result >= 1) {
                return 0;
            }
            hms[0] = measures[i].getNumber();
            if (hms[0].getDouble() < 0.0) {
                return 0;
            }
            result |= 1;
        } else if (isTimeUnit(measures[i].getUnit(), "minute")) {
            // minute must come after hour
            if (result >= 2) {
                return 0;
            }
            hms[1] = measures[i].getNumber();
            if (hms[1].getDouble() < 0.0) {
                return 0;
            }
            result |= 2;
        } else if (isTimeUnit(measures[i].getUnit(), "second")) {
            // second must come after hour and minute
            if (result >= 4) {
                return 0;
            }
            hms[2] = measures[i].getNumber();
            if (hms[2].getDouble() < 0.0) {
                return 0;
            }
            result |= 4;
        } else {
            return 0;
        }
    }
    return result;
}


MeasureFormat::MeasureFormat(
        const Locale &locale, UMeasureFormatWidth w, UErrorCode &status)
        : cache(NULL),
          numberFormat(NULL),
          pluralRules(NULL),
          width(w),
          listFormatter(NULL) {
    initMeasureFormat(locale, w, NULL, status);
}

MeasureFormat::MeasureFormat(
        const Locale &locale,
        UMeasureFormatWidth w,
        NumberFormat *nfToAdopt,
        UErrorCode &status) 
        : cache(NULL),
          numberFormat(NULL),
          pluralRules(NULL),
          width(w),
          listFormatter(NULL) {
    initMeasureFormat(locale, w, nfToAdopt, status);
}

MeasureFormat::MeasureFormat(const MeasureFormat &other) :
        Format(other),
        cache(other.cache),
        numberFormat(other.numberFormat),
        pluralRules(other.pluralRules),
        width(other.width),
        listFormatter(NULL) {
    cache->addRef();
    numberFormat->addRef();
    pluralRules->addRef();
    listFormatter = new ListFormatter(*other.listFormatter);
}

MeasureFormat &MeasureFormat::operator=(const MeasureFormat &other) {
    if (this == &other) {
        return *this;
    }
    Format::operator=(other);
    SharedObject::copyPtr(other.cache, cache);
    SharedObject::copyPtr(other.numberFormat, numberFormat);
    SharedObject::copyPtr(other.pluralRules, pluralRules);
    width = other.width;
    delete listFormatter;
    listFormatter = new ListFormatter(*other.listFormatter);
    return *this;
}

MeasureFormat::MeasureFormat() :
        cache(NULL),
        numberFormat(NULL),
        pluralRules(NULL),
        width(UMEASFMT_WIDTH_SHORT),
        listFormatter(NULL) {
}

MeasureFormat::~MeasureFormat() {
    if (cache != NULL) {
        cache->removeRef();
    }
    if (numberFormat != NULL) {
        numberFormat->removeRef();
    }
    if (pluralRules != NULL) {
        pluralRules->removeRef();
    }
    delete listFormatter;
}

UBool MeasureFormat::operator==(const Format &other) const {
    if (this == &other) { // Same object, equal
        return TRUE;
    }
    if (!Format::operator==(other)) {
        return FALSE;
    }
    const MeasureFormat &rhs = static_cast<const MeasureFormat &>(other);

    // Note: Since the ListFormatter depends only on Locale and width, we
    // don't have to check it here.

    // differing widths aren't equivalent
    if (width != rhs.width) {
        return FALSE;
    }
    // Width the same check locales.
    // We don't need to check locales if both objects have same cache.
    if (cache != rhs.cache) {
        UErrorCode status = U_ZERO_ERROR;
        const char *localeId = getLocaleID(status);
        const char *rhsLocaleId = rhs.getLocaleID(status);
        if (U_FAILURE(status)) {
            // On failure, assume not equal
            return FALSE;
        }
        if (uprv_strcmp(localeId, rhsLocaleId) != 0) {
            return FALSE;
        }
    }
    // Locales same, check NumberFormat if shared data differs.
    return (
            numberFormat == rhs.numberFormat ||
            **numberFormat == **rhs.numberFormat);
}

Format *MeasureFormat::clone() const {
    return new MeasureFormat(*this);
}

UnicodeString &MeasureFormat::format(
        const Formattable &obj,
        UnicodeString &appendTo,
        FieldPosition &pos,
        UErrorCode &status) const {
    if (U_FAILURE(status)) return appendTo;
    if (obj.getType() == Formattable::kObject) {
        const UObject* formatObj = obj.getObject();
        const Measure* amount = dynamic_cast<const Measure*>(formatObj);
        if (amount != NULL) {
            return formatMeasure(
                    *amount, **numberFormat, appendTo, pos, status);
        }
    }
    status = U_ILLEGAL_ARGUMENT_ERROR;
    return appendTo;
}

void MeasureFormat::parseObject(
        const UnicodeString & /*source*/,
        Formattable & /*result*/,
        ParsePosition& /*pos*/) const {
    return;
}

UnicodeString &MeasureFormat::formatMeasurePerUnit(
        const Measure &measure,
        const MeasureUnit &perUnit,
        UnicodeString &appendTo,
        FieldPosition &pos,
        UErrorCode &status) const {
    if (U_FAILURE(status)) {
        return appendTo;
    }
    MeasureUnit *resolvedUnit =
            MeasureUnit::resolveUnitPerUnit(measure.getUnit(), perUnit);
    if (resolvedUnit != NULL) {
        Measure newMeasure(measure.getNumber(), resolvedUnit, status);
        return formatMeasure(
                newMeasure, **numberFormat, appendTo, pos, status);
    }
    FieldPosition fpos(pos.getField());
    UnicodeString result;
    int32_t offset = withPerUnitAndAppend(
            formatMeasure(
                    measure, **numberFormat, result, fpos, status),
            perUnit,
            appendTo,
            status);
    if (U_FAILURE(status)) {
        return appendTo;
    }
    if (fpos.getBeginIndex() != 0 || fpos.getEndIndex() != 0) {
        pos.setBeginIndex(fpos.getBeginIndex() + offset);
        pos.setEndIndex(fpos.getEndIndex() + offset);
    }
    return appendTo;
}

UnicodeString &MeasureFormat::formatMeasures(
        const Measure *measures,
        int32_t measureCount,
        UnicodeString &appendTo,
        FieldPosition &pos,
        UErrorCode &status) const {
    if (U_FAILURE(status)) {
        return appendTo;
    }
    if (measureCount == 0) {
        return appendTo;
    }
    if (measureCount == 1) {
        return formatMeasure(measures[0], **numberFormat, appendTo, pos, status);
    }
    if (width == UMEASFMT_WIDTH_NUMERIC) {
        Formattable hms[3];
        int32_t bitMap = toHMS(measures, measureCount, hms, status);
        if (bitMap > 0) {
            return formatNumeric(hms, bitMap, appendTo, status);
        }
    }
    if (pos.getField() != FieldPosition::DONT_CARE) {
        return formatMeasuresSlowTrack(
                measures, measureCount, appendTo, pos, status);
    }
    UnicodeString *results = new UnicodeString[measureCount];
    if (results == NULL) {
        status = U_MEMORY_ALLOCATION_ERROR;
        return appendTo;
    }
    for (int32_t i = 0; i < measureCount; ++i) {
        const NumberFormat *nf = cache->getIntegerFormat();
        if (i == measureCount - 1) {
            nf = numberFormat->get();
        }
        formatMeasure(
                measures[i],
                *nf,
                results[i],
                pos,
                status);
    }
    listFormatter->format(results, measureCount, appendTo, status);
    delete [] results; 
    return appendTo;
}

void MeasureFormat::initMeasureFormat(
        const Locale &locale,
        UMeasureFormatWidth w,
        NumberFormat *nfToAdopt,
        UErrorCode &status) {
    static const char *listStyles[] = {"unit", "unit-short", "unit-narrow"};
    LocalPointer<NumberFormat> nf(nfToAdopt);
    if (U_FAILURE(status)) {
        return;
    }
    const char *name = locale.getName();
    setLocaleIDs(name, name);

    UnifiedCache::getByLocale(locale, cache, status);
    if (U_FAILURE(status)) {
        return;
    }

    const SharedPluralRules *pr = PluralRules::createSharedInstance(
            locale, UPLURAL_TYPE_CARDINAL, status);
    if (U_FAILURE(status)) {
        return;
    }
    SharedObject::copyPtr(pr, pluralRules);
    pr->removeRef();
    if (nf.isNull()) {
        const SharedNumberFormat *shared = NumberFormat::createSharedInstance(
                locale, UNUM_DECIMAL, status);
        if (U_FAILURE(status)) {
            return;
        }
        SharedObject::copyPtr(shared, numberFormat);
        shared->removeRef();
    } else {
        adoptNumberFormat(nf.orphan(), status);
        if (U_FAILURE(status)) {
            return;
        }
    }
    width = w;
    delete listFormatter;
    listFormatter = ListFormatter::createInstance(
            locale,
            listStyles[widthToIndex(width)],
            status);
}

void MeasureFormat::adoptNumberFormat(
        NumberFormat *nfToAdopt, UErrorCode &status) {
    LocalPointer<NumberFormat> nf(nfToAdopt);
    if (U_FAILURE(status)) {
        return;
    }
    SharedNumberFormat *shared = new SharedNumberFormat(nf.getAlias());
    if (shared == NULL) {
        status = U_MEMORY_ALLOCATION_ERROR;
        return;
    }
    nf.orphan();
    SharedObject::copyPtr(shared, numberFormat);
}

UBool MeasureFormat::setMeasureFormatLocale(const Locale &locale, UErrorCode &status) {
    if (U_FAILURE(status) || locale == getLocale(status)) {
        return FALSE;
    }
    initMeasureFormat(locale, width, NULL, status);
    return U_SUCCESS(status);
} 

const NumberFormat &MeasureFormat::getNumberFormat() const {
    return **numberFormat;
}

const PluralRules &MeasureFormat::getPluralRules() const {
    return **pluralRules;
}

Locale MeasureFormat::getLocale(UErrorCode &status) const {
    return Format::getLocale(ULOC_VALID_LOCALE, status);
}

const char *MeasureFormat::getLocaleID(UErrorCode &status) const {
    return Format::getLocaleID(ULOC_VALID_LOCALE, status);
}

UnicodeString &MeasureFormat::formatMeasure(
        const Measure &measure,
        const NumberFormat &nf,
        UnicodeString &appendTo,
        FieldPosition &pos,
        UErrorCode &status) const {
    if (U_FAILURE(status)) {
        return appendTo;
    }
    const Formattable& amtNumber = measure.getNumber();
    const MeasureUnit& amtUnit = measure.getUnit();
    if (isCurrency(amtUnit)) {
        UChar isoCode[4];
        u_charsToUChars(amtUnit.getSubtype(), isoCode, 4);
        return cache->getCurrencyFormat(widthToIndex(width))->format(
                new CurrencyAmount(amtNumber, isoCode, status),
                appendTo,
                pos,
                status);
    }
    const QuantityFormatter *quantityFormatter = getQuantityFormatter(
            amtUnit.getIndex(), widthToIndex(width), status);
    if (U_FAILURE(status)) {
        return appendTo;
    }
    return quantityFormatter->format(
            amtNumber,
            nf,
            **pluralRules,
            appendTo,
            pos,
            status);
}

// Formats hours-minutes-seconds as 5:37:23 or similar.
UnicodeString &MeasureFormat::formatNumeric(
        const Formattable *hms,  // always length 3
        int32_t bitMap,   // 1=hourset, 2=minuteset, 4=secondset
        UnicodeString &appendTo,
        UErrorCode &status) const {
    if (U_FAILURE(status)) {
        return appendTo;
    }
    UDate millis = 
        (UDate) (((uprv_trunc(hms[0].getDouble(status)) * 60.0
             + uprv_trunc(hms[1].getDouble(status))) * 60.0
                  + uprv_trunc(hms[2].getDouble(status))) * 1000.0);
    switch (bitMap) {
    case 5: // hs
    case 7: // hms
        return formatNumeric(
                millis,
                cache->getNumericDateFormatters()->hourMinuteSecond,
                UDAT_SECOND_FIELD,
                hms[2],
                appendTo,
                status);
        break;
    case 6: // ms
        return formatNumeric(
                millis,
                cache->getNumericDateFormatters()->minuteSecond,
                UDAT_SECOND_FIELD,
                hms[2],
                appendTo,
                status);
        break;
    case 3: // hm
        return formatNumeric(
                millis,
                cache->getNumericDateFormatters()->hourMinute,
                UDAT_MINUTE_FIELD,
                hms[1],
                appendTo,
                status);
        break;
    default:
        status = U_INTERNAL_PROGRAM_ERROR;
        return appendTo;
        break;
    }
    return appendTo;
}

static void appendRange(
        const UnicodeString &src,
        int32_t start,
        int32_t end,
        UnicodeString &dest) {
    dest.append(src, start, end - start);
}

static void appendRange(
        const UnicodeString &src,
        int32_t end,
        UnicodeString &dest) {
    dest.append(src, end, src.length() - end);
}

// Formats time like 5:37:23
UnicodeString &MeasureFormat::formatNumeric(
        UDate date, // Time since epoch 1:30:00 would be 5400000
        const DateFormat &dateFmt, // h:mm, m:ss, or h:mm:ss
        UDateFormatField smallestField, // seconds in 5:37:23.5
        const Formattable &smallestAmount, // 23.5 for 5:37:23.5
        UnicodeString &appendTo,
        UErrorCode &status) const {
    if (U_FAILURE(status)) {
        return appendTo;
    }
    // Format the smallest amount with this object's NumberFormat
    UnicodeString smallestAmountFormatted;

    // We keep track of the integer part of smallest amount so that
    // we can replace it later so that we get '0:00:09.3' instead of
    // '0:00:9.3'
    FieldPosition intFieldPosition(UNUM_INTEGER_FIELD);
    (*numberFormat)->format(
            smallestAmount, smallestAmountFormatted, intFieldPosition, status);
    if (
            intFieldPosition.getBeginIndex() == 0 &&
            intFieldPosition.getEndIndex() == 0) {
        status = U_INTERNAL_PROGRAM_ERROR;
        return appendTo;
    }

    // Format time. draft becomes something like '5:30:45'
    FieldPosition smallestFieldPosition(smallestField);
    UnicodeString draft;
    dateFmt.format(date, draft, smallestFieldPosition, status);

    // If we find field for smallest amount replace it with the formatted
    // smallest amount from above taking care to replace the integer part
    // with what is in original time. For example, If smallest amount
    // is 9.35s and the formatted time is 0:00:09 then 9.35 becomes 09.35
    // and replacing yields 0:00:09.35
    if (smallestFieldPosition.getBeginIndex() != 0 ||
            smallestFieldPosition.getEndIndex() != 0) {
        appendRange(draft, 0, smallestFieldPosition.getBeginIndex(), appendTo);
        appendRange(
                smallestAmountFormatted,
                0,
                intFieldPosition.getBeginIndex(),
                appendTo);
        appendRange(
                draft,
                smallestFieldPosition.getBeginIndex(),
                smallestFieldPosition.getEndIndex(),
                appendTo);
        appendRange(
                smallestAmountFormatted,
                intFieldPosition.getEndIndex(),
                appendTo);
        appendRange(
                draft,
                smallestFieldPosition.getEndIndex(),
                appendTo);
    } else {
        appendTo.append(draft);
    }
    return appendTo;
}

const QuantityFormatter *MeasureFormat::getQuantityFormatter(
        int32_t index,
        int32_t widthIndex,
        UErrorCode &status) const {
    if (U_FAILURE(status)) {
        return NULL;
    }
    const QuantityFormatter *formatters =
            cache->formatters[index];
    if (formatters[widthIndex].isValid()) {
        return &formatters[widthIndex];
    }
    int32_t fallbackWidth = cache->widthFallback[widthIndex];
    if (fallbackWidth != UMEASFMT_WIDTH_COUNT && formatters[fallbackWidth].isValid()) {
        return &formatters[fallbackWidth];
    }
    status = U_MISSING_RESOURCE_ERROR;
    return NULL;
}

const SimplePatternFormatter *MeasureFormat::getPerUnitFormatter(
        int32_t index,
        int32_t widthIndex) const {
    const SimplePatternFormatter * const * perUnitFormatters =
            cache->getPerUnitFormattersByIndex(index);
    if (perUnitFormatters[widthIndex] != NULL) {
        return perUnitFormatters[widthIndex];
    }
    int32_t fallbackWidth = cache->widthFallback[widthIndex];
    if (fallbackWidth != UMEASFMT_WIDTH_COUNT && perUnitFormatters[fallbackWidth] != NULL) {
        return perUnitFormatters[fallbackWidth];
    }
    return NULL;
}

const SimplePatternFormatter *MeasureFormat::getPerFormatter(
        int32_t widthIndex,
        UErrorCode &status) const {
    if (U_FAILURE(status)) {
        return NULL;
    }
    const SimplePatternFormatter * perFormatters = cache->perFormatters;
    if (perFormatters[widthIndex].getPlaceholderCount() == 2) {
        return &perFormatters[widthIndex];
    }
    int32_t fallbackWidth = cache->widthFallback[widthIndex];
    if (fallbackWidth != UMEASFMT_WIDTH_COUNT &&
            perFormatters[fallbackWidth].getPlaceholderCount() == 2) {
        return &perFormatters[fallbackWidth];
    }
    status = U_MISSING_RESOURCE_ERROR;
    return NULL;
}

static void getPerUnitString(
        const QuantityFormatter &formatter,
        UnicodeString &result) {
    result = formatter.getByVariant("one")->getPatternWithNoPlaceholders();
    result.trim();
}

int32_t MeasureFormat::withPerUnitAndAppend(
        const UnicodeString &formatted,
        const MeasureUnit &perUnit,
        UnicodeString &appendTo,
        UErrorCode &status) const {
    int32_t offset = -1;
    if (U_FAILURE(status)) {
        return offset;
    }
    const SimplePatternFormatter *perUnitFormatter = getPerUnitFormatter(
            perUnit.getIndex(), widthToIndex(width));
    if (perUnitFormatter != NULL) {
        const UnicodeString *params[] = {&formatted};
        perUnitFormatter->formatAndAppend(
                params,
                UPRV_LENGTHOF(params),
                appendTo,
                &offset,
                1,
                status);
        return offset;
    }
    const SimplePatternFormatter *perFormatter = getPerFormatter(
            widthToIndex(width), status);
    const QuantityFormatter *qf = getQuantityFormatter(
            perUnit.getIndex(), widthToIndex(width), status);
    if (U_FAILURE(status)) {
        return offset;
    }
    UnicodeString perUnitString;
    getPerUnitString(*qf, perUnitString);
    const UnicodeString *params[] = {&formatted, &perUnitString};
    perFormatter->formatAndAppend(
            params,
            UPRV_LENGTHOF(params),
            appendTo,
            &offset,
            1,
            status);
    return offset;
}

UnicodeString &MeasureFormat::formatMeasuresSlowTrack(
        const Measure *measures,
        int32_t measureCount,
        UnicodeString& appendTo,
        FieldPosition& pos,
        UErrorCode& status) const {
    if (U_FAILURE(status)) {
        return appendTo;
    }
    FieldPosition dontCare(FieldPosition::DONT_CARE);
    FieldPosition fpos(pos.getField());
    UnicodeString *results = new UnicodeString[measureCount];
    int32_t fieldPositionFoundIndex = -1;
    for (int32_t i = 0; i < measureCount; ++i) {
        const NumberFormat *nf = cache->getIntegerFormat();
        if (i == measureCount - 1) {
            nf = numberFormat->get();
        }
        if (fieldPositionFoundIndex == -1) {
            formatMeasure(measures[i], *nf, results[i], fpos, status);
            if (U_FAILURE(status)) {
                delete [] results;
                return appendTo;
            }
            if (fpos.getBeginIndex() != 0 || fpos.getEndIndex() != 0) {
                fieldPositionFoundIndex = i;
            }
        } else {
            formatMeasure(measures[i], *nf, results[i], dontCare, status);
        }
    }
    int32_t offset;
    listFormatter->format(
            results,
            measureCount,
            appendTo,
            fieldPositionFoundIndex,
            offset,
            status);
    if (U_FAILURE(status)) {
        delete [] results;
        return appendTo;
    }
    if (offset != -1) {
        pos.setBeginIndex(fpos.getBeginIndex() + offset);
        pos.setEndIndex(fpos.getEndIndex() + offset);
    }
    delete [] results;
    return appendTo;
}

MeasureFormat* U_EXPORT2 MeasureFormat::createCurrencyFormat(const Locale& locale,
                                                   UErrorCode& ec) {
    CurrencyFormat* fmt = NULL;
    if (U_SUCCESS(ec)) {
        fmt = new CurrencyFormat(locale, ec);
        if (U_FAILURE(ec)) {
            delete fmt;
            fmt = NULL;
        }
    }
    return fmt;
}

MeasureFormat* U_EXPORT2 MeasureFormat::createCurrencyFormat(UErrorCode& ec) {
    if (U_FAILURE(ec)) {
        return NULL;
    }
    return MeasureFormat::createCurrencyFormat(Locale::getDefault(), ec);
}

U_NAMESPACE_END

#endif /* #if !UCONFIG_NO_FORMATTING */
