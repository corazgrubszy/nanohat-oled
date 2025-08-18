#pragma once

#define I2C_BUFFER_SIZE 256

// I2C functions

int i2c_write(char *dev, int i2c_addr, unsigned char *buf, int count);
int i2c_write_reg(char *dev, int i2c_addr, int reg, int value);
int i2c_write_nreg(char *dev, int i2c_addr, int reg, unsigned char *buf, int count);

int i2c_read(char *dev, int i2c_addr, unsigned char *buf, int count);
int i2c_read_reg(char *dev, int i2c_addr, int reg, int *value);
int i2c_read_nreg(char *dev, int i2c_addr, int reg, unsigned char *buf, int count);

int i2c_mask_reg(char *dev, int i2c_addr, int reg, int mask);