#pragma once

#import  <CoreFoundation/CoreFoundation.h>
#include <cf_auto_ref.h>
#include <cf_string.h>
#include <cf_array.h>

namespace U_ICU_NAMESPACE {
    
    class cf_Locale
    {
        bool _isBogus;
        cf_AutoRef<CFLocaleRef> _cfLocale;
        mutable cf_String _cfBaseName;
        mutable cf_String _iso3Language;
        mutable cf_String _iso3Country;
        mutable cf_String _country;
        mutable std::string _scriptCode;
        cf_String _cfFullName;

    private:
        //not public API but cf_* classes used that to wrap locale obtained using different approaches
        friend class cf_Calendar;
        friend class cf_DateTimePatterns;
        
        cf_Locale(CFLocaleRef locale)
            : _isBogus ()
            , _cfLocale (locale)
            , _cfBaseName(nullptr)
            , _country(nullptr)
            , _iso3Country(nullptr)
            , _iso3Language(nullptr)
            , _cfFullName(CFLocaleGetIdentifier(_cfLocale)) {

        }
    public:
        operator CFLocaleRef() const {
            return _cfLocale;
        }
        cf_Locale(const std::string &name)
            : _isBogus ()
            , _cfFullName(name)
            , _cfBaseName(nullptr)
            , _country(nullptr)
            , _iso3Country(nullptr)
            , _iso3Language(nullptr) {
            _cfLocale = CFLocaleCreate(nullptr, _cfFullName);
        }
        cf_Locale()
            : _isBogus ()
            , _cfBaseName(nullptr)
            , _cfFullName(nullptr)
            , _country(nullptr)
            , _iso3Country(nullptr)
            , _iso3Language(nullptr) {
        }

        void setToBogus() {
            _isBogus = true;
        }
        bool isBogus() const {
            return _isBogus;
        }
        
        static cf_Locale createFromName(const char * name) {
            return cf_Locale(name);
        }
        /**
         * Returns the locale's ISO-15924 abbreviation script code.
         * @return      An alias to the code
         * @see uscript_getShortName
         * @see uscript_getCode
         * @stable ICU 2.8
         */
        inline const char * getScript() const {
            _scriptCode = cf_String::fromCFGetFunction(CFLocaleGetValue(_cfLocale, kCFLocaleScriptCode));
            return _scriptCode.c_str();
        }
        /**
         * Returns the programmatic name of the entire locale, with the language,
         * country and variant separated by underbars. If a field is missing, up
         * to two leading underbars will occur. Example: "en", "de_DE", "en_US_WIN",
         * "de__POSIX", "fr__MAC", "__MAC", "_MT", "_FR_EURO"
         * @return      A pointer to "name".
         * @stable ICU 2.0
         */
        inline const char * getName() const {
            return _cfFullName.c_str();
        }
        
        /**
         * Returns the programmatic name of the entire locale as getName() would return,
         * but without keywords.
         * @return      A pointer to "name".
         * @see getName
         * @stable ICU 2.8
         */
        const char * getBaseName() const {
            
            if (_cfBaseName.length() == 0 && _cfFullName.length() != 0 ) {
                _cfBaseName = CFLocaleCreateCanonicalLocaleIdentifierFromString(nullptr, _cfFullName);
            }

            return _cfBaseName.c_str();
        }
        
        /**
         * Fills in "dispLang" with the name of this locale's language in a format suitable for
         * user display in the locale specified by "displayLocale".  For example, if the locale's
         * language code is "en" and displayLocale's language code is "fr", this function would set
         * dispLang to "Anglais".
         * @param displayLocale  Specifies the locale to be used to display the name.  In other words,
         *                  if the locale's language code is "en", passing Locale::getFrench() for
         *                  displayLocale would result in "Anglais", while passing Locale::getGerman()
         *                  for displayLocale would result in "Englisch".
         * @param dispLang  Receives the language's display name.
         * @return          A reference to "dispLang".
         * @stable ICU 2.0
         */
        UnicodeString&  getDisplayLanguage( const   cf_Locale& displayLocale,
                                           UnicodeString&  dispLang) const {
            
            dispLang = translateValue(displayLocale, kCFLocaleLanguageCode);
            return dispLang;
        }

