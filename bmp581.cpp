#include "bmp581.hpp"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdexcept>
#include <string>
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>
#include <iostream>

namespace
{
/*
 * The Bosch driver passes register payloads to the platform write callback.
 * Linux I2C writes need one additional byte at the beginning of the transfer:
 * the first byte is the register address and all following bytes are the
 * register data.  Keeping the limit here prevents the callback's stack buffer
 * from being overrun if the vendor driver ever requests an unusually large
 * transfer.
 */
constexpr std::size_t kMaximumRegisterWriteLength = 256;
}

Bmp581::Bmp581(int bus, uint8_t address)
    : fileDescriptor_(-1),
      device_{},
      configuration_{BMP5_OVERSAMPLING_4X,
                     BMP5_OVERSAMPLING_4X,
                     BMP5_ENABLE,
                     BMP5_ODR_25_HZ},
      initialized_(false)
{
    /*
     * Opening the adapter does not communicate with the sensor yet.  It only
     * gives this object a file descriptor for the Linux I2C subsystem.  Actual
     * sensor communication begins after the slave address is selected and the
     * Bosch driver's callbacks have been installed below.
     */
    const std::string devicePath = "/dev/i2c-" + std::to_string(bus);
    fileDescriptor_ = ::open(devicePath.c_str(), O_RDWR);
    if (fileDescriptor_ < 0)
    {
        throw std::runtime_error("Unable to open " + devicePath +
                                 ": " + std::strerror(errno));
    }

    /*
     * The Linux I2C device represents one selected slave at a time.  Selecting
     * the BMP581 address here means every subsequent read/write callback can
     * use the ordinary POSIX read and write calls without repeatedly changing
     * the address.  The ioctl also verifies that the address is valid for this
     * adapter before the object is returned to the caller.
     */
    if (::ioctl(fileDescriptor_, I2C_SLAVE, address) < 0)
    {
        const std::string error = "Unable to select BMP581 I2C address: " +
                                  std::string(std::strerror(errno));
        ::close(fileDescriptor_);
        fileDescriptor_ = -1;
        throw std::runtime_error(error);
    }

    /*
     * bmp5_dev is the state object expected by Bosch's C driver.  intf_ptr is
     * passed back to every callback, so storing this object there gives the
     * static callbacks access to the I2C file descriptor.  Static functions
     * are required because the C function-pointer types cannot point directly
     * to non-static C++ member functions, which have an implicit "this"
     * parameter.
     */
    device_.intf_ptr = this;
    device_.read = &Bmp581::readRegister;
    device_.write = &Bmp581::writeRegister;
    device_.delay_us = &Bmp581::delayMicroseconds;
    device_.intf = BMP5_I2C_INTF;
}

Bmp581::~Bmp581()
{
    /*
     * The file descriptor owns the connection to the Linux I2C adapter.  It
     * must be closed exactly once when the wrapper is destroyed; the guard also
     * makes cleanup safe if construction failed after open() succeeded.
     */
    if (fileDescriptor_ >= 0)
    {
        ::close(fileDescriptor_);
    }
}

