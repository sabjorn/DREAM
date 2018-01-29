/*SideValue
This class is responsible for keeping track of the parameter output
per side of the dreamIO board.

If the board is placed in a box, each side of the box can act 
as a separate parameter control (think of it like a knob).
This class helps keep track of each side 
(it is designed to be in a list inside MotionState)

The parameter value is 'tab stopped', as in it is clamped between a min/max
This min/max defines how many turns the side can rotate (like an infinite encoder)
*/

#ifndef _SIDEVALUE_H_
#define _SIDEVALUE_H_

#include <cmath>

class SideValue{
    public:
        SideValue();
        ~SideValue();

        void update(float inputVal, int sideChangeFlag);
        float getVal();
        void setMin(float minRot);
        void setMax(float maxRot);

    private:
        
        float _min, _max;
        float _oldInput;
        float _val;
        
};

#endif /* _SIDEVALUE_H_ */