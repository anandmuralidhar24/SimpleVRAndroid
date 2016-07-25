/*
 *    Copyright 2016 Anand Muralidhar
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifndef SIMPLEVRCLASS_H
#define SIMPLEVRCLASS_H

#include "myLogger.h"
#include "myGLFunctions.h"
#include "myGLCamera.h"
#include "assimpLoader.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <mutex>

class SimpleVRClass {
public:
    SimpleVRClass();
    ~SimpleVRClass();
    void    PerformGLInits();
    void    Render();
    void    SetViewport(int width, int height);
    void    DoubleTapAction();
    void    UpdateGravity(float gx, float gy, float gz);
    void    UpdateRotation(float gyroQuatW, float gyroQuatX, float gyroQuatY, float gyroQuatZ);

private:
    bool    initsDone;
    int     screenWidth, screenHeight;

    std::mutex              gravityMutex, gyroMutex;
    std::vector <float>     gravity;

    std::vector<float> modelDefaultPosition;
    MyGLCamera * myGLCamera;
    AssimpLoader * modelObject;
};

#endif //SIMPLEVRCLASS_H
