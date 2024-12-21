#include "file_utils.h"

char *file_getline(FILE * f)
{
    char buf[BUFSIZ];

    char *ret = NULL;

    size_t buf_len = 0;
    size_t ret_size = 0;

    while (fgets(buf, BUFSIZ, f) != NULL) {
        buf_len = strlen(buf);
        if (buf_len == 0) break;
        char *temp = realloc(ret, ret_size + buf_len + 1);
        if (temp == NULL) {
            free(ret);
            return NULL;
        }
        ret = temp;
        memcpy(ret+ret_size, buf, buf_len);
        ret_size += buf_len;
        ret[ret_size] = '\0';
        if (ret[ret_size - 1] == '\n') {
            ret[ret_size - 1] = '\0';
            // Handle carriage returns
            if (ret_size > 1 && ret[ret_size-2] == '\r') {
                ret[ret_size - 2] = '\0';
            }
            break;
        }
    }

    if (ret_size == 0) {
        // should always be NULL in this case
        free(ret);
        return NULL;
    } 
    return ret;
}

bool file_exists(const char *filename) {
    FILE *f = fopen(filename, "r");
    bool exists = f != NULL;
    if (exists) fclose(f);
    return exists;
}

inline uint64_t file_deserialize_uint64(unsigned char *buf) {
    return ((uint64_t)buf[0] << 56) | 
           ((uint64_t)buf[1] << 48) |
           ((uint64_t)buf[2] << 40) |
           ((uint64_t)buf[3] << 32) |
           ((uint64_t)buf[4] << 24) |
           ((uint64_t)buf[5] << 16) |
           ((uint64_t)buf[6] << 8) |
            (uint64_t)buf[7];
}

bool file_read_uint64(FILE *file, uint64_t *value) {
    unsigned char buf[8];

    if (fread(buf, 8, 1, file) == 1) {
        *value = file_deserialize_uint64(buf);
        return true;
    }
    return false;
}

bool file_read_uint64_array(FILE *file, uint64_t *value, size_t n) {
    unsigned char *buf = malloc(n * sizeof(uint64_t));

    if (buf == NULL) return false;

    bool ret = false;

    if (fread(buf, sizeof(uint64_t), n, file) == n) {

        for (size_t i = 0, byte_offset = 0; i < n; i++, byte_offset += sizeof(uint64_t)) {
            unsigned char *ptr = buf + byte_offset;
            value[i] = file_deserialize_uint64(ptr);
        }
        ret = true;
    }
    free(buf);
    return ret;
}

static inline void fill_buffer_uint64(uint8_t *buf, size_t buf_index, uint64_t value) {
    buf[buf_index    ] = (uint8_t)(value >> 56);
    buf[buf_index + 1] = (uint8_t)(value >> 48);
    buf[buf_index + 2] = (uint8_t)(value >> 40);
    buf[buf_index + 3] = (uint8_t)(value >> 32);
    buf[buf_index + 4] = (uint8_t)(value >> 24);
    buf[buf_index + 5] = (uint8_t)(value >> 16);
    buf[buf_index + 6] = (uint8_t)(value >> 8);
    buf[buf_index + 7] = (uint8_t)(value);
}

bool file_write_uint64(FILE *file, uint64_t value) {
    uint8_t buf[8];
    fill_buffer_uint64(buf, 0, value);
    return (fwrite(buf, 8, 1, file) == 1);
}

bool file_write_uint64_array(FILE *file, uint64_t *values, size_t n) {
    uint8_t buf[BUFSIZ];
    size_t buf_index = 0;

    for (size_t i = 0; i < n; i++) {
        if (buf_index + 8 > BUFSIZ) {
            // Write the buffer to the file when it's full
            if (fwrite(buf, 1, buf_index, file) != buf_index) {
                return false;
            }
            buf_index = 0;
        }

        // Pack the uint64_t into the buffer
        fill_buffer_uint64(buf, buf_index, values[i]);
        buf_index += 8;
    }

    // Write any remaining data in the buffer to the file
    if (buf_index > 0) {
        if (fwrite(buf, 1, buf_index, file) != buf_index) {
            return false;
        }
    }

    return true;
}

