#include <linux/gpio.h>
#include <sys/ioctl.h>

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "gpio.h"

#define CONSUMER "gpio-tools"

int gpio_request_line(char *dev, int *lines, int count, struct gpio_v2_line_config *config, int *fd)
{
    // request GPIO lines in a GPIO chip

    struct gpio_v2_line_request req;
    char pathname[255];
    int i;
    int rc;

    // define the path to open
    rc = snprintf(pathname, sizeof(pathname), "/dev/%s", dev);

    // check the filename
    if (rc < 0 || rc >= sizeof(pathname))
        return EXIT_FAILURE;

    // create a file descriptor for the GPIO character device
    *fd = open(pathname, O_RDONLY);

    // check the device handle
    if (*fd < 0)
        return EXIT_FAILURE;

    memset(&req, 0, sizeof(req));
    for (i = 0; i < count; i++)
        req.offsets[i] = lines[i];
    req.config = *config;
    strcpy(req.consumer, CONSUMER);
    req.num_lines = count;

    // request the GPIO lines in the chip
    if (ioctl(*fd, GPIO_V2_GET_LINE_IOCTL, &req) < 0)
        return EXIT_FAILURE;

    // close GPIO character device file
    if (close(*fd) < 0)
        return EXIT_FAILURE;

    *fd = req.fd;

    return EXIT_SUCCESS;
}

int gpio_set_values(int fd, struct gpio_v2_line_values *values)
{
    // set the value of GPIO(s)

    if (ioctl(fd, GPIO_V2_LINE_SET_VALUES_IOCTL, values) < 0)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int gpio_get_values(int fd, struct gpio_v2_line_values *values)
{
    // get the value of GPIO(s)

    if (ioctl(fd, GPIO_V2_LINE_GET_VALUES_IOCTL, values) < 0)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int gpio_release_line(int fd)
{
    // release the line(s) of gpiochip

    if (close(fd) < 0)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int gpio_get(char *dev, int line, int *value)
{
    // get value from specific line

    int lines[] = { line };

    return gpio_getn(dev, lines, 1, value);
}

int gpio_getn(char *dev, int *lines, int count, int *values)
{
    // get values from specific lines

    int fd, i;
    int rc;
    struct gpio_v2_line_config config;
    struct gpio_v2_line_values lv;

    memset(&config, 0, sizeof(config));
    config.flags = GPIO_V2_LINE_FLAG_INPUT;
    rc = gpio_request_line(dev, lines, count, &config, &fd);

    if (rc == EXIT_SUCCESS) {
        for (i = 0; i < count; i++)
            gpio_set_bit(&lv.mask, i);
        rc = gpio_get_values(fd, &lv);
        if (rc == EXIT_SUCCESS)
            for (i = 0; i < count; i++)
                values[i] = gpio_test_bit(lv.bits, i);
    }

    rc |= gpio_release_line(fd);

    return rc;
}

int gpio_set(char *dev, int line, int value)
{
    // set value to specific line

    int lines[] = { line };

    return gpio_setn(dev, lines, 1, &value);
}

int gpio_setn(char *dev, int *lines, int count, int *values)
{
    // set values to specific lines

    int fd, i;
    int rc;
    struct gpio_v2_line_config config;

    memset(&config, 0, sizeof(config));
    config.flags = GPIO_V2_LINE_FLAG_OUTPUT;
    config.num_attrs = 1;
    config.attrs[0].attr.id = GPIO_V2_LINE_ATTR_ID_OUTPUT_VALUES;
    for (i = 0; i < count; i++) {
        gpio_set_bit(&config.attrs[0].mask, i);
        gpio_assign_bit(&config.attrs[0].attr.values, i, values[i]);
    }

    rc = gpio_request_line(dev, lines, count, &config, &fd);
    rc |= gpio_release_line(fd);

    return rc;
}