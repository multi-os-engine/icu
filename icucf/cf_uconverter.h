#pragma once

#include "cf_converter_types.h"

/**
    Aims to substitute  UConverter structure in NativeConverter, since it it initialisation heavily depends on ICU implementation
 */
namespace U_ICU_NAMESPACE {
    namespace cf {
        class UConverter {
            std::string _name;
            mutable std::vector<UniChar> invalidCharsForEncode;
            mutable std::vector<UniChar> invalidCharsForDecode;
            //for decode input might be invalid encoding as well
            mutable std::vector<char> invalidBytesForDecode;
            char replacementCharacterForEncode;
            UChar replacementCharacterForDecode;

            const void *fromUContext;
            ConverterFromUCallback fromUCallback;

            const void *toUContext;
            ConverterToUCallback toUCallback;
            
            mutable bool decoderFlushed;
            mutable bool encoderFlushed;
            
            mutable CFStringEncoding decoderCharset;
            
            enum ErrorAction{
                IGNORE,
                REPLACE,
                ABORT,
            };
            ErrorAction onEncodeError;
            ErrorAction onDecoderror;
            
        public:
            UConverter (const char *name, UErrorCode * err)
                : _name(name)
                , fromUContext()
                , toUContext()
                , fromUCallback()
                , toUCallback()
                , replacementCharacterForEncode()
                , encoderFlushed(true)
                , decoderFlushed(true)
                , onEncodeError(IGNORE)
                , onDecoderror(IGNORE)
            {
                    cf_CharacterSet charset(cf_CharacterSet::fromICUCanonicalName(name));
                    if( !charset.isEncodingSupported()) {
                        *err = U_UNSUPPORTED_ERROR;
                    }
            }
            UErrorCode decodeTo(const char * input, int sourceLen, UChar * output, size_t bufferLen, size_t& sourceBytesConverted, size_t& usedTargetSize, bool bFlush) const {
                //convert from input data to particular encoding
                cf_CharacterSet charSet(_name);
                
                const char * inputOrInvalid = input;
                int lenOrInvalid = sourceLen;
                int invalidBytesCount = invalidBytesForDecode.size();
                if (!invalidBytesForDecode.empty()) {
                    invalidBytesForDecode.insert(invalidBytesForDecode.end(), input, input+sourceLen);
                    inputOrInvalid = &*invalidBytesForDecode.begin();
                    lenOrInvalid = invalidBytesForDecode.size();
                }

                bool parseBOM = false;
                
                int numSourceCharsBOM = 0;
                //in case of beginning of the data we start from checking BOM characters
                if (charSet == kCFStringEncodingUTF16) {
                    if(decoderFlushed) {
                        decoderCharset  = cf_String::getUTF16Endianess((const uint8_t *)inputOrInvalid, lenOrInvalid);
                        parseBOM = !(decoderCharset == kCFStringEncodingInvalidId);
                        //skip BOM since CoreFoundation will always put it
                        if (parseBOM) {
                            numSourceCharsBOM = sizeof(UChar);
                        }
                        //if string has BOM missed, than use default java encoding that is utf16BE
                        decoderCharset = !parseBOM ? kCFStringEncodingUTF16BE : decoderCharset;
                    }
                } else {
                    decoderCharset = charSet;
                }

                cf_String inputCFString( inputOrInvalid + numSourceCharsBOM
                                       , lenOrInvalid   - numSourceCharsBOM
                                       , decoderCharset
                                       , false);

                //if input cannot be converted to CFString that means that it either invalid, or not full, lets request more input
                if(inputCFString.length() == 0) {

                    if (bFlush) {
                        if (!invalidBytesForDecode.empty() && onDecoderror == ABORT) {
                            return U_TRUNCATED_CHAR_FOUND;
                        }
                        //feed output with substitution bytes
                        int numCharsToReplace = (std::min)(invalidBytesForDecode.size(), bufferLen);
                        usedTargetSize = numCharsToReplace;
                        std::fill(output, output + numCharsToReplace, replacementCharacterForDecode);
                        invalidBytesForDecode.clear();
                        
                        //if something has been written that BOM already there
                        if (usedTargetSize != 0) {
                            decoderFlushed = bFlush;
                        }
                        
                    } else {
                        //add chars to invalid buffer
                        if (invalidBytesForDecode.empty()) {
                            invalidBytesForDecode.insert(invalidBytesForDecode.end(), input, input + sourceLen);
                        }

                        //heuristick check - if input len >  maximum size for 1 char encoding, than it is definetelly has invalid characters
                        auto hasBOM = charSet == kCFStringEncodingUTF16 ?
                            cf_String::detectBOM((const uint8_t*)inputOrInvalid + numSourceCharsBOM, lenOrInvalid   - numSourceCharsBOM) : false;
                        
                        auto oneCharLen = getMaxCharSize() + (hasBOM ? 2 : 0);
                        if (oneCharLen < lenOrInvalid - numSourceCharsBOM) {
                            return U_ILLEGAL_CHAR_FOUND;
                        }
                    }
                    sourceBytesConverted = sourceLen;
                    return U_ZERO_ERROR;
                }
                invalidBytesForDecode.clear();
               
                //prepare unique character to replace to
                //2 stages processing can be used in order to get differences and replace them to required symbols
               
                //decode to utf-16 from input encoding
                cf_CharacterSet targetEncoding("UTF-16");
                size_t sourceCharsConverted = 0;
                bool result = runConversion(  targetEncoding
                                            , inputCFString
                                            , (char*)output
                                            , bufferLen * sizeof(UChar)
                                            , sourceCharsConverted
                                            , usedTargetSize
                                            , bFlush
                                            , invalidCharsForDecode
                                            , replacementCharacterForDecode
                                            , onDecoderror
                                            , false);
                
                usedTargetSize /= sizeof(UChar);
                
                //if something has been written that BOM already there
                if (usedTargetSize != 0) {
                    decoderFlushed = bFlush;
                }

                //running size estimation from cf_string to input encoding, usedBufLen will give us an offset in input encoding
                sourceBytesConverted = numSourceCharsBOM + inputCFString.getBytesUsedForConversion(sourceCharsConverted, decoderCharset) - invalidBytesCount;
                
                return result ? U_ZERO_ERROR : U_BUFFER_OVERFLOW_ERROR;
            }
            //return true if buffer enough
            UErrorCode encodeTo( const UnicodeString & uinput
                               , char * output, size_t bufferLen
                               , size_t& sourceSizeConverted
                               , size_t& usedTargetSize
                               , bool bFlush) const {

                cf_CharacterSet sourceEncoding("UTF-16");
                cf_CharacterSet targetEncoding(_name);
                size_t sourceCharsConverted = 0;
                
                //harmony java tests looks not expected bom to happen in utf8 sequences, since they olways check for buffer UNDERFLOW
                bool putBOM = false;
                if (targetEncoding == kCFStringEncodingUTF16 ||
                    targetEncoding == kCFStringEncodingUTF32) {
                    putBOM = true;
                }
                //TODO: have that check for surrogate pairs validness
                for (int i = 0; i + 1 < uinput.length(); i += 2) {
                    if (CFStringIsSurrogateHighCharacter(uinput.getBuffer()[i])) {
                        if (!CFStringIsSurrogateLowCharacter(uinput.getBuffer()[i + 1])) {
                            invalidCharsForEncode.push_back(uinput.getBuffer()[i + 1]);
                            return U_ILLEGAL_CHAR_FOUND;
                        }
                    }
                }
                
                cf_String input(uinput);
                
                bool result = runConversion(targetEncoding, input, output, bufferLen, sourceCharsConverted, usedTargetSize, bFlush, invalidCharsForEncode, replacementCharacterForEncode, onEncodeError, encoderFlushed && putBOM);
                
                //if something has been written that BOM already there
                if (usedTargetSize != 0) {
                    encoderFlushed = bFlush;
                }
                
                sourceSizeConverted = input.getBytesUsedForConversion(sourceCharsConverted, sourceEncoding) / sizeof(UChar);
                return result ?  U_ZERO_ERROR : U_BUFFER_OVERFLOW_ERROR;
            }
            
