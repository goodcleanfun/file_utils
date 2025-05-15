#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/types.h>

#include "byte_order/byte_order.h"
#include "likely/likely.h"

#ifndef BUFSIZ
#define BUFSIZ 4096
static_assert(BUFSIZ > 0, "BUFSIZ must be greater than 0");
static_assert(BUFSIZ % 8 == 0, "BUFSIZ must be a multiple of 8");
#endif

typedef struct file_reader {
    FILE *stream;
    uint8_t buf[BUFSIZ];
    size_t buf_index;
    size_t buf_len;
} file_reader_t;

static inline file_reader_t file_reader_init(FILE *stream) {
    file_reader_t reader;
    reader.stream = stream;
    reader.buf_index = 0;
    reader.buf_len = 0;
    return reader;
}

static inline size_t file_reader_read_block(file_reader_t *reader) {
    size_t bytes_read = fread(reader->buf, 1, BUFSIZ, reader->stream);
    reader->buf_index = 0;
    reader->buf_len = bytes_read;
    return bytes_read;
}

static inline bool file_reader_read_uint8(file_reader_t *reader, uint8_t *value) {
    if (reader->buf_index + 1 > reader->buf_len) {
        if (file_reader_read_block(reader) == 0) {
            return false;
        }
    }
    *value = reader->buf[reader->buf_index];
    reader->buf_index++;
    return true;
}

#define file_reader_read_type_func(name, type, deserialize)                     \
static inline bool name(file_reader_t *reader, type *value) {                   \
    if (LIKELY(reader->buf_index + sizeof(type) <= reader->buf_len)) {          \
        *value = deserialize(reader->buf + reader->buf_index);                  \
        reader->buf_index += sizeof(type);                                      \
        return true;                                                            \
    } else {                                                                    \
        size_t wrap_bytes = reader->buf_len - reader->buf_index;                \
        uint8_t buf[sizeof(type)];                                              \
        if (wrap_bytes != 0) {                                                  \
            memcpy(buf, reader->buf + reader->buf_index, wrap_bytes);           \
        }                                                                       \
        if (file_reader_read_block(reader) == 0) {                              \
            return false;                                                       \
        }                                                                       \
        if (wrap_bytes != 0) {                                                  \
            memcpy(buf + wrap_bytes, reader->buf, sizeof(type) - wrap_bytes);   \
            *value = deserialize(buf);                                          \
        } else {                                                                \
            *value = deserialize(reader->buf);                                  \
        }                                                                       \
        reader->buf_index += sizeof(type) - wrap_bytes;                         \
        return true;                                                            \
    }                                                                           \
}

file_reader_read_type_func(file_reader_read_uint64, uint64_t, read_uint64_big_endian)
file_reader_read_type_func(file_reader_read_uint32, uint32_t, read_uint32_big_endian)
file_reader_read_type_func(file_reader_read_uint16, uint16_t, read_uint16_big_endian)
file_reader_read_type_func(file_reader_read_double, double, read_double_big_endian)
file_reader_read_type_func(file_reader_read_float, float, read_float_big_endian)

#define file_reader_read_array_func(name, type, read_one_func)                  \
static inline bool name(file_reader_t *reader, type *values, size_t n) {        \
    for (size_t i = 0; i < n; i++) {                                            \
        if (!read_one_func(reader, &values[i])) {                               \
            return false;                                                       \
        }                                                                       \
    }                                                                           \
    return true;                                                                \
}

file_reader_read_array_func(file_reader_read_uint64_array, uint64_t, file_reader_read_uint64)
file_reader_read_array_func(file_reader_read_uint32_array, uint32_t, file_reader_read_uint32)
file_reader_read_array_func(file_reader_read_uint16_array, uint16_t, file_reader_read_uint16)
file_reader_read_array_func(file_reader_read_double_array, double, file_reader_read_double)
file_reader_read_array_func(file_reader_read_float_array, float, file_reader_read_float)


static inline bool file_reader_read_uint8_array(file_reader_t *reader, uint8_t *values, size_t n) {
    size_t bytes_remaining = n;
    uint8_t *buf = values;
    while (bytes_remaining != 0) {
        size_t block_remaining = reader->buf_len - reader->buf_index;
        size_t read_size = block_remaining > bytes_remaining ? bytes_remaining : block_remaining;
        memcpy(buf, reader->buf + reader->buf_index, read_size);
        reader->buf_index += read_size;
        buf += read_size;
        bytes_remaining -= read_size;
        if (bytes_remaining > 0) {
            if (!file_reader_read_block(reader)) {
                return false;
            }
        }
    }
    return true;
}


