/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       Team Lumena                                               */
/*    Created:      2026                                                      */
/*    Description:  NASA NCAS Rover Challenge – Lawnmower Traverse            */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/*============================================================================*/
/*  HARDWARE IS DEFINED IN THE PRAGMA REGION ABOVE.                           */
/*============================================================================*/
/*  Expected devices in Robot Configuration:                                  */
/*    motor  LeftFront, LeftBack, RightFront, RightBack                       */
/*    motor  ArmMotor, ClawMotor                                              */
/*    optical Optical10                                                       */
/*    distance DistanceSensor                                                 */
/*============================================================================*/

// Include the V5 Library
#include "vex.h"

// Allows for easier use of the VEX Library
using namespace vex;

// Competition class instance
competition Competition;


/*============================================================================*/
/*  TUNING CONSTANTS – TUNE ALL OF THESE ON THE REAL ROBOT                    */
/*============================================================================*/

//--- Speed defaults (percent) ---
const double DRIVE_SPEED_DEFAULT = 60;
const double TURN_SPEED_DEFAULT  = 45;
const double ARM_SPEED_UP        = 40;
const double ARM_SPEED_DOWN      = 25;
const double CLAW_SPEED_DEFAULT  = 30;

//--- Arm/claw durations (milliseconds) ---
const int ARM_TRAVEL_TIME_MS  = 600;
const int ARM_PICKUP_TIME_MS  = 300;
const int ARM_DROP_TIME_MS    = 800;
const int CLAW_OPEN_TIME_MS   = 400;
const int CLAW_CLOSE_TIME_MS  = 400;

//--- Lawnmower traverse (MUST TUNE) ---
const int TURN_90_TIME_MS       = 600;    // time to turn 90 degrees
const int LANE_DRIVE_TIME_MS    = 3000;   // time to drive one 72in lane
const int SHIFT_OVER_TIME_MS    = 500;    // time to drive 6 inches sideways
const double TRAVERSE_SPEED     = 50;     // slower speed for scanning
const int NUM_LANES             = 12;     // 72 inches / 6 inch spacing

//--- Sensor thresholds (MUST TUNE) ---
const double MINERAL_BRIGHTNESS_THRESHOLD = 80;   // white tape on dark foam
const double ROCK_DETECT_DISTANCE_IN      = 10;   // inches to detect a rock


/*============================================================================*/
/*  UTILITY HELPERS                                                           */
/*============================================================================*/

void waitMs(int timeMs) {
  wait(timeMs, msec);
}

int screenLine = 1;

void clearScreen() {
  Brain.Screen.clearScreen();
  screenLine = 1;
}

void printLine(const char* text) {
  Brain.Screen.setCursor(screenLine, 1);
  Brain.Screen.print(text);
  screenLine++;
  if (screenLine > 12) { screenLine = 1; }
}


/*============================================================================*/
/*  DRIVE HELPERS                                                             */
/*============================================================================*/

void stopDrive() {
  LeftFront.stop(brake);
  LeftBack.stop(brake);
  RightFront.stop(brake);
  RightBack.stop(brake);
}

void driveForwardTimed(double speedPct, int timeMs) {
  LeftFront.spin(forward,  speedPct, percent);
  LeftBack.spin(forward,   speedPct, percent);
  RightFront.spin(forward, speedPct, percent);
  RightBack.spin(forward,  speedPct, percent);
  waitMs(timeMs);
  stopDrive();
}

void turnLeftTimed(double speedPct, int timeMs) {
  LeftFront.spin(reverse,  speedPct, percent);
  LeftBack.spin(reverse,   speedPct, percent);
  RightFront.spin(forward, speedPct, percent);
  RightBack.spin(forward,  speedPct, percent);
  waitMs(timeMs);
  stopDrive();
}

void turnRightTimed(double speedPct, int timeMs) {
  LeftFront.spin(forward,  speedPct, percent);
  LeftBack.spin(forward,   speedPct, percent);
  RightFront.spin(reverse, speedPct, percent);
  RightBack.spin(reverse,  speedPct, percent);
  waitMs(timeMs);
  stopDrive();
}