            int8_t getMaxCharSize() const {
                cf_CharacterSet targetEncoding(_name);
                
                switch((CFStringEncoding) targetEncoding) {
                    case kCFStringEncodingUTF16:
                    case kCFStringEncodingUTF16BE:
                    case kCFStringEncodingUTF16LE:
                        //not sure why surrogate pairs are not count here
                        return 2;
                        
                    //from AOSP tests and wiki it is clear value for GB 1830
                    case kCFStringEncodingGB_18030_2000:
                        return 4;

                    default:
                        return cf_String::getMaximumSizeForEncoding ( 1, targetEncoding, false);
                }
            }
            
            int8_t getMinCharSize() const {
                cf_CharacterSet targetEncoding(_name);
                switch((CFStringEncoding) targetEncoding) {
                    case kCFStringEncodingUTF16:
                    case kCFStringEncodingUTF16BE:
                    case kCFStringEncodingUTF16LE:
                        return 2;
                    default:
                        return 1;
                }
            }
            
            void getSubstChars (char *replacementBytes,
                                int8_t * len,
                                UErrorCode * err)  const{
                if (replacementBytes == nullptr)
                    return;
                
                cf_CharacterSet charset(cf_CharacterSet::fromICUCanonicalName(_name));
                
                switch((CFStringEncoding) charset) {
                    case kCFStringEncodingASCII:
                    case kCFStringEncodingISOLatin1:
                    case kCFStringEncodingUTF8:

                    default:
                        replacementBytes[0] = '?';
                        *len = 1;
                        break;
                    case kCFStringEncodingUTF16:
                    case kCFStringEncodingUTF16BE:
                         replacementBytes[0] =  0xff;
                         replacementBytes[1] =  0xfd;
                        *len = 2;
                        break;
                        
                    case kCFStringEncodingUTF16LE:
                        replacementBytes[0] =  0xfd;
                        replacementBytes[1] =  0xff;
                        *len = 2;
                        break;
                    
                    case kCFStringEncodingGB_18030_2000:
                        replacementBytes[0] =  0x1a;
                        *len = 1;
                        break;
                }
                
            }
            void getToUCallBack (UConverterToUCallback *action, const void **context) const
            {
                *action = toUCallback;
                *context = toUContext;
            }
            
