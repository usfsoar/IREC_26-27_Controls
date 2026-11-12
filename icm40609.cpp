#include "icm40609d.hpp"
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>


ICM40609D::ICM40609D(const char* device = "dev/spidev0.0", uint32_t speed_hz = 8000000)
: speed_(speed_hz) {
    fd_ = open(device, O_RDWR);
    if(fd_ < 0){
        throw std::runtime_error("Failed to open SPI device");
    }

    uint8_t mode = SPI_MODE_0;
    uint8_t bits = 8;
    ioctl(fd_, SPI_IOC_WR_MODE, &mode);
    ioctl(fd_, SPI_IOC_WR_BITS_PER_WORD, &bits);
    ioctl(fd_, SPI_IOC_WR_MAX_SPEED_HZ, &speed_);
}

ICM40609D::~ICM40609D(){
    if(fd_ >= 0) clsoe(fd_);
}

uint8_t ICM40609D::read_register(uint8_t reg){
    uint8_t tx[2] = {static_cast<uint_t>(reg | 0x80), 0x00};
    uint8_t rx[2] = {0,0};

    struct spi_ioc_transfer tr{};
    tr.tx_buf = reinterpret_cast<unsigned long>(tx);
    tr.rx_buf = reinterpret_cast<unsigned long>(rx);
    tr.len = 2;
    tr.speed_hz = speed_;
    tr.bits_per_word = 8;

    if(ioctl(fd_, SPI_IOC_MESSAGE(1), &tr)<1){
        throw std::runtime_error("SPI read failed");
    }
    return rx[1];
}

void ICM40609D::write_register(uint8_t reg, uint8_t value){
    uint8_t tx[2] = {static_cast<uint_8>(reg & 0x7F), value};

    struct spi_ioc_transfer tr{};
    tr.tx_buf = reinterpret_cast<unsigned long>(tx);
    tr.len = 2;
    tr.speed_hz = speed_;
    tr.bits_per_word = 8;

    if(ioctl(fd_,SPI_IOC_MESSAGE(1),&tr) < 1){
        throw std::runtime_error("SPI write failed");
    }
}

void ICM40609D::burst_read(uint8_t start_reg, uint8_t* buffer, size_t length){
    uint8_t tx[15] = {0};
    uint8_t rx[15] = {0};
    tx[0] = start_reg | 0x80;

    struct spi_ioc_transfer tr{};
    tr.tx_buf = reinterpret_cast<unsigned long>(tx);
    tr.rx_buf = reinterpret_cast<unsigned long>(rx);
    tr.len = length + 1;
    tr.speed_hz = speed_;
    tr.bits_per_word = 8;

    if (ioctl(fd_, SPI_IOC_MESSAGE(1), &tr) < 1) {
        throw std::runtime_error("SPI burst read failed");
    }
    std::memcpy(buffer, &rx[1], length);
}

bool ICM40609D::initialize(){
    write_register(REG_BANK_SEL, 0x00);
    uint8_t whomst = read_register(REG_WHO_AM_I);

    if(whomst != 0x3B && whomst != 0x47){
        return false;
    }

    write_register(REG_DEVICE_CONFIG, 0x01);
    usleep(2000);

    write_register(REG_GYRO_CONFIG0, 0x06);
    write_register(REG_ACCEL_CONFIG0, 0x26);
    write_register(REG_PWR_MGMT0, 0x0F);
    usleep(5000);

    return true;
}

void ICM40609D::readIMU(float accel_g[3], float gyro_dps[3]){
    uint8_t raw[12];
    burst_read(REG_ACCEL_DATA_X1, raw, 12);

    int16_t ax = (static_cast<int16_t>(raw[0]) << 8) | raw[1];
    int16_t ay = (static_cast<int16_t>(raw[2]) << 8) | raw[3];
    int16_t az = (static_cast<int16_t>(raw[4]) << 8) | raw[5];

    int16_t gx = (static_cast<int16_t>(raw[6]) << 8) | raw[7];
    int16_t gy = (static_cast<int16_t>(raw[8]) << 8) | raw[9];
    int16_t gz = (static_cast<int16_t>(raw[10]) << 8) | raw[11];

    constexpr float ACCEL_SCALE = 1.0f / 2048.0f;
    constexpr float GYRO_SCALE  = 1.0f / 16.4f;

    accel_g[0] = ax * ACCEL_SCALE;
    accel_g[1] = ay * ACCEL_SCALE;
    accel_g[2] = az * ACCEL_SCALE;

    gyro_dps[0] = gx * GYRO_SCALE;
    gyro_dps[1] = gy * GYRO_SCALE;
    gyro_dps[2] = gz * GYRO_SCALE;
}