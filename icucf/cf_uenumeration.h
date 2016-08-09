#pragma once

#include "uenumimp.h"

namespace cf { 

    class UEnumerationImpl {
        UEnumeration _uenum;
        std::vector<std::string> elements;
        std::vector<UnicodeString> elements_cf;
        int32_t index;
        
        ////////////////
        //implementation
        
        void U_CALLCONV close() {
            delete this;
        }
        
        int32_t U_CALLCONV count(UErrorCode *status){
            return elements.size();
        }
        
        const UChar* U_CALLCONV
        uNext(int32_t* resultLength, UErrorCode* status) {
            if (index + 1 > elements.size() ) {
                return nullptr;
            }
            if (index + 1 > elements_cf.size()) {
                elements_cf.resize(index + 1);
            }
            
            if (!elements_cf[index].isEmpty()) {
                *resultLength = elements_cf[index].length();
                return elements_cf[index++].getBuffer();
            }
            
           
            cf_String cfstr(elements[index]);
            elements_cf[index] = cfstr;
            
            *resultLength = elements_cf[index].length();
            
            //todo convert to uchar
            return elements_cf[index++].getBuffer();
        }
        
        const char* U_CALLCONV
        next(int32_t* resultLength, UErrorCode* status) {
            if (index + 1 > elements.size() ) {
                return nullptr;
            }
            *resultLength = elements[index].length();
            return elements[index++].c_str();
        }
        void U_CALLCONV
        reset() {
            index = 0;
        }
        
    public:
        UEnumerationImpl() {
            _uenum.baseContext = nullptr;
            _uenum.context = this;
            _uenum.close   = _UEnumClose;
            _uenum.count   = _UEnumCount;
            _uenum.uNext   = _UEnumUNext;
            _uenum.next    = _UEnumNext;
            _uenum.reset   = _UEnumReset;

            index = 0;
        }
        
        void push_back(const std::string& element) {
            elements.push_back(element);
        }

        
        operator UEnumeration* () {
            return &_uenum;
        }
        
        
    private:
        static UEnumerationImpl* toThisClass(UEnumeration *en) {
            return reinterpret_cast<UEnumerationImpl*>(en->context);
        }
        
        static void U_CALLCONV _UEnumClose(UEnumeration *en) {
            toThisClass(en)->close();
        }
        static int32_t U_CALLCONV _UEnumCount(UEnumeration *en, UErrorCode *status){
            return toThisClass(en)->count(status);
        }
        
        static const UChar* U_CALLCONV
        _UEnumUNext(UEnumeration* en,
                   int32_t* resultLength,
                   UErrorCode* status) {
            return toThisClass(en)->uNext(resultLength, status);
        }
        
        static const char* U_CALLCONV
        _UEnumNext(UEnumeration* en,
                  int32_t* resultLength,
                  UErrorCode* status) {
            return toThisClass(en)->next(resultLength, status);
        }
        static void U_CALLCONV
        _UEnumReset(UEnumeration* en,
                   UErrorCode* status) {
            return toThisClass(en)->reset();
        }
        
        
    };
}

