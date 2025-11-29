# Changelog

All notable changes to mod_random will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [4.0.0] - 2025-11-29

### BREAKING CHANGES

#### Legacy Single-Token Mode Removed
The v2.x/v3.x "legacy mode" using `RandomEnabled` has been **completely removed** to simplify the codebase and enforce best practices.

**Old configuration (NO LONGER WORKS):**
```apache
RandomEnabled On
RandomLength 32
RandomVarName CSRF_TOKEN
RandomHeader X-CSRF-Token
```

**New configuration (REQUIRED):**
```apache
RandomAddToken CSRF_TOKEN length=32 header=X-CSRF-Token
```

#### Removed Directives
- ❌ `RandomEnabled` - No longer needed (use `RandomAddToken` instead)
- ❌ `RandomVarName` - Specify variable name in `RandomAddToken`
- ❌ `RandomHeader` - Use `header=` parameter in `RandomAddToken`

#### Simplified Configuration Structure
All remaining directives now serve as **defaults** for `RandomAddToken`:
- `RandomLength` - Default token length (default: 16 bytes)
- `RandomFormat` - Default format (base64, hex, base64url, custom)
- `RandomIncludeTimestamp` - Default timestamp inclusion
- `RandomPrefix` / `RandomSuffix` - Default affixes for all tokens
- `RandomTTL` - Default cache TTL

### Changed

- Simplified `random_config` structure (removed 6 legacy fields)
- Streamlined `mod_random.c` from 122 to 86 lines
- Reduced `mod_random_config.c` from 466 to 443 lines
- Module binary size: **35KB** (unchanged)
- Total source lines: **1028 lines** (23 lines less)

### Improved

- **Cleaner codebase** - Single token generation path
- **Better semantics** - All tokens are explicit via `RandomAddToken`
- **Easier maintenance** - No dual-mode logic
- **Clearer documentation** - Removed confusing legacy examples

### Migration Guide

**Single Token:**
```apache
# Before (v3.x)
RandomEnabled On
RandomLength 24
RandomFormat base64url

# After (v4.x)
RandomAddToken RANDOM_STRING length=24 format=base64url
```

**With Header:**
```apache
# Before (v3.x)
RandomEnabled On
RandomVarName CSRF_TOKEN
RandomHeader X-CSRF-Token

# After (v4.x)
RandomAddToken CSRF_TOKEN header=X-CSRF-Token
```

**Multiple Tokens (unchanged):**
```apache
RandomAddToken CSRF_TOKEN length=32 format=base64url
RandomAddToken REQUEST_ID length=16 format=hex header=X-Request-ID
```

## [3.1.0] - 2025-11-29

### Major Refactoring

#### Modular Architecture
- Refactored monolithic 976-line file into 7 modular files for better maintainability
- **mod_random.c** (122 lines): Main module entry point and request handler
- **mod_random_config.c** (466 lines): All configuration directives and handlers
- **mod_random_encode.c** (170 lines): Encoding functions (hex, base64, base64url, custom alphabet)
- **mod_random_crypto.c** (88 lines): HMAC-SHA1 and metadata encoding
- **mod_random_token.c** (89 lines): Token generation with caching
- **mod_random_types.h** (70 lines): Type definitions and constants
- **mod_random.h** (46 lines): Public API declarations

### Fixed

#### Critical Bugs
- **Bug #1 (CRITICAL)**: Fixed race condition in per-token cache
  - Added dedicated `cache_mutex` to each `random_token_spec`
  - Each token now has independent thread-safe caching
- **Bug #2 (CRITICAL)**: Fixed buffer overflow in `encode_custom_alphabet`
  - Changed accumulator from `unsigned int` to `unsigned long long` (64-bit)
  - Added proper bounds checking (`p >= end - 2`)
  - Increased `max_output_len` calculation with safety margins

#### High Priority Bugs
- **Bug #3 (HIGH)**: Replaced non-thread-safe `time()` calls
  - All `time(NULL)` replaced with `apr_time_now()` + `apr_time_sec()`
  - Provides microsecond precision and thread-safety
- **Bug #5 (HIGH)**: Added alphabet validation
  - Detects duplicate characters using bitmap (`unsigned char seen[256]`)
  - Enforces maximum alphabet length (256 characters)
  - Prevents entropy loss from duplicate characters

#### Medium Priority Bugs
- **Bug #8 (MEDIUM)**: Added token count limit
  - Maximum 50 tokens per context (`RANDOM_MAX_TOKENS`)
  - Prevents DoS via excessive token specifications

### Changed

- Module binary size reduced from 78KB to 35KB (55% smaller)
- Total source lines increased from 976 to 1051 (modular overhead)
- Improved code organization and separation of concerns
- Better testability with isolated functional modules
- Updated CMakeLists.txt to build all source files

### Technical

- All compilation warnings eliminated
- No breaking changes to configuration API
- Backward compatible with v3.0.0 configurations
- Thread-safety improved across all subsystems

## [3.0.0] - 2025-01-29

### Major New Features

#### Feature #1: Multiple Tokens per Request
- Added `RandomAddToken` directive for generating multiple tokens with individual configurations
- Supports per-token settings: length, format, header, timestamp, prefix, suffix, ttl
- Enables different token types per request (e.g., CSRF_TOKEN, REQUEST_ID, SESSION_NONCE)
- Each token can have its own caching TTL and output format
- Example: `RandomAddToken CSRF_TOKEN length=32 format=base64url header=X-CSRF-Token ttl=3600`

