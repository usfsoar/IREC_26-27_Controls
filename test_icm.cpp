#include "icm40609d.hpp"
#include <iostream>
#include <fcntl.h>
#include <unistd.h>

int main() {
    try {
        ICM40609D imu("/dev/spidev0.0");
        if (!imu.initialize()) {
            std::cerr << "ICM-40609-D not detected.\n";
            return 1;
        }

        float accel[3], gyro[3];
        while (true) {
            imu.readIMU(accel, gyro);
            std::cout << "Accel [g]: " << accel[0] << ", " << accel[1] << ", " << accel[2]
                      << " | Gyro [dps]: " << gyro[0] << ", " << gyro[1] << ", " << gyro[2] << "\n";
            usleep(100000); // 10 Hz print rate
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}