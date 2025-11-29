# mod_random - Apache Module for Cryptographically Secure Random Tokens

An Apache module that generates cryptographically secure random base64-encoded strings and injects them as environment variables for use by applications and other modules.

## Features

### Core Features
- Generates cryptographically secure random bytes using Apache's `apr_generate_random_bytes()` (CSPRNG)
- Automatically sets `RANDOM_STRING` environment variable on every request
- Configurable random data length (1-1024 bytes, default: 16)
- Multiple output formats: base64, hex, base64url, custom alphabet
- Hierarchical configuration with proper merge support
- Minimal performance overhead
- Thread-safe and production-ready

### Advanced Features
- **Multiple Tokens per Request**: Generate different tokens with individual configurations using `RandomAddToken`
- **Custom Alphabets**: Define custom character sets for human-readable codes (e.g., `ABCD-1234-EFGH`)
- **Metadata Encoding**: Encode expiration timestamps into tokens with optional HMAC-SHA256 signatures for validation
- **TTL Caching**: Cache tokens for configurable time periods to reduce generation overhead
- **URL Pattern Matching**: Conditionally generate tokens based on URL regex patterns
- **Timestamp Prefixes**: Include sortable Unix timestamps in tokens
- **Custom Prefixes/Suffixes**: Add custom text before/after tokens
- **HTTP Header Injection**: Automatically inject tokens into response headers

## Installation

### Prerequisites

```bash
# On Ubuntu/Debian
sudo apt-get install apache2-dev libapr1-dev build-essential

# On CentOS/RHEL
sudo yum install httpd-devel apr-devel gcc
```

### Build and Installation

#### Using apxs (recommended)

```bash
cd src
apxs -c mod_random.c
sudo apxs -i -a mod_random.la
sudo systemctl restart apache2
```

#### Using CMake

```bash
mkdir build && cd build
cmake ..
make
sudo make install
```

The build process will:
- Compile the module as `mod_random.so`
- Install it to the Apache modules directory (typically `/usr/lib/apache2/modules/`)
- Create a `.load` file in `/etc/apache2/mods-available/` (if applicable)

### Module Activation

```bash
# Enable the module (Debian/Ubuntu)
sudo a2enmod random

# Restart Apache
sudo systemctl restart apache2
```

Alternatively, manually load the module by adding to your Apache configuration:
```apache
LoadModule random_module modules/mod_random.so
```

## Configuration

### Available Directives

#### Basic Directives
- **`RandomEnabled On|Off`**: Enable or disable the module for the current context (default: Off)
- **`RandomLength N`**: Set the length in bytes of random data to generate (default: 16, range: 1-1024)
- **`RandomFormat format`**: Set output format: `base64`, `hex`, `base64url`, or `custom` (default: base64)
- **`RandomVarName name`**: Set environment variable name (default: RANDOM_STRING)
- **`RandomHeader name`**: Set HTTP response header name to auto-inject token (optional)

#### Advanced Directives
- **`RandomIncludeTimestamp On|Off`**: Include Unix timestamp prefix in token (default: Off)
- **`RandomPrefix text`**: Set prefix to prepend to token (optional)
- **`RandomSuffix text`**: Set suffix to append to token (optional)
- **`RandomOnlyFor pattern`**: Regex pattern to match URLs for conditional generation (optional)
- **`RandomTTL seconds`**: Cache token for N seconds (0-86400, default: 0 = no cache)

#### Custom Alphabet Directives
- **`RandomAlphabet charset`**: Set custom character set for 'custom' format (e.g., '0123456789ABCDEFGHJKMNPQRSTVWXYZ')
- **`RandomAlphabetGrouping N`**: Group custom alphabet output every N characters with '-' (0 = no grouping)

#### Metadata & Validation Directives
- **`RandomExpiry seconds`**: Set token expiration time in seconds (0-31536000, requires RandomEncodeMetadata On)
- **`RandomEncodeMetadata On|Off`**: Encode expiry metadata into token (requires RandomExpiry > 0)
- **`RandomSigningKey key`**: Set HMAC-SHA256 signing key for token validation (optional, for metadata mode)

#### Multi-Token Directive
- **`RandomAddToken VAR_NAME [key=value ...]`**: Add a token with custom configuration
  - Supported keys: `length`, `format`, `header`, `timestamp`, `prefix`, `suffix`, `ttl`
  - Example: `RandomAddToken CSRF_TOKEN length=32 format=base64url header=X-CSRF-Token ttl=3600`

### How It Works