        /**
         * Fills in "dispCountry" with the name of this locale's country in a format suitable
         * for user display in the locale specified by "displayLocale".  For example, if the locale's
         * country code is "US" and displayLocale's language code is "fr", this function would set
         * dispCountry to "&Eacute;tats-Unis".
         * @param displayLocale      Specifies the locale to be used to display the name.  In other
         *                      words, if the locale's country code is "US", passing
         *                      Locale::getFrench() for displayLocale would result in "&Eacute;tats-Unis", while
         *                      passing Locale::getGerman() for displayLocale would result in
         *                      "Vereinigte Staaten".
         * @param dispCountry   Receives the country's display name.
         * @return              A reference to "dispCountry".
         * @stable ICU 2.0
         */
        UnicodeString&  getDisplayCountry(  const   cf_Locale&         displayLocale,
                                          UnicodeString&  dispCountry) const {
            dispCountry = translateValue(displayLocale, kCFLocaleCountryCode);
            return dispCountry;
        }
        
        /**
         * Fills in "dispScript" with the name of this locale's country in a format suitable
         * for user display in the locale specified by "displayLocale".  For example, if the locale's
         * script code is "LATN" and displayLocale's language code is "en", this function would set
         * dispScript to "Latin".
         * @param displayLocale      Specifies the locale to be used to display the name.  In other
         *                      words, if the locale's script code is "LATN", passing
         *                      Locale::getFrench() for displayLocale would result in "", while
         *                      passing Locale::getGerman() for displayLocale would result in
         *                      "".
         * @param dispScript    Receives the scripts's display name.
         * @return              A reference to "dispScript".
         * @stable ICU 2.8
         */
        UnicodeString&  getDisplayScript(const cf_Locale& displayLocale,
                                         UnicodeString&  dispScript) const {
            dispScript = translateValue(displayLocale, kCFLocaleScriptCode);
            return dispScript;
        }
        
        /**
         * Fills in "dispVar" with the name of this locale's variant code in a format
         * suitable for user display in the locale specified by "displayLocale".
         * @param displayLocale  Specifies the locale to be used to display the name.
         * @param dispVar   Receives the variant's display name.
         * @return          A reference to "dispVar".
         * @stable ICU 2.0
         */
        UnicodeString&  getDisplayVariant(const cf_Locale& displayLocale,
                                          UnicodeString&  dispVar) const {
            dispVar = translateValue(displayLocale, kCFLocaleVariantCode);
            return dispVar;
        }
        
        /**
         * returns the locale's three-letter language code, as specified
         * in ISO draft standard ISO-639-2.
         * @return      An alias to the code, or an empty string
         * @stable ICU 2.0
         */
        const char * getISO3Language() const{
            _iso3Language = cf_String::fromCFGetFunction(CFLocaleGetValue(_cfLocale, kCFLocaleLanguageCode));
            return _iso3Language.c_str();
        }
        
        /**
         * Fills in "name" with the locale's three-letter ISO-3166 country code.
         * @return      An alias to the code, or an empty string
         * @stable ICU 2.0
         */
        const char * getISO3Country() const{
            _iso3Country = cf_String::fromCFGetFunction(CFLocaleGetValue(_cfLocale, kCFLocaleCountryCode));
            return _iso3Country.c_str();
        }
        
        /**
         * Returns the locale's ISO-3166 country code.
         * @return      An alias to the code
         * @stable ICU 2.0
         */
        inline const char *  getCountry( ) const {
            _country = cf_String::fromCFGetFunction(CFLocaleGetValue(_cfLocale, kCFLocaleCountryCode));
            return _country.c_str();
        }
        
