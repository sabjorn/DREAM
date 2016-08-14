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
#include "ImuDataContainer.h"

#define sensitivity float(2000) //the curent sensativity of the gyroscope


class MotionState{
    public:
        //enum for storing current box side
        enum sides{BOTTOM, TOP, BACK, FRONT, LEFT, RIGHT}; //currently unused but should be turned into object

        MotionState();
        ~MotionState();

        void initialize(IMUData *ImuData, long decayTime);
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

        static float int16ToFloat(int16_t x);

    private:
        void _updateGyro();
        void _updateAccel();
        void _updateSide();
        void _updateMotion();

        // these come from outside the MPU6050 can fill them up
        IMUData * _ptrImuData;
        VectorInt16 * _ptraccel; //pointer to raw acceleration values
        VectorInt16 * _ptrgyro; //pointer to raw gyroscope values
        VectorFloat * _ptrgravity;
        float * _ptrypr; //pointer to unalter ypr

        float _accel[3];
        float _gyro[3];
        float _ypr[3];
        uint8_t _side;
        
        uint8_t _motionFlag;

        long _oldTime;
        long _motionDecay; //how long until motion is considered alive
        float _accelThresh;
        float _gyroThresh;
};

#endif /* _MOTIONSTATE_H_ */