#pragma once

#include <cf_locale.h>
#include <unicode/utext.h>
#include <memory>
#include <cf_break_iterator_for_characters.h>
#include <cf_break_iterator_cf.h>

namespace U_ICU_NAMESPACE {
    namespace cf {
        class BreakIteratorFactory  {
        public:
            
            /**
             * Create BreakIterator for word-breaks using the given locale.
             * Returns an instance of a BreakIterator implementing word breaks.
             * WordBreak is useful for word selection (ex. double click)
             * @param where the locale.
             * @param status the error code
             * @return A BreakIterator for word-breaks.  The UErrorCode& status
             * parameter is used to return status information to the user.
             * To check whether the construction succeeded or not, you should check
             * the value of U_SUCCESS(err).  If you wish more detailed information, you
             * can check for informational error results which still indicate success.
             * U_USING_FALLBACK_WARNING indicates that a fall back locale was used.  For
             * example, 'de_CH' was requested, but nothing was found there, so 'de' was
             * used.  U_USING_DEFAULT_WARNING indicates that the default locale data was
             * used; neither the requested locale nor any of its fall back locales
             * could be found.
             * The caller owns the returned object and is responsible for deleting it.
             * @stable ICU 2.0
             */
            static BreakIterator* createWordInstance(const cf_Locale& where, UErrorCode& status) {
                return new BreakIteratorCoreFoundation(where, kCFStringTokenizerUnitWordBoundary);
            }
            
            /**
             * Create BreakIterator for line-breaks using specified locale.
             * Returns an instance of a BreakIterator implementing line breaks. Line
             * breaks are logically possible line breaks, actual line breaks are
             * usually determined based on display width.
             * LineBreak is useful for word wrapping text.
             * @param where the locale.
             * @param status The error code.
             * @return A BreakIterator for line-breaks.  The UErrorCode& status
             * parameter is used to return status information to the user.
             * To check whether the construction succeeded or not, you should check
             * the value of U_SUCCESS(err).  If you wish more detailed information, you
             * can check for informational error results which still indicate success.
             * U_USING_FALLBACK_WARNING indicates that a fall back locale was used.  For
             * example, 'de_CH' was requested, but nothing was found there, so 'de' was
             * used.  U_USING_DEFAULT_WARNING indicates that the default locale data was
             * used; neither the requested locale nor any of its fall back locales
             * could be found.
             * The caller owns the returned object and is responsible for deleting it.
             * @stable ICU 2.0
             */
            static BreakIterator* createLineInstance(const cf_Locale& where, UErrorCode& status) {
                return new BreakIteratorCoreFoundation(where, kCFStringTokenizerUnitLineBreak);
            }
            
            /**
             * Create BreakIterator for character-breaks using specified locale
             * Returns an instance of a BreakIterator implementing character breaks.
             * Character breaks are boundaries of combining character sequences.
             * @param where the locale.
             * @param status The error code.
             * @return A BreakIterator for character-breaks.  The UErrorCode& status
             * parameter is used to return status information to the user.
             * To check whether the construction succeeded or not, you should check
             * the value of U_SUCCESS(err).  If you wish more detailed information, you
             * can check for informational error results which still indicate success.
             * U_USING_FALLBACK_WARNING indicates that a fall back locale was used.  For
             * example, 'de_CH' was requested, but nothing was found there, so 'de' was
             * used.  U_USING_DEFAULT_WARNING indicates that the default locale data was
             * used; neither the requested locale nor any of its fall back locales
             * could be found.
             * The caller owns the returned object and is responsible for deleting it.
             * @stable ICU 2.0
             */
            static BreakIterator* createCharacterInstance(const cf_Locale& where, UErrorCode& status) {
                return new BreakIteratorForChars();
            }
            
            /**
             * Create BreakIterator for sentence-breaks using specified locale
             * Returns an instance of a BreakIterator implementing sentence breaks.
             * @param where the locale.
             * @param status The error code.
             * @return A BreakIterator for sentence-breaks.  The UErrorCode& status
             * parameter is used to return status information to the user.
             * To check whether the construction succeeded or not, you should check
             * the value of U_SUCCESS(err).  If you wish more detailed information, you
             * can check for informational error results which still indicate success.
             * U_USING_FALLBACK_WARNING indicates that a fall back locale was used.  For
             * example, 'de_CH' was requested, but nothing was found there, so 'de' was
             * used.  U_USING_DEFAULT_WARNING indicates that the default locale data was
             * used; neither the requested locale nor any of its fall back locales
             * could be found.
             * The caller owns the returned object and is responsible for deleting it.
             * @stable ICU 2.0
             */
            static BreakIterator* createSentenceInstance(const cf_Locale& where, UErrorCode& status) {
                return new BreakIteratorCoreFoundation(where, kCFStringTokenizerUnitSentence);
            }
        };
    }
}