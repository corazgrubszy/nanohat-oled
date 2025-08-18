#pragma once

#include <stddef.h>

void get_ip(char *out, size_t out_size);
void get_disk_usage(char *out, size_t out_size);
void get_mem_usage(char *out, size_t out_size);
void get_temp_and_load(char *out, size_t out_size);