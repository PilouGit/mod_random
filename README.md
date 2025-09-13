# mod_random - Apache Module for Random String Generation

An Apache module that generates cryptographically secure random base64-encoded strings and sets them as environment variables for use by other modules and applications.

## Features

- Generates cryptographically secure random bytes using Apache's `apr_generate_random_bytes()`
- Automatically sets `UNIQUE_STRING` environment variable on every request
- Configurable random data length (1-1024 bytes)
- Base64 encoding for safe use in web contexts
- Comprehensive debug logging
- Easy integration with other Apache modules

## Installation

### Prerequisites

```bash
# On Ubuntu/Debian
sudo apt-get install apache2-dev libapr1-dev cmake build-essential

# On CentOS/RHEL
sudo yum install httpd-devel apr-devel cmake gcc
```

### Build and Installation

```bash
mkdir build && cd build
cmake ..
make
sudo make install
```

The build process will:
- Compile the module as `mod_random.so`
- Install it to the Apache modules directory (typically `/usr/lib/apache2/modules/`)
- Create a `.load` file in `/etc/apache2/mods-available/` (if the directory exists)

### Module Activation

```bash
# Enable the module
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

The module operates using Apache's `post_read_request` hook, which means:
- It runs early in the request processing cycle
- It processes every request when enabled
- It sets the `UNIQUE_STRING` environment variable before other modules execute
- Other modules can access this variable using `%{UNIQUE_STRING}e` syntax

### Configuration Examples

#### Enable for entire server
```apache
# In your main server configuration or virtual host
RandomEnabled On
RandomLength 32
```

#### Enable for specific locations
```apache
<Location "/secure">
    RandomEnabled On
    RandomLength 24
</Location>

<Directory "/var/www/secure">
    RandomEnabled On
    RandomLength 16
</Directory>
```

#### Integration with mod_headers
```apache
RandomEnabled On
RandomLength 32
Header set X-Request-ID "%{UNIQUE_STRING}e"
```

#### Integration with mod_rewrite
```apache
RandomEnabled On
RewriteEngine On
RewriteRule ^(.*)$ - [E=REQUEST_ID:%{UNIQUE_STRING}e]
```

## Environment Variable

The module sets the following environment variable on each request:

- **`UNIQUE_STRING`**: Contains the generated base64-encoded random string

This variable can be accessed by:
- Other Apache modules using `%{UNIQUE_STRING}e` syntax
- CGI scripts and server-side applications as a standard environment variable
- Logging configurations for request tracking

## Use Cases

### Request Tracking
```apache
RandomEnabled On
RandomLength 16
LogFormat "%h %l %u %t \"%r\" %>s %O \"%{Referer}i\" \"%{User-Agent}i\" %{UNIQUE_STRING}e" combined_with_id
CustomLog logs/access.log combined_with_id
```

### CSRF Protection
```apache
<Location "/api">
    RandomEnabled On
    RandomLength 32
    Header set X-CSRF-Token "%{UNIQUE_STRING}e"
</Location>
```

### Session Management Integration
```apache
RandomEnabled On
RandomLength 24
# Pass to backend applications via header
Header set X-Session-Nonce "%{UNIQUE_STRING}e"
```

## Technical Details

- **Random Generation**: Uses Apache's `apr_generate_random_bytes()` which provides cryptographically secure random data
- **Base64 Encoding**: Uses Apache's `apr_base64_encode()` for consistent encoding
- **Memory Management**: All allocations use Apache's pool-based memory management
- **Performance**: Minimal overhead - only generates random data when enabled
- **Thread Safety**: Fully thread-safe using Apache's request pool allocation

### Output Length

The final base64 string length will be approximately 4/3 of the configured byte length:
- 16 bytes → ~22 characters
- 24 bytes → ~32 characters  
- 32 bytes → ~44 characters

## Logging

Enable debug logging to monitor module operation:

```apache
LogLevel random:debug
```

The module logs:
- Configuration changes
- Random string generation
- Environment variable setting
- Request processing status

## Security Considerations

- Uses cryptographically secure random number generation
- No predictable patterns in generated strings
- Memory is automatically cleaned up via Apache's pool system
- Safe for use in security-sensitive applications

## Compatibility

- Apache 2.4+
- Requires APR (Apache Portable Runtime)
- Compatible with all major operating systems
- Works with both prefork and threaded MPMs