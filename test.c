#include <stdint.h>
#include <limits.h>
#include <float.h>
#include "file_utils.h"
#include "greatest/greatest.h"


TEST test_file_utils(void) {
    char *filename = "test_file_utils.txt";
    FILE *f = fopen(filename, "wb");

    file_writer_t writer = file_writer_init(f);

    uint64_t written_uint64 = ULLONG_MAX;
    ASSERT(file_writer_write_uint64(&writer, written_uint64));

    file_writer_flush(&writer);
    fclose(f);

    f = fopen(filename, "rb");

    file_reader_t reader = file_reader_init(f);

    uint64_t read_uint64;
    ASSERT(file_reader_read_uint64(&reader, &read_uint64));
    ASSERT_EQ(read_uint64, written_uint64);

    fclose(f);

    f = fopen(filename, "wb");

    writer = file_writer_init(f);

    double written_double = DBL_MAX;

    ASSERT(file_writer_write_double(&writer, written_double));

    file_writer_flush(&writer);
    fclose(f);

    f = fopen(filename, "rb");

    reader = file_reader_init(f);

    double read_double;
    ASSERT(file_reader_read_double(&reader, &read_double));
    ASSERT_EQ(read_double, written_double);

    fclose(f);

    f = fopen(filename, "wb");
    writer = file_writer_init(f);
    double da[] = {1.0, 2.0, 3.0, 4.0, 5.0, DBL_MAX};
    size_t len_da = sizeof(da) / sizeof(double);
    ASSERT(file_writer_write_double_array(&writer, da, len_da));
    file_writer_flush(&writer);
    fclose(f);

    f = fopen(filename, "rb");
    reader = file_reader_init(f);
    double *read_da = malloc(sizeof(double) * len_da);
    ASSERT(file_reader_read_double_array(&reader, read_da, len_da));
    for (size_t i = 0; i < len_da; i++) {
        ASSERT_EQ(read_da[i], da[i]);
    }
    free(read_da);
    fclose(f);

    f = fopen(filename, "wb");
    writer = file_writer_init(f);
    float fa[] = {1.0, 2.0, 3.0, 4.0, 5.0, FLT_MAX};
    size_t len_fa = sizeof(fa) / sizeof(float);
    ASSERT(file_writer_write_float_array(&writer, fa, len_fa));
    file_writer_flush(&writer);
    fclose(f);

    f = fopen(filename, "rb");
    reader = file_reader_init(f);
    float *read_fa = malloc(sizeof(float) * len_fa);
    ASSERT(file_reader_read_float_array(&reader, read_fa, len_fa));
    for (size_t i = 0; i < len_fa; i++) {
        ASSERT_EQ(read_fa[i], fa[i]);
    }
    free(read_fa);
    fclose(f);

    f = fopen(filename, "wb");
    writer = file_writer_init(f);
    uint32_t ia[] = {1, 2, 3, 4, 5, INT_MAX};
    size_t len_ia = sizeof(ia) / sizeof(int32_t);
    ASSERT(file_writer_write_uint32_array(&writer, ia, len_ia));
    file_writer_flush(&writer);
    fclose(f);

    f = fopen(filename, "rb");
    reader = file_reader_init(f);
    uint32_t *read_ia = malloc(sizeof(uint32_t) * len_ia);
    ASSERT(file_reader_read_uint32_array(&reader, read_ia, len_ia));
    for (size_t i = 0; i < len_ia; i++) {
        ASSERT_EQ(read_ia[i], ia[i]);
    }
    free(read_ia);
    fclose(f);

    f = fopen(filename, "wb");
    writer = file_writer_init(f);
    uint64_t la[] = {1, 2, 3, 4, 5, ULLONG_MAX};
    size_t len_la = sizeof(la) / sizeof(uint64_t);
    ASSERT(file_writer_write_uint64_array(&writer, la, len_la));
    file_writer_flush(&writer);
    fclose(f);

    f = fopen(filename, "rb");
    reader = file_reader_init(f);
    uint64_t *read_la = malloc(sizeof(uint64_t) * len_la);
    ASSERT(file_reader_read_uint64_array(&reader, read_la, len_la));
    for (size_t i = 0; i < len_la; i++) {
        ASSERT_EQ(read_la[i], la[i]);
    }
    free(read_la);
    fclose(f);

    f = fopen(filename, "wb");
    uint64_t la_large[10000];
    for (size_t i = 0; i < 9999; i++) {
        la_large[i] = i;
    }
    la_large[9999] = ULLONG_MAX;
    size_t len_la_large = sizeof(la_large) / sizeof(uint64_t);
    writer = file_writer_init(f);
    ASSERT(file_writer_write_uint64_array(&writer, la_large, len_la_large));
    file_writer_flush(&writer);
    fclose(f);

    f = fopen(filename, "rb");
    reader = file_reader_init(f);
    uint64_t *read_la_large = malloc(sizeof(uint64_t) * len_la_large);
    ASSERT(file_reader_read_uint64_array(&reader, read_la_large, len_la_large));
    for (size_t i = 0; i < len_la_large; i++) {
        ASSERT_EQ(read_la_large[i], la_large[i]);
    }
    free(read_la_large);

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