            void setToUCallBack (UConverterToUCallback newAction,
                                 const void* newContext,
                                 UConverterToUCallback *oldAction,
                                 const void** oldContext,
                                 UErrorCode * err)
            {
                if (U_FAILURE (*err))
                    return;
                if (oldAction) *oldAction = toUCallback;
                toUCallback = newAction;
                if (oldContext) *oldContext = toUContext;
                toUContext = newContext;
                
                UChar replacement[10] = {0};
                
                /* prepare the converter arguments */
                ConverterToUnicodeArgs args;
                args.converter=this;
                args.flush=0;
                args.offsets=0;
                args.source=nullptr;
                args.sourceLimit=0;
                args.target=replacement;
                args.targetLimit=replacement + sizeof(replacement) / sizeof(*replacement);
                args.size=sizeof(args);
                
                
                onDecoderror = IGNORE;
                toUCallback(toUContext, &args, nullptr, 0, UNASSIGNED, err);
                
                if (U_FAILURE (*err)) {
                    replacementCharacterForDecode = 0;
                    return;
                }
                //TODO: add support for multi characters substitution it requires for utf-16 encodings
                replacementCharacterForDecode = replacement[0];
                
                if (onDecoderror == IGNORE && replacementCharacterForDecode != 0) {
                    onDecoderror = REPLACE;
                }
            }
            

            
            void getFromUCallBack (ConverterFromUCallback *action,
                                   const void **context) const {
                *action  = fromUCallback;
                *context = fromUContext;
            }
            
