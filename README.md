# mod_random - Apache Module for Cryptographically Secure Random Tokens

An Apache module that generates cryptographically secure random base64-encoded strings and injects them as environment variables for use by applications and other modules.

## Features

- Generates cryptographically secure random bytes using Apache's `apr_generate_random_bytes()` (CSPRNG)
- Automatically sets `RANDOM_STRING` environment variable on every request
- Configurable random data length (1-1024 bytes, default: 16)
- Base64 encoding for safe use in web contexts
- Hierarchical configuration with proper merge support
- Minimal performance overhead
- Thread-safe and production-ready

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

- **`RandomEnabled On|Off`**: Enable or disable the module for the current context (default: Off)
- **`RandomLength N`**: Set the length in bytes of random data to generate (default: 16, range: 1-1024)

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

The final base64 string length will be approximately 4/3 of the configured byte length:
- 16 bytes (default) yields approximately 22 characters
- 24 bytes yields approximately 32 characters
- 32 bytes yields approximately 44 characters
- 64 bytes yields approximately 88 characters

### Performance

- **Minimal overhead**: Random generation only occurs when enabled
- **Memory efficient**: Uses Apache's pool-based allocation
- **Thread-safe**: Fully compatible with all Apache MPMs (prefork, worker, event)
- **No logging overhead**: Debug logging has been removed for production use

## Security Considerations

### Strengths

- Uses cryptographically secure random number generation (CSPRNG)
- No predictable patterns in generated strings
- Memory automatically cleaned up via Apache's pool system
- Safe for security-sensitive applications (CSRF, nonces, etc.)

### Limitations

- NOT suitable as sole mechanism for unique ID generation (use `mod_unique_id` instead)
- Collision probability exists (though negligible with 16+ bytes)
- Generated tokens are random, not sequential or sortable

### Best Practices

- Use at least 16 bytes (128 bits) for security-sensitive tokens
- For CSRF tokens: 24-32 bytes recommended
- For nonces: 16-24 bytes typically sufficient
- For request IDs: 16 bytes usually adequate (collision risk acceptable)

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
