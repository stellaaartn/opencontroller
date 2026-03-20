#ifdef UNIT_TEST

#include <unity.h>
#include <stdint.h>
#include "../src/preset.c"  // pulls in the in-memory stub impl

void setUp(void)    { preset_init(); }
void tearDown(void) {}

void test_preset_load_returns_none_when_unset(void)
{
    TEST_ASSERT_EQUAL(PRESET_NONE, preset_load(PRESET_SIT));
    TEST_ASSERT_EQUAL(PRESET_NONE, preset_load(PRESET_STAND));
}

void test_preset_save_and_load(void)
{
    preset_save(PRESET_SIT, 1234);
    TEST_ASSERT_EQUAL(1234, preset_load(PRESET_SIT));
}

void test_preset_stand_independent_of_sit(void)
{
    preset_save(PRESET_SIT,   100);
    preset_save(PRESET_STAND, 500);
    TEST_ASSERT_EQUAL(100, preset_load(PRESET_SIT));
    TEST_ASSERT_EQUAL(500, preset_load(PRESET_STAND));
}

void test_preset_clear(void)
{
    preset_save(PRESET_SIT, 999);
    preset_clear(PRESET_SIT);
    TEST_ASSERT_EQUAL(PRESET_NONE, preset_load(PRESET_SIT));
}

void test_preset_negative_ticks(void)
{
    preset_save(PRESET_SIT, -200);
    TEST_ASSERT_EQUAL(-200, preset_load(PRESET_SIT));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_preset_load_returns_none_when_unset);
    RUN_TEST(test_preset_save_and_load);
    RUN_TEST(test_preset_stand_independent_of_sit);
    RUN_TEST(test_preset_clear);
    RUN_TEST(test_preset_negative_ticks);
    return UNITY_END();
}

#endif // UNIT_TEST
