#ifdef UNIT_TEST

#include <unity.h>
#include <stdint.h>

#include "../src/hal_esp32.h"
#include "../src/hall.c"   // include directly; no hardware deps in UNIT_TEST mode

void setUp(void)    { hall_init(); }
void tearDown(void) {}

// Simulate the 6-state forward sequence: 1->3->2->6->4->5->1
static const int FWD_SEQ[] = {1, 3, 2, 6, 4, 5, 1};

void test_forward_ticks_increment(void)
{
    for (int i = 0; i < 6; i++) {
        hall_process_tick(MOTOR_A, FWD_SEQ[i]);
        hall_process_tick(MOTOR_A, FWD_SEQ[i+1]);
    }
    TEST_ASSERT_EQUAL(6, hall_get(MOTOR_A).ticks);
    TEST_ASSERT_EQUAL(+1, hall_get(MOTOR_A).direction);
}

// Reverse sequence: 1->5->4->6->2->3->1
static const int REV_SEQ[] = {1, 5, 4, 6, 2, 3, 1};

void test_reverse_ticks_decrement(void)
{
    for (int i = 0; i < 6; i++) {
        hall_process_tick(MOTOR_A, REV_SEQ[i]);
        hall_process_tick(MOTOR_A, REV_SEQ[i+1]);
    }
    TEST_ASSERT_EQUAL(-6, hall_get(MOTOR_A).ticks);
    TEST_ASSERT_EQUAL(-1, hall_get(MOTOR_A).direction);
}

void test_invalid_state_ignored(void)
{
    hall_process_tick(MOTOR_A, 0); // invalid
    hall_process_tick(MOTOR_A, 7); // invalid
    TEST_ASSERT_EQUAL(0, hall_get(MOTOR_A).ticks);
}

void test_hall_reset_zeroes_counter(void)
{
    hall_process_tick(MOTOR_A, 1);
    hall_process_tick(MOTOR_A, 3);
    hall_reset(MOTOR_A);
    TEST_ASSERT_EQUAL(0, hall_get(MOTOR_A).ticks);
}

void test_motors_independent(void)
{
    hall_process_tick(MOTOR_A, 1);
    hall_process_tick(MOTOR_A, 3);
    TEST_ASSERT_EQUAL(0, hall_get(MOTOR_B).ticks);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_forward_ticks_increment);
    RUN_TEST(test_reverse_ticks_decrement);
    RUN_TEST(test_invalid_state_ignored);
    RUN_TEST(test_hall_reset_zeroes_counter);
    RUN_TEST(test_motors_independent);
    return UNITY_END();
}

#endif // UNIT_TEST
