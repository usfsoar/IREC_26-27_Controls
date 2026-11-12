#ifndef BAROMETER_HPP
#define BAROMETER_HPP
#include "bmp581.hpp"
#include <cstdint>

class Barometer
{
public:
    explicit Barometer();
    ~Barometer() = default;

    int8_t initialize();

    std::int64_t get_time();
    float get_altitude();
    float get_pressure();
    float get_temperature();
    int8_t update_data();
private:
    bmp5_sensor_data data{};
    int8_t get_data(struct bmp5_sensor_data& data);
    int8_t fix_sensor(Bmp581& sensor);
    Bmp581 sensor1;
    Bmp581 sensor2;
    std::int64_t time_ns_{0};
};
#endif