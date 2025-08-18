#include "stats.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/statvfs.h>

void get_ip(char *out, size_t out_size)
{
    FILE *fp = popen("hostname -I", "r");
    if (!fp) {
        snprintf(out, out_size, "IP: unavailable");
        return;
    }
    char ip[64] = {0};
    fgets(ip, sizeof(ip), fp);
    pclose(fp);
    ip[strcspn(ip, " \n")] = '\0';
    snprintf(out, out_size, "IP:%13s", ip);
}

void get_disk_usage(char *out, size_t out_size)
{
    FILE *fp = fopen("/proc/mounts", "r");
    if (!fp) {
        snprintf(out, out_size, "/: error");
        return;
    }

    int found = 0;
    char dev[64], mount[64], type[32];
    while (fscanf(fp, "%63s %63s %31s", dev, mount, type) == 3) {
        if (strcmp(mount, "/") == 0) {
            found = 1;
            break;
        }
    }
    fclose(fp);

    if (!found) {
        snprintf(out, out_size, "/: not mounted");
        return;
    }

    struct statvfs vfs;
    if (statvfs("/", &vfs) != 0) {
        snprintf(out, out_size, "/: error");
        return;
    }

    // użyj 64-bitowej arytmetyki, by uniknąć przepełnienia
    unsigned long long block_size = (unsigned long long)vfs.f_frsize;
    unsigned long long total_bytes = block_size * vfs.f_blocks;
    unsigned long long used_bytes = block_size * (vfs.f_blocks - vfs.f_bfree);

    // oblicz rozmiary w GiB z zaokrągleniem (dodaj 0.5 GiB przed podziałem)
    unsigned total_gib = (total_bytes + (1ULL << 29)) >> 30;
    unsigned used_gib  = (used_bytes  + (1ULL << 29)) >> 30;

    snprintf(out, out_size, "/:       %2u/%2uGB", used_gib, total_gib);
}

void get_mem_usage(char *out, size_t out_size)
{
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) {
        snprintf(out, out_size, "RAM: error");
        return;
    }
    unsigned mem_total = 0, mem_available = 0;
    char line[128];
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "MemTotal: %u", &mem_total) == 1) continue;
        if (sscanf(line, "MemAvailable: %u", &mem_available) == 1) break;
    }
    fclose(fp);
    unsigned mem_used = mem_total - mem_available;
    snprintf(out, out_size, "RAM:   %3u/%3uMB", mem_used / 1024, mem_total / 1024);
}

void get_temp_and_load(char *buf, size_t buflen)
{
    // writes a 16-char CPU load and temperature string to buf
    //
    // format: "CPU: xx.x% yy.yC" (always null-terminated)
    //
    // buf:     output buffer (min. 17 bytes)
    // buflen:  size of buf
    //
    // returns: void

    if (!buf || buflen < 17) {
        if (buf && buflen > 0) buf[0] = '\0';
        return;
    }

    FILE *fp;
    float temp_c = 0.0f;
    float cpu_load = 0.0f;

    // get CPU temperature
    fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (fp) {
        int temp_raw;
        if (fscanf(fp, "%d", &temp_raw) == 1)
            temp_c = temp_raw / 1000.0f;
        fclose(fp);
    }

    // get 1-minute average CPU load
    fp = fopen("/proc/loadavg", "r");
    if (fp) {
        float load_avg;
        if (fscanf(fp, "%f", &load_avg) == 1) {
            cpu_load = load_avg * 100.0f;
            if (cpu_load > 99.9f)
                cpu_load = 99.9f;
        }
        fclose(fp);
    }

    // format: exactly 16 chars: "CPU:  99.9% 55.2C"
    // (CPU: + space + 5.1f%% + space + 4.1fC)
    snprintf(buf, buflen, "CPU: %4.1f%% %4.1fC", cpu_load, temp_c);
}