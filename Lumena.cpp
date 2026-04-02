/*============================================================================*/
/*  VEXcode Generated Robot Configuration                                     */
/*============================================================================*/
#pragma region VEXcode Generated Robot Configuration
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "vex.h"

using namespace vex;

brain Brain;

#define waitUntil(condition)                                                   \
  do {                                                                         \
    wait(5, msec);                                                             \
  } while (!(condition))

#define repeat(iterations)                                                     \
  for (int iterator = 0; iterator < iterations; iterator++)

optical Optical7 = optical(PORT7);
motor LeftDriveSmart = motor(PORT1, ratio18_1, false);
motor RightDriveSmart = motor(PORT2, ratio18_1, true);
drivetrain Drivetrain = drivetrain(LeftDriveSmart, RightDriveSmart, 319.19, 295, 40, mm, 1);
distance Distance6 = distance(PORT6);
inertial Inertial11 = inertial(PORT11);
motor RoboticArm3_mJ1 = motor(PORT3, ratio18_1, false);
motor RoboticArm3_mJ2 = motor(PORT4, ratio18_1, true);
motor RoboticArm3_mJ3 = motor(PORT5, ratio18_1, false);
motor RoboticArm3_mJ4 = motor(PORT8, ratio18_1, false);
pot RoboticArm3_mJ1_pot = pot(Brain.ThreeWirePort.A);
pot RoboticArm3_mJ2_pot = pot(Brain.ThreeWirePort.B);
pot RoboticArm3_mJ3_pot = pot(Brain.ThreeWirePort.C);
pot RoboticArm3_mJ4_pot = pot(Brain.ThreeWirePort.D);
RoboticArm RoboticArm3 = RoboticArm(RoboticArm3_mJ1, RoboticArm3_mJ1_pot, RoboticArm3_mJ2, RoboticArm3_mJ2_pot, RoboticArm3_mJ3, RoboticArm3_mJ3_pot, RoboticArm3_mJ4, RoboticArm3_mJ4_pot);

void initializeRandomSeed(){
  int systemTime = Brain.Timer.systemHighResolution();
  double batteryCurrent = Brain.Battery.current();
  double batteryVoltage = Brain.Battery.voltage(voltageUnits::mV);
  int seed = int(batteryVoltage + batteryCurrent * 100) + systemTime;
  srand(seed);
}

void vexcodeInit() { initializeRandomSeed(); }
#pragma endregion VEXcode Generated Robot Configuration


/*============================================================================*/
/*  TUNING CONSTANTS                                                          */
/*============================================================================*/

// Speeds
const double DRIVE_SPEED      = 40;   // percent
const double TURN_SPEED       = 25;   // percent — increased from 10 for faster turns
const double RETURN_SPEED     = 50;   // percent — faster when returning to spawn (no scanning)
const double ARM_RAISE_SPEED  = 30;
const double ARM_HOLD_SPEED   = 15;
const double ARM_LOWER_SPEED  = 20;
const double CLAW_SPEED       = 40;

// Olympus Mons path (from spawn)
const double OLYMPUS_LEG1     = 54.0; // inches forward from spawn
const double OLYMPUS_LEG2     = 40.0; // inches left to reach Olympus Mons

// Lawnmower dimensions
const double LANE_LENGTH      = 54.0; // inches — same depth as Olympus leg 1
const double LANE_SHIFT       = 12.0; // inches — sideways shift per lane
const int    TOTAL_LANES      = 5;    // 5 lanes × 12in = 60in coverage across 72in width

// Sensor thresholds
const double ROCK_DETECT_MM   = 240.0;
const double YELLOW_HUE_MIN   = 40.0;
const double YELLOW_HUE_MAX   = 75.0;
const double GREEN_HUE_MIN    = 80.0;
const double GREEN_HUE_MAX    = 160.0;
const double BRIGHT_MIN       = 10.0;

// Arm/claw timing (ms)
const int ARM_RAISE_MS        = 6000;
const int ARM_LOWER_MS        = 5000;
const int CLAW_OPEN_MS        = 4000;
const int CLAW_CLOSE_MS       = 1000;


/*============================================================================*/
/*  SCREEN HELPERS                                                            */
/*============================================================================*/

int screenLine = 1;

void lcdClear() { Brain.Screen.clearScreen(); screenLine = 1; }

void lcdPrint(const char* text) {
  Brain.Screen.setCursor(screenLine++, 1);
  Brain.Screen.print(text);
  if (screenLine > 12) screenLine = 1;
}

void lcdBig(const char* text, color bg) {
  Brain.Screen.clearScreen(bg);
  Brain.Screen.setFont(mono40);
  Brain.Screen.setCursor(3, 1);
  Brain.Screen.print(text);
  Brain.Screen.setFont(mono20);
}