        /**
         * Gets a list of all available 2-letter country codes defined in ISO 3166.  This is a
         * pointer to an array of pointers to arrays of char.  All of these pointers are
         * owned by ICU-- do not delete them, and do not write through them.  The array is
         * terminated with a null pointer.
         * @return a list of all available country codes
         * @stable ICU 2.0
         */
        static const char* const* U_EXPORT2 getISOCountries() {
            static std::vector<std::string> isoCountryCodes;
            static std::vector<const char *> isoCountryCodesCArray;

            isoCountryCodes = cf_Array<CFStringRef>(CFLocaleCopyISOCountryCodes());
            isoCountryCodesCArray.clear();
            for(size_t i = 0; i < isoCountryCodes.size(); i++) {
                isoCountryCodesCArray.push_back(isoCountryCodes[i].c_str());
            }
            isoCountryCodesCArray.push_back(nullptr);
            return &*isoCountryCodesCArray.begin();
        }
        
        /**
         * Gets a list of all available language codes defined in ISO 639.  This is a pointer
         * to an array of pointers to arrays of char.  All of these pointers are owned
         * by ICU-- do not delete them, and do not write through them.  The array is
         * terminated with a null pointer.
         * @return a list of all available language codes
         * @stable ICU 2.0
         */
        static const char* const* U_EXPORT2 getISOLanguages() {
            static std::vector<std::string>  isoLanguages;
            static std::vector<const char *> isoLangCArray;

            isoLanguages = cf_Array<CFStringRef>(CFLocaleCopyISOLanguageCodes());
            isoLangCArray.clear();
            for(size_t i = 0; i < isoLanguages.size(); i++) {
                isoLangCArray.push_back(isoLanguages[i].c_str());
            }
            isoLangCArray.push_back(nullptr);
            return &*isoLangCArray.begin();
        }
        
        //icu4c uses direct functions for that, while it is reasonable to have function similar to isoCountries, and isoLanguages
        static const char* const* U_EXPORT2 getAvailable() {
            static std::vector<std::string>  availLocales;
            static std::vector<const char *> availLocalesCArray;

            availLocales = cf_Array<CFStringRef>(CFLocaleCopyAvailableLocaleIdentifiers());
            availLocalesCArray.clear();
            for(size_t i = 0; i < availLocales.size(); i++) {
                availLocalesCArray.push_back(availLocales[i].c_str());
            }
            availLocalesCArray.push_back(nullptr);
            return &*availLocalesCArray.begin();
        }
        
        
        /**
         * Sets the default. Normally set once at the beginning of a process,
         * then never reset.
         * setDefault() only changes ICU's default locale ID, <strong>not</strong>
         * the default locale ID of the runtime environment.
         *
         * @param newLocale Locale to set to.  If NULL, set to the value obtained
         *                  from the runtime environement.
         * @param success The error code.
         * @system
         * @stable ICU 2.0
         */
        static void setDefault(const cf_Locale& newLocale, UErrorCode&   success) {
            //TODO: do we need to copy ???
            cf_Locale::getDefaultLocaleRef() = newLocale;
        }
        
        /**
         * Common methods of getting the current default Locale. Used for the
         * presentation: menus, dialogs, etc. Generally set once when your applet or
         * application is initialized, then never reset. (If you do reset the
         * default locale, you probably want to reload your GUI, so that the change
         * is reflected in your interface.)
         *
         * More advanced programs will allow users to use different locales for
         * different fields, e.g. in a spreadsheet.
         *
         * Note that the initial setting will match the host system.
         * @return a reference to the Locale object for the default locale ID
         * @system
         * @stable ICU 2.0
         */
        static const cf_Locale& getDefault(void) {
            return cf_Locale::getDefaultLocaleRef();
        }
        
        
        
        //corefoundation wrapper
        CFTypeRef getValue (CFStringRef key) const {
            
            if (_cfLocale == nullptr) {
                return nullptr;
            }
            
            return CFLocaleGetValue (  _cfLocale, key);
        }
        
    private:
        static cf_Locale & getDefaultLocaleRef() {
            static cf_Locale defaultLocale(CFLocaleCopyCurrent());
            return defaultLocale;
        }
        //translate this locale value to display locale language
        cf_String translateValue(const cf_Locale& displayLocale, CFStringRef key) const {
            CFStringRef thisLocaleValue = (CFStringRef)CFLocaleGetValue(_cfLocale, key);
            return CFLocaleCopyDisplayNameForPropertyValue(displayLocale, key, thisLocaleValue);
        }
       
    };
}
