#pragma once

#include <vector>
#include <map>
#import  <CoreFoundation/CoreFoundation.h>

namespace U_ICU_NAMESPACE {
    namespace cf {
        class TransliteratorPatterns {
        public:
            static std::vector<UnicodeString> & getAvailableIDsList() {
                static std::vector<UnicodeString> ustrings;
                
                if (ustrings.empty()) {
                    for(const auto & s : getMergedMap()) {
                        ustrings.push_back(UnicodeString(s.first.c_str(), s.first.length()));
                    }
                }
                return ustrings;
            }
            
            static const std::string & Upper() {
                static std::string id = "Upper";
                return id;
            }
            static const std::string & Lower() {
                static std::string id = "Lower";
                return id;
            }
            static const std::string & To() {
                static std::string id = "-";
                return id;
            }
            static const std::string & Any() {
                static std::string id = "Any";
                return id;
            }
            static const std::string & Latin() {
                static std::string id = "Latin";
                return id;
            }
            //map to string ref and direction
            static const std::map<std::string, std::pair<const CFStringRef, bool>> & getMergedMap() {
                static std::map<std::string, std::pair<const CFStringRef, bool>> mergedMap;
                if (mergedMap.empty()) {
                    for (const auto &i : getForwardMap()) {
                        mergedMap.insert(std::make_pair(i.first, std::make_pair(i.second, true)));
                    }
                    for (const auto &i : getReverseMap()) {
                        mergedMap.insert(std::make_pair(i.first, std::make_pair(i.second, false)));
                    }
                }
                    
                return mergedMap;
            }

            static const std::map<std::string, const CFStringRef> & getForwardMap() {
                static std::map<std::string, const CFStringRef> forwardMap;
                if (forwardMap.empty()) {
                    for (const auto & item : getICUToCoreFoundationMap()) {
                        if ( std::get<1>(item).empty() ) {
                            forwardMap.insert(std::make_pair(std::get<0>(item), std::get<2>(item)));
                        } else {
                            forwardMap.insert(std::make_pair(std::get<0>(item)+To() + std::get<1>(item), std::get<2>(item)));
                        }
                    }
                }
                return forwardMap;
            }
            static const std::map<std::string, const CFStringRef> & getReverseMap() {
                static std::map<std::string, const CFStringRef> reverseMap;
                if (reverseMap.empty()) {
                    for (const auto & item : getICUToCoreFoundationMap()) {
                        //empty will be in forward map
                        if (std::get<1>(item).empty() ) continue;
                        
                        //check for AnyToSmth mapping reverse cannot hapen
                        if ( std::get<0>(item)==Any()) continue;
                        
                        reverseMap.insert(std::make_pair(std::get<1>(item)+To() + std::get<0>(item), std::get<2>(item)));
                    }
                }
                return reverseMap;
            }


            static const std::vector<std::tuple<std::string, std::string, const CFStringRef>> & getICUToCoreFoundationMap() {
                static std::vector<std::tuple<std::string, std::string, const CFStringRef> > transliterationStrings=
                {
                    std::make_tuple(Any(), Upper(), nullptr),
                    std::make_tuple(Any(), Lower(), nullptr),
                    std::make_tuple(Any(), Latin(), kCFStringTransformToLatin),
                    std::make_tuple(Latin(), "Katakana", kCFStringTransformLatinKatakana),
                    std::make_tuple(Latin(), "Hiragana", kCFStringTransformLatinHiragana),
                    std::make_tuple("Hiragana", "Katakana", kCFStringTransformHiraganaKatakana),
                    std::make_tuple("Mandarin", Latin(), kCFStringTransformMandarinLatin),
                    std::make_tuple(Latin(),"Hangul", kCFStringTransformLatinHangul),
                    std::make_tuple(Latin(),"Arabic", kCFStringTransformLatinArabic),
                    std::make_tuple(Latin(),"Hebrew", kCFStringTransformLatinHebrew),
                    std::make_tuple(Latin(),"Thai", kCFStringTransformLatinThai),
                    std::make_tuple(Latin(),"Cyrillic", kCFStringTransformLatinCyrillic),
                    std::make_tuple(Latin(),"Greek", kCFStringTransformLatinGreek),
                    std::make_tuple(Any(),"Hex/XML", kCFStringTransformToXMLHex),
                    std::make_tuple(Any(),"Name", kCFStringTransformToUnicodeName),
                    std::make_tuple("StripDiacritics", "", kCFStringTransformStripDiacritics),
                    std::make_tuple("StripCombiningMarks", "", kCFStringTransformStripCombiningMarks),
                    std::make_tuple("FullwidthHalfwidth", "", kCFStringTransformFullwidthHalfwidth),
                };

                return transliterationStrings;
            }
        };
    }
}
