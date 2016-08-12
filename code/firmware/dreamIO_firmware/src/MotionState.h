/*MotionState
This class is designed to analyze the motion of the MPU6050,
determine if the device is moving, and then store this state.

This should include hysterisis and/or 
time based measurements (minimum amount of transition time after tracking motion)

This functionality is supposed to be supported in the MPU6050 but 
I haven't been able to get it working.
Ideally it would run on the MPU6050 
*/

#ifndef _MOTIONSTATE_H_
#define _MOTIONSTATE_H_

#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h" //needed for definition of types

class MotionState{
    public:
        MotionState();
        ~MotionState();

        void initialize(VectorInt16 *accel, float accelthresh, VectorInt16 *gyro, float gyrothresh);

        float accel_thresh, gyro_thresh;

    private:
        VectorInt16 * accel_;
        VectorInt16 * gyro_;
};

#endif /* _MOTIONSTATE_H_ */