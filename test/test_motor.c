#ifdef UNIT_TEST

#include <unity.h>
#include <stdint.h>

// --- stubs ------------------------------------------------------------------
static int stub_rpwm[2], stub_lpwm[2], stub_enabled[2];
void hal_set_pwm(int m, int r, int l) { stub_rpwm[m]=r; stub_lpwm[m]=l; }
void hal_enable(int m, int en)         { stub_enabled[m]=en; }
void hal_init(void)                          {}

#include "../src/motor.c"

void test_sync_ok(void) {
    sync_state_t s = sync_update(70);
    TEST_ASSERT_EQUAL(SYNC_OK, s);
    TEST_ASSERT_EQUAL(70, stub_rpwm[MOTOR_A]);
    TEST_ASSERT_EQUAL(70, stub_rpwm[MOTOR_B]);

}

int main(void) {
    UNIT_BEGIN();

    RUN_TEST(test_sync_ok);

    return UNITY_END();
}

#endif
