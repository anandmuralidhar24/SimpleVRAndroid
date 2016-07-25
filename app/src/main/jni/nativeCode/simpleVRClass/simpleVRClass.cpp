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

#include "myShader.h"
#include "simpleVRClass.h"


#include "assimp/Importer.hpp"
#include <opencv2/opencv.hpp>
#include <myJNIHelper.h>

/**
 * Class constructor
 */
SimpleVRClass::SimpleVRClass() {

    MyLOGD("SimpleVRClass::SimpleVRClass");
    initsDone = false;

    // create MyGLCamera object and place camera in center of the world, zPosition=0
    myGLCamera = new MyGLCamera(45, 0);
    float pos[]={0.,0.,0.,0.,0.,0.}; // center of cube (ourWorld) coincides with center of world
    std::copy(&pos[0], &pos[5], std::back_inserter(modelDefaultPosition));
    myGLCamera->SetModelPosition(modelDefaultPosition);

    modelObject = NULL;
    gravityMutex.unlock();
    gravity.assign(3, 0);
    gravity[2] = 1.0f;

    gyroMutex.unlock();
}

SimpleVRClass::~SimpleVRClass() {

    MyLOGD("SimpleVRClass::~SimpleVRClass");
    if (myGLCamera) {
        delete myGLCamera;
    }
    if (modelObject) {
        delete modelObject;
    }
}

/**
 * Perform inits and load the triangle's vertices/colors to GLES
 */
void SimpleVRClass::PerformGLInits() {

    MyLOGD("SimpleVRClass::PerformGLInits");

    MyGLInits();

    modelObject = new AssimpLoader();

    // extract the OBJ and companion files from assets
    // its a long list since ourWorld.obj has 6 textures corresponding to faces of the cube
    std::string objFilename, mtlFilename, texFilename;
    bool isFilesPresent  =
            gHelperObject->ExtractAssetReturnFilename("ourWorld/ourWorld.obj", objFilename) &&
            gHelperObject->ExtractAssetReturnFilename("ourWorld/ourWorld.mtl", mtlFilename) &&
            gHelperObject->ExtractAssetReturnFilename("ourWorld/deception_pass_bk.jpg", texFilename) &&
            gHelperObject->ExtractAssetReturnFilename("ourWorld/deception_pass_dn.jpg", texFilename) &&
            gHelperObject->ExtractAssetReturnFilename("ourWorld/deception_pass_ft.jpg", texFilename) &&
            gHelperObject->ExtractAssetReturnFilename("ourWorld/deception_pass_lf.jpg", texFilename) &&
            gHelperObject->ExtractAssetReturnFilename("ourWorld/deception_pass_rt.jpg", texFilename) &&
            gHelperObject->ExtractAssetReturnFilename("ourWorld/deception_pass_up.jpg", texFilename);
    if( !isFilesPresent ) {
        MyLOGE("Model %s does not exist!", objFilename.c_str());
        return;
    }

    modelObject->Load3DModel(objFilename);

    CheckGLError("SimpleVRClass::PerformGLInits");
    initsDone = true;
}


/**
 * Render to the display
 */
void SimpleVRClass::Render() {

    // clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gravityMutex.lock();
    gyroMutex.lock();

    glm::mat4 mvpMat = myGLCamera->GetMVPAlignedWithGravity(gravity);

    gyroMutex.unlock();
    gravityMutex.unlock();

    modelObject->Render3DModel(&mvpMat);

    CheckGLError("SimpleVRClass::Render");

}

/**
 * set the viewport, function is also called when user changes device orientation
 */
void SimpleVRClass::SetViewport(int width, int height) {

    screenHeight = height;
    screenWidth = width;
    glViewport(0, 0, width, height);
    CheckGLError("Cube::SetViewport");

    myGLCamera->SetAspectRatio((float) width / height);
}


/**
 * reset model's position in double-tap
 */
void SimpleVRClass::DoubleTapAction() {

    myGLCamera->SetModelPosition(modelDefaultPosition);
}

/**
 * Copy gravity vector from sensor into private variable
 */
void SimpleVRClass::UpdateGravity(float gx, float gy, float gz) {

    gravityMutex.try_lock();
    gravity[0] = gx;
    gravity[1] = gy;
    gravity[2] = gz;
    gravityMutex.unlock();
    return;
}

/**
 * Construct rotation mat from gyro's quaternion and update camera's rotation matrix
 */
void SimpleVRClass::UpdateRotation(float gyroQuatW, float gyroQuatX, float gyroQuatY, float gyroQuatZ) {

    // construct rotation matrix from quaternion
    glm::quat gyroQuaternion(gyroQuatW, gyroQuatX, gyroQuatY, gyroQuatZ);
    glm::mat4 gyroRotationGLMMat = glm::toMat4(gyroQuaternion);

    // transpose of rotation matrix = inverse of rotation matrix
    // need to move model in opposite direction of device movement
    gyroRotationGLMMat = glm::transpose(gyroRotationGLMMat);

    // update rotation mat in myGLCamera under mutex since rendering call might be reading it
    gyroMutex.try_lock();
    myGLCamera->AddDeltaRotation(gyroRotationGLMMat);
    gyroMutex.unlock();

}