#include "MotionState.h"

MotionState::MotionState()//:
            //ptraccel_(Null),
            //ptrgyro_(Null)
{

}

MotionState::~MotionState(){

}

// void MotionState::initialize(VectorInt16 *accel, 
//                              VectorInt16 *gyro, 
//                              float *ypr, 
//                              float accelthresh, 
//                              float gyrothresh, 
//                              long decay){
    

// }


// converts Int16 to [-1, 1]
// float MotionState::scaleInt16(int16_t x){
//   if(x > 0)
//     return float(x)/float(INT16_MAX);
//   else if(x < 0)
//     return float(x)/float(INT16_MIN);
//   else
//     return 0;
// }