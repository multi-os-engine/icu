#pragma once
#include <vector>
#include <cf_auto_ref.h>

template <class TArrayItems>
class cf_Array { };


class cf_ArrayBase {
protected:
    cf_AutoRef<CFArrayRef> _array;
public:
    cf_ArrayBase(CFArrayRef sourceArray)
        : _array(sourceArray) {
    }
};


template <>
class cf_Array <CFStringRef>  : public cf_ArrayBase {
public:
    cf_Array(CFTypeRef sourceArray)
        : cf_ArrayBase((CFArrayRef)sourceArray) {
    }
    template <class TElements>
    operator std::vector<TElements> () const {
        std::vector<TElements> propertyArray(CFArrayGetCount(_array));
        for (int i=0; i < propertyArray.size(); i++) {
            cf_String value = cf_String::fromCFGetFunction(CFArrayGetValueAtIndex(_array, i));
            propertyArray[i] = TElements(value.operator TElements());
        }
        return propertyArray;

    }
    //todo:  move impl to cpp file if that functionality required
    /*   template <>
    operator std::vector<CFStringRef> () const {
        std::vector<CFStringRef> propertyArray(CFArrayGetCount(_array));
        for (int i=0; i < propertyArray.size(); i++) {
            propertyArray[i] = (CFStringRef) CFArrayGetValueAtIndex(_array, i);
        }
        return propertyArray;
        
    }*/

};
