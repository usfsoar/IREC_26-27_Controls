#include "barometer.hpp"
#include <stdio.h>
#include <iostream>
#include <chrono>
#include <thread>

int main()
{
    try
    {
        Barometer bmp;
        int8_t result = bmp.initialize();
        if (result != BMP5_OK)
        {
            std::cerr << "Initialization failed: "
                      << static_cast<int>(result) << '\n';
            return 1;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        while(1){
            result = bmp.update_data();
            if (result != BMP5_OK){
                std::cerr << "Sensor read failed: "
                << static_cast<int>(result) << '\n';
                return 1;
            }
            std::cout << "Unix time(ns)" << bmp.get_time() << '\n';
            std::cout << "Pressure: " << bmp.get_pressure() << '\n';
            std::cout << "Temperature: " << bmp.get_temperature() << '\n';
        }
       
    }
    catch (const std::exception& error)
    {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}