#### Feature #2: Custom Alphabet Support
- Added `RandomFormat custom` for human-readable codes
- Added `RandomAlphabet` directive to define custom character sets
- Added `RandomAlphabetGrouping` directive for formatted output (e.g., ABCD-1234-EFGH)
- Supports any alphabet size (2-256 characters)
- Ideal for voucher codes, promo codes, and user-facing identifiers
- Example: `RandomAlphabet "0123456789ABCDEFGHJKMNPQRSTVWXYZ"` (removes ambiguous chars)

#### Feature #3: Metadata Encoding with HMAC Signatures
- Added `RandomExpiry` directive to set token expiration time (0-31536000 seconds)
- Added `RandomEncodeMetadata` flag to encode expiry timestamp into token
- Added `RandomSigningKey` directive for HMAC-SHA1 signature generation
- Token format: `timestamp:token_data:hmac_signature` (or without signature if no key)
- Enables server-side token validation and expiration checking
- Prevents token tampering when signing key is configured
- HMAC-SHA1 implementation using APR crypto functions

### Added

- New output format type: `RANDOM_FORMAT_CUSTOM` for custom alphabet encoding
- `encode_custom_alphabet()` function with configurable alphabet and grouping
- `encode_token_with_metadata()` function for JWT-like token structure
- `hmac_sha1()` implementation using APR SHA1 functions
- `generate_random_string_ex()` extended generation function with alphabet support
- `generate_token_from_spec()` helper for unified token generation
- `random_token_spec` struct for individual token configurations
- Linked list support for multiple token specifications per context
- Configuration fields: `custom_alphabet`, `alphabet_grouping`, `expiry_seconds`, `encode_metadata`, `signing_key`
- Per-token caching with individual TTL support
- 8 new configuration directives (total: 16 directives)

### Changed

- Token generation refactored into modular helper functions
- `random_fixups()` now supports both legacy single-token and multi-token modes
- Configuration merge logic updated to handle new fields
- Module size increased from 44KB to 72KB
- Line count increased from ~450 to 924 lines
- Enhanced thread-safety for per-token caching

### Security

- HMAC-SHA1 signatures prevent token tampering
- Metadata encoding enables time-based token expiration
- Server-side validation examples provided in README
- Custom alphabets can exclude ambiguous characters for better security
- Thread-safe caching with mutex protection for all token types

### Documentation

- Comprehensive README update with 20+ configuration examples
- Added validation code examples (PHP) for metadata tokens
- Documented all output format specifications
- Added best practices for token length by use case
- Security considerations for signing key management
- Performance characteristics of TTL caching

### Technical

- Added includes: `apr_sha1.h`, `string.h` for HMAC implementation
- HMAC-SHA1 follows RFC 2104 specification
- Custom alphabet encoding uses efficient bit-packing algorithm
- Metadata encoding compatible with standard Unix timestamp validation
- All features compile cleanly with no warnings (tested with GCC)

## [2.0.0] - 2025-01-28

### Breaking Changes

- Renamed environment variable from `UNIQUE_STRING` to `RANDOM_STRING` to accurately reflect that the generated values are random, not guaranteed unique
- Changed hook from `post_read_request` to `fixups` for proper configuration resolution

### Added

- Comprehensive module documentation (34 lines) explaining purpose, use cases, and security considerations
- Configuration merge support for hierarchical Apache configuration contexts
- Sentinel value system for proper detection of explicitly configured vs inherited values
- Named constants for all magic numbers (RANDOM_LENGTH_DEFAULT, RANDOM_LENGTH_MIN, RANDOM_LENGTH_MAX)
- Default value application in request handler for unset configuration values
- Language-specific access examples in README (PHP, Python, Node.js)
- Troubleshooting section in README
- Security best practices section in README
- Detailed technical notes about collision probability

### Changed

- Hook timing from `post_read_request` to `fixups` to ensure configuration is fully resolved before processing
- Configuration initialization to use sentinel values (-1) instead of defaults (0, 16)
- Merge logic to properly detect explicitly set values vs unset values
- Parameter name from `char *x` to `char *dir` for clarity
- Error messages to use defined constants instead of hardcoded values
- Validation logic to use named constants (RANDOM_LENGTH_MIN, RANDOM_LENGTH_MAX)
- Function name from `random_post_read_request` to `random_fixups` to match hook type

### Fixed

- Critical bug in merge_config where explicitly configured default values (e.g., length=16) would incorrectly inherit parent values
- Configuration inheritance now works correctly in nested Directory/Location contexts
- Child configurations can now explicitly set values to defaults without being overridden by parent

### Removed

- Unused header includes: `apr_strings.h` and `apr_lib.h`
- All DEBUG level logging to reduce production overhead
- INFO level logging from hook registration
- Verbose logging from configuration functions
- Empty lines and code formatting inconsistencies

### Security

- Confirmed use of cryptographically secure random number generator (CSPRNG)
- Documented that module is NOT suitable for guaranteed unique IDs
- Added recommendations for appropriate token lengths for different use cases
- Clarified collision probability and limitations

## [1.0.0] - Initial Release

### Added

- Basic random string generation using `apr_generate_random_bytes()`
- Base64 encoding of random bytes
- `RandomEnabled` directive to enable/disable module
- `RandomLength` directive to configure byte length (1-1024)
- Environment variable injection (`UNIQUE_STRING`)
- Post-read-request hook for early processing
- Basic configuration support
- Debug logging

### Security

- Cryptographically secure random generation
- Input validation for length parameter
