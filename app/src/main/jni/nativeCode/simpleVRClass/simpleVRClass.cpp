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

    // create MyGLCamera object and set default position for the object
    myGLCamera = new MyGLCamera(45, 0);
    float pos[]={0.,0.,0.,0.,0.,0.};
    std::copy(&pos[0], &pos[5], std::back_inserter(modelDefaultPosition));
    myGLCamera->SetModelPosition(modelDefaultPosition);

    modelObject = NULL;
    gravityMutex.unlock();
    gravity.assign(3, 0); // assign 3 values
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
    std::string objFilename, mtlFilename, texFilename;
    bool isFilesPresent  =
            gHelperObject->ExtractAssetReturnFilename("cube.obj", objFilename) &&
            gHelperObject->ExtractAssetReturnFilename("cube.mtl", mtlFilename) &&
            gHelperObject->ExtractAssetReturnFilename("cube-sky.jpg", texFilename);
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
//    glm::mat4 mvpMat = myGLCamera->GetMVP();

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
 * rotate the model if user scrolls with one finger
 */
void SimpleVRClass::ScrollAction(float distanceX, float distanceY, float positionX, float positionY) {

    myGLCamera->RotateModel(distanceX, distanceY, positionX, positionY);
}

/**
 * pinch-zoom: move the model closer or farther away
 */
void SimpleVRClass::ScaleAction(float scaleFactor) {

    myGLCamera->ScaleModel(scaleFactor);
}

/**
 * two-finger drag: displace the model by changing its x-y coordinates
 */
void SimpleVRClass::MoveAction(float distanceX, float distanceY) {

    myGLCamera->TranslateModel(distanceX, distanceY);
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
void SimpleVRClass::UpdateRotation(float gyroQuatX, float gyroQuatY, float gyroQuatZ) {

    //apply rotation according to delta-rotation vector from gyro
    cv::Mat gyroQuaternion = cv::Mat::zeros(3, 1, CV_32F);
    gyroQuaternion.at<float>(0, 0) = gyroQuatX;
    gyroQuaternion.at<float>(1, 0) = gyroQuatY;
    gyroQuaternion.at<float>(2, 0) = gyroQuatZ;
    cv::Mat gyroRotationMat;
    cv::Rodrigues(gyroQuaternion, gyroRotationMat);
    cv::Mat gyroRotationCVMat4 = cv::Mat::eye(4, 4, CV_32F);
    cv::Rect roi(0, 0, 3, 3);
    gyroRotationMat.copyTo(gyroRotationCVMat4(roi));

//    gyroRotationCVMat4 = gyroRotationCVMat4.t();

    glm::mat4 gyroRotationGLMMat = glm::make_mat4((float *) gyroRotationCVMat4.data);

    gyroMutex.try_lock();
    myGLCamera->AddDeltaRotation(gyroRotationGLMMat);
    gyroMutex.unlock();

}