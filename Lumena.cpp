/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       Team Lumena                                               */
/*    Created:      2026                                                      */
/*    Description:  NASA NCAS Rover Challenge – Phase 1 Foundation            */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/*============================================================================*/
/*  ASSUMPTIONS / WARNINGS – READ BEFORE RUNNING                              */
/*============================================================================*/
/*                                                                            */
/*  - All motor PORTS are PLACEHOLDERS. Verify on real robot.                 */
/*  - All motor REVERSALS are PLACEHOLDERS. Verify on real robot.             */
/*  - All GEAR RATIOS are PLACEHOLDERS. Verify on real robot.                 */
/*  - All TIMING values are PLACEHOLDERS. Must be tuned on real robot.        */
/*  - Robot may NOT actually be 4-wheel drive. Adjust as needed.              */
/*  - Distance sensor behavior is PROVISIONAL. Verify on real hardware.       */
/*  - This is DAY 1 / PHASE 1 code. Not a final mission program.             */
/*                                                                            */
/*  IMPORTANT – HARDWARE IS DEFINED IN THE PRAGMA REGION ABOVE.              */
/*  Use the VEXcode V5 "Robot Configuration" screen (gear icon or the         */
/*  collapsed #pragma region at the top of this file) to set:                 */
/*    - Motor ports                                                           */
/*    - Motor reversals                                                       */
/*    - Gear ratios                                                           */
/*    - Sensor ports                                                          */
/*  Do NOT re-declare brain, motors, or sensors down here.                    */
/*                                                                            */
/*  EXPECTED DEVICE NAMES IN ROBOT CONFIGURATION:                             */
/*    motor  LeftFront   (PORT1,  ratio18_1, false)                           */
/*    motor  LeftBack    (PORT2,  ratio18_1, false)                           */
/*    motor  RightFront  (PORT10, ratio18_1, true)                            */
/*    motor  RightBack   (PORT11, ratio18_1, true)                            */
/*    motor  ArmMotor    (PORT5,  ratio36_1, false)                           */
/*    motor  ClawMotor   (PORT6,  ratio18_1, false)                           */
/*    distance DistanceSensor (PORT8)                                         */
/*  Make sure these names match EXACTLY in your Robot Configuration.          */
/*                                                                            */
/*============================================================================*/

// Include the V5 Library
#include "vex.h"

// Allows for easier use of the VEX Library
using namespace vex;

// Competition class instance – needed for field control
competition Competition;


/*============================================================================*/
/*  SECTION 1 – PROGRAM MODE SELECTION                                        */
/*============================================================================*/
/*  Change SELECTED_MODE to pick which routine runs during autonomous.        */
/*  This lets you quickly swap between test modes and auto routines.           */
/*============================================================================*/

enum ProgramMode {
  DRIVE_TEST,
  TURN_TEST,
  ARM_TEST,
  CLAW_TEST,
  SENSOR_TEST,
  TINY_AUTO,
  SAFE_AUTO_A,
  SAFE_AUTO_B,
  BACKUP_AUTO
};

const ProgramMode SELECTED_MODE = DRIVE_TEST;   // <--- CHANGE THIS


/*============================================================================*/
/*  SECTION 2 – TUNING CONSTANTS                                              */
/*============================================================================*/
/*  ALL of these are placeholders. Tune every value on the real robot.         */
/*============================================================================*/

//--- Speed defaults (percent) ---
const double DRIVE_SPEED_DEFAULT = 60;
const double TURN_SPEED_DEFAULT  = 45;
const double ARM_SPEED_UP        = 40;
const double ARM_SPEED_DOWN      = 25;
const double CLAW_SPEED_DEFAULT  = 30;

//--- Timed-move durations (milliseconds) ---
const int ARM_TRAVEL_TIME_MS  = 600;   // arm to travel position
const int ARM_PICKUP_TIME_MS  = 300;   // arm down to pickup height
const int ARM_DROP_TIME_MS    = 800;   // arm down to drop height
const int CLAW_OPEN_TIME_MS   = 400;
const int CLAW_CLOSE_TIME_MS  = 400;


/*============================================================================*/
/*  SECTION 3 – UTILITY HELPERS                                               */
/*============================================================================*/

void waitMs(int timeMs) {
  wait(timeMs, msec);
}

int screenLine = 1;   // tracks next available line on brain screen

void clearScreen() {
  Brain.Screen.clearScreen();
  screenLine = 1;
}

void printLine(const char* text) {
  Brain.Screen.setCursor(screenLine, 1);
  Brain.Screen.print(text);
  screenLine++;
  if (screenLine > 12) { screenLine = 1; }   // wrap if screen fills
}


/*============================================================================*/
/*  SECTION 4 – DRIVE HELPERS                                                 */
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