typedef union {
    uint64_t u;
    double d;
} uint64_double_t;

bool file_read_double(FILE *file, double *value) {
    uint64_double_t ud;
    if (!file_read_uint64(file, &ud.u)) {
        return false;
    }
    *value = ud.d;
    return true;
}

bool file_read_double_array(FILE *file, double *value, size_t n) {
    unsigned char *buf = malloc(n * sizeof(uint64_t));

    if (buf == NULL) return false;

    bool ret = false;

    if (fread(buf, sizeof(uint64_t), n, file) == n) {
        uint64_double_t ud;

        for (size_t i = 0, byte_offset = 0; i < n; i++, byte_offset += sizeof(uint64_t)) {
            unsigned char *ptr = buf + byte_offset;
            ud.u = file_deserialize_uint64(ptr);
            value[i] = ud.d;
        }
        ret = true;
    }
    free(buf);
    return ret;
}

bool file_write_double(FILE *file, double value) {
    uint64_double_t ud;
    ud.d = value;
    return file_write_uint64(file, ud.u);
}

bool file_write_double_array(FILE *file, double *values, size_t n) {
    uint8_t buf[BUFSIZ];
    size_t buf_index = 0;
    uint64_double_t ud;

    for (size_t i = 0; i < n; i++) {
        if (buf_index + 8 > BUFSIZ) {
            // Write the buffer to the file when it's full
            if (fwrite(buf, 1, buf_index, file) != buf_index) {
                return false;
            }
            buf_index = 0;
        }

        // Pack the uint64_t into the buffer
        ud.d = values[i];
        uint64_t u = ud.u;
        fill_buffer_uint64(buf, buf_index, u);
        buf_index += 8;
    }
    if (buf_index > 0) {
        if (fwrite(buf, 1, buf_index, file) != buf_index) {
            return false;
        }
    }
    return true;
}

typedef union {
    uint32_t u;
    float f;
} uint32_float_t;

bool file_read_float(FILE *file, float *value) {
    uint32_float_t uf;

    if (!file_read_uint32(file, &uf.u)) {
        return false;
    }
    *value = uf.f;
    return true;
}

bool file_read_float_array(FILE *file, float *value, size_t n) {
    unsigned char *buf = malloc(n * sizeof(uint32_t));

    if (buf == NULL) return false;

    bool ret = false;

    if (fread(buf, sizeof(uint32_t), n, file) == n) {
        uint32_float_t uf;

        for (size_t i = 0, byte_offset = 0; i < n; i++, byte_offset += sizeof(uint32_t)) {
            unsigned char *ptr = buf + byte_offset;
            uf.u = file_deserialize_uint32(ptr);
            value[i] = uf.f;
        }
        ret = true;
    }
    free(buf);
    return ret;
}

bool file_write_float(FILE *file, float value) {
    uint32_float_t uf;
    uf.f = value;
    return file_write_uint32(file, uf.u);

}

bool file_write_float_array(FILE *file, float *values, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (!file_write_float(file, values[i])) {
            return false;
        }
    }
    return true;
}

inline uint32_t file_deserialize_uint32(unsigned char *buf) {
    return ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) | (uint32_t)buf[3];
}

bool file_read_uint32(FILE *file, uint32_t *value) {
    unsigned char buf[4];

    if (fread(buf, 4, 1, file) == 1) {
        *value = file_deserialize_uint32(buf);
        return true;
    }
    return false;
}

