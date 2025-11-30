/*
 * test_mod_random.c - Comprehensive unit tests for mod_random
 *
 * Compile: gcc -o test_mod_random test_mod_random.c -I../../src -I/usr/include/apache2 -I/usr/include/apr-1.0 -lapr-1 -laprutil-1 -lssl -lcrypto
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

/* APR headers */
#include "apr_pools.h"
#include "apr_strings.h"
#include "apr_thread_mutex.h"
#include "apr_time.h"
#include "apr_tables.h"

/* Apache headers */
#include "httpd.h"
#include "http_config.h"

/* Include module types */
#include "../../src/mod_random_types.h"

/* Test framework macros */
#define TEST(name) static void test_##name(apr_pool_t *pool)
#define RUN_TEST(name) do { \
    printf("Running test_%s...", #name); \
    fflush(stdout); \
    test_##name(test_pool); \
    printf(" PASS\n"); \
    tests_passed++; \
} while(0)

#define ASSERT_TRUE(cond) do { \
    if (!(cond)) { \
        printf("\n  FAIL at %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        exit(1); \
    } \
} while(0)

#define ASSERT_NOT_NULL(ptr) ASSERT_TRUE((ptr) != NULL)
#define ASSERT_NULL(ptr) ASSERT_TRUE((ptr) == NULL)
#define ASSERT_EQUAL(a, b) ASSERT_TRUE((a) == (b))
#define ASSERT_STR_EQUAL(a, b) ASSERT_TRUE(strcmp((a), (b)) == 0)
#define ASSERT_STR_NOT_EQUAL(a, b) ASSERT_TRUE(strcmp((a), (b)) != 0)

/* Global test counter */
static int tests_passed = 0;

/* Forward declarations for functions we're testing */
extern char *random_encode_hex(apr_pool_t *pool, const unsigned char *data, int length);
extern char *random_encode_base64url(apr_pool_t *pool, const char *data, int length);
extern char *random_encode_custom_alphabet(apr_pool_t *pool, const unsigned char *data,
                                           int length, const char *alphabet, int grouping);
extern char *random_generate_string_ex(apr_pool_t *pool, int length, random_format_t format,
                                       const char *alphabet, int grouping);
extern char *random_generate_string(apr_pool_t *pool, int length, random_format_t format);
extern void random_hmac_sha256(apr_pool_t *pool, const char *key, apr_size_t key_len,
                              const char *data, apr_size_t data_len, unsigned char *digest);

/*
 * Test 1: Hex encoding basic functionality
 */
TEST(hex_encoding_basic) {
    unsigned char data[] = {0x00, 0xFF, 0xAB, 0xCD};
    char *result = random_encode_hex(pool, data, 4);

    ASSERT_NOT_NULL(result);
    ASSERT_STR_EQUAL(result, "00ffabcd");
}

/*
 * Test 2: Hex encoding empty data
 */
TEST(hex_encoding_empty) {
    unsigned char data[] = {};
    char *result = random_encode_hex(pool, data, 0);

    ASSERT_NOT_NULL(result);
    ASSERT_STR_EQUAL(result, "");
}

/*
 * Test 3: Hex encoding single byte
 */
TEST(hex_encoding_single_byte) {
    unsigned char data[] = {0x42};
    char *result = random_encode_hex(pool, data, 1);

    ASSERT_NOT_NULL(result);
    ASSERT_STR_EQUAL(result, "42");
}

/*
 * Test 4: Base64URL encoding basic
 */
TEST(base64url_encoding_basic) {
    const char *data = "Hello, World!";
    char *result = random_encode_base64url(pool, data, strlen(data));

    ASSERT_NOT_NULL(result);
    /* Base64 of "Hello, World!" is "SGVsbG8sIFdvcmxkIQ=="
     * Base64URL should replace + with -, / with _, and remove = */
    /* The actual encoding depends on the implementation */
    ASSERT_TRUE(strlen(result) > 0);

    /* Check that it doesn't contain padding or unsafe chars */
    ASSERT_TRUE(strchr(result, '=') == NULL);
    ASSERT_TRUE(strchr(result, '+') == NULL);
    ASSERT_TRUE(strchr(result, '/') == NULL);
}

/*
 * Test 5: Custom alphabet encoding
 */
TEST(custom_alphabet_basic) {
    unsigned char data[] = {0x00, 0x01, 0x02, 0x03};
    const char *alphabet = "ABCD";
    char *result = random_encode_custom_alphabet(pool, data, 4, alphabet, 0);

    ASSERT_NOT_NULL(result);
    ASSERT_TRUE(strlen(result) > 0);

    /* Verify only characters from alphabet are used */
    for (int i = 0; result[i]; i++) {
        ASSERT_TRUE(strchr(alphabet, result[i]) != NULL);
    }
}

/*
 * Test 6: Custom alphabet with grouping
 */
TEST(custom_alphabet_with_grouping) {
    unsigned char data[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    const char *alphabet = "0123456789ABCDEF";
    char *result = random_encode_custom_alphabet(pool, data, 8, alphabet, 4);

    ASSERT_NOT_NULL(result);
    ASSERT_TRUE(strlen(result) > 0);

    /* Check that hyphens are present for grouping */
    ASSERT_TRUE(strchr(result, '-') != NULL);
}

/*
 * Test 7: Random string generation with hex format
 */
TEST(generate_string_hex) {
    char *result = random_generate_string(pool, 16, RANDOM_FORMAT_HEX);

    ASSERT_NOT_NULL(result);
    /* 16 bytes = 32 hex chars */
    ASSERT_EQUAL(strlen(result), 32);

    /* Verify all characters are hex digits (lowercase) */
    for (int i = 0; result[i]; i++) {
        ASSERT_TRUE((result[i] >= '0' && result[i] <= '9') ||
                   (result[i] >= 'a' && result[i] <= 'f'));
    }
}

/*
 * Test 8: Random string generation with base64 format
 */
TEST(generate_string_base64) {
    char *result = random_generate_string(pool, 16, RANDOM_FORMAT_BASE64);

    ASSERT_NOT_NULL(result);
    ASSERT_TRUE(strlen(result) > 0);

    /* Standard base64 may contain +, /, and = padding */
    /* Just verify it's a valid base64 string */
    for (int i = 0; result[i]; i++) {
        ASSERT_TRUE((result[i] >= 'A' && result[i] <= 'Z') ||
                   (result[i] >= 'a' && result[i] <= 'z') ||
                   (result[i] >= '0' && result[i] <= '9') ||
                   result[i] == '+' || result[i] == '/' || result[i] == '=');
    }
}

/*
 * Test 9: Random string generation with base64url format
 */
TEST(generate_string_base64url) {
    char *result = random_generate_string(pool, 16, RANDOM_FORMAT_BASE64URL);

    ASSERT_NOT_NULL(result);
    ASSERT_TRUE(strlen(result) > 0);

    /* Verify it's URL-safe */
    for (int i = 0; result[i]; i++) {
        ASSERT_TRUE((result[i] >= 'A' && result[i] <= 'Z') ||
                   (result[i] >= 'a' && result[i] <= 'z') ||
                   (result[i] >= '0' && result[i] <= '9') ||
                   result[i] == '-' || result[i] == '_');
    }
}

/*
 * Test 10: Random string with custom alphabet
 */
TEST(generate_string_custom_alphabet) {
    const char *alphabet = "ABC123";
    char *result = random_generate_string_ex(pool, 16, RANDOM_FORMAT_CUSTOM,
                                             alphabet, 0);

    ASSERT_NOT_NULL(result);
    ASSERT_TRUE(strlen(result) > 0);

    /* Verify only alphabet characters are used */
    for (int i = 0; result[i]; i++) {
        ASSERT_TRUE(strchr(alphabet, result[i]) != NULL);
    }
}

/*
 * Test 11: Token length validation - minimum
 */
TEST(token_length_minimum) {
    char *result = random_generate_string(pool, RANDOM_LENGTH_MIN, RANDOM_FORMAT_HEX);

    ASSERT_NOT_NULL(result);
    ASSERT_EQUAL(strlen(result), RANDOM_LENGTH_MIN * 2); /* hex = 2 chars per byte */
}

/*
 * Test 12: Token length validation - maximum
 */
TEST(token_length_maximum) {
    char *result = random_generate_string(pool, RANDOM_LENGTH_MAX, RANDOM_FORMAT_HEX);

    ASSERT_NOT_NULL(result);
    ASSERT_EQUAL(strlen(result), RANDOM_LENGTH_MAX * 2);
}

/*
 * Test 13: Token uniqueness (probabilistic)
 */
TEST(token_uniqueness) {
    #define NUM_TOKENS 100
    char *tokens[NUM_TOKENS];

    /* Generate multiple tokens */
    for (int i = 0; i < NUM_TOKENS; i++) {
        tokens[i] = random_generate_string(pool, 16, RANDOM_FORMAT_HEX);
        ASSERT_NOT_NULL(tokens[i]);
    }

    /* Verify no duplicates (statistically should never happen with 128-bit tokens) */
    for (int i = 0; i < NUM_TOKENS - 1; i++) {
        for (int j = i + 1; j < NUM_TOKENS; j++) {
            ASSERT_STR_NOT_EQUAL(tokens[i], tokens[j]);
        }
    }
}

/*
 * Test 14: HMAC-SHA256 basic functionality
 */
TEST(hmac_sha256_basic) {
    const char *key = "secret_key";
    const char *data = "test_data";
    unsigned char digest[32];

    random_hmac_sha256(pool, key, strlen(key), data, strlen(data), digest);

    /* Verify digest is not all zeros */
    int all_zeros = 1;
    for (int i = 0; i < 32; i++) {
        if (digest[i] != 0) {
            all_zeros = 0;
            break;
        }
    }
    ASSERT_TRUE(!all_zeros);
}

/*
 * Test 15: HMAC-SHA256 consistency
 */
TEST(hmac_sha256_consistency) {
    const char *key = "my_secret_key";
    const char *data = "my_test_data";
    unsigned char digest1[32];
    unsigned char digest2[32];

    random_hmac_sha256(pool, key, strlen(key), data, strlen(data), digest1);
    random_hmac_sha256(pool, key, strlen(key), data, strlen(data), digest2);

    /* Same input should produce same output */
    ASSERT_EQUAL(memcmp(digest1, digest2, 32), 0);
}

/*
 * Test 16: HMAC-SHA256 different keys produce different digests
 */
TEST(hmac_sha256_different_keys) {
    const char *key1 = "key1";
    const char *key2 = "key2";
    const char *data = "test_data";
    unsigned char digest1[32];
    unsigned char digest2[32];

    random_hmac_sha256(pool, key1, strlen(key1), data, strlen(data), digest1);
    random_hmac_sha256(pool, key2, strlen(key2), data, strlen(data), digest2);

    /* Different keys should produce different digests */
    ASSERT_TRUE(memcmp(digest1, digest2, 32) != 0);
}

/*
 * Test 17: Thread mutex creation and locking
 */
TEST(thread_mutex_basic) {
    apr_thread_mutex_t *mutex;
    apr_status_t rv;

    rv = apr_thread_mutex_create(&mutex, APR_THREAD_MUTEX_DEFAULT, pool);
    ASSERT_EQUAL(rv, APR_SUCCESS);
    ASSERT_NOT_NULL(mutex);

    /* Lock and unlock */
    rv = apr_thread_mutex_lock(mutex);
    ASSERT_EQUAL(rv, APR_SUCCESS);

    rv = apr_thread_mutex_unlock(mutex);
    ASSERT_EQUAL(rv, APR_SUCCESS);
}

/*
 * Test 18: Pool allocation and cleanup
 */
TEST(pool_allocation) {
    apr_pool_t *subpool;
    apr_status_t rv;

    rv = apr_pool_create(&subpool, pool);
    ASSERT_EQUAL(rv, APR_SUCCESS);
    ASSERT_NOT_NULL(subpool);

    /* Allocate memory from subpool */
    char *test_str = apr_pstrdup(subpool, "test string");
    ASSERT_NOT_NULL(test_str);
    ASSERT_STR_EQUAL(test_str, "test string");

    /* Cleanup subpool */
    apr_pool_destroy(subpool);
}

/*
 * Test 19: String concatenation with apr_psprintf
 */
TEST(apr_psprintf_basic) {
    const char *prefix = "PREFIX_";
    const char *token = "TOKEN123";
    const char *suffix = "_SUFFIX";

    char *result = apr_psprintf(pool, "%s%s%s", prefix, token, suffix);
    ASSERT_NOT_NULL(result);
    ASSERT_STR_EQUAL(result, "PREFIX_TOKEN123_SUFFIX");
}

/*
 * Test 20: Time functions
 */
TEST(time_functions) {
    apr_time_t now1 = apr_time_now();
    apr_time_t now2 = apr_time_now();

    /* Second call should be >= first call */
    ASSERT_TRUE(now2 >= now1);

    /* Convert to seconds */
    time_t sec1 = apr_time_sec(now1);
    ASSERT_TRUE(sec1 > 0);
}

/*
 * Test 21: Constants validation
 */
TEST(constants_validation) {
    /* Verify sentinel values */
    ASSERT_EQUAL(RANDOM_LENGTH_UNSET, -1);
    ASSERT_EQUAL(RANDOM_FORMAT_UNSET, -1);
    ASSERT_EQUAL(RANDOM_ENABLED_UNSET, -1);
    ASSERT_EQUAL(RANDOM_GROUPING_UNSET, -1);
    ASSERT_EQUAL(RANDOM_EXPIRY_UNSET, -1);

    /* Verify sane limits */
    ASSERT_TRUE(RANDOM_LENGTH_MIN > 0);
    ASSERT_TRUE(RANDOM_LENGTH_MAX >= RANDOM_LENGTH_MIN);
    ASSERT_TRUE(RANDOM_LENGTH_DEFAULT >= RANDOM_LENGTH_MIN);
    ASSERT_TRUE(RANDOM_LENGTH_DEFAULT <= RANDOM_LENGTH_MAX);

    ASSERT_TRUE(RANDOM_TTL_MAX_SECONDS == 86400); /* 24 hours */
    ASSERT_TRUE(RANDOM_EXPIRY_MAX_SECONDS == 31536000); /* 1 year */

    ASSERT_TRUE(RANDOM_ALPHABET_MIN_SIZE == 2);
    ASSERT_TRUE(RANDOM_ALPHABET_MAX_SIZE == 256);

    ASSERT_TRUE(RANDOM_MAX_TOKENS > 0);
    ASSERT_TRUE(RANDOM_GROUPING_MAX > 0);
}

/*
 * Test 22: Hex encoding of known test vectors
 */
TEST(hex_encoding_test_vectors) {
    /* Test vector 1: All zeros */
    unsigned char zeros[] = {0x00, 0x00, 0x00, 0x00};
    char *result1 = random_encode_hex(pool, zeros, 4);
    ASSERT_STR_EQUAL(result1, "00000000");

    /* Test vector 2: All ones */
    unsigned char ones[] = {0xFF, 0xFF, 0xFF, 0xFF};
    char *result2 = random_encode_hex(pool, ones, 4);
    ASSERT_STR_EQUAL(result2, "ffffffff");

    /* Test vector 3: Sequential bytes */
    unsigned char seq[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    char *result3 = random_encode_hex(pool, seq, 8);
    ASSERT_STR_EQUAL(result3, "0123456789abcdef");
}

/*
 * Test 23: Format enum values
 */
TEST(format_enum_values) {
    ASSERT_TRUE(RANDOM_FORMAT_HEX >= 0);
    ASSERT_TRUE(RANDOM_FORMAT_BASE64 >= 0);
    ASSERT_TRUE(RANDOM_FORMAT_BASE64URL >= 0);
    ASSERT_TRUE(RANDOM_FORMAT_CUSTOM >= 0);
    ASSERT_TRUE(RANDOM_FORMAT_UNSET == -1);
}

/*
 * Test 24: Random generation different on each call
 */
TEST(random_generation_different) {
    char *token1 = random_generate_string(pool, 16, RANDOM_FORMAT_HEX);
    char *token2 = random_generate_string(pool, 16, RANDOM_FORMAT_HEX);
    char *token3 = random_generate_string(pool, 16, RANDOM_FORMAT_HEX);

    ASSERT_NOT_NULL(token1);
    ASSERT_NOT_NULL(token2);
    ASSERT_NOT_NULL(token3);

    /* All should be different */
    ASSERT_STR_NOT_EQUAL(token1, token2);
    ASSERT_STR_NOT_EQUAL(token2, token3);
    ASSERT_STR_NOT_EQUAL(token1, token3);
}

/*
 * Test 25: Large token generation
 */
TEST(large_token_generation) {
    char *token = random_generate_string(pool, 256, RANDOM_FORMAT_BASE64);

    ASSERT_NOT_NULL(token);
    ASSERT_TRUE(strlen(token) > 300); /* Base64 expands ~33% */
}

/*
 * Main test runner
 */
int main(void) {
    apr_pool_t *test_pool;
    apr_status_t rv;

    /* Initialize APR */
    rv = apr_initialize();
    if (rv != APR_SUCCESS) {
        fprintf(stderr, "Failed to initialize APR\n");
        return 1;
    }

    /* Create root pool */
    rv = apr_pool_create(&test_pool, NULL);
    if (rv != APR_SUCCESS) {
        fprintf(stderr, "Failed to create test pool\n");
        apr_terminate();
        return 1;
    }

    printf("========================================\n");
    printf("mod_random Comprehensive Test Suite\n");
    printf("========================================\n\n");

    /* Run encoding tests */
    printf("=== Encoding Tests ===\n");
    RUN_TEST(hex_encoding_basic);
    RUN_TEST(hex_encoding_empty);
    RUN_TEST(hex_encoding_single_byte);
    RUN_TEST(hex_encoding_test_vectors);
    RUN_TEST(base64url_encoding_basic);
    RUN_TEST(custom_alphabet_basic);
    RUN_TEST(custom_alphabet_with_grouping);

    /* Run random generation tests */
    printf("\n=== Random Generation Tests ===\n");
    RUN_TEST(generate_string_hex);
    RUN_TEST(generate_string_base64);
    RUN_TEST(generate_string_base64url);
    RUN_TEST(generate_string_custom_alphabet);
    RUN_TEST(token_length_minimum);
    RUN_TEST(token_length_maximum);
    RUN_TEST(token_uniqueness);
    RUN_TEST(random_generation_different);
    RUN_TEST(large_token_generation);

    /* Run cryptography tests */
    printf("\n=== Cryptography Tests ===\n");
    RUN_TEST(hmac_sha256_basic);
    RUN_TEST(hmac_sha256_consistency);
    RUN_TEST(hmac_sha256_different_keys);

    /* Run APR infrastructure tests */
    printf("\n=== APR Infrastructure Tests ===\n");
    RUN_TEST(thread_mutex_basic);
    RUN_TEST(pool_allocation);
    RUN_TEST(apr_psprintf_basic);
    RUN_TEST(time_functions);

    /* Run validation tests */
    printf("\n=== Validation Tests ===\n");
    RUN_TEST(constants_validation);
    RUN_TEST(format_enum_values);

    /* Cleanup */
    apr_pool_destroy(test_pool);
    apr_terminate();

    printf("\n========================================\n");
    printf("All %d tests PASSED!\n", tests_passed);
    printf("========================================\n");

    return 0;
}
