#ifndef SENSORPOSITION_H
#define SENSORPOSITION_H

#include <random>
using namespace std;
#pragma once

#define SENSORS std::vector<sensorPositions> *sensors
class sensorPositions
{
public:
    sensorPositions(){
        sensorPositions((std::mt19937)0.0,0.0,1.0);    
    }
    sensorPositions(std::mt19937 gen, float mu=0.0, float sigma =1 ){
        mu_  = mu;
        sigma_ = sigma;
        gen_ = gen;
    }
    void updateNoiseLevel( std::vector<float> args){
        std::normal_distribution<float> noise(mu_,sigma_); 
        noiseLevel +=  0.001*noise(gen_);
    }
    void updatePosition(){
        std::normal_distribution<float> noise(mu_,sigma_); 
        noiseLevel +=  noise(gen_);
    }
    float getNoiseLevel(){
        return noiseLevel;
    }
    void reCalibrate(){
        noiseLevel = 0.0001;
    }
    void setNoiseParams(float mu, float sigma  ){
        mu_  = mu;
        sigma_ = sigma;
    }

    pair <float,float> getNoiseParams(){
        return make_pair(mu_, sigma_);
    }

private:
    std::mt19937 gen_;
    float noiseLevel =0.0001; //this is never trunked as sensors have drift;
    float  mu_; 
    float  sigma_ ; 
};

#endif