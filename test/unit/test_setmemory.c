#include <stdio.h>
#include <complex.h>
#include "setmemory.h"

static int failures = 0;

#define ASSERT_TRUE(cond, name)                                                    \
  do {                                                                             \
    if (!(cond)) {                                                                 \
      fprintf(stderr, "[FAIL] %s\n", (name));                                      \
      failures++;                                                                  \
    }                                                                              \
  } while (0)

#define ASSERT_INT_EQ(expected, actual, name)                                      \
  do {                                                                             \
    if ((expected) != (actual)) {                                                  \
      fprintf(stderr, "[FAIL] %s: expected=%d actual=%d\n",                        \
              (name), (expected), (actual));                                       \
      failures++;                                                                  \
    }                                                                              \
  } while (0)

static void test_i_2d_allocate(void) {
  const long unsigned int n = 3;
  const long unsigned int m = 4;
  int **a = i_2d_allocate(n, m);
  ASSERT_TRUE(a != NULL, "i_2d_allocate base pointer");
  ASSERT_TRUE(a[0] != NULL, "i_2d_allocate data pointer");
  ASSERT_TRUE(a[1] == a[0] + m, "i_2d_allocate contiguous row");

  ASSERT_INT_EQ(0, a[2][3], "i_2d_allocate zero initialized");
  a[1][2] = 7;
  ASSERT_INT_EQ(7, a[0][m + 2], "i_2d_allocate shared contiguous backing");
  free_i_2d_allocate(a);
}

static void test_i_3d_allocate(void) {
  const long unsigned int n = 2;
  const long unsigned int m = 3;
  const long unsigned int l = 4;
  int ***a = i_3d_allocate(n, m, l);
  ASSERT_TRUE(a != NULL, "i_3d_allocate base pointer");
  ASSERT_TRUE(a[0] != NULL, "i_3d_allocate mid pointer");
  ASSERT_TRUE(a[0][0] != NULL, "i_3d_allocate data pointer");
  ASSERT_TRUE(a[1] == a[0] + m, "i_3d_allocate contiguous first dimension");
  ASSERT_TRUE(a[0][1] == a[0][0] + l, "i_3d_allocate contiguous second dimension");

  ASSERT_INT_EQ(0, a[1][2][3], "i_3d_allocate zero initialized");
  a[1][2][3] = 11;
  ASSERT_INT_EQ(11, a[0][0][(1 * m + 2) * l + 3], "i_3d_allocate contiguous data");
  free_i_3d_allocate(a);
}

static void test_scalar_allocators(void) {
  unsigned int *u = ui_1d_allocate(5);
  long int *li = li_1d_allocate(5);
  long unsigned int *lui = lui_1d_allocate(5);
  double *d = d_1d_allocate(5);
  double complex *cd = cd_1d_allocate(5);

  ASSERT_TRUE(u != NULL, "ui_1d_allocate");
  ASSERT_TRUE(li != NULL, "li_1d_allocate");
  ASSERT_TRUE(lui != NULL, "lui_1d_allocate");
  ASSERT_TRUE(d != NULL, "d_1d_allocate");
  ASSERT_TRUE(cd != NULL, "cd_1d_allocate");
  ASSERT_INT_EQ(0, (int)u[4], "ui_1d_allocate zero initialized");
  ASSERT_INT_EQ(0, (int)li[2], "li_1d_allocate zero initialized");
  ASSERT_INT_EQ(0, (int)lui[1], "lui_1d_allocate zero initialized");
  ASSERT_INT_EQ(0, (int)d[3], "d_1d_allocate zero initialized");
  ASSERT_INT_EQ(0, (int)creal(cd[0]), "cd_1d_allocate zero initialized");

  free_ui_1d_allocate(u);
  free_li_1d_allocate(li);
  free_lui_1d_allocate(lui);
  free_d_1d_allocate(d);
  free_cd_1d_allocate(cd);
}

static void test_d_2d_and_cd_3d_allocate(void) {
  double **d2 = d_2d_allocate(2, 3);
  double complex ***cd3 = cd_3d_allocate(2, 2, 2);

  ASSERT_TRUE(d2 != NULL, "d_2d_allocate base pointer");
  ASSERT_TRUE(d2[0] != NULL, "d_2d_allocate data pointer");
  ASSERT_TRUE(d2[1] == d2[0] + 3, "d_2d_allocate contiguous row");
  d2[1][2] = 1.25;
  ASSERT_TRUE(d2[0][5] == 1.25, "d_2d_allocate contiguous data");

  ASSERT_TRUE(cd3 != NULL, "cd_3d_allocate base pointer");
  ASSERT_TRUE(cd3[0] != NULL, "cd_3d_allocate mid pointer");
  ASSERT_TRUE(cd3[0][0] != NULL, "cd_3d_allocate data pointer");
  cd3[1][1][1] = 2.0 + 3.0 * I;
  ASSERT_TRUE(cd3[0][0][7] == 2.0 + 3.0 * I, "cd_3d_allocate contiguous data");

  free_d_2d_allocate(d2);
  free_cd_3d_allocate(cd3);
}

int main(void) {
  test_i_2d_allocate();
  test_i_3d_allocate();
  test_scalar_allocators();
  test_d_2d_and_cd_3d_allocate();

  if (failures != 0) {
    fprintf(stderr, "[FAIL] unit_setmemory failures=%d\n", failures);
    return 1;
  }
  printf("[PASS] unit_setmemory\n");
  return 0;
}
