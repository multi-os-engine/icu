#pragma once

#include <map>
class cf_CurrencyCache {
public:
    struct CacheRecord {
        bool isValid;
        cf_String code;
        cf_String symbol;
        CacheRecord () : isValid (true) {
            
        }
    };
    
    static bool getCountryCodeViaCF(const std::string & localeName, std::string & countryCode) {
        cf_AutoRef<CFDictionaryRef> dict = CFLocaleCreateComponentsFromLocaleIdentifier ( nullptr, cf_String(localeName) );
        CFStringRef value;
        if (FALSE == CFDictionaryGetValueIfPresent(dict, kCFLocaleCountryCode, reinterpret_cast<const void**>(&value))) {
            return false;
        }
        countryCode = cf_String::fromCFGetFunction(value);
        return true;
    }
    
    static bool getCountryCode(const std::string & localeName, std::string &countryCode) {
        
        if (!getCountryCodeViaCF(localeName, countryCode)) {
            return false;
        }
        
        //making country code upper-case
        //auto to_upper = [] (char ch) { return std::use_facet<std::ctype<char>>(std::locale()).toupper(ch); };
        //std::transform(countryCode.begin(), countryCode.end(), countryCode.begin(), to_upper);
        
        return true;
    }
    
    static const std::map<std::string, CacheRecord > & getCurrencyCache() {
        cacheCurrencies();
        return getCountryToCurrencyCache();
    }
    
private:
    
    static void cacheCurrencies() {
        auto & cache = getCountryToCurrencyCache();
        if (!cache.empty())
            return;
        
        //creating special country based currency codes hash
        cf_Array<CFStringRef> ar = CFLocaleCopyAvailableLocaleIdentifiers();
        std::vector<std::string> locales = ar;
        for (const std::string & localeName : locales) {
            
            std::string countryCode;
            auto isCountryCode = getCountryCode(localeName, countryCode);
            
            //skip locales that missed countrycode
            if (!isCountryCode) {
                continue;
            }
            
            //country has incompatible currency values among variants, we will report curency as "" in that case
            if (!cache[countryCode].isValid)
                continue;
            
            cf_Locale locale(localeName);
            auto currencyCode = cf_String::fromCFGetFunction(CFLocaleGetValue(locale, kCFLocaleCurrencyCode));
            auto & currency = cache[countryCode];
            
            if (currency.code.length() != 0 && currencyCode.length() != 0 && (currency.code != currencyCode)) {
                currency.isValid = false;
                currency.code.clear();
                currency.symbol.clear();
                continue;
            }
            currency.code = currencyCode;
            currency.symbol = cf_String::fromCFGetFunction(CFLocaleGetValue (  locale, kCFLocaleCurrencySymbol ));
            
            // printf("locale = %s: %s-%s\n", locale.getBaseName(), currency.code.c_str(), currency.symbol.c_str());
        }
        
    }
    
    static std::map<std::string, CacheRecord > & getCountryToCurrencyCache() {
        static std::map<std::string, CacheRecord > countryToCurrencyMap;
        return countryToCurrencyMap;
    }
};