/*============================================================================*/
/*  SENSOR CHECKS                                                             */
/*============================================================================*/

bool isMineralDetected() {
  if (Optical7.brightness() < BRIGHT_MIN) return false;
  double hue = Optical7.hue();
  return (hue >= YELLOW_HUE_MIN && hue <= YELLOW_HUE_MAX) ||
         (hue >= GREEN_HUE_MIN  && hue <= GREEN_HUE_MAX);
}

bool isRockDetected() {
  return Distance6.objectDistance(mm) <= ROCK_DETECT_MM;
}


/*============================================================================*/
/*  ARM + CLAW                                                                */
/*============================================================================*/

void clawOpen() {
  RoboticArm3_mJ4.spin(forward, CLAW_SPEED, percent);
  wait(CLAW_OPEN_MS, msec);
  RoboticArm3_mJ4.stop();
}

void clawClose() {
  RoboticArm3_mJ4.spin(reverse, CLAW_SPEED, percent);
  wait(CLAW_CLOSE_MS, msec);
  RoboticArm3_mJ4.stop();
}

void armHold() {
  RoboticArm3_mJ2.spin(forward, ARM_HOLD_SPEED, percent);
}

void armRaise() {
  RoboticArm3_mJ2.spin(forward, ARM_RAISE_SPEED, percent);
  wait(ARM_RAISE_MS, msec);
  armHold();
}

void armLower() {
  RoboticArm3_mJ2.spin(reverse, ARM_LOWER_SPEED, percent);
  wait(ARM_LOWER_MS, msec);
  RoboticArm3_mJ2.stop();
}


/*============================================================================*/
/*  GRAB + DROP SEQUENCES                                                     */
/*============================================================================*/

int rocksDelivered = 0;
int mineralsFound  = 0;

void grabRock() {
  Drivetrain.stop();
  lcdClear(); lcdPrint("Lowering arm...");
  armLower();
  lcdClear(); lcdPrint("Opening claw...");
  clawOpen();
  wait(500, msec);
  lcdClear(); lcdPrint("Grabbing...");
  clawClose();
  lcdClear(); lcdPrint("Raising arm...");
  armRaise();
  lcdBig("GRABBED!", color::orange);
  wait(800, msec);
  lcdClear();
}

void dropRock() {
  lcdClear(); lcdPrint("Dropping rock...");
  armLower();
  clawOpen();
  wait(300, msec);
  armRaise();
  rocksDelivered++;
  lcdBig("DELIVERED!", color::blue);
  Brain.Screen.setCursor(5, 1);
  Brain.Screen.setFont(mono20);
  Brain.Screen.print("Total: %d", rocksDelivered);
  wait(800, msec);
  lcdClear();
}

void handleMineral() {
  mineralsFound++;
  Drivetrain.stop();
  lcdBig("MINERAL!", color::green);
  Brain.Screen.setCursor(5, 1);
  Brain.Screen.setFont(mono20);
  Brain.Screen.print("Total: %d", mineralsFound);
  wait(1500, msec);
  lcdClear();
}


/*============================================================================*/
/*  DRIVING                                                                   */
/*============================================================================*/

// Drive forward scanning for minerals and rocks.
// Returns true if a rock was detected and grabbed (caller should return to spawn).
bool driveInchesScanning(double inches) {
  LeftDriveSmart.resetPosition();
  double wheelCircIn = 319.19 / 25.4;
  double targetDeg   = (inches / wheelCircIn) * 360.0;

  Drivetrain.setDriveVelocity(DRIVE_SPEED, percent);
  Drivetrain.drive(forward);

  while (fabs(LeftDriveSmart.position(degrees)) < targetDeg) {
    if (isMineralDetected()) {
      Drivetrain.stop();
      handleMineral();
      Drivetrain.drive(forward);
    }
    if (isRockDetected()) {
      Drivetrain.stop();
      grabRock();
      return true; // signal: need to return to spawn
    }
    wait(20, msec);
  }

  Drivetrain.stop();
  return false; // no rock grabbed
}

// Drive a fixed distance at a given speed, no scanning (used for returns and shifts)
void driveInchesBlind(double inches, double speed) {
  LeftDriveSmart.resetPosition();
  double wheelCircIn = 319.19 / 25.4;
  double targetDeg   = (inches / wheelCircIn) * 360.0;

  Drivetrain.setDriveVelocity(speed, percent);
  Drivetrain.drive(forward);
  while (fabs(LeftDriveSmart.position(degrees)) < targetDeg) {
    wait(20, msec);
  }
  Drivetrain.stop();
}

