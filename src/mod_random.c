#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "ap_config.h"
#include "apr_strings.h"
#include "apr_lib.h"
#include "apr_base64.h"
#include "http_log.h"
#include <time.h>
#include <stdlib.h>

module AP_MODULE_DECLARE_DATA random_module;

typedef struct {
    int length;
    int enabled;
} random_config;

static void *create_random_config(apr_pool_t *pool, char *x)
{
    random_config *cfg = apr_pcalloc(pool, sizeof(random_config));
    cfg->length = 16;
    cfg->enabled = 0;
    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, NULL, "mod_random: config created with length=%d, enabled=%d", cfg->length, cfg->enabled);
    return cfg;
}

static const char *set_random_length(cmd_parms *cmd, void *cfg, const char *arg)
{
    random_config *config = (random_config *)cfg;
    int length = atoi(arg);
    
    if (length < 1 || length > 1024) {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, NULL, "mod_random: invalid length %d (must be 1-1024)", length);
        return "RandomLength must be between 1 and 1024";
    }
    
    config->length = length;
    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, NULL, "mod_random: length set to %d", length);
    return NULL;
}

static const char *set_random_enabled(cmd_parms *cmd, void *cfg, int flag)
{
    random_config *config = (random_config *)cfg;
    config->enabled = flag;
    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, NULL, "mod_random: enabled set to %s", flag ? "On" : "Off");
    return NULL;
}

static char *generate_random_base64(apr_pool_t *pool, int length)
{
    unsigned char *random_bytes;
    char *base64_string;
    int i;
    
    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, NULL, "mod_random: generating %d random bytes", length);
    
    random_bytes = apr_palloc(pool, length);
    apr_generate_random_bytes(random_bytes, length);
   
    
    int encoded_len = apr_base64_encode_len(length);
    base64_string = apr_palloc(pool, encoded_len);
    apr_base64_encode(base64_string, (char *)random_bytes, length);
    
    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, NULL, "mod_random: generated base64 string of length %d", (int)strlen(base64_string));
    
    return base64_string;
}

static int random_post_read_request(request_rec *r)
{
    random_config *cfg;
    char *random_string;
    
    if (r->main) {
        ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "mod_random: skipping subrequest");
        return DECLINED;
    }
    
    ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "mod_random: processing request for URI: %s", r->uri);
   
    cfg = ap_get_module_config(r->per_dir_config, &random_module);
    
    if (!cfg->enabled) {
        ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "mod_random: module disabled for this location");
        return DECLINED;
    }
    
    ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, "mod_random: generating random string with length %d", cfg->length);
    
    random_string = generate_random_base64(r->pool, cfg->length);
    
    apr_table_set(r->subprocess_env, "UNIQUE_STRING", random_string);
    
    ap_log_rerror(APLOG_MARK, APLOG_INFO, 0, r, "mod_random: generated random string and set UNIQUE_STRING environment variable");
    
    return DECLINED;
}

static void random_register_hooks(apr_pool_t *p)
{
    ap_log_error(APLOG_MARK, APLOG_INFO, 0, NULL, "mod_random: registering hooks");
    ap_hook_post_read_request(random_post_read_request, NULL, NULL, APR_HOOK_MIDDLE);
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
    NULL,
    NULL,
    NULL,
    random_directives,
    random_register_hooks
};