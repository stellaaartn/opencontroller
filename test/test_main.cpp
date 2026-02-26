#ifdef UNIT_TEST

#include <unity.h>
#include <stdint.h>

// --- stub the motor logic inline (no hardware needed) ----------------------

#define PWM_PERIOD 50   // simulate 50 ticks = 100%

static int stub_cmp_a = 0;
static int stub_cmp_b = 0;

// Copy of motor_set() with hardware calls replaced by stub writes
static void motor_set(int duty)
{
    if (duty > 0) {
        stub_cmp_a = PWM_PERIOD * duty / 100;
        stub_cmp_b = 0;
    } else if (duty < 0) {
        stub_cmp_a = 0;
        stub_cmp_b = PWM_PERIOD * (-duty) / 100;
    } else {
        stub_cmp_a = 0;
        stub_cmp_b = 0;
    }
}

// --- tests -----------------------------------------------------------------

void test_motor_stop(void)
{
    motor_set(0);
    TEST_ASSERT_EQUAL(0, stub_cmp_a);
    TEST_ASSERT_EQUAL(0, stub_cmp_b);
}

void test_motor_up(void)
{
    motor_set(100);
    TEST_ASSERT_EQUAL(PWM_PERIOD, stub_cmp_a);  // full forward
    TEST_ASSERT_EQUAL(0, stub_cmp_b);
}

void test_motor_down(void)
{
    motor_set(-100);
    TEST_ASSERT_EQUAL(0, stub_cmp_a);
    TEST_ASSERT_EQUAL(PWM_PERIOD, stub_cmp_b);  // full reverse
}

void test_motor_half_up(void)
{
    motor_set(50);
    TEST_ASSERT_EQUAL(PWM_PERIOD / 2, stub_cmp_a);
    TEST_ASSERT_EQUAL(0, stub_cmp_b);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_motor_stop);
    RUN_TEST(test_motor_up);
    RUN_TEST(test_motor_down);
    RUN_TEST(test_motor_half_up);
    return UNITY_END();
}

#endif // UNIT_TEST