The module operates using Apache's `fixups` hook, which means:
- It runs late in the request processing cycle, after all configuration is resolved
- The `RANDOM_STRING` environment variable is available to handlers and applications
- Configuration directives properly merge in hierarchical contexts (Directory, Location)
- Sub-requests are automatically skipped to avoid duplicate generation

### Configuration Examples

#### Enable for entire virtual host

```apache
<VirtualHost *:80>
    ServerName example.com
    DocumentRoot /var/www/html

    RandomEnabled On
    RandomLength 32
</VirtualHost>
```

#### Enable for specific locations with inheritance

```apache
<Directory /var/www>
    RandomEnabled On
    RandomLength 16
</Directory>

<Directory /var/www/api>
    # Inherits RandomEnabled On from parent
    RandomLength 32  # Overrides length to 32 bytes
</Directory>
```

#### Hierarchical configuration example

```apache
<VirtualHost *:80>
    <Location />
        RandomEnabled On
        RandomLength 16
    </Location>

    <Location /api>
        # Inherits enabled=On, overrides length
        RandomLength 64
    </Location>

    <Location /public>
        # Explicitly disable for public endpoints
        RandomEnabled Off
    </Location>
</VirtualHost>
```

#### Integration with mod_headers

```apache
RandomEnabled On
RandomLength 32
Header set X-Request-ID "%{RANDOM_STRING}e"
```

#### Integration with mod_rewrite

```apache
RandomEnabled On
RewriteEngine On
RewriteRule ^(.*)$ - [E=REQUEST_ID:%{RANDOM_STRING}e]
```

#### Multiple tokens with different configurations

```apache
<Location /api>
    # Generate multiple tokens per request
    RandomAddToken CSRF_TOKEN length=32 format=base64url header=X-CSRF-Token ttl=3600
    RandomAddToken REQUEST_ID length=16 format=hex timestamp=on header=X-Request-ID
    RandomAddToken SESSION_NONCE length=24 prefix=sess_ format=base64
</Location>
```

#### Human-readable codes with custom alphabet

```apache
<Location /vouchers>
    RandomEnabled On
    RandomFormat custom
    RandomAlphabet "0123456789ABCDEFGHJKMNPQRSTVWXYZ"
    RandomAlphabetGrouping 4
    RandomLength 12
    # Generates codes like: ABCD-1234-EFGH
</Location>
```

#### Tokens with expiration and validation

```apache
<Location /secure>
    RandomEnabled On
    RandomLength 24
    RandomFormat base64url
    RandomExpiry 3600
    RandomEncodeMetadata On
    RandomSigningKey "your-secret-key-here"
    # Generates: 1735567890:token_data:hmac_signature
</Location>
```

#### Combined advanced features

```apache
<VirtualHost *:443>
    <Location /api/v1>
        # High-security API tokens with all features
        RandomAddToken API_TOKEN length=32 format=base64url ttl=300
        RandomExpiry 900
        RandomEncodeMetadata On
        RandomSigningKey "api-secret-key-2025"
        RandomOnlyFor "^/api/v1/(protected|admin)/"
    </Location>

    <Location /vouchers>
        # User-friendly voucher codes
        RandomEnabled On
        RandomFormat custom
        RandomAlphabet "0123456789ABCDEFGHJKMNPQRSTVWXYZ"
        RandomAlphabetGrouping 4
        RandomLength 16
        RandomPrefix "VCH-"
    </Location>
</VirtualHost>
```

## Environment Variable

The module sets the following environment variable on each request:

- **`RANDOM_STRING`**: Contains the generated base64-encoded random string

This variable can be accessed by:
- Other Apache modules using `%{RANDOM_STRING}e` syntax
- CGI scripts and server-side applications as a standard environment variable
- Logging configurations for request tracking

### Language-specific access

```php
// PHP
$random_token = $_SERVER['RANDOM_STRING'];
```

```python
# Python (CGI)
import os
random_token = os.environ.get('RANDOM_STRING')
```

```javascript
// Node.js
const randomToken = process.env.RANDOM_STRING;
```

## Use Cases

### Request Tracking and Logging

```apache
RandomEnabled On
RandomLength 16
LogFormat "%h %l %u %t \"%r\" %>s %O \"%{Referer}i\" \"%{User-Agent}i\" %{RANDOM_STRING}e" combined_with_id
CustomLog logs/access.log combined_with_id
```

### CSRF Token Generation

```apache
<Location /api>
    RandomEnabled On
    RandomLength 32
    Header set X-CSRF-Token "%{RANDOM_STRING}e"
</Location>
```