int8_t Bmp581::initialize()
{
    initialized_ = false;

    // Reset the sensor even when Linux reboot did not remove sensor power.
    int8_t result = bmp5_soft_reset(&device_);
    if (result != BMP5_OK)
    {
        return result;
    }

    for (int attempt = 0; attempt < 5; ++attempt)
    {
        result = bmp5_init(&device_);

        if (result == BMP5_OK)
        {
            break;
        }

        if (result != BMP5_E_NVM_NOT_READY)
        {
            return result;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (result != BMP5_OK)
    {
        return result;
    }

    result = configure(configuration_);
    if (result != BMP5_OK)
    {
        return result;
    }

    result = bmp5_set_power_mode(BMP5_POWERMODE_CONTINUOUS, &device_);
    if (result == BMP5_OK)
    {
        initialized_ = true;
    }

    return result;
}

int8_t Bmp581::configure(
    const struct bmp5_osr_odr_press_config& configuration)
{
    /*
     * The configuration API relies on both transport callbacks.  Checking them
     * here makes a misuse of a partially constructed device explicit instead
     * of allowing a null function pointer to be called by the C driver.
     */
    if (device_.read == nullptr || device_.write == nullptr)
    {
        return BMP5_E_NULL_PTR;
    }

    /*
     * The BMP581 requires configuration changes to be made outside an active
     * conversion sequence.  Standby pauses measurements, then the Bosch
     * driver writes the oversampling, pressure-enable, and output-data-rate
     * fields as one validated configuration.
     */
    int8_t result = bmp5_set_power_mode(BMP5_POWERMODE_STANDBY, &device_);
    if (result != BMP5_OK)
    {
        return result;
    }

    result = bmp5_set_osr_odr_press_config(&configuration, &device_);
    if (result == BMP5_OK)
    {
        // Keep the cached configuration synchronized with the hardware.
        configuration_ = configuration;
    }
    return result;
}

int8_t Bmp581::readSensorData(struct bmp5_sensor_data& sensorData)
{
    if (!initialized_)
    {
        return BMP5_E_DEV_NOT_FOUND;
    }

    // uint8_t interruptStatus = 0;
    // int8_t result =
    //     bmp5_get_interrupt_status(&interruptStatus, &device_);

    // if (result != BMP5_OK)
    // {
    //     return result;
    // }

    // if ((interruptStatus & BMP5_INT_ASSERTED_DRDY) == 0)
    // {
    //     return BMP5_E_COM_FAIL;
    // }

    return bmp5_get_sensor_data(
        &sensorData,
        &configuration_,
        &device_);
}

bool Bmp581::isInitialized() const noexcept
{
    return initialized_;
}

BMP5_INTF_RET_TYPE Bmp581::readRegister(uint8_t registerAddress,
                                        uint8_t* data,
                                        uint32_t length,
                                        void* interfacePointer)
{
    /*
     * A register read is a two-part I2C transaction.  First, write the
     * register address so the sensor's internal pointer selects the requested
     * register.  Then read the requested number of bytes from that register.
     * The Linux I2C_SLAVE selection made by the constructor applies to both
     * operations.
     */
    if (data == nullptr || interfacePointer == nullptr)
    {
        return BMP5_E_NULL_PTR;
    }

    const auto* sensor = static_cast<const Bmp581*>(interfacePointer);
    if (::write(sensor->fileDescriptor_, &registerAddress, 1) != 1)
    {
        std::cerr << "I2C register write failed: "
              << std::strerror(errno) << '\n';
        // A short address write means the sensor was not selected reliably.
        return BMP5_E_COM_FAIL;
    }

    /*
     * POSIX read() may return fewer bytes than requested.  The Bosch callback
     * contract expects the complete register block, so a short read is
     * reported as a communication failure rather than returning partial data.
     */
    return (::read(sensor->fileDescriptor_, data, length) ==
            static_cast<ssize_t>(length))
               ? BMP5_INTF_RET_SUCCESS
               : BMP5_E_COM_FAIL;
}

BMP5_INTF_RET_TYPE Bmp581::writeRegister(uint8_t registerAddress,
                                         const uint8_t* data,
                                         uint32_t length,
                                         void* interfacePointer)
{
    /*
     * Linux expects register writes in one contiguous buffer.  The first byte
     * identifies the destination register and the remaining bytes contain the
     * payload supplied by the Bosch driver.
     */
    if (data == nullptr || interfacePointer == nullptr ||
        length > kMaximumRegisterWriteLength)
    {
        return (data == nullptr || interfacePointer == nullptr)
                   ? BMP5_E_NULL_PTR
                   : BMP5_E_COM_FAIL;
    }

    const auto* sensor = static_cast<const Bmp581*>(interfacePointer);
    uint8_t buffer[kMaximumRegisterWriteLength + 1];
    buffer[0] = registerAddress;
    std::copy(data, data + length, buffer + 1);

    /*
     * As with reads, a short write is not treated as success.  Accepting a
     * partial register update could leave a multi-byte sensor configuration in
     * an inconsistent state.
     */
    return (::write(sensor->fileDescriptor_, buffer, length + 1) ==
            static_cast<ssize_t>(length + 1))
               ? BMP5_INTF_RET_SUCCESS
               : BMP5_E_COM_FAIL;
}

void Bmp581::delayMicroseconds(uint32_t period, void*)
{
    /*
     * The Bosch driver uses this callback after reset and during operations
     * that require the sensor to settle.  sleep_for() yields the thread rather
     * than busy-waiting, which is appropriate for a Raspberry Pi application
     * running under Linux.
     */
    std::this_thread::sleep_for(std::chrono::microseconds(period));
}