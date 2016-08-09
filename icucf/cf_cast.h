#pragma once

#import <CoreFoundation/CoreFoundation.h>

template <class cf_Type>
struct cf_Cast {
};

//normal pointers - use const cast to remove const from cftyperef
template <class cf_Type>
struct cf_Cast<cf_Type *> {
    static cf_Type* cast(CFTypeRef ref) {
        return static_cast<cf_Type*>(const_cast<void*>(ref));
    }
};

//const pointers - use normal cast
template <class cf_Type>
struct cf_Cast<const cf_Type *> {
    static const cf_Type * cast(CFTypeRef ref) {
        return static_cast<const cf_Type *>(ref);
    }
};

inline template<class cf_Type> auto cf_cast(CFTypeRef ref) -> decltype(cf_Cast<cf_Type>::cast(ref)) {
    return cf_Cast<cf_Type>::cast(ref);
}