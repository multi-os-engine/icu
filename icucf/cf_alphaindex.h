//
//  cf_alphaindex.h
//  
//
//  Created by user on 30/04/15.
//  Copyright (c) 2015 МАК. All rights reserved.
//

#pragma once

#include <unicode/utypes.h>
#include <vector>

/**
 * Constants for Alphabetic Index Label Types.
 * The form of these enum constants anticipates having a plain C API
 * for Alphabetic Indexes that will also use them.
 * @stable ICU 4.8
 */
typedef enum AlphabeticIndexLabelType {
    /**
     *  Normal Label, typically the starting letter of the names
     *  in the bucket with this label.
     * @stable ICU 4.8
     */
    U_ALPHAINDEX_NORMAL    = 0,
    
    /**
     * Undeflow Label.  The bucket with this label contains names
     * in scripts that sort before any of the bucket labels in this index.
     * @stable ICU 4.8
     */
    U_ALPHAINDEX_UNDERFLOW = 1,
    
    /**
     * Inflow Label.  The bucket with this label contains names
     * in scripts that sort between two of the bucket labels in this index.
     * Inflow labels are created when an index contains normal labels for
     * multiple scripts, and skips other scripts that sort between some of the
     * included scripts.
     * @stable ICU 4.8
     */
    U_ALPHAINDEX_INFLOW    = 2,
    
    /**
     * Overflow Label. Te bucket with this label contains names in scripts
     * that sort after all of the bucket labels in this index.
     * @stable ICU 4.8
     */
    U_ALPHAINDEX_OVERFLOW  = 3
} AlphabeticIndexLabelType;


namespace U_ICU_NAMESPACE {
    namespace cf {

        class AlphabeticIndex {
            int32_t    _maxLabelCount;        // Limit on # of labels permitted in the index.
            int32_t    _currentBucket;        // While an iteration of the index in underway,
                                              // point to the bucket for the current label.
                                              // NULL when no iteration underway.
            UnicodeString uString;
            //std::vector<Bucket> _buckets;

        public:
            
            /**
             * An index "bucket" with a label string and type.
             * It is referenced by getBucketIndex(),
             * and returned by ImmutableIndex.getBucket().
             *
             * The Bucket class is not intended for public subclassing.
             * @stable ICU 51
             */
            class Bucket{
            public:
                /**
                 * Destructor.
                 * @stable ICU 51
                 */
                virtual ~Bucket() {}
                
                /**
                 * Returns the label string.
                 *
                 * @return the label string for the bucket
                 * @stable ICU 51
                 */
                const UnicodeString &getLabel() const { return label_; }
                /**
                 * Returns whether this bucket is a normal, underflow, overflow, or inflow bucket.
                 *
                 * @return the bucket label type
                 * @stable ICU 51
                 */
                AlphabeticIndexLabelType getLabelType() const { return labelType_; }
                
            private:
                UnicodeString label_;
                AlphabeticIndexLabelType labelType_;
            };
            
            /**
             * Immutable, thread-safe version of AlphabeticIndex.
             * This class provides thread-safe methods for bucketing,
             * and random access to buckets and their properties,
             * but does not offer adding records to the index.
             *
             * The ImmutableIndex class is not intended for public subclassing.
             *
             * @stable ICU 51
             */
            class ImmutableIndex {
            public:
                /**
                 * Destructor.
                 * @stable ICU 51
                 */
                virtual ~ImmutableIndex() {
                    
                }
                
                /**
                 * Returns the number of index buckets and labels, including underflow/inflow/overflow.
                 *
                 * @return the number of index buckets
                 * @stable ICU 51
                 */
                int32_t getBucketCount() const {
                    return 0;
                }
                
                /**
                 * Finds the index bucket for the given name and returns the number of that bucket.
                 * Use getBucket() to get the bucket's properties.
                 *
                 * @param name the string to be sorted into an index bucket
                 * @return the bucket number for the name
                 * @stable ICU 51
                 */
                int32_t getBucketIndex(const UnicodeString &name, UErrorCode &errorCode) const {
                    return 0;
                }
                
                /**
                 * Returns the index-th bucket. Returns NULL if the index is out of range.
                 *
                 * @param index bucket number
                 * @return the index-th bucket
                 * @stable ICU 51
                 */
                const Bucket *getBucket(int32_t index) const {
                    return nullptr;
                }
                
            };
            
            
            AlphabeticIndex(const cf_Locale &locale, UErrorCode &status)
                : _maxLabelCount(99),
        //          _buckets(NULL),
                  _currentBucket(NULL)
                /*inputList_(NULL),
                labelsIterIndex_(-1),
                itemsIterIndex_(0),
                initialLabels_(NULL),
                firstCharsInScripts_(NULL),
                collator_(NULL),
                collatorPrimaryOnly_(NULL),
                 */
            {
                
            }
            