void driveInchesBlindReverse(double inches, double speed) {
  LeftDriveSmart.resetPosition();
  double wheelCircIn = 319.19 / 25.4;
  double targetDeg   = (inches / wheelCircIn) * 360.0;

  Drivetrain.setDriveVelocity(speed, percent);
  Drivetrain.drive(reverse);
  while (fabs(LeftDriveSmart.position(degrees)) < targetDeg) {
    wait(20, msec);
  }
  Drivetrain.stop();
}


/*============================================================================*/
/*  TURNS                                                                     */
/*============================================================================*/

void turnRight90() {
  Drivetrain.setTurnVelocity(TURN_SPEED, percent);
  Drivetrain.turnFor(right, 90, degrees, true);
  wait(150, msec);
}

void turnLeft90() {
  Drivetrain.setTurnVelocity(TURN_SPEED, percent);
  Drivetrain.turnFor(left, 90, degrees, true);
  wait(150, msec);
}


/*============================================================================*/
/*  SPAWN RETURN & RESUME                                                     */
/*                                                                            */
/*  Used after grabbing any rock during the lawnmower.                        */
/*  Tracks how far into the current lane the rover was when it grabbed,       */
/*  returns to spawn, drops, then drives back out to resume.                  */
/*============================================================================*/

// Returns to spawn from current position in a lane.
// laneNumber: which lane we're in (0-indexed, each is 12in to the left of spawn)
// distanceIntoLane: how far forward we drove before grabbing
void returnToSpawnFromLane(int laneNumber, double distanceIntoLane) {
  lcdClear(); lcdPrint("Returning to spawn...");

  // Reverse back to the start of the lane
  driveInchesBlindReverse(distanceIntoLane, RETURN_SPEED);

  // Turn right to face spawn (we're currently facing forward/away from spawn)
  turnRight90();

  // Drive back across the lane shifts to reach spawn column
  double shiftDistance = laneNumber * LANE_SHIFT;
  if (shiftDistance > 0) {
    driveInchesBlind(shiftDistance, RETURN_SPEED);
  }

  // Now at spawn — drop the rock
  dropRock();
}

// After dropping, drive back out to resume the lane
void resumeLane(int laneNumber, double distanceIntoLane) {
  lcdClear(); lcdPrint("Resuming lane...");

  // Drive back out to the lane column
  double shiftDistance = laneNumber * LANE_SHIFT;
  if (shiftDistance > 0) {
    driveInchesBlindReverse(shiftDistance, RETURN_SPEED);
  }

  // Turn left to face back down the lane
  turnLeft90();

  // Drive forward to where we left off
  driveInchesBlind(distanceIntoLane, DRIVE_SPEED);
}


/*============================================================================*/
/*  OLYMPUS MONS — FIRST PRIORITY                                             */
/*                                                                            */
/*  From spawn (top-right corner):                                            */
/*    1. Drive forward 54in                                                   */
/*    2. Turn left 90°                                                        */
/*    3. Drive forward 40in → rock in front                                   */
/*    4. Grab                                                                 */
/*    5. Reverse 40in                                                         */
/*    6. Turn right 90°                                                       */
/*    7. Reverse 54in → back at spawn                                         */
/*    8. Drop                                                                 */
/*============================================================================*/

void grabOlympusMons() {
  lcdClear();
  lcdBig("OLYMPUS!", color::purple);
  wait(800, msec);
  lcdClear();
  lcdPrint("Heading to Olympus Mons...");

  // Leg 1 — forward 54in toward centerline
  driveInchesBlind(OLYMPUS_LEG1, DRIVE_SPEED);

  // Turn left toward Olympus Mons
  turnLeft90();

  // Leg 2 — forward 40in to rock
  driveInchesBlind(OLYMPUS_LEG2, DRIVE_SPEED);

  // Grab it
  grabRock();

  // Return — reverse 40in
  driveInchesBlindReverse(OLYMPUS_LEG2, RETURN_SPEED);

  // Turn right to face back toward spawn
  turnRight90();

  // Reverse 54in back to spawn
  driveInchesBlindReverse(OLYMPUS_LEG1, RETURN_SPEED);

  // Drop at spawn
  dropRock();

  lcdClear();
  lcdPrint("Olympus Mons delivered!");
}


/*============================================================================*/
/*  LAWNMOWER WITH SPAWN RETURNS                                              */
/*                                                                            */
/*  Rover starts at spawn facing forward (same direction as Olympus trip).   */
/*  Sweeps lanes across the field width, returning to spawn for every rock.  */
/*============================================================================*/

