#pragma once

#include <stdbool.h>
#include <stdint.h>

#define GPIO_BUFFER_SIZE 256

// GPIO functions

int gpio_request_line(char *dev, int *lines, int count, struct gpio_v2_line_config *config, int *fd);
int gpio_set_values(int fd, struct gpio_v2_line_values *values);
int gpio_get_values(int fd, struct gpio_v2_line_values *values);
int gpio_release_line(int fd);

int gpio_get(char *dev, int line, int *value);
int gpio_getn(char *dev, int *lines, int count, int *values);
int gpio_set(char *dev, int line, int value);
int gpio_setn(char *dev, int *lines, int count, int *values);


// helper functions for gpio_v2_line_values bits

inline void gpio_set_bit(uint64_t * b, int n)
{
    *b |= _BITULL(n);
}

static inline void gpio_change_bit(uint64_t * b, int n)
{
    *b ^= _BITULL(n);
}

static inline void gpio_clear_bit(uint64_t * b, int n)
{
    *b &= ~_BITULL(n);
}

static inline int gpio_test_bit(uint64_t b, int n)
{
    return !!(b & _BITULL(n));
}

static inline void gpio_assign_bit(uint64_t * b, int n, bool value)
{
    if (value)
        gpio_set_bit(b, n);
    else
        gpio_clear_bit(b, n);
}