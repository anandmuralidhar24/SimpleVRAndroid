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

#include <jni.h>
#include "simpleVRClass.h"
#include "myJNIHelper.h"

#ifdef __cplusplus
extern "C" {
#endif

extern SimpleVRClass *gSimpleVRObject;
JNIEXPORT void JNICALL
Java_com_anandmuralidhar_simplevrandroid_SensorClass_SendGravityToNative(JNIEnv *env,
                                                                         jobject instance,
                                                                         jfloat gravityX,
                                                                         jfloat gravityY,
                                                                         jfloat gravityZ) {

    if(gSimpleVRObject == NULL){
        return;
    }
    gSimpleVRObject->UpdateGravity(gravityX, gravityY, gravityZ);

}

JNIEXPORT void JNICALL
Java_com_anandmuralidhar_simplevrandroid_SensorClass_SendGyroQuatToNative(JNIEnv *env,
                                                                          jobject instance,
                                                                          jfloat gyroQuatX,
                                                                          jfloat gyroQuatY,
                                                                          jfloat gyroQuatZ) {

    if(gSimpleVRObject == NULL){
        return;
    }
    gSimpleVRObject->UpdateRotation(gyroQuatX, gyroQuatY, gyroQuatZ);

}

#ifdef __cplusplus
}
#endif
