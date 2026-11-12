#include "barometer.hpp"
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <cstdint>

Barometer::Barometer()
    :sensor1(1, BMP5_I2C_ADDR_SEC),
    sensor2(1, BMP5_I2C_ADDR_PRIM)
{
    /**
    if (sensor2.initialize() != BMP5_OK){
        throw std::runtime_error("Sensor 2 init failed");
    }
        **/
}

int8_t Barometer::initialize(){
    int8_t result = sensor1.initialize();

    if (result != BMP5_OK)
    {
        return result;
    }

    return BMP5_OK;
    // if (sensor2.initialize() != BMP5_OK){
    //     throw std::runtime_error("Sensor 2 init failed");
    // }
}

std::int64_t Barometer::get_time(){
    return time_ns_;
}

float Barometer::get_pressure(){
    return data.pressure;
}

float Barometer::get_temperature(){
    return data.temperature;
}
int8_t Barometer::update_data(){
    return get_data(data);
}
int8_t Barometer::get_data(struct bmp5_sensor_data& output){
    bmp5_sensor_data s1_dat{};
    // bmp5_sensor_data s2_dat{};
    int8_t result = sensor1.readSensorData(s1_dat);
    
    const auto now = std::chrono::system_clock::now();
    time_ns_ = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();

    if ( result == BMP5_OK)
    {
        // Use frontData.pressure and frontData.temperature.
        // data.pressure = s1_dat.pressure;
        // data.temperature = s1_dat.temperature;
        output = s1_dat;
        return result;
    }
    // if (sensor2.readSensorData(s2_dat) == BMP5_OK)
    // {
    //     // Use rearData.pressure and rearData.temperature.
    //     data.pressure = s2_dat.pressure;
    //     data.temperature = s2_dat.temperature;
    //     return BMP5_OK;
    // }
    return result;
}

int8_t Barometer::fix_sensor(Bmp581& sensor){
    return 0;
}