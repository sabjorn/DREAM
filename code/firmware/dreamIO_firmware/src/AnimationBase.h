/*AnimationBase
An abstract base class for designing different animations
*/

#ifndef _ANIMATIONBASE_H_
#define _ANIMATIONBASE_H_


class AnimationBase{
    public:
        AnimationBase();
        virtual ~AnimationBase();
        
        virtual void update();

    private:

};

#endif /* _ANIMATIONBASE_H_ */