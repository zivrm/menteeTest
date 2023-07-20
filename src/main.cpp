#include <iostream>
#include <tuple>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <functional>
#include <mutex>


#include <cstdint>
#include "imageHandler.h"
#include "motor.h"
#include "sensorPositions.h"
#include "RPNfuncEval.h"
using namespace jpegHandle;
using namespace std;
#define __DEBUG__ false
#define __tests__ true

#define ImageSizeInBytes 2293760 // 1280*1024*14/8 


// random device class instance, source of 'true' randomness for initializing random seed
std::random_device rd; 
// Mersenne twister PRNG, initialized with seed from previous random device instance
std::mt19937 gen(rd()); 


struct brainMemory{
    std::vector<motor> motors;
    std::vector<sensorPositions> sensors ;
    std::map<string, pair <uint16_t, std::vector<string> >> netFunctions;
    std::mutex positionMutex;
    std::mutex motorsMutex;
    std::mutex imageBufferMutex;
    std::mutex functionsMutex;
    std::vector<uint8_t> bufferForImage;

};
brainMemory memory;




float evalRPNFunction(vector<string>& RPN_representation)
{
    // Create a stack to store the operands
    stack<float> operands;
    vector<string>& updatedValuesRPN = RPN_representation;
    // Iterate over the elements in the RPN expression
    for (auto &element : updatedValuesRPN) {
        // If the element is not an operator, it is an operand 
        if(element[0] == 'S'){  //the item is a sensor number
            uint16_t sensorNumber = stoi(element.substr(1, element.size() - 1));
            element =  std::to_string(  memory.sensors[sensorNumber].getNoiseLevel());                
            continue;
            }
        if(element[0] == 'M'){  //the item is a motor number
            uint16_t motorNumber = stoi(element.substr(1, element.size() - 1));
            element = std::to_string(memory.motors[motorNumber].getPosition());                
            continue;
            }
        }
    using namespace RPNevalNameSpace;
    return RPNevalNameSpace::evalRPN(updatedValuesRPN);
}



std::vector<std::vector<float>>     getStatus(){
    std::vector<std::vector<float>> results;
    std::vector<float> motorsRes;
    std::vector<float> sensorsRes;
    std::vector<float> functionsRes;
    for( auto motorPosition : memory.motors)
        motorsRes.push_back(motorPosition.getPosition());
    for( auto noiseLevel: memory.sensors)
        sensorsRes.push_back(noiseLevel.getNoiseLevel());
    for (auto func : memory.netFunctions)
        functionsRes.push_back(evalRPNFunction(func.second.second ));
    
    results.push_back(motorsRes);
    results.push_back(sensorsRes);
    results.push_back(functionsRes);
    return results;
}




void callback(uint8_t *data, int len){
    memory.imageBufferMutex.lock();
    for(int counter=0; counter <len; ++counter ){
        memory.bufferForImage.push_back(data[counter]);
    }
    memory.imageBufferMutex.unlock();
}


void printState(MOTORS, SENSORS){
    for (motor x : motors[0])
        cout << "pos: " <<  x.getPosition() << "\tspeed: " << x.getSpeed() <<"\n";
    for (sensorPositions s : sensors[0])
        cout << "total level: " <<  s.getNoiseLevel() << "\tparams\t: mu: " << s.getNoiseParams().first << "\t\t sigma: " <<s.getNoiseParams().second << "\n";
    }


void updateMotorsensors(){
    while (true){
        memory.motorsMutex.lock();
        for (motor& m : memory.motors){
            m.updatePosition();
        }
        memory.motorsMutex.unlock();
        auto ms = std::chrono::steady_clock::now() + std::chrono::milliseconds(100);
        std::this_thread::sleep_until(ms);
    }
    
}



void updateNoise(){
    #if __tests__
        std::vector<uint8_t> testBuffer;
        testBuffer.assign(250, 0x55);
        uint8_t *pointer = testBuffer.data();
    #endif
    while (true){
        memory.positionMutex.lock();
        for (sensorPositions& pos : memory.sensors){
            pos.updatePosition();
        }
        memory.positionMutex.unlock();
        #if __tests__
            callback(pointer, 211);
        #endif
        auto ms = std::chrono::steady_clock::now() + std::chrono::milliseconds(4);
        std::this_thread::sleep_until(ms);
    }
    
}


void saveImageFromBuffer(std::vector<uint8_t> buffer){
    string filename;
    FILE* outfile;
    for(uint32_t filenumber =0;filenumber< UINT_MAX;filenumber++ ){
        filename = "output" + to_string(filenumber) + ".jpeg";
        std::ifstream file(filename);
        if(!file ) 
            return saveAsJPEG(buffer, filename.c_str());

    }
    fprintf(stderr, "no more files left \n" );
}




void handleImageBUffer(){
    while(1){
        uint32_t currentSize =0;
        memory.imageBufferMutex.lock();
        currentSize = memory.bufferForImage.size();
        if(currentSize >= ImageSizeInBytes){
            std::vector<uint8_t> copy = memory.bufferForImage;
            auto start = memory.bufferForImage.begin();
            memory.bufferForImage.erase(start,start+ ImageSizeInBytes);
            memory.imageBufferMutex.unlock();
            saveImageFromBuffer(copy);
            copy.clear();
            continue;
        }
        memory.imageBufferMutex.unlock();

    }
    
}


