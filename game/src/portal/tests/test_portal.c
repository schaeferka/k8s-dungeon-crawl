#include <stddef.h>   // For size_t
#include <setjmp.h>   // For jmp_buf
#include <stdarg.h>   // For va_list
#include <cmocka.h>   // For CMocka framework
#include <curl/curl.h>
#include "portal.h"
#include "portal_urls.h"
#include "Rogue.h"


// Mock curl_easy_perform to test without making actual network requests
static CURLcode mock_curl_easy_perform(CURL *curl) {
    (void)curl; // Unused parameter
    return CURLE_OK; // Simulate success
}

// Test sending monster death data to the portal
static void test_send_monster_death_to_portal(void **state) {
    (void)state;

    // Mock CURL functions
    will_return(cmocka_mock, CURLE_OK); // Simulate successful HTTP request

    // Test sending monster death data
    expect_string(cmocka_mock, get_monster_death_url, "http://example.com/monster-death");
    send_monster_death_to_portal("{\"monster\": \"dragon\", \"id\": \"1\"}");
}

// Test sending data to portal with NULL data
static void test_send_data_to_portal_null_data(void **state) {
    (void)state;

    // Simulate successful HTTP request
    will_return(cmocka_mock, CURLE_OK);

    // Test sending data with NULL as the data (e.g., reset request)
    send_data_to_portal("http://example.com/reset", NULL);
}

// Test sending data to portal when CURL initialization fails
static void test_send_data_to_portal_curl_init_fail(void **state) {
    (void)state;

    // Simulate a CURL initialization failure
    will_return(cmocka_mock, NULL);

    // Check that the function properly handles initialization failure
    send_data_to_portal("http://example.com/reset", "{\"status\": \"fail\"}");
}

// Test sending data to portal when curl_easy_perform fails
static void test_send_data_to_portal_curl_perform_fail(void **state) {
    (void)state;

    // Simulate a failure in curl_easy_perform (e.g., network error)
    will_return(cmocka_mock, CURLE_COULDNT_CONNECT); // Simulate connection error

    // Test sending data, expecting an error
    send_data_to_portal("http://example.com/monster-death", "{\"monster\": \"dragon\", \"status\": \"dead\"}");
}

// Test sending monster update data to the portal
static void test_send_monsters_to_portal(void **state) {
    (void)state;

    // Mock the URL and simulate success
    will_return(cmocka_mock, CURLE_OK);

    // Expect the URL for monster updates
    expect_string(cmocka_mock, get_monster_update_url, "http://example.com/monster-update");

    // Test sending monster data
    send_monsters_to_portal("{\"monster\": \"orc\", \"status\": \"alive\"}");
}

// Test sending player death data to the portal
static void test_send_player_death_to_portal(void **state) {
    (void)state;

    // Mock the URL and simulate success
    will_return(cmocka_mock, CURLE_OK);

    // Expect the URL for player death
    expect_string(cmocka_mock, get_player_death_url, "http://example.com/player-death");

    // Test sending player death data
    send_player_death_to_portal("{\"player\": \"hero\", \"status\": \"dead\"}");
}

// Test sending player reset data to the portal
static void test_send_player_reset_to_portal(void **state) {
    (void)state;

    // Mock the URL and simulate success
    will_return(cmocka_mock, CURLE_OK);

    // Test sending player reset data
    send_player_reset_to_portal();
}

// Test sending monster reset data to the portal
static void test_send_monster_reset_to_portal(void **state) {
    (void)state;

    // Mock the URL and simulate success
    will_return(cmocka_mock, CURLE_OK);

    // Test sending monster reset data
    send_monster_reset_to_portal();
}

// Test sending gamestate data to the portal
static void test_send_gamestate_to_portal(void **state) {
    (void)state;

    // Mock the URL and simulate success
    will_return(cmocka_mock, CURLE_OK);

    // Test sending gamestate data
    send_gamestate_to_portal("{\"state\": \"active\"}");
}

// Test sending gamestats data to the portal
static void test_send_gamestats_to_portal(void **state) {
    (void)state;

    // Mock the URL and simulate success
    will_return(cmocka_mock, CURLE_OK);

    // Test sending gamestats data
    send_gamestats_to_portal("{\"turns\": 100, \"gold\": 50}");
}

// Test invalid URL format
static void test_send_data_to_portal_invalid_url(void **state) {
    (void)state;

    // Simulate successful HTTP request
    will_return(cmocka_mock, CURLE_OK);

    // Test with an invalid URL
    send_data_to_portal("invalid-url", "{\"monster\": \"dragon\"}");
}

// Test multiple requests being sent
static void test_send_multiple_requests(void **state) {
    (void)state;

    // Mock the successful curl request
    will_return(cmocka_mock, CURLE_OK);

    // Simulate sending different types of data
    send_monster_reset_to_portal();
    send_player_reset_to_portal();
    send_gamestate_to_portal("{\"state\": \"active\"}");
}

// Test the logging/output handling (redirect stdout)
static void test_send_data_to_portal_logging(void **state) {
    (void)state;

    // Redirect stdout to a buffer for testing printed output
    FILE *output = fopen("output.log", "w");
    if (!output) {
        fail_msg("Failed to open output file");
    }
    stdout = output;

    // Simulate a successful request
    will_return(cmocka_mock, CURLE_OK);

    // Test sending data and check for printed success message
    send_data_to_portal("http://example.com/monster-death", "{\"monster\": \"dragon\", \"status\": \"dead\"}");

    // Check if the correct success message was printed
    fclose(output);
    // You can add checks on the content of the output here
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_send_monster_death_to_portal),
        cmocka_unit_test(test_send_data_to_portal_null_data),
        cmocka_unit_test(test_send_data_to_portal_curl_init_fail),
        cmocka_unit_test(test_send_data_to_portal_curl_perform_fail),
        cmocka_unit_test(test_send_monsters_to_portal),
        cmocka_unit_test(test_send_player_death_to_portal),
        cmocka_unit_test(test_send_player_reset_to_portal),
        cmocka_unit_test(test_send_monster_reset_to_portal),
        cmocka_unit_test(test_send_gamestate_to_portal),
        cmocka_unit_test(test_send_gamestats_to_portal),
        cmocka_unit_test(test_send_data_to_portal_invalid_url),
        cmocka_unit_test(test_send_multiple_requests),
        cmocka_unit_test(test_send_data_to_portal_logging),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
