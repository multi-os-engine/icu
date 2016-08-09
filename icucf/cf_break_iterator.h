#pragma once

namespace U_ICU_NAMESPACE {
    namespace cf {
        enum {
            /**
             * DONE is returned by previous() and next() after all valid
             * boundaries have been returned.
             * @stable ICU 2.0
             */
            DONE = (int32_t)-1
        };
        
        class BreakIterator {
        public:
            /**
             * Return a polymorphic copy of this object.  This is an abstract
             * method which subclasses implement.
             * @stable ICU 2.0
             */
            virtual BreakIterator* clone(void) const  = 0;
            
            /**
             * Reset the break iterator to operate over the text represented by
             * the UText.  The iterator position is reset to the start.
             *
             * This function makes a shallow clone of the supplied UText.  This means
             * that the caller is free to immediately close or otherwise reuse the
             * Utext that was passed as a parameter, but that the underlying text itself
             * must not be altered while being referenced by the break iterator.
             *
             * All index positions returned by break iterator functions are
             * native indices from the UText. For example, when breaking UTF-8
             * encoded text, the break positions returned by next(), previous(), etc.
             * will be UTF-8 string indices, not UTF-16 positions.
             *
             * @param text The UText used to change the text.
             * @param status receives any error codes.
             * @stable ICU 3.4
             */
            virtual void  setText(UText *text, UErrorCode &status) = 0;
            
            /**
             *  Set the subject text string upon which the break iterator is operating
             *  without changing any other aspect of the matching state.
             *  The new and previous text strings must have the same content.
             *
             *  This function is intended for use in environments where ICU is operating on
             *  strings that may move around in memory.  It provides a mechanism for notifying
             *  ICU that the string has been relocated, and providing a new UText to access the
             *  string in its new position.
             *
             *  Note that the break iterator implementation never copies the underlying text
             *  of a string being processed, but always operates directly on the original text
             *  provided by the user. Refreshing simply drops the references to the old text
             *  and replaces them with references to the new.
             *
             *  Caution:  this function is normally used only by very specialized,
             *  system-level code.  One example use case is with garbage collection that moves
             *  the text in memory.
             *
             * @param input      The new (moved) text string.
             * @param status     Receives errors detected by this function.
             * @return           *this
             *
             * @stable ICU 49
             */
            virtual BreakIterator &refreshInputText(UText *input, UErrorCode &status) = 0;
            
            /**
             * Set the iterator position to the index of the first character in the text being scanned.
             * @return The index of the first character in the text being scanned.
             * @stable ICU 2.0
             */
            virtual int32_t first(void)   = 0;
            /**
             * Set the iterator position to the index immediately BEYOND the last character in the text being scanned.
             * @return The index immediately BEYOND the last character in the text being scanned.
             * @stable ICU 2.0
             */
            virtual int32_t last(void)  = 0;
            /**
             * Set the iterator position to the boundary preceding the current boundary.
             * @return The character index of the previous text boundary or DONE if all
             * boundaries have been returned.
             * @stable ICU 2.0
             */
            virtual int32_t previous(void)  = 0;
            /**
             * Advance the iterator to the boundary following the current boundary.
             * @return The character index of the next text boundary or DONE if all
             * boundaries have been returned.
             * @stable ICU 2.0
             */
            virtual int32_t next(void) = 0;
            /**
             * Return character index of the current interator position within the text.
             * @return The boundary most recently returned.
             * @stable ICU 2.0
             */
            virtual int32_t current(void) const  = 0;
            /**
             * Set the iterator position to the first boundary preceding the specified offset.
             * The value returned is always smaller than the offset or
             * the value BreakIterator.DONE
             * @param offset the offset to begin scanning.
             * @return The first boundary before the specified offset.
             * @stable ICU 2.0
             */
            virtual int32_t preceding(int32_t offset)  = 0;
            /**
             * Advance the iterator to the first boundary following the specified offset.
             * The value returned is always greater than the offset or
             * the value BreakIterator.DONE
             * @param offset the offset to begin scanning.
             * @return The first boundary after the specified offset.
             * @stable ICU 2.0
             */
            virtual int32_t following(int32_t offset)  = 0;
            /**
             * Return true if the specfied position is a boundary position.
             * As a side effect, the current position of the iterator is set
             * to the first boundary position at or following the specified offset.
             * @param offset the offset to check.
             * @return True if "offset" is a boundary position.
             * @stable ICU 2.0
             */
            virtual UBool isBoundary(int32_t offset)  = 0;
        };
    }
}