/*============================================================================*/
/*  ARM & CLAW HELPERS                                                        */
/*============================================================================*/

void stopArm() {
  ArmMotor.stop(hold);
}

void stopClaw() {
  ClawMotor.stop(hold);
}

void armUpTimed(double speedPct, int timeMs) {
  ArmMotor.spin(forward, speedPct, percent);
  waitMs(timeMs);
  stopArm();
}

void armDownTimed(double speedPct, int timeMs) {
  ArmMotor.spin(reverse, speedPct, percent);
  waitMs(timeMs);
  stopArm();
}

void armToTravelPosition() {
  armUpTimed(ARM_SPEED_UP, ARM_TRAVEL_TIME_MS);
}

void armToPickupPosition() {
  armDownTimed(ARM_SPEED_DOWN, ARM_PICKUP_TIME_MS);
}

void clawOpenTimed(double speedPct, int timeMs) {
  ClawMotor.spin(forward, speedPct, percent);
  waitMs(timeMs);
  stopClaw();
}

void clawCloseTimed(double speedPct, int timeMs) {
  ClawMotor.spin(reverse, speedPct, percent);
  waitMs(timeMs);
  stopClaw();
}


/*============================================================================*/
/*  SENSOR DETECTION                                                          */
/*============================================================================*/

bool isMineralDetected() {
  return (Optical10.brightness() >= MINERAL_BRIGHTNESS_THRESHOLD);
}

bool isRockDetected() {
  double dist = DistanceSensor.objectDistance(inches);
  return (dist > 0 && dist <= ROCK_DETECT_DISTANCE_IN);
}


/*============================================================================*/
/*  LAWNMOWER TRAVERSE                                                        */
/*============================================================================*/

enum DetectionType { NOTHING, MINERAL, ROCK };

int mineralsFound = 0;
int rocksFound    = 0;

// Drive in a direction while checking both sensors
DetectionType driveScanningDirection(double speedPct, int timeMs, bool goForward) {
  if (goForward) {
    LeftFront.spin(forward,  speedPct, percent);
    LeftBack.spin(forward,   speedPct, percent);
    RightFront.spin(forward, speedPct, percent);
    RightBack.spin(forward,  speedPct, percent);
  } else {
    LeftFront.spin(reverse,  speedPct, percent);
    LeftBack.spin(reverse,   speedPct, percent);
    RightFront.spin(reverse, speedPct, percent);
    RightBack.spin(reverse,  speedPct, percent);
  }

  int elapsed = 0;
  while (elapsed < timeMs) {
    if (isRockDetected()) {
      stopDrive();
      return ROCK;
    }
    if (isMineralDetected()) {
      stopDrive();
      return MINERAL;
    }
    waitMs(20);
    elapsed += 20;
  }

  stopDrive();
  return NOTHING;
}

// Mineral: announce on screen, then keep going
void handleMineralFound() {
  mineralsFound++;
  clearScreen();
  Brain.Screen.clearScreen(green);
  Brain.Screen.setCursor(3, 1);
  Brain.Screen.setFont(mono40);
  Brain.Screen.print("MINERAL FOUND!");
  Brain.Screen.setCursor(5, 1);
  Brain.Screen.setFont(mono20);
  Brain.Screen.print("Total: %d", mineralsFound);
  waitMs(2000);
  Brain.Screen.setFont(mono20);
  clearScreen();
}