void driveBackwardTimed(double speedPct, int timeMs) {
  LeftFront.spin(reverse,  speedPct, percent);
  LeftBack.spin(reverse,   speedPct, percent);
  RightFront.spin(reverse, speedPct, percent);
  RightBack.spin(reverse,  speedPct, percent);
  waitMs(timeMs);
  stopDrive();
}

void turnLeftTimed(double speedPct, int timeMs) {
  // left side backward, right side forward
  LeftFront.spin(reverse,  speedPct, percent);
  LeftBack.spin(reverse,   speedPct, percent);
  RightFront.spin(forward, speedPct, percent);
  RightBack.spin(forward,  speedPct, percent);
  waitMs(timeMs);
  stopDrive();
}

void turnRightTimed(double speedPct, int timeMs) {
  // left side forward, right side backward
  LeftFront.spin(forward,  speedPct, percent);
  LeftBack.spin(forward,   speedPct, percent);
  RightFront.spin(reverse, speedPct, percent);
  RightBack.spin(reverse,  speedPct, percent);
  waitMs(timeMs);
  stopDrive();
}


/*============================================================================*/
/*  SECTION 5 – ARM HELPERS                                                   */
/*============================================================================*/

void stopArm() {
  ArmMotor.stop(hold);   // hold position so arm doesn't fall
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

// Named positions – just wrap timed moves for now. Tune times above.
void armToTravelPosition() {
  armUpTimed(ARM_SPEED_UP, ARM_TRAVEL_TIME_MS);
}

void armToPickupPosition() {
  armDownTimed(ARM_SPEED_DOWN, ARM_PICKUP_TIME_MS);
}

void armToDropPosition() {
  armDownTimed(ARM_SPEED_DOWN, ARM_DROP_TIME_MS);
}


/*============================================================================*/
/*  SECTION 6 – CLAW HELPERS                                                  */
/*============================================================================*/

void stopClaw() {
  ClawMotor.stop(hold);   // hold grip
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
/*  SECTION 7 – SENSOR HELPERS                                                */
/*============================================================================*/
/*  Sensor logic is PROVISIONAL. Verify behavior on the real robot.           */
/*  Distance sensor readings can be noisy or wrong if not mounted properly.   */
/*============================================================================*/

double getDistanceInches() {
  return DistanceSensor.objectDistance(inches);
}

bool objectDetectedWithin(double inchesThreshold) {
  double reading = getDistanceInches();
  // objectDistance returns a large value (~9999) if nothing is detected
  return (reading > 0 && reading <= inchesThreshold);
}

void runSensorReadoutForSeconds(int totalMs) {
  int elapsed = 0;
  clearScreen();
  printLine("Sensor Readout:");
  while (elapsed < totalMs) {
    double dist = getDistanceInches();
    Brain.Screen.setCursor(3, 1);
    Brain.Screen.clearLine();
    Brain.Screen.print("Dist: %.1f in", dist);
    waitMs(200);
    elapsed += 200;
  }
}


/*============================================================================*/
/*  SECTION 8 – TEST ROUTINES                                                 */
/*============================================================================*/

void runDriveTest() {
  clearScreen();
  printLine("== DRIVE TEST ==");

  printLine("Forward...");
  driveForwardTimed(DRIVE_SPEED_DEFAULT, 1000);
  waitMs(500);

  printLine("Backward...");
  driveBackwardTimed(DRIVE_SPEED_DEFAULT, 1000);
  waitMs(500);

  printLine("Drive test done.");
}

void runTurnTest() {
  clearScreen();
  printLine("== TURN TEST ==");

  printLine("Turn left...");
  turnLeftTimed(TURN_SPEED_DEFAULT, 800);
  waitMs(500);

  printLine("Turn right...");
  turnRightTimed(TURN_SPEED_DEFAULT, 800);
  waitMs(500);

  printLine("Turn test done.");
}

void runArmTest() {
  clearScreen();
  printLine("== ARM TEST ==");

  printLine("Arm up...");
  armUpTimed(ARM_SPEED_UP, 600);
  waitMs(500);

  printLine("Arm down...");
  armDownTimed(ARM_SPEED_DOWN, 600);
  waitMs(500);

  printLine("Arm test done.");
}

void runClawTest() {
  clearScreen();
  printLine("== CLAW TEST ==");

  printLine("Claw open...");
  clawOpenTimed(CLAW_SPEED_DEFAULT, CLAW_OPEN_TIME_MS);
  waitMs(500);

  printLine("Claw close...");
  clawCloseTimed(CLAW_SPEED_DEFAULT, CLAW_CLOSE_TIME_MS);
  waitMs(500);

  printLine("Claw test done.");
}

void runSensorTest() {
  clearScreen();
  printLine("== SENSOR TEST ==");
  printLine("Reading for 5 seconds...");
  runSensorReadoutForSeconds(5000);
  printLine("Sensor test done.");
}


/*============================================================================*/
/*  SECTION 9 – AUTONOMOUS ROUTINES                                           */
/*============================================================================*/
/*  These are PHASE 1 placeholders. Not full mission strategies.              */
/*============================================================================*/

// Tiny integration test: drive -> arm -> claw -> drive back
void runTinyAuto() {
  clearScreen();
  printLine("== TINY AUTO ==");

  printLine("Drive forward...");
  driveForwardTimed(DRIVE_SPEED_DEFAULT, 1000);
  waitMs(300);

  printLine("Arm down...");
  armToPickupPosition();
  waitMs(300);

  printLine("Claw close...");
  clawCloseTimed(CLAW_SPEED_DEFAULT, CLAW_CLOSE_TIME_MS);
  waitMs(300);

  printLine("Arm up...");
  armToTravelPosition();
  waitMs(300);

  printLine("Drive backward...");
  driveBackwardTimed(DRIVE_SPEED_DEFAULT, 1000);
  waitMs(300);

  printLine("Tiny auto done.");
}

// Simple placeholder auto A
void runSafeAutoA() {
  clearScreen();
  printLine("== SAFE AUTO A ==");

  driveForwardTimed(DRIVE_SPEED_DEFAULT, 1500);
  waitMs(300);
  turnLeftTimed(TURN_SPEED_DEFAULT, 600);
  waitMs(300);
  driveForwardTimed(DRIVE_SPEED_DEFAULT, 1000);
  waitMs(300);

  printLine("Safe Auto A done.");
}

// Simple placeholder auto B – slightly different path
void runSafeAutoB() {
  clearScreen();
  printLine("== SAFE AUTO B ==");

  driveForwardTimed(DRIVE_SPEED_DEFAULT, 1200);
  waitMs(300);
  turnRightTimed(TURN_SPEED_DEFAULT, 500);
  waitMs(300);
  driveForwardTimed(DRIVE_SPEED_DEFAULT, 800);
  waitMs(300);

  printLine("Safe Auto B done.");
}

// Dead-simple backup: just drive forward and stop
void runBackupAuto() {
  clearScreen();
  printLine("== BACKUP AUTO ==");

  driveForwardTimed(DRIVE_SPEED_DEFAULT, 2000);

  printLine("Backup auto done.");
}


/*============================================================================*/
/*  SECTION 10 – SAFE SHUTDOWN                                                */
/*============================================================================*/

void stopEverything() {
  stopDrive();
  stopArm();
  stopClaw();
}


/*============================================================================*/
/*  SECTION 11 – COMPETITION CALLBACKS                                        */
/*============================================================================*/
/*  These three functions are called by the competition class:                */
/*    pre_auton  – runs once when the program starts                          */
/*    autonomous – runs when field control triggers autonomous period          */
/*    usercontrol – runs when field control triggers driver control period     */
/*                                                                            */
/*  If you are NOT using field control, you can also just run the program     */
/*  normally and the autonomous() function will execute immediately.           */
/*============================================================================*/

void pre_auton(void) {
  // Initializing Robot Configuration. DO NOT REMOVE!
  vexcodeInit();

  clearScreen();
  printLine("Lumena - Phase 1");
  printLine("Waiting for start...");

  // Add any sensor calibration or setup here
}

void autonomous(void) {
  // Runs the selected mode during the autonomous period.
  // Change SELECTED_MODE at the top of this file to pick which routine runs.

  switch (SELECTED_MODE) {
    case DRIVE_TEST:   runDriveTest();   break;
    case TURN_TEST:    runTurnTest();    break;
    case ARM_TEST:     runArmTest();     break;
    case CLAW_TEST:    runClawTest();    break;
    case SENSOR_TEST:  runSensorTest();  break;
    case TINY_AUTO:    runTinyAuto();    break;
    case SAFE_AUTO_A:  runSafeAutoA();   break;
    case SAFE_AUTO_B:  runSafeAutoB();   break;
    case BACKUP_AUTO:  runBackupAuto();  break;
  }

  // Safety: stop all motors when autonomous ends
  stopEverything();
  printLine("Auto complete.");
}

void usercontrol(void) {
  // Driver control period.
  // For Phase 1 this is a placeholder. Add joystick control here later.

  clearScreen();
  printLine("Driver Control");
  printLine("(not yet implemented)");

  while (true) {
    // Future: add joystick drive, arm, claw controls here

    wait(20, msec);   // don't hog the CPU
  }
}


/*============================================================================*/
/*  SECTION 12 – MAIN                                                         */
/*============================================================================*/

int main() {
  // Run pre-autonomous setup
  pre_auton();

  // Register competition callbacks with the field control system
  Competition.autonomous(autonomous);
  Competition.drivercontrol(usercontrol);

  // Prevent main from exiting
  while (true) {
    wait(100, msec);
  }
}
