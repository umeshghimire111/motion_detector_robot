#ifndef MOTORS_H
#define MOTORS_H

#include "config.h"

// Initialize motor control pins and PWM channel attach
void motorsBegin();

// Stop all motor movement
void motorsStop();

// Set individual motor speed and direction
void setMotor(int pwmPin, int ch, int in1, int in2, int spd, bool fwd);

// Apply a standard movement command (Forward, Backward, Left, Right, Stop)
void applyDriveCmd(DriveCmd cmd);

// Differential speed control mixing X and Y values (-255 to 255)
void driveJoystick(int x, int y);

#endif // MOTORS_H