void runFunction(uint16_t targertMotor, std::vector<string> RPNfunction){
    memory.motorsMutex.lock();
        memory.motors[targertMotor].setPosition(evalRPNFunction(RPNfunction));
    memory.motorsMutex.unlock();
}


void updateByFunctions(){
    std::vector<float> currentMotorPosition(memory.motors.size());
    std::vector<float> currentSensorValues(memory.sensors.size());
    
    while (true){
        memory.positionMutex.lock();
            for (int i =0 ; i<memory.sensors.size();++i)
                currentSensorValues[i] = memory.sensors[i].getNoiseLevel();
        memory.positionMutex.unlock();
        memory.motorsMutex.lock();
            for (int i =0 ; i<memory.motors.size();++i)
                currentMotorPosition[i] = memory.motors[i].getPosition();
        memory.motorsMutex.unlock();

        for (auto func : memory.netFunctions){
            runFunction(func.second.first, func.second.second);
        }
        
        if (__DEBUG__) printState(&memory.motors,&memory.sensors);
        auto ms = std::chrono::steady_clock::now() + std::chrono::milliseconds(100);
        std::this_thread::sleep_until(ms);
    }
}

bool runSpecificNetFunction(string name){
    memory.functionsMutex.lock();
    if (memory.netFunctions.count(name) == 0){ //no function with that name
        cout << "no such function " << name << "\n";
        memory.functionsMutex.unlock();
        return false;
    }
    pair <uint16_t, std::vector<string> > func = memory.netFunctions[name];
    memory.functionsMutex.unlock();
    runFunction(func.first, func.second);
    return true;
}

void addNetFunction(string name, uint16_t targetMotorToUpdate,std::vector<string> parsedFunction ){
    pair <uint16_t, std::vector<string>> functionN = make_pair(targetMotorToUpdate, parsedFunction);
    memory.functionsMutex.lock();
    memory.netFunctions.insert(make_pair( name, functionN));
    memory.functionsMutex.unlock();

}

void addMotor(float speed){
    memory.motorsMutex.lock();
    memory.motors.push_back(motor(speed));
    memory.motorsMutex.unlock();
}

void addSensor(float mu, float sigma){
    memory.sensors.push_back(sensorPositions(gen, mu, sigma));
}


int main(){
    if(__tests__){
    // for testing
    std::vector<float> speeds = {12,4,-5,0};
    std::vector<pair<float,float> > noiseParams = {make_pair(0,1),make_pair(0,2),make_pair(0,3),make_pair(0,4)};
    for (uint16_t i=0; i< 4; ++i){
       addMotor(speeds[i]);
       addSensor(noiseParams[i].first,noiseParams[i].second );
    }
    

    printState(&memory.motors, &memory.sensors);
    addNetFunction("test1", 3, {"3", "S0","+"});
    runSpecificNetFunction("test1");
    
    vector<string> A = { "10", "M2", "9",  "S3", "+", "-11", "*","/",  "*", "17", "+", "5", "+" };
    cout <<"\n\nthe reuslt is: " <<evalRPNFunction(A);//, &memory.motors, &memory.sensors)<<"\n\n";
    RPNevalNameSpace::RegisterNewFunction("^", powf);
    vector<string> B = { "10", "5", "^" };
    cout <<"\n\nthe reuslt is: " <<evalRPNFunction(B);//, &memory.motors, &memory.sensors)<<"\n\n";

    std::vector<uint8_t> testBuffer;
    testBuffer.assign(ImageSizeInBytes*2, 0x55);
    const char* filename = "output.jpg";
    saveAsJPEG(testBuffer, filename);



    }
    std::thread motorThread(updateMotorsensors);//, &memory.motors,std::ref (memory.motorsMutex));
    std::thread noiseThread(updateNoise);//, &memory.sensors,std::ref (memory.positionMutex));
    std::thread mainControlLoopThread(updateByFunctions);//, &memory.motors, &memory.sensors,std::ref (memory.motorsMutex),std::ref (memory.positionMutex));
    std::thread imageProcessingThread(handleImageBUffer);
    noiseThread.join();
    motorThread.join();
    mainControlLoopThread.join();
    imageProcessingThread.join();

    while(1);
    exit(0);

}

//#define MYGODPYBINDWORKS 
#ifdef MYGODPYBINDWORKS
#include <pybind11/pybind11.h>
namespace py = pybind11;

PYBIND11_MODULE(menteeRobTest, m) {
    m.doc() = "pybind11 plugin"; // optional module docstring
    m.def("addSensors", &addSensor, "A function that addes another sensor (or position noise to the loop)");
    m.def("addMotors" , &addMotor,  "A function that addes a motor)");
    m.def("addNetFunction" , &addNetFunction,  "A function adds a net function, these functions take inputs from multiple sensors and motor state.)");
    m.def("callback" , &callback,  "A function that inserts data into a buffer for image creation)");
    m.def("getStatus" , &getStatus,  "A function that returns a vector of all values)");

} 
#endif


