#pragma once
#include <cf_currency_cache.h>

class cf_CurrencyCode {
    cf_String currencyCodeForLocale;
    bool mIsValid;
    
public:
    cf_CurrencyCode (const cf_String  & currencyCode)
    : mIsValid(checkIsValid(currencyCode)) {
        if (!isValid()) {
            return;
        }
        currencyCodeForLocale = currencyCode;
    }
    cf_CurrencyCode (const cf_String  & currencyCode, UErrorCode & err)
    : mIsValid(checkIsValid(currencyCode)) {
        if (!isValid()) {
            err = U_ILLEGAL_ARGUMENT_ERROR;
            return;
        }
        currencyCodeForLocale = currencyCode;
    }
    
    const cf_String & code() const {
        return currencyCodeForLocale;
    }
    bool isValid() const {
        return mIsValid;
    }
private:
    static bool checkIsValid(const cf_String & currencyCode) {
        int32_t defaultFractionDigits;
        double roundingIncrement;
        
        if (nullptr == (CFStringRef)currencyCode) {
            return false;
        }
        
        Boolean res = CFNumberFormatterGetDecimalInfoForCurrencyCode (                                                                      currencyCode, &defaultFractionDigits, &roundingIncrement);
        
        if (res == FALSE) {
            return false;
        }
        //TODO: CF seems cannot understand BOGO-DOLLARS as invalid currency code
        if (currencyCode.length() != 3) {
            return false;
        }
        
        //rfc 4217 - technical code, XXX, XTS, android dont have tests against XTS code, will do so
        if (((std::string)currencyCode) == "XXX") {
            return true;
        }
        
        //last chance is to look at currencies store; TODO: remap currencies to be key, instead of value
        for (const auto & item : cf_CurrencyCache::getCurrencyCache()) {
            if (item.second.code == currencyCode) {
                return true;
            }
        }
        
        return false;
    }
    
};
