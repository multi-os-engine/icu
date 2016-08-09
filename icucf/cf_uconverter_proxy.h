#pragma once

#include "cf_character_set.h"
#include "cf_uenumeration.h"
#include "cf_uconverter.h"
#include <map>

#define UConverter ICU_CF(UConverter)

//functions
#define ucnv_openStandardNames CF_UCNV(openStandardNames)
#define UCNV_FROM_U_CALLBACK_STOP CF_UCNV(FROM_U_CALLBACK_STOP)
#define UCNV_FROM_U_CALLBACK_SKIP CF_UCNV(FROM_U_CALLBACK_SKIP)
#define UCNV_TO_U_CALLBACK_STOP CF_UCNV(TO_U_CALLBACK_STOP)
#define ucnv_cbToUWriteUChars CF_UCNV(cbToUWriteUChars)
#define ucnv_cbFromUWriteBytes CF_UCNV(cbFromUWriteBytes)
#define ucnv_open CF_UCNV(open)
#define ucnv_close CF_UCNV(close)
#define ucnv_getUnicodeSet  CF_UCNV(getUnicodeSet)
#define ucnv_getMaxCharSize CF_UCNV(getMaxCharSize)
#define ucnv_getMinCharSize CF_UCNV(getMinCharSize)
#define ucnv_countAvailable CF_UCNV(countAvailable)
#define ucnv_getAvailableName CF_UCNV(getAvailableName)
#define ucnv_resetToUnicode CF_UCNV(resetToUnicode)
#define ucnv_resetFromUnicode CF_UCNV(resetFromUnicode)
#define ucnv_getAlias CF_UCNV(getAlias)
#define ucnv_getSubstChars CF_UCNV(getSubstChars)
#define ucnv_getCanonicalName CF_UCNV(getCanonicalName)
#define ucnv_getFromUCallBack CF_UCNV(getFromUCallBack)
#define ucnv_setFromUCallBack CF_UCNV(setFromUCallBack)
#define ucnv_getToUCallBack CF_UCNV(getToUCallBack)
#define ucnv_setToUCallBack CF_UCNV(setToUCallBack)
#define ucnv_getInvalidChars CF_UCNV(getInvalidChars)
#define ucnv_getInvalidUChars CF_UCNV(getInvalidUChars)


namespace U_ICU_NAMESPACE {
    
    namespace cf {
        
        class UCnv {
        public:
            static void  FROM_U_CALLBACK_STOP (const void *context,
                                        ConverterFromUnicodeArgs *fromUArgs,
                                        const UChar* codeUnits,
                                        int32_t length,
                                        UChar32 codePoint,
                                        ConverterCallbackReason reason,
                                        UErrorCode * err){
                ((UConverter*)fromUArgs->converter)->stopOnEncode(reason);
            }
            
            static void FROM_U_CALLBACK_SKIP ( const void *context,
                                        ConverterFromUnicodeArgs *fromUArgs,
                                        const UChar* codeUnits,
                                        int32_t length,
                                        UChar32 codePoint,
                                        ConverterCallbackReason reason,
                                        UErrorCode * err) {
                
            }
            static void TO_U_CALLBACK_STOP (const void *context,
                                             ConverterToUnicodeArgs *toUArgs,
                                             const char* codeUnits,
                                             int32_t length,
                                             ConverterCallbackReason reason,
                                             UErrorCode * err) {

                ((UConverter*)toUArgs->converter)->stopOnDecode(reason);
                
            }

            static void cbToUWriteUChars (ConverterToUnicodeArgs *args,
                                   const UChar* source,
                                   int32_t length,
                                   int32_t sourceIndex,
                                   UErrorCode * pErrorCode) {
                UConverter *cnv = args->converter;
                UChar **target = &args->target;
                const UChar *targetLimit = args->targetLimit;
                int32_t **offsets = &args->offsets;

                
                UChar *t=*target;
                int32_t *o;
                
                /* write UChars */
                if(offsets==NULL || (o=*offsets)==NULL) {
                    while(length>0 && t<targetLimit) {
                        *t++=*source++;
                        --length;
                    }
                } else {
                    /* output with offsets */
                    while(length>0 && t<targetLimit) {
                        *t++=*source++;
                        *o++=sourceIndex;
                        --length;
                    }
                    *offsets=o;
                }
                *target=t;
                
                /* write overflow */
                if(length>0) {
                    /*if(cnv!=NULL) {
                        t=cnv->UCharErrorBuffer;
                        cnv->UCharErrorBufferLength=(int8_t)length;
                        do {
                            *t++=*uchars++;
                        } while(--length>0);
                    }*/
                    *pErrorCode=U_BUFFER_OVERFLOW_ERROR;
                }
            }

