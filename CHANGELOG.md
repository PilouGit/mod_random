# Changelog

All notable changes to mod_random will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

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