typedef struct file_writer {
    FILE *stream;
    uint8_t buf[BUFSIZ];
    size_t buf_used;
} file_writer_t;

static inline file_writer_t file_writer_init(FILE *stream) {
    file_writer_t writer;
    writer.stream = stream;
    writer.buf_used = 0;
    return writer;
}

static inline bool file_writer_write_block(file_writer_t *writer) {
    size_t expected_bytes = writer->buf_used;
    size_t bytes_written = fwrite(writer->buf, 1, writer->buf_used, writer->stream);
    writer->buf_used = 0;
    return bytes_written == expected_bytes;
}

static inline bool file_writer_flush(file_writer_t *writer) {
    if (writer->buf_used > 0) {
        if (!file_writer_write_block(writer)) {
            return false;
        }
        fflush(writer->stream);
    }
    return true;
}

static inline bool file_writer_close(file_writer_t *writer) {
    if (!file_writer_flush(writer)) {
        return false;
    }

    fclose(writer->stream);
    return true;
}

#define file_writer_write_type_func(name, type, serialize)                      \
static inline bool name(file_writer_t *writer, type value) {                    \
    if (LIKELY(writer->buf_used + sizeof(type) < BUFSIZ)) {                     \
        serialize(writer->buf + writer->buf_used, value);                       \
        writer->buf_used += sizeof(type);                                       \
        return true;                                                            \
    } else {                                                                    \
        size_t wrap_bytes = BUFSIZ - writer->buf_used;                          \
        uint8_t buf[sizeof(type)];                                              \
        if (wrap_bytes != 0) {                                                  \
            serialize(buf, value);                                              \
            memcpy(writer->buf + writer->buf_used, buf, wrap_bytes);            \
            writer->buf_used += wrap_bytes;                                     \
        } else {                                                                \
            serialize(writer->buf + writer->buf_used, value);                   \
            writer->buf_used += sizeof(type);                                   \
        }                                                                       \
        if (!file_writer_write_block(writer)) {                                 \
            return false;                                                       \
        }                                                                       \
        if (wrap_bytes != 0) {                                                  \
            memcpy(writer->buf, buf + wrap_bytes, sizeof(type) - wrap_bytes);   \
        }                                                                       \
        return true;                                                            \
    }                                                                           \
}

static inline bool file_writer_write_uint8(file_writer_t *writer, uint8_t value) {
    if (writer->buf_used + 1 > BUFSIZ) {
        if (!file_writer_write_block(writer)) {
            return false;
        }
    }
    writer->buf[writer->buf_used] = value;
    writer->buf_used++;
    return true;
}

file_writer_write_type_func(file_writer_write_uint64, uint64_t, write_uint64_big_endian)
file_writer_write_type_func(file_writer_write_uint32, uint32_t, write_uint32_big_endian)
file_writer_write_type_func(file_writer_write_uint16, uint16_t, write_uint16_big_endian)
file_writer_write_type_func(file_writer_write_double, double, write_double_big_endian)
file_writer_write_type_func(file_writer_write_float, float, write_float_big_endian)

#define file_write_array_func(name, type, write_one_func)                       \
static inline bool name(file_writer_t *writer, type *values, size_t n) {        \
    for (size_t i = 0; i < n; i++) {                                            \
        if (!write_one_func(writer, values[i])) {                               \
            return false;                                                       \
        }                                                                       \
    }                                                                           \
    return true;                                                                \
}

file_write_array_func(file_writer_write_uint64_array, uint64_t, file_writer_write_uint64)
file_write_array_func(file_writer_write_uint32_array, uint32_t, file_writer_write_uint32)
file_write_array_func(file_writer_write_uint16_array, uint16_t, file_writer_write_uint16)
file_write_array_func(file_writer_write_double_array, double, file_writer_write_double)
file_write_array_func(file_writer_write_float_array, float, file_writer_write_float)


static inline bool file_writer_write_uint8_array(file_writer_t *writer, uint8_t *values, size_t n) {
    size_t bytes_remaining = n;
    uint8_t *buf = values;
    while (bytes_remaining != 0) {
        size_t block_remaining = BUFSIZ - writer->buf_used;
        size_t write_size = block_remaining > bytes_remaining ? bytes_remaining : block_remaining;
        memcpy(writer->buf + writer->buf_used, buf, write_size);
        writer->buf_used += write_size;
        buf += write_size;
        bytes_remaining -= write_size;
        if (bytes_remaining > 0) {
            if (!file_writer_write_block(writer)) {
                return false;
            }
        }
    }
    return true;
}

#endif

