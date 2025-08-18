#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "i2c.h"

// helper functions

static int i2c_get_fd(char *dev, int *fd);
static int i2c_release_fd(int fd);

static int i2c_set_device(int fd, int i2c_addr);
static int i2c_set_device_10bit(int fd, int i2c_addr);

static int i2c_get_fd(char *dev, int *fd)
{
    // get a file handle to the device

    int rc;
    char pathname[255];

    // define the path to open
    rc = snprintf(pathname, sizeof(pathname), "/dev/%s", dev);

    // check the filename
    if (rc < 0 || rc >= sizeof(pathname))
        return EXIT_FAILURE;

    // create a file descriptor for the I2C bus
    *fd = open(pathname, O_RDWR);

    // check the device handle
    if (*fd < 0)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

static int i2c_release_fd(int fd)
{
    // release the device file handle

    if (close(fd) < 0)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

static int i2c_set_device(int fd, int i2c_addr)
{
    // set the device address

    // set to 7-bit addr
    if (ioctl(fd, I2C_TENBIT, 0) < 0)
        return EXIT_FAILURE;

    // set the address
    if (ioctl(fd, I2C_SLAVE, i2c_addr) < 0)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

static int i2c_set_device_10bit(int fd, int i2c_addr)
{
    // set the 10-bit device address

    // set to 10-bit addr
    if (ioctl(fd, I2C_TENBIT, 1) < 0)
        return EXIT_FAILURE;

    // set the address
    if (i2c_set_device(fd, i2c_addr) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int i2c_write(char *dev, int i2c_addr, unsigned char *buf, int count)
{
    // generic function to write a buffer to the I2C bus (no in-device address)

    int rc;
    int fd;

    // open the file handle
    rc = i2c_get_fd(dev, &fd);

    // set the device address
    if (rc == EXIT_SUCCESS)
        rc = i2c_set_device(fd, i2c_addr);

    // perform the write
    if (rc == EXIT_SUCCESS) {
        // write to the I2C device
        rc = write(fd, buf, count);
        if (rc != count)
            rc = EXIT_FAILURE;
        else
            rc = EXIT_SUCCESS;
    }

    // release the device file handle
    rc |= i2c_release_fd(fd);

    return rc;
}

int i2c_write_reg(char *dev, int i2c_addr, int reg, int value)
{
    // write a value to the register of the I2C device.

    int rc;
    int count, tmp, i;
    unsigned char buf[I2C_BUFFER_SIZE];

    // buffer setup
    memset(buf, 0, sizeof(buf));	// clear the buffer
    // push the address and data values into the buffer
    buf[0] = (reg & 0xff);
    buf[1] = (value & 0xff);
    count = 2;

    // if value is more than 1-byte, add to the buffer
    tmp = (value >> 8);	// start with byte 1
    i = 2;
    while (tmp > 0x00) {
        buf[i] = (unsigned char)(tmp & 0xff);

        tmp = tmp >> 8;	// advance the tmp data by a byte
        i++;		    // increment the index

        count++;	    // increase the size
    }

    // write the buffer
    rc = i2c_write(dev, i2c_addr, buf, count);

    return rc;
}

int i2c_write_nreg(char *dev, int i2c_addr, int reg, unsigned char *buf, int count)
{
    // generic function to write a buffer to the I2C bus

    int rc;
    unsigned char *buf_new;

    // allocate the new buffer
    count++;            // adding reg to buffer
    buf_new = malloc(count * sizeof(*buf_new));

    // add the address to the data buffer
    buf_new[0] = reg;
    memcpy(&buf_new[1], &buf[0], (count - 1) * sizeof(*buf));

    // perform the write
    rc = i2c_write(dev, i2c_addr, buf_new, count);

    // free the allocated memory
    free(buf_new);

    return rc;
}

int i2c_read(char *dev, int i2c_addr, unsigned char *buf, int count)
{
    // read raw bytes from the I2C bus (no in-device address)

    int rc;
    int fd;

    // open the device file handle
    rc = i2c_get_fd(dev, &fd);

    // set the device address
    if (rc == EXIT_SUCCESS)
        rc = i2c_set_device(fd, i2c_addr);

    // perform the read
    if (rc == EXIT_SUCCESS) {
        // read data
        // clear the buffer
        memset(buf, 0, count);

        // read from the I2C device
        rc = read(fd, buf, count);
        if (rc != count)
            rc = EXIT_FAILURE;
        else
            rc = EXIT_SUCCESS;
    }

    // release the device file handle
    rc |= i2c_release_fd(fd);

    return rc;
}

int i2c_read_reg(char *dev, int i2c_addr, int reg, int *value)
{
    // read a single byte from a register of the I2C device

    int rc;
    unsigned char buf[I2C_BUFFER_SIZE];

    rc = i2c_read_nreg(dev, i2c_addr, reg, buf, 1);

    *value = (int)(buf[0]);

    return rc;
}

int i2c_read_nreg(char *dev, int i2c_addr, int reg, unsigned char *buf, int count)
{
    // read data from a register of the I2C device

    int rc;
    int fd;

    // open the device file handle
    rc = i2c_get_fd(dev, &fd);

    // set the device address
    if (rc == EXIT_SUCCESS)
        rc = i2c_set_device(fd, i2c_addr);

    // perform the read
    if (rc == EXIT_SUCCESS) {
        // set addr
        // clear the buffer
        memset(buf, 0, count);
        // push the address into the buffer
        buf[0] = (reg & 0xff);

        // write to the I2C device
        rc = write(fd, buf, 1);

        // read data
        if (rc != 1)
            rc = EXIT_FAILURE;
        else {
            // clear the buffer
            memset(buf, 0, count);

            // read from the I2C device
            rc = read(fd, buf, count);
            if (rc != count)
                rc = EXIT_FAILURE;
            else
                rc = EXIT_SUCCESS;
        }
    }

    // release the device file handle
    rc |= i2c_release_fd(fd);

    return rc;
}

int i2c_mask_reg(char *dev, int i2c_addr, int reg, int mask)
{
    // apply a mask to a register of the I2C device

    int rc;
    int value = 0;

    // read from the I2C device
    rc = i2c_read_reg(dev, i2c_addr, reg, &value);

    // apply the mask
    if (rc == EXIT_SUCCESS) {
        value |= mask;
        // write to the I2C device
        rc = i2c_write_reg(dev, i2c_addr, reg, value);
    }

    return rc;
}