bool file_read_uint32_array(FILE *file, uint32_t *value, size_t n) {
    unsigned char *buf = malloc(n * sizeof(uint32_t));

    if (buf == NULL) return false;

    bool ret = false;

    if (fread(buf, sizeof(uint32_t), n, file) == n) {

        for (size_t i = 0, byte_offset = 0; i < n; i++, byte_offset += sizeof(uint32_t)) {
            unsigned char *ptr = buf + byte_offset;
            value[i] = file_deserialize_uint32(ptr);
        }
        ret = true;
    }
    free(buf);
    return ret;
}

void fill_buffer_uint32(uint8_t *buf, size_t buf_index, uint32_t value) {
    buf[buf_index    ] = (uint8_t)(value >> 24);
    buf[buf_index + 1] = (uint8_t)(value >> 16);
    buf[buf_index + 2] = (uint8_t)(value >> 8);
    buf[buf_index + 3] = (uint8_t)(value);
}


bool file_write_uint32(FILE *file, uint32_t value) {
    uint8_t buf[4];
    fill_buffer_uint32(buf, 0, value);
    return (fwrite(buf, 4, 1, file) == 1);
}

bool file_write_uint32_array(FILE *file, uint32_t *values, size_t n) {
    uint8_t buf[BUFSIZ];
    size_t buf_index = 0;

    for (size_t i = 0; i < n; i++) {
        if (buf_index + 4 > BUFSIZ) {
            // Write the buffer to the file when it's full
            if (fwrite(buf, 1, buf_index, file) != buf_index) {
                return false;
            }
            buf_index = 0;
        }

        // Pack the uint32_t into the buffer
        fill_buffer_uint32(buf, buf_index, values[i]);
        buf_index += 4;
    }

    // Write any remaining data in the buffer to the file
    if (buf_index > 0) {
        if (fwrite(buf, 1, buf_index, file) != buf_index) {
            return false;
        }
    }

    return true;
}

inline uint16_t file_deserialize_uint16(unsigned char *buf) {
    return ((uint16_t)buf[0] << 8) | buf[1];
}

bool file_read_uint16(FILE *file, uint16_t *value) {
    unsigned char buf[2];

    if (fread(buf, 2, 1, file) == 1) {
        *value = file_deserialize_uint16(buf);
        return true;
    }
    return false;
}

static void fill_buffer_uint16(uint8_t *buf, size_t buf_index, uint16_t value) {
    buf[buf_index    ] = (uint8_t)(value >> 8);
    buf[buf_index + 1] = (uint8_t)(value);
}

bool file_write_uint16(FILE *file, uint16_t value) {
    uint8_t buf[2];
    fill_buffer_uint16(buf, 0, value);
    return (fwrite(buf, 2, 1, file) == 1);
}

bool file_write_uint16_array(FILE *file, uint16_t *values, size_t n) {
    uint8_t buf[BUFSIZ];
    size_t buf_index = 0;

    for (size_t i = 0; i < n; i++) {
        if (buf_index + 2 > BUFSIZ) {
            // Write the buffer to the file when it's full
            if (fwrite(buf, 1, buf_index, file) != buf_index) {
                return false;
            }
            buf_index = 0;
        }

        // Pack the uint16_t into the buffer
        fill_buffer_uint16(buf, buf_index, values[i]);
        buf_index += 2;
    }

    // Write any remaining data in the buffer to the file
    if (buf_index > 0) {
        if (fwrite(buf, 1, buf_index, file) != buf_index) {
            return false;
        }
    }

    return true;
}

bool file_read_uint8(FILE *file, uint8_t *value) {
    return (fread(value, sizeof(int8_t), 1, file) == 1);
}

bool file_write_uint8(FILE *file, uint8_t value) {
    return (fwrite(&value, sizeof(int8_t), 1, file) == 1);
}

bool file_read_chars(FILE *file, char *buf, size_t len) {
    return (fread(buf, sizeof(char), len, file) == len);
}

bool file_write_chars(FILE *file, const char *buf, size_t len) {
    return (fwrite(buf, sizeof(char), len, file) == len);
}