### Nonce for Content Security Policy

```apache
RandomEnabled On
RandomLength 24
Header set Content-Security-Policy "script-src 'nonce-%{RANDOM_STRING}e' 'strict-dynamic';"
```

### Session Token Generation

```apache
RandomEnabled On
RandomLength 32
# Pass to backend applications
RequestHeader set X-Session-Nonce "%{RANDOM_STRING}e"
```

### Validating Tokens with Metadata Encoding

When using `RandomEncodeMetadata On` with `RandomExpiry` and optionally `RandomSigningKey`, tokens are generated in the format:
- Without signature: `expiry_timestamp:token_data`
- With signature: `expiry_timestamp:token_data:hmac_signature`

Example PHP validation code:

```php
<?php
function validate_token($token, $signing_key = null) {
    $parts = explode(':', $token);

    if (count($parts) < 2) {
        return ['valid' => false, 'error' => 'Invalid token format'];
    }

    $expiry = (int)$parts[0];
    $data = $parts[1];
    $signature = isset($parts[2]) ? $parts[2] : null;

    // Check expiration
    if (time() > $expiry) {
        return ['valid' => false, 'error' => 'Token expired'];
    }

    // Verify signature if present
    if ($signature && $signing_key) {
        $payload = $expiry . ':' . $data;
        $expected_signature = hash_hmac('sha256', $payload, $signing_key);

        if (!hash_equals($expected_signature, $signature)) {
            return ['valid' => false, 'error' => 'Invalid signature'];
        }
    }

    return [
        'valid' => true,
        'expiry' => $expiry,
        'data' => $data,
        'remaining_ttl' => $expiry - time()
    ];
}

// Usage
$token = $_SERVER['RANDOM_STRING'] ?? '';
$result = validate_token($token, 'your-secret-key-here');

if ($result['valid']) {
    echo "Token is valid, expires in " . $result['remaining_ttl'] . " seconds";
} else {
    echo "Token validation failed: " . $result['error'];
}
?>
```

## Technical Details

### Cryptographic Security

- **Algorithm**: Uses Apache's `apr_generate_random_bytes()` which provides cryptographically secure pseudo-random data
- **Entropy Source**: `/dev/urandom` on Linux/Unix, `CryptGenRandom` on Windows
- **Suitable for**: CSRF tokens, session tokens, nonces, request IDs

### Important Notes

- **NOT guaranteed unique**: While collision probability is negligible with sufficient length, this module does NOT guarantee uniqueness
- **For unique IDs**: Use Apache's `mod_unique_id` if you need guaranteed unique identifiers
- **Collision probability**: With 16 bytes (128 bits), collision becomes significant only after approximately 2^64 generations

### Output Length

The final string length depends on the format used:

**Base64 format** (default): Approximately 4/3 of configured byte length
- 16 bytes → ~22 characters
- 24 bytes → ~32 characters
- 32 bytes → ~44 characters

**Hex format**: Exactly 2x configured byte length
- 16 bytes → 32 characters
- 32 bytes → 64 characters

**Base64URL format**: Similar to base64 but URL-safe (no padding)
- 16 bytes → ~22 characters (no = padding)

**Custom alphabet format**: Variable length based on alphabet size
- With 32-character alphabet: ~1.6x byte length
- With grouping enabled: additional separator characters

### Signature & Metadata

When using `RandomEncodeMetadata On`:
- **Format**: `timestamp:token[:signature]`
- **Timestamp**: Unix timestamp (10 digits)
- **Signature**: HMAC-SHA256 in hex (64 characters)
- **Total overhead**: 11-75 additional characters

### Performance

- **Minimal overhead**: Random generation only occurs when enabled
- **Memory efficient**: Uses Apache's pool-based allocation
- **Thread-safe**: Fully compatible with all Apache MPMs (prefork, worker, event)
- **TTL caching**: Reduces generation overhead for high-traffic scenarios
- **No logging overhead**: Debug logging has been removed for production use

## Security Considerations

### Strengths

- Uses cryptographically secure random number generation (CSPRNG) with error checking
- CSPRNG failures are detected and logged as critical errors
- No predictable patterns in generated strings
- Memory automatically cleaned up via Apache's pool system
- Safe for security-sensitive applications (CSRF, nonces, etc.)
- HMAC-SHA256 signatures prevent token tampering when enabled
- Metadata encoding enables server-side expiration validation
- Thread-safe implementation with mutex-protected caching and error handling

