#include "SideValue.h"

SideValue::SideValue(): _min(0), _max(1), _oldInput(0), _val(0){

}

SideValue::~SideValue(){

}

void SideValue::update(float inputVal, int sideChangeFlag){
    //prevent jump if just moved to new side
    if(sideChangeFlag)
        _oldInput = inputVal;

    float diff = _oldInput - inputVal;

    // make sure there isn't a massive jump
    // which would indicate full rotation
    if(std::abs(diff) < 0.8)
        _val += diff;

    if (_val > _max)
        _val = _max;
    else if (_val < _min)
        _val = _min;

    _oldInput = inputVal;
}

float SideValue::getVal(){
    return _val;
}

void SideValue::setMin(float minRot){
    _min = minRot;
}

void SideValue::setMax(float maxRot){
    _max = maxRot;
}