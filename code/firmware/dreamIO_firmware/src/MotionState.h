/*MotionState
This class is designed to represent the physical state of the device
through analyzing the motion of the MPU6050.

Some of this functionality is supposed to be supported in the MPU6050 
but I haven't been able to get it working.
Ideally it would run on the MPU6050.
*/

#ifndef _MOTIONSTATE_H_
#define _MOTIONSTATE_H_

#include "I2Cdev.h"
#include "helper_3dmath.h" //needed for definition of types

#define sensitivity float(2000) //the curent sensativity of the gyroscope

class MotionState{
    public:
        MotionState();
        ~MotionState();

        void initialize(VectorInt16 *accel, VectorInt16 *gyro, float *ypr, float accelthresh, float gyrothresh, long decay);
        void update();

        void setAccelThresh();
        void setGyroThresh();
        void setMotionDecay();

        // accessors
        uint8_t whichSide(); //checks the side the cube is on
        uint8_t isMotion(); //checks if cube is moving
        
        float * getGyro(); //scaled -1, 1
        float * getAccel(); //scaled -1, 1
        float * getYPR(); //scaled 0, 1 and only updates if there is motion

        static float scaleInt16(int16_t x);

    private:
        // these come from outside the MPU6050 can fill them up
        VectorInt16 * _ptraccel; //pointer to raw acceleration values
        VectorInt16 * _ptrgyro; //pointer to raw gyroscope values
        float * _ptrypr; //pointer to unalter ypr

        float _accel[3];
        float _gyro[3];
        float _ypr[3];
        
        uint8_t _motionflag;

        long _motion_decay; //how long until motion is considered alive
        long _old_time;
        float _accel_thresh;
        float _gyro_thresh;
};

#endif /* _MOTIONSTATE_H_ */