### Limitations

- NOT suitable as sole mechanism for unique ID generation (use `mod_unique_id` instead)
- Collision probability exists (though negligible with 16+ bytes)
- Generated tokens are random, not sequential or sortable (unless timestamp prefix enabled)
- Signing key is stored in plain text in Apache configuration (use file-based secrets in production)

### Best Practices

**Token Length Recommendations:**
- CSRF tokens: 24-32 bytes recommended
- API tokens: 32+ bytes for high-security applications
- Nonces (CSP, etc.): 16-24 bytes typically sufficient
- Request IDs: 16 bytes usually adequate (collision risk acceptable)
- Custom alphabet codes: 12-16 bytes for human-readable vouchers

**Security Best Practices:**
- Always use `RandomEncodeMetadata On` with `RandomExpiry` for time-limited tokens
- Use `RandomSigningKey` to prevent token tampering in untrusted environments
- Rotate signing keys periodically (monthly or quarterly)
- Use `RandomOnlyFor` to restrict token generation to specific URL patterns
- Validate tokens server-side before trusting them
- Use `RandomFormat base64url` for tokens passed in URLs or headers
- For public-facing codes, use custom alphabets without ambiguous characters (avoid 0/O, 1/I/l)

## Recent Security Improvements

**Version 3.x includes critical security enhancements:**

### CSPRNG Error Detection (Critical)
- Added verification of `apr_generate_random_bytes()` return value
- CSPRNG failures now logged as **APLOG_CRIT** and token generation aborted
- Prevents use of uninitialized or predictable random data
- **Impact**: Eliminates risk of weak tokens due to system entropy exhaustion

### Thread-Safety Improvements (Important)
- Added error checking for mutex creation (`apr_thread_mutex_create`)
- Failed mutex creation gracefully disables caching instead of crashing
- Prevents potential NULL pointer dereferences in concurrent requests
- **Impact**: Improved stability under high concurrency

### Configuration Merge Fixes (Important)
- Fixed incorrect merge behavior for `RandomAlphabetGrouping=0` and `RandomExpiry=0`
- Added proper sentinel values (`RANDOM_GROUPING_UNSET`, `RANDOM_EXPIRY_UNSET`)
- Child configuration now correctly overrides parent when explicitly set to 0
- **Impact**: Configuration now behaves as documented

### HMAC Upgrade (Security)
- Upgraded from HMAC-SHA1 to **HMAC-SHA256** for token signatures
- SHA256 provides 256-bit security vs SHA1's theoretical weaknesses
- Update your validation code: `hash_hmac('sha256', ...)` instead of `hash_hmac('sha1', ...)`
- **Impact**: Stronger cryptographic protection against token tampering

### Code Quality Improvements
- Eliminated magic numbers with defined constants (`RANDOM_TTL_MAX_SECONDS`, etc.)
- Improved buffer size calculations in custom alphabet encoding
- Reduced code duplication with helper functions
- Enhanced logging for debugging and security monitoring

**Migration Notes:**
- **HMAC signature change**: If using `RandomSigningKey`, tokens generated before v3.x are incompatible
- **No action required** for most users (backwards compatible for non-signed tokens)
- Update any server-side validation code to use SHA256 instead of SHA1

## Compatibility

- Apache 2.4+
- Requires APR (Apache Portable Runtime)
- Compatible with all major operating systems (Linux, BSD, macOS, Windows)
- Works with all Apache MPMs (prefork, worker, event)
- No external dependencies beyond Apache and APR

## Troubleshooting

### Module not loading

```bash
# Verify module is installed
ls -l /usr/lib/apache2/modules/mod_random.so

# Check Apache error log
sudo tail -f /var/log/apache2/error.log

# Verify module is loaded
apachectl -M | grep random
```

### Environment variable not set

```apache
# Verify RandomEnabled is On
<Directory /var/www>
    RandomEnabled On
</Directory>

# Test with simple PHP script
<?php var_dump($_SERVER['RANDOM_STRING'] ?? 'NOT SET'); ?>
```

### Configuration not applying

```bash
# Test configuration syntax
sudo apachectl configtest

# Restart (not reload) Apache after changes
sudo systemctl restart apache2
```

## License

This module is provided as-is for use with Apache HTTP Server.

## Contributing

Contributions, bug reports, and feature requests are welcome.

## See Also

- Apache `mod_unique_id` - For guaranteed unique identifiers
- Apache `mod_headers` - For manipulating HTTP headers
- Apache `mod_setenvif` - For conditional environment variables
