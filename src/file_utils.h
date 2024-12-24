#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/types.h>

#ifndef BUFSIZ
#define BUFSIZ 4096
static_assert(BUFSIZ > 0, "BUFSIZ must be greater than 0");
static_assert(BUFSIZ % 8 == 0, "BUFSIZ must be a multiple of 8");
#endif

#define TAB_SEPARATOR "\t"
#define TAB_SEPARATOR_LEN strlen(TAB_SEPARATOR)

#define COMMA_SEPARATOR ","
#define COMMA_SEPARATOR_LEN strlen(COMMA_SEPARATOR)

char *file_getline(FILE * f);
bool file_exists(const char *filename);

uint64_t file_deserialize_uint64(unsigned char *buf);
bool file_read_uint64(FILE *file, uint64_t *value);
bool file_write_uint64(FILE *file, uint64_t value);

bool file_read_uint64_array(FILE *file, uint64_t *value, size_t n);
bool file_write_uint64_array(FILE *file, uint64_t *values, size_t n);

bool file_read_float(FILE *file, float *value);
bool file_write_float(FILE *file, float value);

float file_deserialize_float(unsigned char *buf);
bool file_read_float_array(FILE *file, float *value, size_t n);
bool file_write_float_array(FILE *file, float *values, size_t n);

double file_deserialize_double(unsigned char *buf);
bool file_read_double(FILE *file, double *value);
bool file_write_double(FILE *file, double value);

bool file_read_double_array(FILE *file, double *value, size_t n);
bool file_write_double_array(FILE *file, double *values, size_t n);

uint32_t file_deserialize_uint32(unsigned char *buf);
bool file_read_uint32(FILE *file, uint32_t *value);
bool file_write_uint32(FILE *file, uint32_t value);

bool file_read_uint32_array(FILE *file, uint32_t *value, size_t n);
bool file_write_uint32_array(FILE *file, uint32_t *values, size_t n);

uint16_t file_deserialize_uint16(unsigned char *buf);
bool file_read_uint16(FILE *file, uint16_t *value);
bool file_write_uint16(FILE *file, uint16_t value);

bool file_read_uint8(FILE *file, uint8_t *value);
bool file_write_uint8(FILE *file, uint8_t value);

bool file_read_chars(FILE *file, char *buf, size_t len);
bool file_write_chars(FILE *file, const char *buf, size_t len);

#endif

