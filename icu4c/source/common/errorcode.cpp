/*
*******************************************************************************
*
*   Copyright (C) 2009-2011, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
*******************************************************************************
*   file name:  errorcode.cpp
*   encoding:   US-ASCII
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 2009mar10
*   created by: Markus W. Scherer
*/

#ifndef GOOGLE_VENDOR_SRC_BRANCH
#include "base/logging.h"
#endif
#include "unicode/utypes.h"
#include "unicode/errorcode.h"

U_NAMESPACE_BEGIN

#ifdef GOOGLE_VENDOR_SRC_BRANCH
ErrorCode::~ErrorCode() {}
#else
ErrorCode::~ErrorCode() {
    LOG_IF(DFATAL, isFailure()) << errorName();
}

void ErrorCode::handleFailure() const {
    LOG(FATAL) << errorName();
}
#endif

UErrorCode ErrorCode::reset() {
    UErrorCode code = errorCode;
    errorCode = U_ZERO_ERROR;
    return code;
}

void ErrorCode::assertSuccess() const {
    if(isFailure()) {
        handleFailure();
    }
}

const char* ErrorCode::errorName() const {
  return u_errorName(errorCode);
}

U_NAMESPACE_END
