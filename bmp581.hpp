#ifndef BMP581_HPP_
#define BMP581_HPP_

#include "bmp5.h"

#include <cstdint>

/**
 * @brief Linux I2C wrapper for the Bosch BMP581 pressure sensor.
 *
 * The Bosch driver is intentionally platform independent: it asks the
 * application to provide read, write, and delay callbacks.  This class
 * supplies those callbacks using the Raspberry Pi Linux I2C device API and
 * keeps the C driver state private, so application code can work with a
 * normal C++ object instead of manually filling a bmp5_dev structure.
 */
class Bmp581
{
public:
    /**
     * @param bus I2C bus number. Raspberry Pi OS normally exposes the GPIO
     *             I2C bus as /dev/i2c-1.
     * @param address BMP5_I2C_ADDR_PRIM or BMP5_I2C_ADDR_SEC.
     *
     * @throws std::runtime_error if the device cannot be opened or selected.
     */
    explicit Bmp581(int bus = 1, uint8_t address = BMP5_I2C_ADDR_PRIM);
    ~Bmp581();

    Bmp581(const Bmp581&) = delete;
    Bmp581& operator=(const Bmp581&) = delete;
    Bmp581(Bmp581&&) = delete;
    Bmp581& operator=(Bmp581&&) = delete;

    /**
     * @brief Initializes the sensor and applies the default measurement setup.
     *
     * The sensor starts in continuous mode at 25 Hz, with 4x oversampling for
     * both pressure and temperature. Call this method before reading data.
     */
    int8_t initialize();

    /**
     * @brief Applies a new oversampling, pressure-enable, and ODR setup.
     *
     * The sensor is placed in standby while the configuration is changed.
     * The caller may use the constants from bmp5_defs.h to fill the structure.
     */
    int8_t configure(const struct bmp5_osr_odr_press_config& configuration);

    /**
     * @brief Reads the latest compensated pressure and temperature sample.
     * @return BMP5_OK on success, or a Bosch BMP5 error code.
     */
    int8_t readSensorData(struct bmp5_sensor_data& sensorData);

    /**
     * @brief Reports whether initialize() completed successfully.
     */
    bool isInitialized() const noexcept;

private:
    static BMP5_INTF_RET_TYPE readRegister(uint8_t registerAddress,
                                           uint8_t* data,
                                           uint32_t length,
                                           void* interfacePointer);
    static BMP5_INTF_RET_TYPE writeRegister(uint8_t registerAddress,
                                            const uint8_t* data,
                                            uint32_t length,
                                            void* interfacePointer);
    static void delayMicroseconds(uint32_t period, void* interfacePointer);

    int fileDescriptor_;
    struct bmp5_dev device_;
    struct bmp5_osr_odr_press_config configuration_;
    bool initialized_;
};

using BMP581 = Bmp581;

#endif