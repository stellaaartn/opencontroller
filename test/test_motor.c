#ifdef UNIT_TEST

#include <unity.h>
#include <stdlib.h>

// --- HAL stub ---------------------------------------------------------------
static int stub_rpwm[2], stub_lpwm[2], stub_enabled[2], stub_hal_init;

void hal_motor_set_pwm(int motor, int r, int l) { stub_rpwm[motor]=r; stub_lpwm[motor]=l; }
void hal_motor_enable(int motor, int en)         { stub_enabled[motor]=en; }
void hal_init(void)                              { stub_hal_init=1; }

#include "../src/motor.c"

void setUp(void)
{
    stub_rpwm[0]=stub_rpwm[1]=stub_lpwm[0]=stub_lpwm[1]=0;
    stub_enabled[0]=stub_enabled[1]=0;
    motor_init();
}
void tearDown(void) {}

void test_motor_init_enables_both(void)
{
    TEST_ASSERT_EQUAL(1, stub_enabled[0]);
    TEST_ASSERT_EQUAL(1, stub_enabled[1]);
}

void test_motor_forward(void)
{
    motor_set(MOTOR_A, 80);
    TEST_ASSERT_EQUAL(80, stub_rpwm[0]);
    TEST_ASSERT_EQUAL(0,  stub_lpwm[0]);
}

void test_motor_reverse(void)
{
    motor_set(MOTOR_A, -80);
    TEST_ASSERT_EQUAL(0,  stub_rpwm[0]);
    TEST_ASSERT_EQUAL(80, stub_lpwm[0]);
}

void test_motor_stop_immediate(void)
{
    motor_set(MOTOR_A, 70);
    motor_stop_immediate(MOTOR_A);
    TEST_ASSERT_EQUAL(0, stub_rpwm[0]);
    TEST_ASSERT_EQUAL(0, stub_lpwm[0]);
    TEST_ASSERT_EQUAL(0, motor_get_duty(MOTOR_A));
}

void test_motor_clamps(void)
{
    motor_set(MOTOR_A, 150);
    TEST_ASSERT_EQUAL(100, stub_rpwm[0]);
    motor_set(MOTOR_A, -150);
    TEST_ASSERT_EQUAL(100, stub_lpwm[0]);
}

void test_soft_stop_ramps_down(void)
{
    motor_set(MOTOR_A, 50);
    int done = 0;
    for (int i = 0; i < 20 && !done; i++) {
        done = motor_soft_stop_tick(MOTOR_A);
    }
    TEST_ASSERT_EQUAL(1, done);
    TEST_ASSERT_EQUAL(0, motor_get_duty(MOTOR_A));
}

void test_soft_stop_reverse(void)
{
    motor_set(MOTOR_A, -50);
    int done = 0;
    for (int i = 0; i < 20 && !done; i++) {
        done = motor_soft_stop_tick(MOTOR_A);
    }
    TEST_ASSERT_EQUAL(1, done);
    TEST_ASSERT_EQUAL(0, motor_get_duty(MOTOR_A));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_motor_init_enables_both);
    RUN_TEST(test_motor_forward);
    RUN_TEST(test_motor_reverse);
    RUN_TEST(test_motor_stop_immediate);
    RUN_TEST(test_motor_clamps);
    RUN_TEST(test_soft_stop_ramps_down);
    RUN_TEST(test_soft_stop_reverse);
    return UNITY_END();
}

#endif // UNIT_TEST
