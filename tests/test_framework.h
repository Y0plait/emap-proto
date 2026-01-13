/**
 * @file test_framework.h
 * @brief Simple test framework macros and utilities for EMAP tests.
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========================================================================== */
/*                              Test Framework */
/* ========================================================================== */

extern int tests_passed;
extern int tests_failed;

#define TEST_ASSERT(cond, msg)                                                 \
	do {                                                                       \
		if (!(cond)) {                                                         \
			printf("  [FAIL] %s (line %d): %s\n", __func__, __LINE__, msg);    \
			tests_failed++;                                                    \
			return;                                                            \
		}                                                                      \
	} while (0)

#define TEST_ASSERT_EQ(a, b, msg)                                              \
	do {                                                                       \
		if ((a) != (b)) {                                                      \
			printf("  [FAIL] %s (line %d): %s (expected %d, got %d)\n",        \
				   __func__, __LINE__, msg, (int)(b), (int)(a));               \
			tests_failed++;                                                    \
			return;                                                            \
		}                                                                      \
	} while (0)

#define TEST_PASS()                                                            \
	do {                                                                       \
		printf("  [PASS] %s\n", __func__);                                     \
		tests_passed++;                                                        \
	} while (0)

#define RUN_TEST(test_func)                                                    \
	do {                                                                       \
		test_func();                                                           \
	} while (0)

#endif /* TEST_FRAMEWORK_H */