            //copy pasted from ICU4C implemntation
            static void cbFromUWriteBytes (ConverterFromUnicodeArgs *args,
                               const char* bytes,
                               int32_t length,
                               int32_t sourceIndex,
                               UErrorCode * pErrorCode) {
                
                UConverter *cnv = args->converter;
                char **target = &args->target;
                const char *targetLimit = args->targetLimit;
                int32_t **offsets = &args->offsets;
                
                
                char *t=*target;
                int32_t *o;
                
                /* write bytes */
                if(offsets==NULL || (o=*offsets)==NULL) {
                    while(length>0 && t<targetLimit) {
                        *t++=*bytes++;
                        --length;
                    }
                } else {
                    /* output with offsets */
                    while(length>0 && t<targetLimit) {
                        *t++=*bytes++;
                        *o++=sourceIndex;
                        --length;
                    }
                    *offsets=o;
                }
                *target=t;
                
                /* write overflow */
                if(length>0) {
                    /*if(cnv!=NULL) {
                        t=(char *)cnv->charErrorBuffer;
                        cnv->charErrorBufferLength=(int8_t)length;
                        do {
                            *t++=(uint8_t)*bytes++;
                        } while(--length>0);
                    }*/
                    *pErrorCode=U_BUFFER_OVERFLOW_ERROR;
                }
            }
            
            static UEnumeration* openStandardNames	(	const char * 	convName,
                                                        const char * 	standard,
                                                         UErrorCode * 	pErrorCode)  {
                
                std::auto_ptr<::cf::UEnumerationImpl> result(new ::cf::UEnumerationImpl());
                cf_CharacterSet charSet(cf_CharacterSet::fromICUCanonicalName(convName));
                result->push_back(charSet.getAlias(standard));
                return *result.release();
            }
            
            static UConverter* open(const char *converterName, UErrorCode *err) {
                return new UConverter(converterName, err);
            }
            
            static void close(UConverter * converter) {
                delete converter;
            }
            
            static void getUnicodeSet(const UConverter *cnv,
                               USet *setFillIn,
                               ConverterUnicodeSet whichSet,
                               UErrorCode *pErrorCode) {
                
            }
            
            static int8_t getMaxCharSize(const UConverter *converter) {
                return converter->getMaxCharSize();
            }

            static int8_t getMinCharSize(const UConverter *converter) {
                return converter->getMinCharSize();
            }
            
            static int32_t countAvailable () {
                return cf_CharacterSet::getAvailableEncodings().size();
            }
            
            static const char*  getAvailableName (int32_t n) {
                return cf_CharacterSet::getAvailableEncodings()[n].c_str();
            }
            
            static void resetToUnicode(UConverter *converter) {
                converter->resetToUnicode();
            }
            
            static void resetFromUnicode(UConverter * converter) {
                converter->resetFromUnicode();
            }
            

            
            static const char *
            getCanonicalName(const char *alias, const char *standard, UErrorCode *pErrorCode) {
                static std::map<std::string, std::string> canonical_name_for_alias;
                try {
                    return canonical_name_for_alias.at(alias).c_str();
                }catch(...) {
                    cf_CharacterSet charSet(alias);
                    canonical_name_for_alias[alias] = charSet.getICUCanonicalName();
                }
                return canonical_name_for_alias.at(alias).c_str();
            }
            static const char*
            getAlias(const char *alias, uint16_t n, UErrorCode *pErrorCode) {
                return nullptr;
            }
            
            static void
            getSubstChars (const UConverter * converter,
                           char *mySubChar,
                           int8_t * len,
                           UErrorCode * err) {
                converter->getSubstChars (mySubChar, len, err);
            }
            static void
            getFromUCallBack (const UConverter * converter,
                              UConverterFromUCallback *action,
                              const void **context) {
                converter->getFromUCallBack(action, context);
            }
            static void setFromUCallBack (UConverter * converter,
                                          ConverterFromUCallback newAction,
                                          const void* newContext,
                                          ConverterFromUCallback *oldAction,
                                          const void** oldContext,
                                          UErrorCode * err) {
                converter->setFromUCallBack(newAction, newContext, oldAction, oldContext, err);
            }
            
            static void setToUCallBack (UConverter * converter,
                                 UConverterToUCallback newAction,
                                 const void* newContext,
                                 UConverterToUCallback *oldAction,
                                 const void** oldContext,
                                 UErrorCode * err) {
                converter->setToUCallBack(newAction, newContext, oldAction, oldContext, err);
            }
            static void getToUCallBack (const UConverter * converter,
                                        UConverterToUCallback *action,
                                        const void **context) {
                converter->getToUCallBack(action, context);
            }
            
            static void getInvalidChars (const UConverter * converter,
                                         char *errBytes,
                                         int8_t * len,
                                         UErrorCode * err) {
                converter->getInvalidChars(errBytes, len, err);
            }
            static void getInvalidUChars (const UConverter * converter,
                                          UChar *errChars,
                                          int8_t * len,
                                          UErrorCode * err) {
                converter->getInvalidUChars(errChars, len, err);
            }

        };
    }

    U_DEFINE_LOCAL_OPEN_POINTER(LocalUConverterPointer, UConverter, ucnv_close);
}
