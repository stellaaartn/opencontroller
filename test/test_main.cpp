#ifdef UNIT_TEST

#include <unity.h>
#include <string.h>

// --- stub HAL (replaces hal_esp32.c on native) ------------------------------

static int stub_a = 0;
static int stub_b = 0;
static int hal_init_called = 0;

void hal_motor_init(void)   { hal_init_called = 1; }
void hal_motor_set_pwm(int a, int b) { stub_a = a; stub_b = b; }

// --- include the REAL motor logic -------------------------------------------

#include "../src/motor.c"

// --- tests ------------------------------------------------------------------

void setUp(void)    { stub_a = 0; stub_b = 0; hal_init_called = 0; }
void tearDown(void) {}

void test_motor_init_calls_hal(void)
{
    motor_init();
    TEST_ASSERT_EQUAL(1, hal_init_called);
}

void test_motor_stop(void)
{
    motor_set(0);
    TEST_ASSERT_EQUAL(0, stub_a);
    TEST_ASSERT_EQUAL(0, stub_b);
}

void test_motor_full_up(void)
{
    motor_set(100);
    TEST_ASSERT_EQUAL(100, stub_a);
    TEST_ASSERT_EQUAL(0,   stub_b);
}

void test_motor_full_down(void)
{
    motor_set(-100);
    TEST_ASSERT_EQUAL(0,   stub_a);
    TEST_ASSERT_EQUAL(100, stub_b);
}

void test_motor_half_up(void)
{
    motor_set(50);
    TEST_ASSERT_EQUAL(50, stub_a);
    TEST_ASSERT_EQUAL(0,  stub_b);
}

void test_motor_clamps_over_100(void)
{
    motor_set(150);
    TEST_ASSERT_EQUAL(100, stub_a);
    TEST_ASSERT_EQUAL(0,   stub_b);
}

void test_motor_clamps_under_minus_100(void)
{
    motor_set(-150);
    TEST_ASSERT_EQUAL(0,   stub_a);
    TEST_ASSERT_EQUAL(100, stub_b);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_motor_init_calls_hal);
    RUN_TEST(test_motor_stop);
    RUN_TEST(test_motor_full_up);
    RUN_TEST(test_motor_full_down);
    RUN_TEST(test_motor_half_up);
    RUN_TEST(test_motor_clamps_over_100);
    RUN_TEST(test_motor_clamps_under_minus_100);
    return UNITY_END();
}

#endif // UNIT_TEST
