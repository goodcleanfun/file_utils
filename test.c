#include <stdint.h>
#include <limits.h>
#include <float.h>
#include "file_utils.h"
#include "greatest/greatest.h"


TEST test_file_utils(void) {
    char *filename = "test_file_utils.txt";
    FILE *f = fopen(filename, "w");
    size_t size = BUFSIZ * 2;
    char long_line[size + 1];
    long_line[size] = '\0';
    for (int i = 0; i < size - 2; i++) {
        long_line[i] = 'a';     
    }
    long_line[size - 1] = '\n';
    long_line[size - 2] = '\r';

    size_t len_long_line = size - 2;
    size_t written = fwrite(long_line, 1, size, f);

    ASSERT_EQ(written, size);

    char *short_string = "foobar";
    size_t len_short_string = strlen(short_string);
    ASSERT(file_write_chars(f, short_string, len_short_string));
    ASSERT(file_write_chars(f, "\n", 1));
    ASSERT_EQ(fwrite("\n", 1, 1, f), 1);
    ASSERT_EQ(fwrite("\n", 1, 1, f), 1);

    fclose(f);
    f = fopen(filename, "r");

    char *read_line = file_getline(f);
    ASSERT_NEQ(read_line, NULL);
    ASSERT_EQ(strncmp(read_line, long_line, len_long_line), 0);
    free(read_line);

    read_line = file_getline(f);
    ASSERT_NEQ(read_line, NULL);
    ASSERT_EQ(strncmp(read_line, short_string, len_short_string), 0);
    free(read_line);

    read_line = file_getline(f);
    ASSERT_NEQ(read_line, NULL);
    ASSERT_EQ(strlen(read_line), 0);
    free(read_line);

    fclose(f);
    f = fopen(filename, "w");

    uint64_t written_uint64 = ULLONG_MAX;
    ASSERT(file_write_uint64(f, written_uint64));

    fclose(f);
    f = fopen(filename, "r");

    uint64_t read_uint64;
    ASSERT(file_read_uint64(f, &read_uint64));
    ASSERT_EQ(read_uint64, written_uint64);

    fclose(f);

    f = fopen(filename, "w");

    double written_double = DBL_MAX;

    ASSERT(file_write_double(f, written_double));

    fclose(f);
    f = fopen(filename, "r");

    double read_double;
    ASSERT(file_read_double(f, &read_double));
    ASSERT_EQ(read_double, written_double);

    f = fopen(filename, "w");
    double da[] = {1.0, 2.0, 3.0, 4.0, 5.0, DBL_MAX};
    size_t len_da = sizeof(da) / sizeof(double);
    ASSERT(file_write_double_vector(f, da, len_da));
    fclose(f);

    f = fopen(filename, "r");
    double *read_da = malloc(sizeof(double) * len_da);
    ASSERT(file_read_double_vector(f, read_da, len_da));
    for (size_t i = 0; i < len_da; i++) {
        ASSERT_EQ(read_da[i], da[i]);
    }
    free(read_da);
    fclose(f);

    f = fopen(filename, "w");
    float fa[] = {1.0, 2.0, 3.0, 4.0, 5.0, FLT_MAX};
    size_t len_fa = sizeof(fa) / sizeof(float);
    ASSERT(file_write_float_vector(f, fa, len_fa));
    fclose(f);

    f = fopen(filename, "r");
    float *read_fa = malloc(sizeof(float) * len_fa);
    ASSERT(file_read_float_vector(f, read_fa, len_fa));
    for (size_t i = 0; i < len_fa; i++) {
        ASSERT_EQ(read_fa[i], fa[i]);
    }
    free(read_fa);
    fclose(f);

    f = fopen(filename, "w");
    uint32_t ia[] = {1, 2, 3, 4, 5, INT_MAX};
    size_t len_ia = sizeof(ia) / sizeof(int32_t);
    ASSERT(file_write_uint32_vector(f, ia, len_ia));
    fclose(f);

    f = fopen(filename, "r");
    uint32_t *read_ia = malloc(sizeof(uint32_t) * len_ia);
    ASSERT(file_read_uint32_vector(f, read_ia, len_ia));
    for (size_t i = 0; i < len_ia; i++) {
        ASSERT_EQ(read_ia[i], ia[i]);
    }
    free(read_ia);
    fclose(f);

    f = fopen(filename, "w");
    uint64_t la[] = {1, 2, 3, 4, 5, ULLONG_MAX};
    size_t len_la = sizeof(la) / sizeof(uint64_t);
    ASSERT(file_write_uint64_vector(f, la, len_la));
    fclose(f);

    f = fopen(filename, "r");
    uint64_t *read_la = malloc(sizeof(uint64_t) * len_la);
    ASSERT(file_read_uint64_vector(f, read_la, len_la));
    for (size_t i = 0; i < len_la; i++) {
        ASSERT_EQ(read_la[i], la[i]);
    }
    free(read_la);

    int ret = remove(filename);
    ASSERT_EQ(ret, 0);

    PASS();
}

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int32_t main(int32_t argc, char **argv) {
    GREATEST_MAIN_BEGIN();      /* command-long_line options, initialization. */

    RUN_TEST(test_file_utils);

    GREATEST_MAIN_END();        /* display results */
}
