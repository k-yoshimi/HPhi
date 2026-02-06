#include <stdio.h>
#include "bitcalc.h"

static int failures = 0;

#define ASSERT_INT_EQ(expected, actual, name)                                     \
  do {                                                                             \
    if ((expected) != (actual)) {                                                  \
      fprintf(stderr, "[FAIL] %s: expected=%d actual=%d\n",                       \
              (name), (expected), (actual));                                       \
      failures++;                                                                  \
    }                                                                              \
  } while (0)

#define ASSERT_ULL_EQ(expected, actual, name)                                      \
  do {                                                                             \
    if ((expected) != (actual)) {                                                  \
      fprintf(stderr, "[FAIL] %s: expected=%lu actual=%lu\n",                      \
              (name), (unsigned long)(expected), (unsigned long)(actual));         \
      failures++;                                                                  \
    }                                                                              \
  } while (0)

static void test_get_split_bit(void) {
  unsigned long irght = 0;
  unsigned long ilft = 0;
  unsigned long ihfbit = 0;

  ASSERT_INT_EQ(0, GetSplitBit(4, &irght, &ilft, &ihfbit), "GetSplitBit return");
  ASSERT_ULL_EQ(3UL, irght, "GetSplitBit irght");
  ASSERT_ULL_EQ(12UL, ilft, "GetSplitBit ilft");
  ASSERT_ULL_EQ(4UL, ihfbit, "GetSplitBit ihfbit");

  ASSERT_INT_EQ(-1, GetSplitBit(0, &irght, &ilft, &ihfbit), "GetSplitBit invalid");
}

static void test_get_split_bit_by_model(void) {
  unsigned long irght = 0;
  unsigned long ilft = 0;
  unsigned long ihfbit = 0;

  ASSERT_INT_EQ(0, GetSplitBitByModel(4, Hubbard, &irght, &ilft, &ihfbit),
                "GetSplitBitByModel Hubbard return");
  ASSERT_ULL_EQ(15UL, irght, "GetSplitBitByModel Hubbard irght");
  ASSERT_ULL_EQ(240UL, ilft, "GetSplitBitByModel Hubbard ilft");
  ASSERT_ULL_EQ(16UL, ihfbit, "GetSplitBitByModel Hubbard ihfbit");

  ASSERT_INT_EQ(0, GetSplitBitByModel(4, Spin, &irght, &ilft, &ihfbit),
                "GetSplitBitByModel Spin return");
  ASSERT_ULL_EQ(3UL, irght, "GetSplitBitByModel Spin irght");
  ASSERT_ULL_EQ(12UL, ilft, "GetSplitBitByModel Spin ilft");
  ASSERT_ULL_EQ(4UL, ihfbit, "GetSplitBitByModel Spin ihfbit");

  ASSERT_INT_EQ(-1, GetSplitBitByModel(4, -999, &irght, &ilft, &ihfbit),
                "GetSplitBitByModel invalid");
}

static void test_split_and_offcomp(void) {
  unsigned long right = 0;
  unsigned long left = 0;
  unsigned long ioff = 0;
  unsigned long list_2_1[] = {0UL, 5UL, 7UL, 11UL};
  unsigned long list_2_2[] = {0UL, 3UL, 11UL, 13UL};

  SplitBit(9UL, 3UL, 12UL, 4UL, &right, &left);
  ASSERT_ULL_EQ(1UL, right, "SplitBit right");
  ASSERT_ULL_EQ(2UL, left, "SplitBit left");

  ASSERT_INT_EQ(TRUE, GetOffComp(list_2_1, list_2_2, 9UL, 3UL, 12UL, 4UL, &ioff),
                "GetOffComp true");
  ASSERT_ULL_EQ(14UL, ioff, "GetOffComp value");

  list_2_2[2] = 0UL;
  ASSERT_INT_EQ(FALSE, GetOffComp(list_2_1, list_2_2, 9UL, 3UL, 12UL, 4UL, &ioff),
                "GetOffComp false");
  ASSERT_ULL_EQ(0UL, ioff, "GetOffComp value when false");
}

static void test_general_spin_helpers(void) {
  long int site_to_bit[] = {2, 3, 2};
  unsigned long ihfbit = 0;

  ASSERT_INT_EQ(0, GetSplitBitForGeneralSpin(3, &ihfbit, site_to_bit),
                "GetSplitBitForGeneralSpin return");
  ASSERT_ULL_EQ(6UL, ihfbit, "GetSplitBitForGeneralSpin ihfbit");
  ASSERT_INT_EQ(-1, GetSplitBitForGeneralSpin(0, &ihfbit, site_to_bit),
                "GetSplitBitForGeneralSpin invalid");

  ASSERT_INT_EQ(4, pop(0xF0U), "pop");
  ASSERT_ULL_EQ(10UL, snoob(9UL), "snoob");
}

int main(void) {
  test_get_split_bit();
  test_get_split_bit_by_model();
  test_split_and_offcomp();
  test_general_spin_helpers();

  if (failures != 0) {
    fprintf(stderr, "[FAIL] unit_bitcalc failures=%d\n", failures);
    return 1;
  }
  printf("[PASS] unit_bitcalc\n");
  return 0;
}