// Rock: pick up, hold 5 sec, set back down, done
void handleRockFound() {
  rocksFound++;
  clearScreen();
  printLine("ROCK DETECTED!");
  Brain.Screen.setCursor(3, 1);
  Brain.Screen.print("Rocks found: %d", rocksFound);

  // Arm down
  printLine("Arm down...");
  armToPickupPosition();
  waitMs(300);

  // Grab
  printLine("Grabbing...");
  clawCloseTimed(CLAW_SPEED_DEFAULT, CLAW_CLOSE_TIME_MS);
  waitMs(300);

  // Lift up
  printLine("Lifting...");
  armToTravelPosition();
  waitMs(500);

  // Hold for 5 seconds
  clearScreen();
  Brain.Screen.setCursor(3, 1);
  Brain.Screen.setFont(mono40);
  Brain.Screen.print("ROCK HELD!");
  Brain.Screen.setFont(mono20);
  Brain.Screen.setCursor(5, 1);
  Brain.Screen.print("Total: %d", rocksFound);
  waitMs(5000);

  // Set it back down slowly
  printLine("Setting down...");
  armDownTimed(ARM_SPEED_DOWN, ARM_DROP_TIME_MS);
  waitMs(300);

  // Release
  clawOpenTimed(CLAW_SPEED_DEFAULT, CLAW_OPEN_TIME_MS);
  waitMs(300);

  // Lift arm to clear the rock
  armToTravelPosition();
  waitMs(300);

  clearScreen();
  printLine("Rock set down. Done.");
}

// Shift over 6 inches to the next lane
void shiftToNextLane(bool shiftRight) {
  if (shiftRight) {
    turnRightTimed(TURN_SPEED_DEFAULT, TURN_90_TIME_MS);
  } else {
    turnLeftTimed(TURN_SPEED_DEFAULT, TURN_90_TIME_MS);
  }
  waitMs(300);

  driveForwardTimed(TRAVERSE_SPEED, SHIFT_OVER_TIME_MS);
  waitMs(300);

  if (shiftRight) {
    turnRightTimed(TURN_SPEED_DEFAULT, TURN_90_TIME_MS);
  } else {
    turnLeftTimed(TURN_SPEED_DEFAULT, TURN_90_TIME_MS);
  }
  waitMs(300);
}

// Main traverse routine
void runLawnmower() {
  clearScreen();
  printLine("== LAWNMOWER ==");

  mineralsFound = 0;
  rocksFound = 0;

  Optical10.setLight(ledState::on);
  waitMs(300);

  for (int lane = 0; lane < NUM_LANES; lane++) {
    bool goForward = (lane % 2 == 0);

    Brain.Screen.setCursor(2, 1);
    Brain.Screen.clearLine();
    Brain.Screen.print("Lane %d/%d %s", lane + 1, NUM_LANES,
                       goForward ? ">>>" : "<<<");

    DetectionType result = driveScanningDirection(TRAVERSE_SPEED,
                                                  LANE_DRIVE_TIME_MS,
                                                  goForward);

    if (result == MINERAL) {
      handleMineralFound();
    } else if (result == ROCK) {
      handleRockFound();
      break;
    }

    if (lane < NUM_LANES - 1) {
      shiftToNextLane(goForward);
    }
  }

  // Summary
  clearScreen();
  printLine("== DONE ==");
  Brain.Screen.setCursor(3, 1);
  Brain.Screen.print("Minerals: %d", mineralsFound);
  Brain.Screen.setCursor(4, 1);
  Brain.Screen.print("Rocks: %d", rocksFound);

  Optical10.setLight(ledState::off);
}


/*============================================================================*/
/*  SAFE SHUTDOWN                                                             */
/*============================================================================*/

void stopEverything() {
  stopDrive();
  stopArm();
  stopClaw();
}


/*============================================================================*/
/*  COMPETITION CALLBACKS                                                     */
/*============================================================================*/

void pre_auton(void) {
  vexcodeInit();
  clearScreen();
  printLine("Lumena");
  printLine("Waiting for start...");
}

void autonomous(void) {
  runLawnmower();
  stopEverything();
  printLine("Auto complete.");
}

void usercontrol(void) {
  clearScreen();
  printLine("Driver Control");
  printLine("(not implemented)");
  while (true) {
    wait(20, msec);
  }
}


/*============================================================================*/
/*  MAIN                                                                      */
/*============================================================================*/

int main() {
  pre_auton();
  Competition.autonomous(autonomous);
  Competition.drivercontrol(usercontrol);
  while (true) {
    wait(100, msec);
  }
}