            /**
             * Get the limit on the number of labels permitted in the index.
             * The number does not include over, under and inflow labels.
             *
             * @return maxLabelCount maximum number of labels.
             * @stable ICU 4.8
             */
            virtual int32_t getMaxLabelCount() const {
                return _maxLabelCount;
            }
            /**
             * Set a limit on the number of labels permitted in the index.
             * The number does not include over, under and inflow labels.
             * Currently, if the number is exceeded, then every
             * nth item is removed to bring the count down.
             * A more sophisticated mechanism may be available in the future.
             *
             * @param maxLabelCount the maximum number of labels.
             * @param status error code
             * @return This, for chaining
             * @stable ICU 4.8
             */
            virtual AlphabeticIndex &setMaxLabelCount(int32_t maxLabelCount, UErrorCode &status) {
                return *this;
            }

            
            /**
             * Add the index characters from a Locale to the index.  The labels
             * are added to those that are already in the index; they do not replace the
             * existing index characters.  The collation order for this index is not
             * changed; it remains that of the locale that was originally specified
             * when creating this Index.
             *
             * @param locale The locale whose index characters are to be added.
             * @param status Error code, will be set with the reason if the
             *               operation fails.
             * @return this, for chaining
             * @stable ICU 4.8
             */
            virtual AlphabeticIndex &addLabels(const cf_Locale &locale, UErrorCode &status) {
                return *this;
            }
            
            /**
             * Add Labels to this Index.  The labels are additions to those
             * that are already in the index; they do not replace the existing
             * ones.
             * @param additions The additional characters to add to the index, such as A-Z.
             * @param status Error code, will be set with the reason if the
             *               operation fails.
             * @return this, for chaining
             * @stable ICU 4.8
             */
            virtual AlphabeticIndex &addLabels(const UnicodeSet &additions, UErrorCode &status) {
                return *this;
            }
            
            
            /**  Get the number of labels in this index.
             *      Note: may trigger lazy index construction.
             *
             * @param status  Error code, will be set with the reason if the operation fails.
             * @return        The number of labels in this index, including any under, over or
             *                in-flow labels.
             * @stable ICU 4.8
             */
            virtual int32_t  getBucketCount(UErrorCode &status) {
                return 0;
            }
            
            /**
             *   Given the name of a record, return the zero-based index of the Bucket
             *   in which the item should appear.  The name need not be in the index.
             *   A Record will not be added to the index by this function.
             *   Bucket numbers are zero-based, in Bucket iteration order.
             *
             * @param itemName  The name whose bucket position in the index is to be determined.
             * @param status  Error code, will be set with the reason if the operation fails.
             * @return The bucket number for this name.
             * @stable ICU 4.8
             *
             */
            virtual int32_t  getBucketIndex(const UnicodeString &itemName, UErrorCode &status) {
                return 0;
            }
            
            /**
             *  Reset the Bucket iteration for this index.  The next call to nextBucket()
             *  will restart the iteration at the first label.
             *
             * @param status  Error code, will be set with the reason if the operation fails.
             * @return        this, for chaining.
             * @stable ICU 4.8
             */
            virtual AlphabeticIndex &resetBucketIterator(UErrorCode &status) {
                return *this;
            }
            
            /**
             *   Advance the iteration over the Buckets of this index.  Return FALSE if
             *   there are no more Buckets.
             *
             *   @param status  Error code, will be set with the reason if the operation fails.
             *   U_ENUM_OUT_OF_SYNC_ERROR will be reported if the index is modified while
             *   an enumeration of its contents are in process.
             *
             *   @return TRUE if success, FALSE if at end of iteration
             *   @stable ICU 4.8
             */
            virtual UBool nextBucket(UErrorCode &status) {
                return FALSE;
            }
            
            /**
             *   Return the name of the Label of the current bucket from an iteration over the buckets.
             *   If the iteration is before the first Bucket (nextBucket() has not been called),
             *   or after the last, return an empty string.
             *
             *   @return the bucket label.
             *   @stable ICU 4.8
             */
            virtual const UnicodeString &getBucketLabel() const {
                return uString;
            }
            
            /**
             *  Return the type of the label for the current Bucket (selected by the
             *  iteration over Buckets.)
             *
             * @return the label type.
             * @stable ICU 4.8
             */
            virtual AlphabeticIndexLabelType getBucketLabelType() const {
                return U_ALPHAINDEX_NORMAL;
            }
            
            /**
             * Builds an immutable, thread-safe version of this instance, without data records.
             *
             * @return an immutable index instance
             * @stable ICU 51
             */
            ImmutableIndex *buildImmutableIndex(UErrorCode &errorCode) {
                return nullptr;
            }
        };
    }
}

//remapping to our types
#define ICU_TYPE(type) cf::type
#define AlphabeticIndex ICU_TYPE(AlphabeticIndex)
