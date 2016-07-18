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

package com.anandmuralidhar.simplevrandroid;

import android.app.Activity;
import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.util.Log;

public class SensorClass implements SensorEventListener {

    public SensorManager mSensorManager;

    private float[] gyroQuaternion = new float[3];
    private boolean isGyroSensorAvailable;

    private float[] gravity = new float[3];
    private boolean isAccelSensorAvailable;

    Sensor mGyro, mAccel;
    private native void SendGravityToNative(float gravityX, float gravityY, float gravityZ);
    private native void SendGyroQuatToNative(float gyroQuatX, float gyroQuatY, float gyroQuatZ);

    public boolean isSensorsAvailable() {
        return isSensorsAvailable;
    }

    private boolean isSensorsAvailable;

    public SensorClass(Activity mainActivity) {

        mSensorManager = (SensorManager) mainActivity.getSystemService(Context.SENSOR_SERVICE);

        gravity[0] = 0.0f;
        gravity[1] = 0.0f;
        gravity[2] = 0.0f;

        mGyro = mSensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);
        mAccel = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);

        isSensorsAvailable = (mGyro != null) && (mAccel != null);
    }

    public boolean RegisterSensors() {

        Log.d("SensorClass", "Registering sensors");
        if (isSensorsAvailable == false) {
            return false;
        }

        isGyroSensorAvailable = mSensorManager.registerListener(this, mGyro,
                SensorManager.SENSOR_DELAY_GAME);

        isAccelSensorAvailable = mSensorManager.registerListener(this, mAccel,
                SensorManager.SENSOR_DELAY_GAME);

        isSensorsAvailable = isGyroSensorAvailable && isAccelSensorAvailable;
        return isSensorsAvailable;
    }

    public void UnregisterSensors() {

        Log.d("SensorClass", "Unregistering sensor listener");
        mSensorManager.unregisterListener(this);

    }

    // IIR filter parameter for accelerometer filtering. range [0.0f -> 1.0f]
    // higher values -> more filtering
    private static final float ACCEL_ALPHA = 0.8f;

    private void CalculateGravityFromAccelerometer(SensorEvent event) {

        // Isolate the force of gravity with the low-pass filter.
        gravity[0] = ACCEL_ALPHA * gravity[0] + (1 - ACCEL_ALPHA) * event.values[0];
        gravity[1] = ACCEL_ALPHA * gravity[1] + (1 - ACCEL_ALPHA) * event.values[1];
        gravity[2] = ACCEL_ALPHA * gravity[2] + (1 - ACCEL_ALPHA) * event.values[2];

    }

    // maximum allowable margin of error in vector magnitude
    public static final float EPSILON = 0.000000001f;
    // nanosec -> sec
    private static final float NS2S = 1.0f / 1000000000.0f;
    private float timestamp = 0.0f;

    private void CalculateRotationVectorFromGyro(SensorEvent event) {

        // This timestep's delta rotation to be multiplied by the current rotation
        // after computing it from the gyro sample data.
        if (timestamp != 0) {
            final float dT = (event.timestamp - timestamp) * NS2S;
            // Axis of the rotation sample, not normalized yet.
            float axisX = event.values[0];
            float axisY = event.values[1];
            float axisZ = event.values[2];

            // Calculate the angular speed of the sample
            float omegaMagnitude = (float) Math.sqrt(axisX*axisX + axisY*axisY + axisZ*axisZ);

            // Normalize the rotation vector if it's big enough to get the axis
            // (that is, EPSILON should represent your maximum allowable margin of error)
            if (omegaMagnitude > EPSILON) {
                axisX /= omegaMagnitude;
                axisY /= omegaMagnitude;
                axisZ /= omegaMagnitude;
            }

            // Integrate around this axis with the angular speed by the timestep
            // in order to get a delta rotation from this sample over the timestep
            // We will convert this axis-angle representation of the delta rotation
            // into an input for OpenCV's cv::Rodrigues function

            float theta = omegaMagnitude * dT ;
            //TODO: handle change in orientation
            gyroQuaternion[0] = theta * axisX;
            gyroQuaternion[1] = theta * axisY;
            gyroQuaternion[2] = theta * axisZ;
        }
        timestamp = event.timestamp;

    }


    @Override
    public void onSensorChanged(SensorEvent event) {

        switch (event.sensor.getType()) {

            case Sensor.TYPE_ACCELEROMETER:
                CalculateGravityFromAccelerometer(event);
                SendGravityToNative(gravity[0], gravity[1], gravity[2]);
                break;

            case Sensor.TYPE_GYROSCOPE:
                CalculateRotationVectorFromGyro(event);
                SendGyroQuatToNative(gyroQuaternion[0],
                        gyroQuaternion[1], gyroQuaternion[2]);
                break;
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        // Do something here if sensor accuracy changes.
    }

}


