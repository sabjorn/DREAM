#include "MotionState.h"

MotionState::MotionState():
            _motionFlag(0),
            _oldTime(0){

}

MotionState::~MotionState(){

}

void MotionState::initialize(IMUData *ImuData, long decayTime){
    _ptrImuData = ImuData;
    _motionDecay = decayTime;
}

void MotionState::update(){
    //update Gyro
    _updateGyro();

    //update Accel
    _updateAccel();

    //set side
    _updateSide();
    
    //check motion
    _updateMotion();

    //update YPR

}

void MotionState::_updateSide(){
    if(_ptrgravity->z < -.8)
        _side = 0; //bottom down
    else if(_ptrgravity->z > .8)
        _side = 1; //top
    else if(_ptrgravity->x < -.8)
        _side = 2; //back
    else if(_ptrgravity->x > .8)
        _side = 3; //front
    else if(_ptrgravity->y < -.8)
        _side = 4; //left
    else if(_ptrgravity->y > .8)
        _side = 5; //right
    else
        _side = -1; //error!
}

void MotionState::_updateGyro(){
    _gyro[0] = _ptrgyro->x/sensitivity;
    _gyro[1] = _ptrgyro->y/sensitivity;
    _gyro[2] = _ptrgyro->z/sensitivity;
}

void MotionState::_updateAccel(){
    _accel[0] = MotionState::int16ToFloat(_ptraccel->x);
    _accel[1] = MotionState::int16ToFloat(_ptraccel->y);
    _accel[2] = MotionState::int16ToFloat(_ptraccel->z);
}

void MotionState::_updateMotion(){
    _motionFlag = 0; //just in case

    int accu = 0;
    for(uint8_t i = 0; i < 3; i++){
        accu += (_accel[i] > _accelThresh);
        accu += (_gyro[i] > _gyroThresh);
    }

    _motionFlag = (accu > 0);
}



// converts Int16 to [-1, 1]
float MotionState::int16ToFloat(int16_t x){
  if(x > 0)
    return float(x)/float(INT16_MAX);
  else if(x < 0)
    return float(x)/float(INT16_MIN);
  else
    return 0;
}