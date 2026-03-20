#ifdef UNIT_TEST

#include <unity.h>
#include <stdint.h>

// --- stubs ------------------------------------------------------------------
static int stub_rpwm[2], stub_lpwm[2], stub_enabled[2];
void hal_motor_set_pwm(int m, int r, int l) { stub_rpwm[m]=r; stub_lpwm[m]=l; }
void hal_motor_enable(int m, int en)         { stub_enabled[m]=en; }
void hal_init(void)                          {}

#include "../src/motor.c"
#include "../src/hall.c"
#include "../src/sync.c"

static void reset_all(void)
{
    stub_rpwm[0]=stub_rpwm[1]=stub_lpwm[0]=stub_lpwm[1]=0;
    motor_init();
    hall_init();
    sync_init();
    sync_reset();
}

void setUp(void)    { reset_all(); }
void tearDown(void) {}

void test_sync_ok_when_equal(void)
{
    // Both motors at same tick — should just set full duty
    sync_state_t s = sync_update(70);
    TEST_ASSERT_EQUAL(SYNC_OK, s);
    TEST_ASSERT_EQUAL(70, stub_rpwm[MOTOR_A]);
    TEST_ASSERT_EQUAL(70, stub_rpwm[MOTOR_B]);
}

void test_sync_throttles_faster_motor(void)
{
    // Make A ahead by 2 ticks
    hall_process_tick(MOTOR_A, 1);
    hall_process_tick(MOTOR_A, 3);
    // B gets 0 ticks
    sync_state_t s = sync_update(70);
    TEST_ASSERT_EQUAL(SYNC_OK, s);
    // A should be throttled
    TEST_ASSERT_LESS_THAN(stub_rpwm[MOTOR_B], stub_rpwm[MOTOR_A] + 1);
}

void test_sync_triggers_correction_on_large_delta(void)
{
    // Push A ahead by more than threshold (5)
    for (int i = 0; i < 6; i++) {
        int fwd[] = {1,3,2,6,4,5};
        hall_process_tick(MOTOR_A, 1);
        hall_process_tick(MOTOR_A, fwd[i]);
    }
    sync_state_t s = sync_update(70);
    TEST_ASSERT_EQUAL(SYNC_CORRECTING, s);
    // Both motors should be stopped
    TEST_ASSERT_EQUAL(0, stub_rpwm[MOTOR_A]);
    TEST_ASSERT_EQUAL(0, stub_rpwm[MOTOR_B]);
}

void test_sync_clear_fault(void)
{
    sync_clear_fault();
    TEST_ASSERT_EQUAL(SYNC_OK, sync_get_state());
}

void test_sync_delta(void)
{
    hall_process_tick(MOTOR_A, 1);
    hall_process_tick(MOTOR_A, 3);
    TEST_ASSERT_EQUAL(1, sync_get_delta());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_sync_ok_when_equal);
    RUN_TEST(test_sync_throttles_faster_motor);
    RUN_TEST(test_sync_triggers_correction_on_large_delta);
    RUN_TEST(test_sync_clear_fault);
    RUN_TEST(test_sync_delta);
    return UNITY_END();
}

#endif // UNIT_TEST
