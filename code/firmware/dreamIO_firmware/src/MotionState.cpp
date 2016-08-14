#include "MotionState.h"

MotionState::MotionState(){

}

MotionState::~MotionState(){

}

void MotionState::initialize(IMUData *ImuData, float accelThresh, float gyroThresh, long decayTime){
    
    _ptrImuData = ImuData;
    _accelThresh = accelThresh;
    _gyroThresh = gyroThresh;
    _motionDecay = decayTime;

    _side = 0;
    _motionFlag = 0;
    _oldTime = 0;

    for(uint8_t i = 0; i < 3; ++i){
        _accel[i] = 0;
        _gyro[i] = 0;
        _ypr[i] = 0;
        _yprOffsets[i] = 0;
    }

}

void MotionState::update(){
    //update Gyro
    _updateGyro();

    // //update Accel
    _updateAccel();

    // //set side
    _updateSide();
    
    // //check motion
    _updateMotion();

    // //update YPR
    _updateYPR();

}

uint8_t MotionState::whichSide(){
    return _side;
}

uint8_t MotionState::isMotion(){
    return _motionFlag;
}

float * MotionState::getGyro(){
    return _gyro;
}

float * MotionState::getAccel(){
    return _accel;
}

float * MotionState::getYPR(){
    return _ypr;
}

void MotionState::setAccelThresh(float accelThresh)
{
    _accelThresh = accelThresh;
}

void MotionState::setAccelThresh(OSCMessage &msg)
{
    if(msg.isFloat(0))
        _accelThresh = msg.getFloat(0);
}

void MotionState::setGyroThresh(float gyroThresh){
    _gyroThresh = gyroThresh;
}

void MotionState::setGyroThresh(OSCMessage &msg){
    if(msg.isFloat(0))
        _gyroThresh = msg.getFloat(0);
}

void MotionState::setMotionDecay(long decayTime){
    _motionDecay = decayTime;
}

void MotionState::setMotionDecay(OSCMessage &msg){
    if(msg.isInt(0))
        _motionDecay = msg.getInt(0);
    else if(msg.isFloat(0))
        _motionDecay = int(msg.getFloat(0));
}



void MotionState::_updateSide(){
    if(_ptrImuData->gravity.z < -.8)
        _side = 0; //bottom down
    else if(_ptrImuData->gravity.z > .8)
        _side = 1; //top
    else if(_ptrImuData->gravity.x < -.8)
        _side = 2; //back
    else if(_ptrImuData->gravity.x > .8)
        _side = 3; //front
    else if(_ptrImuData->gravity.y < -.8)
        _side = 4; //left
    else if(_ptrImuData->gravity.y > .8)
        _side = 5; //right
    else
        _side = -1; //error!
}

void MotionState::_updateGyro(){
    _gyro[0] = _ptrImuData->gyro.x/sensitivity;
    _gyro[1] = _ptrImuData->gyro.y/sensitivity;
    _gyro[2] = _ptrImuData->gyro.z/sensitivity;
}

void MotionState::_updateAccel(){
    _accel[0] = MotionState::int16ToFloat(_ptrImuData->aaReal.x);
    _accel[1] = MotionState::int16ToFloat(_ptrImuData->aaReal.y);
    _accel[2] = MotionState::int16ToFloat(_ptrImuData->aaReal.z);
}

void MotionState::_updateMotion(){
    int accu = 0;
    for(uint8_t i = 0; i < 3; ++i){
        accu += (abs(_accel[i]) > _accelThresh);
        accu += (abs(_gyro[i]) > _gyroThresh);
    }
    _motionFlag = (accu > 0);

    if(_motionFlag)
        _oldTime = millis(); //clock resets from motion
    else if(!_motionFlag){
        long now = millis();
        if (now - _oldTime < _motionDecay)
            _motionFlag = 1; //keep 1 if under time
    }
}

void MotionState::_updateYPR(){
    for(uint8_t i = 0; i < 3; ++i){
        //scale [0, 1]
        float scale = 0;
        if (i == 0)
            scale = ((_ptrImuData->ypr[i] / M_PI) + 1) / 2;
        else
            scale = ((_ptrImuData->ypr[i] / (M_PI/2.)) + 1) / 2;

        // This should stabilize the YPR measurements when not moving
        // but right now it doesn't work.
        // if(_motionFlag)
        //     _ypr[i] = MotionState::fakefmod(scale + _yprOffsets[i], 1);
        // else
        //     _yprOffsets[i] = _ypr[i] - scale;

        //'regular' YPR
        _ypr[i] = scale;
    }

}

/*Static Members*/
// converts Int16 to [-1, 1]
float MotionState::int16ToFloat(int16_t x){
  if(x > 0)
    return float(x)/float(INT16_MAX);
  else if(x < 0)
    return float(x)/float(INT16_MIN);
  else
    return 0;
}

float MotionState::fakefmod(float in, float mod){
    return ((int)(in * 100) % (int)(mod * 100))/100.0;
}