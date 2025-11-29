/*
 * mod_random_token.c - Token generation with caching and metadata
 */

#include "mod_random.h"
#include "apr_time.h"
#include "apr_thread_mutex.h"
#include "apr_strings.h"

/* Helper function to generate a single token based on spec and defaults */
char *random_generate_token_from_spec(request_rec *r, random_config *cfg,
                                      random_token_spec *spec,
                                      int default_length, random_format_t default_format,
                                      int default_timestamp,
                                      const char *default_prefix, const char *default_suffix,
                                      int default_ttl,
                                      char **cached_token, apr_time_t *cache_time,
                                      apr_thread_mutex_t *cache_mutex, apr_pool_t *cache_pool)
{
    char *random_string, *final_token;
    int use_cached = 0;
    apr_time_t now;
    int final_length, final_timestamp, final_ttl;
    random_format_t final_format;
    const char *final_prefix, *final_suffix;

    /* Apply spec values or use defaults */
    final_length = (spec && spec->length != RANDOM_LENGTH_UNSET) ? spec->length : default_length;
    final_format = (spec && spec->format != RANDOM_FORMAT_UNSET) ? spec->format : default_format;
    final_timestamp = (spec && spec->include_timestamp != RANDOM_ENABLED_UNSET) ? spec->include_timestamp : default_timestamp;
    final_prefix = (spec && spec->prefix) ? spec->prefix : default_prefix;
    final_suffix = (spec && spec->suffix) ? spec->suffix : default_suffix;
    final_ttl = (spec && spec->ttl_seconds != RANDOM_LENGTH_UNSET) ? spec->ttl_seconds : default_ttl;

    /* Check TTL cache with thread-safe mutex protection */
    if (final_ttl > 0 && cache_mutex && cached_token && cache_time) {
        apr_thread_mutex_lock(cache_mutex);
        now = apr_time_now();
        if (*cached_token) {
            apr_time_t elapsed = apr_time_sec(now - *cache_time);
            if (elapsed < final_ttl) {
                /* Use cached token - duplicate to request pool */
                final_token = apr_pstrdup(r->pool, *cached_token);
                use_cached = 1;
            }
        }
        apr_thread_mutex_unlock(cache_mutex);
    }

    /* Generate new token if not using cache */
    if (!use_cached) {
        /* Generate random string with specified format */
        random_string = random_generate_string_ex(r->pool, final_length, final_format,
                                                  cfg->custom_alphabet, cfg->alphabet_grouping);

        /* Add timestamp if requested */
        if (final_timestamp) {
            apr_time_t now = apr_time_now();
            time_t timestamp = apr_time_sec(now);
            random_string = apr_psprintf(r->pool, "%ld-%s", (long)timestamp, random_string);
        }

        /* Encode metadata with expiry and signature if requested */
        if (cfg->encode_metadata && cfg->expiry_seconds > 0) {
            random_string = random_encode_with_metadata(r->pool, random_string,
                                                       cfg->expiry_seconds, cfg->signing_key);
        }

        /* Add prefix/suffix if configured */
        final_token = random_string;
        if (final_prefix && final_suffix) {
            final_token = apr_psprintf(r->pool, "%s%s%s", final_prefix, random_string, final_suffix);
        } else if (final_prefix) {
            final_token = apr_psprintf(r->pool, "%s%s", final_prefix, random_string);
        } else if (final_suffix) {
            final_token = apr_psprintf(r->pool, "%s%s", random_string, final_suffix);
        }

        /* Update cache if TTL is enabled - use provided pool and mutex */
        if (final_ttl > 0 && cache_mutex && cached_token && cache_time) {
            apr_thread_mutex_lock(cache_mutex);
            *cached_token = apr_pstrdup(cache_pool, final_token);
            *cache_time = apr_time_now();
            apr_thread_mutex_unlock(cache_mutex);
        }
    }

    return final_token;
}
