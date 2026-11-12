#ifndef ICM40609D_HPP
#define ICM40609D_HPP
#include <cstdint>

constexpr uint8_t REG_DEVICE_CONFIG   = 0x11;
constexpr uint8_t REG_TEMP_DATA1      = 0x1D;
constexpr uint8_t REG_ACCEL_DATA_X1   = 0x1F; // 0x1F..0x24 (Accel X, Y, Z)
constexpr uint8_t REG_GYRO_DATA_X1    = 0x25; // 0x25..0x2A (Gyro X, Y, Z)
constexpr uint8_t REG_PWR_MGMT0       = 0x4E;
constexpr uint8_t REG_GYRO_CONFIG0    = 0x4F;
constexpr uint8_t REG_ACCEL_CONFIG0   = 0x50;
constexpr uint8_t REG_WHO_AM_I        = 0x75;
constexpr uint8_t REG_BANK_SEL        = 0x76;

class ICM40609D{
    public:
        ICM40609D(const char* device = "dev/spidev0.0", uint32_t speed_hz = 8000000);
        ~ICM40609D();
        uint8_t read_register(uint8_t reg);
        void write_register(uint8_t reg, uint8_t value);
        void burst_read(uint8_t start_reg, uint8_t* buffer, size_t length);
        bool initialize();
        void readIMU(float accel_g[3], float gyro_dps[3]);
    private:
        int fd_ = -1;
        uint32_t speed_;
};

#endif