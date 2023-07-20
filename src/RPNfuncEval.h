#ifndef RPNFUNCEVAL_H
#define RPNFUNCEVAL_H

#pragma once
#include <bits/stdc++.h>
//source https://www.geeksforgeeks.org/cpp-map-of-functions/

namespace RPNevalNameSpace{
    using func=function<float(float,float)>;
    unordered_map<string,func> mp{
        {"+", plus<float>()},
        {"-", minus<float>()},
        {"/", divides<float>()},
        {"*", multiplies<float>()}
    };

    static void RegisterNewFunction(const string& name, func function) {
        mp[name] = function;   
    }
    // Function to evaluate a Reverse Polish Notation (RPN)
    // expression
    float evalRPN(vector<string>& A)
    {
        // Create a stack to store the operands
        stack<float> s;
    
        // Iterate over the elements in the RPN expression
        for (auto i : A) {
            // If the element is not an operator, it is an
            // operand Convert it to an integer and push it onto
            // the stack
            if (!mp[i]) {
                s.push(stof(i));
            }
            // If the element is an operator, pop the top two
            // operands from the stack Perform the operation and
            // push the result back onto the stack
            else {
                auto num1 = s.top();
                s.pop();
                auto num2 = s.top();
                s.pop();
                s.push(mp[i](num2, num1));
            }
        }
    
        // Return the result, which is the top element of the
        // stack
        return s.top();
    }

}
#endif