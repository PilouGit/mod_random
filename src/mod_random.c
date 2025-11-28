/*
 * mod_random - Apache module for generating cryptographically secure random tokens
 *
 * Purpose:
 *   Generates a cryptographically secure random base64 string and injects
 *   it into the RANDOM_STRING environment variable for each HTTP request.
 *
 * Use cases:
 *   - CSRF tokens
 *   - Request IDs for logging/tracing (non-unique, probabilistic)
 *   - Nonces for security headers (CSP, etc.)
 *   - One-time tokens
 *
 * Security notes:
 *   - Uses CSPRNG (apr_generate_random_bytes)
 *   - NOT guaranteed unique - collision probability exists but is negligible
 *   - Default 16 bytes = 128 bits of entropy
 *   - For guaranteed unique IDs, use mod_unique_id instead
 *
 * Configuration directives:
 *   RandomEnabled On|Off     - Enable/disable token generation
 *   RandomLength <1-1024>    - Bytes of random data (default: 16)
 *
 * Example:
 *   <Directory /var/www/api>
 *       RandomEnabled On
 *       RandomLength 32
 *   </Directory>
 *
 * The generated token is available in scripts via:
 *   $_SERVER['RANDOM_STRING'] (PHP)
 *   os.environ['RANDOM_STRING'] (Python)
 *   process.env.RANDOM_STRING (Node.js)
 */

#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "ap_config.h"
#include "apr_base64.h"
#include "http_log.h"

/* Configuration constants */
#define RANDOM_LENGTH_DEFAULT  16    /* 128 bits of entropy */
#define RANDOM_LENGTH_MIN      1     /* Minimum allowed length */
#define RANDOM_LENGTH_MAX      1024  /* Maximum to prevent DoS */
#define RANDOM_LENGTH_UNSET    -1    /* Sentinel: not configured */
#define RANDOM_ENABLED_UNSET   -1    /* Sentinel: not configured */

module AP_MODULE_DECLARE_DATA random_module;

typedef struct {
    int length;
    int enabled;
} random_config;

static void *create_random_config(apr_pool_t *pool, char *dir)
{
    random_config *cfg = apr_pcalloc(pool, sizeof(random_config));
    cfg->length = RANDOM_LENGTH_UNSET;
    cfg->enabled = RANDOM_ENABLED_UNSET;
    return cfg;
}

static void *merge_random_config(apr_pool_t *pool, void *base, void *override)
{
    random_config *parent = (random_config *)base;
    random_config *child = (random_config *)override;
    random_config *merged = apr_pcalloc(pool, sizeof(random_config));

    /* Child settings take precedence if explicitly set, otherwise inherit from parent */
    merged->enabled = (child->enabled != RANDOM_ENABLED_UNSET) ? child->enabled : parent->enabled;
    merged->length = (child->length != RANDOM_LENGTH_UNSET) ? child->length : parent->length;

    return merged;
}

static const char *set_random_length(cmd_parms *cmd, void *cfg, const char *arg)
{
    random_config *config = (random_config *)cfg;
    char *endptr;
    long length = strtol(arg, &endptr, 10);

    if (*endptr != '\0' || length < RANDOM_LENGTH_MIN || length > RANDOM_LENGTH_MAX) {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, NULL,
                     "mod_random: invalid length %ld (must be %d-%d)",
                     length, RANDOM_LENGTH_MIN, RANDOM_LENGTH_MAX);
        return "RandomLength must be between "
               APR_STRINGIFY(RANDOM_LENGTH_MIN) " and "
               APR_STRINGIFY(RANDOM_LENGTH_MAX);
    }

    config->length = (int)length;
    return NULL;
}

static const char *set_random_enabled(cmd_parms *cmd, void *cfg, int flag)
{
    random_config *config = (random_config *)cfg;
    config->enabled = flag;
    return NULL;
}

static char *generate_random_base64(apr_pool_t *pool, int length)
{
    char *random_bytes;
    char *base64_string;
    int encoded_len;

    random_bytes = apr_palloc(pool, length);
    apr_generate_random_bytes((unsigned char *)random_bytes, length);

    encoded_len = apr_base64_encode_len(length);
    base64_string = apr_palloc(pool, encoded_len);
    apr_base64_encode(base64_string, random_bytes, length);

    return base64_string;
}

static int random_fixups(request_rec *r)
{
    random_config *cfg;
    char *random_string;
    int final_enabled;
    int final_length;

    if (r->main) {
        return DECLINED;
    }

    cfg = ap_get_module_config(r->per_dir_config, &random_module);
    if (!cfg) {
        return DECLINED;
    }

    /* Apply defaults for unset configuration values */
    final_enabled = (cfg->enabled != RANDOM_ENABLED_UNSET) ? cfg->enabled : 0;
    final_length = (cfg->length != RANDOM_LENGTH_UNSET) ? cfg->length : RANDOM_LENGTH_DEFAULT;

    if (!final_enabled) {
        return DECLINED;
    }

    random_string = generate_random_base64(r->pool, final_length);
    apr_table_set(r->subprocess_env, "RANDOM_STRING", random_string);

    return DECLINED;
}

static void random_register_hooks(apr_pool_t *p)
{
    ap_hook_fixups(random_fixups, NULL, NULL, APR_HOOK_MIDDLE);
}

static const command_rec random_directives[] = {
    AP_INIT_TAKE1("RandomLength", set_random_length, NULL, OR_ALL,
                  "Set the length of random bytes to generate (default: 16)"),
    AP_INIT_FLAG("RandomEnabled", set_random_enabled, NULL, OR_ALL,
                 "Enable or disable random base64 generation"),
    {NULL}
};

module AP_MODULE_DECLARE_DATA random_module = {
    STANDARD20_MODULE_STUFF,
    create_random_config,
    merge_random_config,
    NULL,
    NULL,
    random_directives,
    random_register_hooks
};