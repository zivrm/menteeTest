#ifndef MOTOR_H
#define MOTOR_H

#pragma once
#define MOTORS std::vector<motor> *motors
class motor
{
public:
    motor(){
        motor(0.0);
    }
    motor(float speed){
        speed_ = speed;
    }
    float getPosition(){
        return position;
    }
    // args should be a float value for new position
    void setPosition( float newPosition){ 
        position = newPosition;
    }

    void updatePosition( ){ 
        position += speed_;
    }

    float getSpeed(){
        return speed_;
    }

    void setSpeed(float newSpeed){
        speed_ = newSpeed;
    }

    

private:
    float speed_ = 0;
    float position =0;
};

#endif