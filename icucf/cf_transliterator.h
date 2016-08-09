#pragma once

#include <cf_transliterator_single.h>
#include <cf_transliterator_case_change.h>
#include <cf_transliterator_direct.h>
#include <cf_string.h>

namespace U_ICU_NAMESPACE {
    namespace cf {
        class Transliterator {
            std::vector<std::shared_ptr<ITransliterator>> transliters;
        public:
            Transliterator(const UnicodeString& ID, UTransDirection dir, UErrorCode& status){
                try {
                    std::string strID = cf_String(ID);
                    int ch;
                    for (ch = 0; ch < strID.length(); ch++) {
                        auto offset = strID.find_first_of(';', ch);
                        if (offset == std::string::npos) {
                            break;
                        }

                        transliters.push_back(createTransliterator(strID.substr(ch, offset)));
                        ch = offset;
                    }

                    transliters.push_back(createTransliterator(strID.substr(ch)));
                } catch (...) {
                    status = U_INVALID_ID;
                }
            }

            static Transliterator* createInstance(const UnicodeString& ID, UTransDirection dir, UErrorCode& status){
                return new Transliterator(ID, dir, status);
            }

            void transliterate(Replaceable& text){
                for(auto &transliter : transliters) {
                    transliter->transliterate(text);
                }
            }

            static StringEnumeration* getAvailableIDs(UErrorCode& ec){
                static Enumeration e(TransliteratorPatterns::getAvailableIDsList());
                return &e;
            }

        protected:
            
            std::shared_ptr<ITransliterator> createTransliterator(const std::string& ID) {
                auto & lst = TransliteratorPatterns::getMergedMap();
                try {
                    if (lst.at(ID).first != nullptr) {
                        return std::make_shared<TransliteratorSingle>(ID.c_str());
                    }
                    
                    //only case change tranliterators are remained
                    return std::make_shared<TransliteratorCaseChange>(ID == (
                        TransliteratorPatterns::Any() + TransliteratorPatterns::To() + TransliteratorPatterns::Upper()));
                }catch(...){}
                
                //attempt to initiate transliteration with direct ID but for latin character, hope it succeed
                return std::make_shared<TransliteratorDirect>(ID);
            }
        
            class Enumeration : public StringEnumeration {
                std::vector<UnicodeString> * data;
                int32_t index;
                public:
                Enumeration(std::vector<UnicodeString> & ref) {
                    this->data = &ref;
                    index = 0;
                }
                virtual int32_t count(UErrorCode& status) const {
                    return data->size();
                }
                virtual const UnicodeString* snext(UErrorCode& status) {
                    if (index == data->size()) {
                        return nullptr;
                    }
                    return &((*data)[index++]);
                    
                }
                virtual void reset(UErrorCode& status) {
                    index = 0;
                }
            };
        };
    }
}

#ifndef ICU_TYPE
#define ICU_TYPE(type) cf::type
#endif

#define Transliterator ICU_TYPE(Transliterator)