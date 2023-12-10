#pragma once
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_LSM6DSL.h>

// PINS payload v2  (if not used just comment it out don't delete)
int _SDA = 14;
int _SCL = 15;
TwoWire *_WIRE = &Wire1;
int imu_address = 0x6B;

void start()
{
    //_WIRE->setSCL(15);
    //_WIRE->setSDA(14);
    Adafruit_LSM6DS _imu;

    if (!_imu.begin_I2C(imu_address, _WIRE))
    {
        Serial.println("IMU ERROR");
    }
    else
    {
        Serial.println("IMU STARTED");
    }
    // set range
    _imu.setAccelRange(LSM6DS_ACCEL_RANGE_16_G);
    _imu.setGyroRange(LSM6DS_GYRO_RANGE_1000_DPS);
    // check if it was set

    switch (_imu.getAccelRange())
    {
    case LSM6DS_ACCEL_RANGE_2_G:
        Serial.println("+-2G");
        break;
    case LSM6DS_ACCEL_RANGE_4_G:
        Serial.println("+-4G");
        break;
    case LSM6DS_ACCEL_RANGE_8_G:
        Serial.println("+-8G");
        break;
    case LSM6DS_ACCEL_RANGE_16_G:
        Serial.println("+-16G");
        break;
    }

    switch (_imu.getGyroRange())
    {
    case LSM6DS_GYRO_RANGE_125_DPS:
        Serial.println("+-125DPS");
        break;
    case LSM6DS_GYRO_RANGE_250_DPS:
        Serial.println("+-250DPS");
        break;
    case LSM6DS_GYRO_RANGE_500_DPS:
        Serial.println("+-500DPS");
        break;
    case LSM6DS_GYRO_RANGE_1000_DPS:
        Serial.println("+-1000DPS");
        break;
    }
    // set data rate

    _imu.setAccelDataRate(LSM6DS_RATE_104_HZ);
    Serial.print("Accelerometer data rate set to: ");
    switch (_imu.getAccelDataRate())
    {
    case LSM6DS_RATE_SHUTDOWN:
        Serial.println("0 Hz");
        break;
    case LSM6DS_RATE_12_5_HZ:
        Serial.println("12.5 Hz");
        break;
    case LSM6DS_RATE_26_HZ:
        Serial.println("26 Hz");
        break;
    case LSM6DS_RATE_52_HZ:
        Serial.println("52 Hz");
        break;
    case LSM6DS_RATE_104_HZ:
        Serial.println("104 Hz");
        break;
    case LSM6DS_RATE_208_HZ:
        Serial.println("208 Hz");
        break;
    case LSM6DS_RATE_416_HZ:
        Serial.println("416 Hz");
        break;
    case LSM6DS_RATE_833_HZ:
        Serial.println("833 Hz");
        break;
    case LSM6DS_RATE_1_66K_HZ:
        Serial.println("1.66 KHz");
        break;
    case LSM6DS_RATE_3_33K_HZ:
        Serial.println("3.33 KHz");
        break;
    case LSM6DS_RATE_6_66K_HZ:
        Serial.println("6.66 KHz");
        break;
    }

    _imu.setGyroDataRate(LSM6DS_RATE_104_HZ);
    Serial.print("Gyro data rate set to: ");
    switch (_imu.getGyroDataRate())
    {
    case LSM6DS_RATE_SHUTDOWN:
        Serial.println("0 Hz");
        break;
    case LSM6DS_RATE_12_5_HZ:
        Serial.println("12.5 Hz");
        break;
    case LSM6DS_RATE_26_HZ:
        Serial.println("26 Hz");
        break;
    case LSM6DS_RATE_52_HZ:
        Serial.println("52 Hz");
        break;
    case LSM6DS_RATE_104_HZ:
        Serial.println("104 Hz");
        break;
    case LSM6DS_RATE_208_HZ:
        Serial.println("208 Hz");
        break;
    case LSM6DS_RATE_416_HZ:
        Serial.println("416 Hz");
        break;
    case LSM6DS_RATE_833_HZ:
        Serial.println("833 Hz");
        break;
    case LSM6DS_RATE_1_66K_HZ:
        Serial.println("1.66 KHz");
        break;
    case LSM6DS_RATE_3_33K_HZ:
        Serial.println("3.33 KHz");
        break;
    case LSM6DS_RATE_6_66K_HZ:
        Serial.println("6.66 KHz");
        break;
    }

    Serial.println("Starting reading");

    while (true)
    {
        // Read the sensor
        sensors_event_t accel;
        sensors_event_t gyro;
        sensors_event_t temp;
        _imu.getEvent(&accel, &gyro, &temp);

        /* Display the results (acceleration is measured in m/s^2) */
        Serial.print("\t\tAccel X: ");
        Serial.print(accel.acceleration.x);
        Serial.print(" \tY: ");
        Serial.print(accel.acceleration.y);
        Serial.print(" \tZ: ");
        Serial.print(accel.acceleration.z);
        Serial.println(" m/s^2 ");

        /* Display the results (rotation is measured in rad/s) */
        Serial.print("\t\tGyro X: ");
        Serial.print(gyro.gyro.x);
        Serial.print(" \tY: ");
        Serial.print(gyro.gyro.y);
        Serial.print(" \tZ: ");
        Serial.print(gyro.gyro.z);
        Serial.println(" radians/s ");
        Serial.println();

        delay(500);
    }
}
