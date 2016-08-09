#pragma once

#include <cf_cast.h>

template<typename T> class cf_AutoRef {
public:
    cf_AutoRef(T ref = nullptr)
        : _ref(ref) {}

    cf_AutoRef(const cf_AutoRef & that)
        : _ref(nullptr) {
        retain(that._ref);
    }
    ~cf_AutoRef() {
        assign();
    }
    operator T() const {
        return _ref;
    }
    cf_AutoRef<T>& operator=(T ref) {
        assign(ref);
        return *this;
    }
    cf_AutoRef<T>& operator=(const cf_AutoRef<T> & that) {
        retain(that._ref);
        return *this;
    }
    T get() const {
        return _ref;
    }
    void reset (T ref = nullptr) {
        assign(ref);
    }
private:
    void assign(T ref = nullptr) {
        if (_ref) {
            CFRelease(_ref);
        }
        _ref = ref;
    }
    void retain(T ref) {
        assign(nullptr == ref ? nullptr : cf_cast<T>(CFRetain(ref)));
    }
    
    T _ref;
};

