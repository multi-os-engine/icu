#pragma once

#include "cf_currency_code.h"

class cf_Currency {

    cf_CurrencyCode _code;
    cf_String _symbol;
    cf_Locale  locale;

public:
    //construct default currency for given country code
    cf_Currency (const std::string & countryCode)
        : _code(getCurrencyCodeFromCache(countryCode))
        , _symbol(getCurrencySymbolFromCache(countryCode)){
    }

    //construct with specific code, most likely, it affects symbol representation
    cf_Currency (const cf_Locale &  locale, const cf_CurrencyCode & code)
        : _code(code)
        , locale(locale) {
            
        auto nativeCode = getCurrencyCode(locale);
        if (nativeCode.code() != _code.code()) {
            //CoreFoundation has ability to format number if to set currency code
            cf_AutoRef<CFNumberFormatterRef> formatter( CFNumberFormatterCreate ( nullptr, locale, kCFNumberFormatterCurrencyStyle));
            
            CFNumberFormatterSetProperty ( formatter, kCFNumberFormatterCurrencyCode, _code.code());

            //setting symbol that initicates currency symbol in formater : usualy it is "¤"
            CFNumberFormatterSetFormat ( formatter, CFSTR("¤"));
            
            int val = 0;
            cf_String formatedValue (CFNumberFormatterCreateStringWithValue ( nullptr,  formatter, kCFNumberIntType, &val));
            
            //now string consist of $0 or €0 etc, so have to cut 0 symbol
            _symbol = formatedValue.substring(0, formatedValue.length()-1);
            
            // backup plan if above gives something bad
            //_symbol = _code.code();
        } else {
            _symbol = getCurrencySymbol(locale);
        }
    }
    //construct with default currency code for given locale
    cf_Currency (const cf_Locale &  locale)
        : _code(getCurrencyCode(locale))
        , _symbol(getCurrencySymbol(locale))
        , locale(locale) {
    }

    //returns currency symbolic code for given locale, i.e. $ or €
    const cf_String & symbol() const {
        return _symbol;
    }

    const cf_String & code() const {
        return _code.code();
    }
    
    cf_String  displayName() const {
        if (locale.isBogus()) {
            return cf_String();
        }
        
        return CFLocaleCopyDisplayNameForPropertyValue ( locale, kCFLocaleCurrencyCode, code() );
    }
    
    int fractionDigits() const {
        int32_t defaultFractionDigits;
        double roundingIncrement;

        Boolean res = CFNumberFormatterGetDecimalInfoForCurrencyCode ( code(), &defaultFractionDigits, &roundingIncrement);

        return defaultFractionDigits;
    }
    
    static std::vector<std::string> getAvailableCodes() {
        std::vector<std::string> codesOnly;
        //TODO: cache unique currencies
        for( const auto & currency : cf_CurrencyCache::getCurrencyCache()){
            if (currency.second.code.length() != 3) {
                continue;
            }
            codesOnly.push_back(currency.second.code.c_str());
        }
        return codesOnly;
    }
    
private :
    
    static cf_CurrencyCode getCurrencyCode(const cf_Locale & locale) {

        cf_CurrencyCode code(cf_String::fromCFGetFunction(locale.getValue(kCFLocaleCurrencyCode)));
        
        if (!code.isValid()) {
            code = getCurrencyCodeFromCache(locale);
        }
        
        return code;
    }
    
    static cf_String getCurrencySymbol(const cf_Locale & locale) {
        auto  sym(cf_String::fromCFGetFunction(locale.getValue(kCFLocaleCurrencySymbol)));
        
        if (sym.length() == 0) {
            sym = getCurrencySymbolFromCache(locale);
        }
        
        return sym;
    }
    static cf_String getCurrencyCodeFromCache(const std::string & countryCode) {
        try {
            return cf_CurrencyCache::getCurrencyCache().at(countryCode).code;
        }catch(...)  {
            return cf_String();
        }
    }
    
    static cf_String getCurrencySymbolFromCache(const std::string & countryCode) {
        try {
            return cf_CurrencyCache::getCurrencyCache().at(countryCode).symbol;
        }catch(...) {
            return cf_String();
        }
    }
    static cf_String getCurrencyCodeFromCache(const cf_Locale & locale) {
        try {
            std::string countryCode;
            if (!cf_CurrencyCache::getCountryCode(locale.getBaseName(), countryCode)) {
                return cf_String();
            }
            return cf_CurrencyCache::getCurrencyCache().at(countryCode).code;
        }catch(...)
        {
            return cf_String();
        }
    }
    
    static cf_String getCurrencySymbolFromCache(const cf_Locale & locale) {
        try {
            std::string countryCode;
            if (!cf_CurrencyCache::getCountryCode(locale.getBaseName(), countryCode)) {
                return cf_String();
            }
            return cf_CurrencyCache::getCurrencyCache().at(countryCode).symbol;
        }catch(...) {
            return cf_String();
        }
    }
};