            void setFromUCallBack (ConverterFromUCallback newAction,
                                   const void* newContext,
                                   ConverterFromUCallback *oldAction,
                                   const void** oldContext,
                                   UErrorCode * err) {
                if (U_FAILURE (*err))
                    return;
                if (oldAction) *oldAction = fromUCallback;
                fromUCallback = newAction;
                if (oldContext) *oldContext = fromUContext;
                fromUContext = newContext;
                
                
                //we are getting replacement bytes right now, since CoreFoundation doesnot support live callback on invalid bytes,
                //while it dose support providing replacement char directly
                char replacement[10] = {0};
                
                ConverterFromUnicodeArgs args;
                args.converter=this;
                args.flush=0;
                args.offsets=0;
                args.source=nullptr;
                args.sourceLimit=0;
                args.target=replacement;
                args.targetLimit=replacement + sizeof(replacement) / sizeof(*replacement);
                args.size = sizeof(args);
                
                onEncodeError = IGNORE;
                fromUCallback(fromUContext, &args, nullptr, 0, 0, UNASSIGNED, err);

                if (U_FAILURE (*err)) {
                    replacementCharacterForEncode = 0;
                    return;
                }
                //TODO: add support for multi characters substitution it requires for utf-16 encodings
                replacementCharacterForEncode = replacement[0];

                if (onEncodeError == IGNORE && replacementCharacterForEncode != 0) {
                    onEncodeError = REPLACE;
                }
            }
            
            void stopOnEncode(ConverterCallbackReason reason) {
                onEncodeError = ABORT;
                
            }

            void stopOnDecode(ConverterCallbackReason reason) {
                onDecoderror = ABORT;
            }


            void resetToUnicode() {
                //reset decoder
                decoderFlushed = true;
                invalidBytesForDecode.clear();
            }
            
            void resetFromUnicode() {
                //reset encoder
                encoderFlushed = true;
                invalidCharsForEncode.clear();
            }
            
            void getInvalidChars (char *errBytes,
                                  int8_t * len,
                                  UErrorCode * err) const {
                *len = invalidBytesForDecode.size();
            }
            
            void getInvalidUChars (UChar *errChars,
                                   int8_t * len,
                                   UErrorCode * err) const {
                *len = invalidCharsForEncode.size();
            }



        private:
            
            //return true if buffer enough
            bool runConversion( const cf_CharacterSet &charSet
                              , const cf_String & input
                              , char * output
                              , size_t bufferLen
                              , size_t& sourceCharsConverted
                              , size_t& usedTargetSize
                              , bool bFlush
                              , std::vector<UniChar> &invalidChars
                              , char replacementSymbol
                              , ErrorAction action
                              , bool putBOM) const {
                
                CFIndex usedSize;
                bool isConverted;
                
                if (!invalidChars.empty()) {
                    //first combine invalid chars
                    for (int i=0; i < input.length(); i++) {
                        invalidChars.push_back(input.getChar(i));
                    }
                    
                    cf_String invalidInput(&*invalidChars.begin(), invalidChars.size());
                    isConverted = runConversionNoInvalid( charSet
                                                         , invalidInput
                                                         , output
                                                         , bufferLen
                                                         , sourceCharsConverted
                                                         , usedTargetSize
                                                         , bFlush
                                                         , replacementSymbol
                                                         , action
                                                         , putBOM);
                }
                else {
                    isConverted = runConversionNoInvalid( charSet
                                                         , input
                                                         , output
                                                         , bufferLen
                                                         , sourceCharsConverted
                                                         , usedTargetSize
                                                         , bFlush
                                                         , replacementSymbol
                                                         , action
                                                         , putBOM);
                }
                
                //printf("sourceCharsConverted = %d, CFStringGetLength(input)=%d\n", sourceCharsConverted, input.length());

                //case when nothing is converted dueto input bufer contains surogate pair for example
                if (sourceCharsConverted == 0 && bFlush) {
                    if (invalidChars.empty()) {
                        return true;
                    }
                    cf_String strInvalid(&*invalidChars.begin(), invalidChars.size());
                    if (strInvalid.getMaximumSizeForEncoding(charSet) > bufferLen) {
                        return true;
                    }
                    
                    //applying unicode 6.3 paragraph 3.19 recomendations, to replace all ill-formed chars with fffd sequence
                    for (int j =0; j < invalidChars.size(); j++) {
                        if (bufferLen < 2) {
                            return true;
                        }
                        //in case of 4 bytes encoding need to put extra 2 bytes
                        if (charSet == kCFStringEncodingUTF32  ||
                            charSet == kCFStringEncodingUTF32BE ||
                            charSet == kCFStringEncodingUTF32LE)
                        {
                            bufferLen -= 2;
                            *output = 0x00;
                            output++;
                            
                            *output = 0x00;
                            output++;
                            usedTargetSize += 2;
                        }
                        
                        if (bufferLen < 2) {
                            return true;
                        }
                        
                        *output = 0xff;
                        output++;
                            
                        *output = 0xfd;
                        output++;
                        usedTargetSize += 2;
                        bufferLen -= 2;
                    }
                    return true;
                }
                //nothing is convered but input has data and not a flush mode
                if (sourceCharsConverted  == 0 && input.length() != 0) {
                    //check for space firstly
                    if (input.getMaximumSizeForEncoding(charSet, putBOM) > bufferLen) {
                        return false;
                    }
                    //then assume that input invalid
                    if (invalidChars.empty()) {
                        //lets store invalid characters count if that case happen
                        for (int i=0; i < input.length(); i++) {
                            invalidChars.push_back(input.getChar(i));
                        }
                    }
                    
                    return true;
                }
                
                invalidChars.clear();
                return isConverted;
            }
            