void lawnmower() {
  for (int lane = 0; lane < TOTAL_LANES; lane++) {

    lcdClear();
    Brain.Screen.setCursor(1, 1);
    Brain.Screen.print("Lane %d / %d", lane + 1, TOTAL_LANES);

    // --- Shift into this lane (all lanes except lane 0) ---
    if (lane > 0) {
      // From spawn, turn left and drive out one lane width
      turnLeft90();
      driveInchesBlind(LANE_SHIFT, DRIVE_SPEED);
      turnRight90();
    }

    // --- Drive the lane, scanning ---
    double distanceDriven = 0;
    bool rockGrabbed = false;

    LeftDriveSmart.resetPosition();
    double wheelCircIn = 319.19 / 25.4;
    double targetDeg   = (LANE_LENGTH / wheelCircIn) * 360.0;

    Drivetrain.setDriveVelocity(DRIVE_SPEED, percent);
    Drivetrain.drive(forward);

    while (fabs(LeftDriveSmart.position(degrees)) < targetDeg) {
      distanceDriven = (fabs(LeftDriveSmart.position(degrees)) / 360.0) * wheelCircIn;

      if (isMineralDetected()) {
        Drivetrain.stop();
        handleMineral();
        Drivetrain.drive(forward);
      }

      if (isRockDetected()) {
        Drivetrain.stop();
        grabRock();
        rockGrabbed = true;
        break;
      }

      wait(20, msec);
    }

    if (!rockGrabbed) {
      // Finished lane with no rock — just reverse back to spawn row
      Drivetrain.stop();
      distanceDriven = LANE_LENGTH;
    }

    // --- Return to spawn, drop if grabbed, then come back ---
    driveInchesBlindReverse(distanceDriven, RETURN_SPEED);

    // Turn right to face spawn direction
    turnRight90();

    // Drive back to spawn column (undo all lane shifts)
    double totalShift = lane * LANE_SHIFT;
    if (totalShift > 0) {
      driveInchesBlind(totalShift, RETURN_SPEED);
    }

    // At spawn — drop if we have a rock
    if (rockGrabbed) {
      dropRock();
    }

    // --- Go back out to resume at same lane position ---
    if (totalShift > 0) {
      driveInchesBlindReverse(totalShift, RETURN_SPEED);
    }

    // Turn left to face down field again
    turnLeft90();

    // If rock was grabbed mid-lane, finish the rest of that lane
    if (rockGrabbed && distanceDriven < LANE_LENGTH) {
      double remaining = LANE_LENGTH - distanceDriven;

      LeftDriveSmart.resetPosition();
      double remainDeg = (remaining / wheelCircIn) * 360.0;
      Drivetrain.setDriveVelocity(DRIVE_SPEED, percent);
      Drivetrain.drive(forward);

      while (fabs(LeftDriveSmart.position(degrees)) < remainDeg) {
        if (isMineralDetected()) {
          Drivetrain.stop();
          handleMineral();
          Drivetrain.drive(forward);
        }
        if (isRockDetected()) {
          Drivetrain.stop();
          grabRock();
          // Return to spawn again for this second rock
          double d2 = (fabs(LeftDriveSmart.position(degrees)) / 360.0) * wheelCircIn;
          driveInchesBlindReverse(distanceDriven + d2, RETURN_SPEED);
          turnRight90();
          if (totalShift > 0) driveInchesBlind(totalShift, RETURN_SPEED);
          dropRock();
          if (totalShift > 0) driveInchesBlindReverse(totalShift, RETURN_SPEED);
          turnLeft90();
          break;
        }
        wait(20, msec);
      }
      Drivetrain.stop();
    }

    // --- Reverse back to spawn row to prep for next lane ---
    driveInchesBlindReverse(LANE_LENGTH, RETURN_SPEED);
  }

  // All lanes done
  lcdBig("COMPLETE!", color::green);
}


/*============================================================================*/
/*  STARTUP + MAIN                                                            */
/*============================================================================*/

void pre_auton() {
  vexcodeInit();
  lcdClear();
  lcdPrint("Lumena");
  lcdPrint("Calibrating inertial...");

  Inertial11.calibrate();
  waitUntil(!Inertial11.isCalibrating());
  lcdPrint("Inertial OK.");

  lcdClear();
  lcdPrint("Lumena ready.");
  Optical7.setLight(ledState::on);

  Drivetrain.setDriveVelocity(DRIVE_SPEED, percent);
  Drivetrain.setTurnVelocity(TURN_SPEED, percent);
  Drivetrain.setStopping(brake);
}

void autonomous() {
  armHold();        // hold arm up from the start
  wait(300, msec);

  // Phase 1 — Grab Olympus Mons immediately
  grabOlympusMons();
  wait(300, msec);

  // Phase 2 — Lawnmower with spawn returns
  lawnmower();
}

void usercontrol() {
  lcdClear();
  lcdPrint("Driver Control");
  while (true) { wait(20, msec); }
}

int main() {
  pre_auton();
  autonomous();
  while (true) { wait(100, msec); }
}
