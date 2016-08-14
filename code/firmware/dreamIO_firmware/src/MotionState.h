/*MotionState
This class is designed to represent the physical state of the device
through analyzing the motion of the MPU6050.

Some of this functionality is supposed to be supported in the MPU6050 
but I haven't been able to get it working.
Ideally it would run on the MPU6050.
*/

#ifndef _MOTIONSTATE_H_
#define _MOTIONSTATE_H_

#include <math.h>
#include "I2Cdev.h"
#include "helper_3dmath.h" //needed for definition of types
#include "ImuDataContainer.h"
#include "OSCMessage.h"

#define sensitivity float(2000) //the curent sensativity of the gyroscope


class MotionState{
    public:
        //enum for storing current box side
        enum sides{BOTTOM, TOP, BACK, FRONT, LEFT, RIGHT}; //currently unused but should be turned into object

        MotionState();
        ~MotionState();

        void initialize(IMUData *ImuData, float accelThresh, float gyroThresh, long decayTime);
        void update();

        void setAccelThresh(float accelThresh);
        void setGyroThresh(float gyroThresh);
        void setMotionDecay(long decayTime);

        //OSC endpoint versions
        // currently not supported (callback doesn't like methods!)
        void setAccelThresh(OSCMessage &msg);
        void setGyroThresh(OSCMessage &msg);
        void setMotionDecay(OSCMessage &msg);

        // accessors
        uint8_t whichSide(); //checks the side the cube is on
        uint8_t isMotion(); //checks if cube is moving
        float * getGyro(); //scaled -1, 1
        float * getAccel(); //scaled -1, 1
        float * getYPR(); //scaled 0, 1 and only updates if there is motion

        static float int16ToFloat(int16_t x);
        static float fakefmod(float in, float mod);

    private:
        void _updateGyro();
        void _updateAccel();
        void _updateSide();
        void _updateMotion();
        void _updateYPR();

        //comes from outside the MPU6050 can fill
        IMUData * _ptrImuData;

        //thresholds
        float _accelThresh;
        float _gyroThresh;

        //internal representations
        float _accel[3];
        float _gyro[3];
        float _ypr[3];
        uint8_t _side;
        uint8_t _motionFlag;

        // for keeping track of things
        long _oldTime;
        long _motionDecay; //how long until motion is considered alive
        float _yprOffsets[3];
};

#endif /* _MOTIONSTATE_H_ */