            //return true if buffer enough
            bool runConversionNoInvalid( const cf_CharacterSet &charSet
                               , const cf_String & input
                               , char * output
                               , size_t bufferLen
                               , size_t& sourceCharsConverted
                               , size_t& usedTargetSize
                               , bool bFlush
                               , char replacementSymbol
                               , ErrorAction action
                               , bool putBOM) const {
                
                CFIndex usedSize;
                
                //ignore uses 2 steps conversion 1 - replace to some X character,
                //second to replace to some y character, after that we can compare chars by chars, and remove differenced characters which means replacement symbols
                if (action == IGNORE) {
                    char replacementX = 'x';
                    char replacementY = 'y';
                    
                    sourceCharsConverted = input.getBytes(charSet, replacementX, output, bufferLen, &usedSize, putBOM);
                    
                    if (usedSize) {
                        std::vector<char> output2(usedSize);
                        
                        input.getBytes(charSet, replacementY, &*output2.begin(), output2.size(), &usedSize, putBOM);
                        
                        for (int i = 0, j=0; i < usedSize; i++, j++) {
                            if (output[j] != output2[j]) {
                                i--;
                            } else if (i != j){
                                output [i] = output[j];
                            }
                        }
                    }
                    
                } else {
                    sourceCharsConverted = input.getBytes(charSet, replacementSymbol, output, bufferLen, &usedSize, putBOM);
                }
                
                //todo: solve null terminator issue of 4 extra bytes for utf8
                if (usedSize) {
                    if (usedSize == bufferLen) {
                        usedTargetSize = bufferLen;
                    } else {
                       // if (charSet == kCFStringEncodingUTF8) {
                       ////     usedTargetSize = std::min(strlen(output), bufferLen);
                       // }else
                        {
                            //hope null terminated sting problems only for utf8 encoding
                            usedTargetSize = usedSize;
                        }
                    }
                } else {
                    usedTargetSize = 0;
                }
                
                if((CFStringRef)input == nullptr)
                    return false;

                //auto convertedSourceRange = input.getRangeOfComposedCharactersSequence(0, sourceCharsConverted);
                //sourceCharsConverted = convertedSourceRange.length;
         
                //cString len represent actual characters without null-terminator, while input string migh be constructed from  external buffer
                if (sourceCharsConverted != input.length() && input.length() != 0) {
                    return false;
                }
                return true;
            }
        };
    }
}