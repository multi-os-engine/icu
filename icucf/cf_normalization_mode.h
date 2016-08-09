#pragma once
/**
 * Constants for normalization modes.
 * @stable ICU 2.0
 */
typedef enum {
    /** No decomposition/composition. @stable ICU 2.0 */
    UNORM_NONE = 1,
    /** Canonical decomposition. @stable ICU 2.0 */
    UNORM_NFD = 2,
    /** Compatibility decomposition. @stable ICU 2.0 */
    UNORM_NFKD = 3,
    /** Canonical decomposition followed by canonical composition. @stable ICU 2.0 */
    UNORM_NFC = 4,
    /** Default normalization. @stable ICU 2.0 */
    UNORM_DEFAULT = UNORM_NFC,
    /** Compatibility decomposition followed by canonical composition. @stable ICU 2.0 */
    UNORM_NFKC =5,
    /** "Fast C or D" form. @stable ICU 2.0 */
    UNORM_FCD = 6,
    
} UNormalizationMode;
