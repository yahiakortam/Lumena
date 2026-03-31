#pragma region VEXcode Generated Robot Configuration
// Make sure all required headers are included.
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>


#include "vex.h"

using namespace vex;

// Brain should be defined by default
brain Brain;


// START V5 MACROS
#define waitUntil(condition)                                                   \
  do {                                                                         \
    wait(5, msec);                                                             \
  } while (!(condition))

#define repeat(iterations)                                                     \
  for (int iterator = 0; iterator < iterations; iterator++)
// END V5 MACROS


// Robot configuration code.
optical Optical7 = optical(PORT7);
motor LeftDriveSmart = motor(PORT1, ratio18_1, false);
motor RightDriveSmart = motor(PORT2, ratio18_1, true);
drivetrain Drivetrain = drivetrain(LeftDriveSmart, RightDriveSmart, 319.19, 295, 40, mm, 1);

distance Distance6 = distance(PORT6);
motor RoboticArm3_mJ1 = motor(PORT3, ratio18_1, false);
motor RoboticArm3_mJ2 = motor(PORT4, ratio18_1, true);
motor RoboticArm3_mJ3 = motor(PORT5, ratio18_1, false);
motor RoboticArm3_mJ4 = motor(PORT8, ratio18_1, false);
pot RoboticArm3_mJ1_pot = pot(Brain.ThreeWirePort.A);
pot RoboticArm3_mJ2_pot = pot(Brain.ThreeWirePort.B);
pot RoboticArm3_mJ3_pot = pot(Brain.ThreeWirePort.C);
pot RoboticArm3_mJ4_pot = pot(Brain.ThreeWirePort.D);
RoboticArm RoboticArm3 = RoboticArm(RoboticArm3_mJ1, RoboticArm3_mJ1_pot, RoboticArm3_mJ2, RoboticArm3_mJ2_pot, RoboticArm3_mJ3, RoboticArm3_mJ3_pot, RoboticArm3_mJ4, RoboticArm3_mJ4_pot);



// generating and setting random seed
void initializeRandomSeed(){
  int systemTime = Brain.Timer.systemHighResolution();
  double batteryCurrent = Brain.Battery.current();
  double batteryVoltage = Brain.Battery.voltage(voltageUnits::mV);

  // Combine these values into a single integer
  int seed = int(batteryVoltage + batteryCurrent * 100) + systemTime;

  // Set the seed
  srand(seed);
}

// Converts a color to a string
const char* convertColorToString(color col) {
  if (col == color::red) return "red";
  else if (col == color::green) return "green";
  else if (col == color::blue) return "blue";
  else if (col == color::white) return "white";
  else if (col == color::yellow) return "yellow";
  else if (col == color::orange) return "orange";
  else if (col == color::purple) return "purple";
  else if (col == color::cyan) return "cyan";
  else if (col == color::black) return "black";
  else if (col == color::transparent) return "transparent";
  else return "unknown";
}


void vexcodeInit() {

  //Initializing random seed.
  initializeRandomSeed(); 
}


// Helper to make playing sounds from the V5 in VEXcode easier and
// keeps the code cleaner by making it clear what is happening.
void playVexcodeSound(const char *soundName) {
  printf("VEXPlaySound:%s\n", soundName);
  wait(5, msec);
}

#pragma endregion VEXcode Generated Robot Configuration
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       Team Lumena                                               */
/*    Created:      2026                                                      */
/*    Description:  NASA NCAS Rover Challenge – Lawnmower Traverse            */
/*                                                                            */
/*----------------------------------------------------------------------------*/

// Competition class instance
competition Competition;


/*============================================================================*/
/*  TUNING CONSTANTS – TUNE ALL OF THESE ON THE REAL ROBOT                    */
/*============================================================================*/

//--- Speed defaults (percent) ---
const double DRIVE_SPEED_DEFAULT = 50;
const double TURN_SPEED_DEFAULT  = 45;

//--- Lawnmower traverse (MUST TUNE) ---
const double LANE_DISTANCE_IN    = 72.0;
const double SHIFT_DISTANCE_IN   = 6.0;
const double TURN_90_DEGREES     = 90.0;
const int NUM_LANES              = 12;

//--- Sensor thresholds (MUST TUNE) ---
const double MINERAL_BRIGHTNESS_THRESHOLD = 80;
const double ROCK_DETECT_DISTANCE_IN      = 10;

//--- Robotic arm positions (MUST TUNE on real robot) ---
const int TRAVEL_J1 = 0;
const int TRAVEL_J2 = 0;
const int TRAVEL_J3 = 0;

const int PICKUP_J1 = 45;
const int PICKUP_J2 = 30;
const int PICKUP_J3 = 20;

const int HOLD_J1 = 0;
const int HOLD_J2 = 0;
const int HOLD_J3 = 0;

const int CLAW_OPEN_ANGLE  = 90;
const int CLAW_CLOSED_ANGLE = 0;


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
/*  DRIVE HELPERS (using Drivetrain)                                          */
/*============================================================================*/

void stopDrive() {
  Drivetrain.stop();
}

void setupDrivetrain() {
  Drivetrain.setDriveVelocity(DRIVE_SPEED_DEFAULT, percent);
  Drivetrain.setTurnVelocity(TURN_SPEED_DEFAULT, percent);
  Drivetrain.setStopping(brake);
}


/*============================================================================*/
/*  ARM & CLAW HELPERS (using RoboticArm)                                    */
/*============================================================================*/

void armToTravelPosition() {
  RoboticArm3.moveToPositionJoint(TRAVEL_J1, TRAVEL_J2, TRAVEL_J3);
  waitMs(1000);
}

void armToPickupPosition() {
  RoboticArm3.moveToPositionJoint(PICKUP_J1, PICKUP_J2, PICKUP_J3);
  waitMs(1000);
}

void armToHoldPosition() {
  RoboticArm3.moveToPositionJoint(HOLD_J1, HOLD_J2, HOLD_J3);
  waitMs(1000);
}

void clawOpen() {
  RoboticArm3.setArmAngle(CLAW_OPEN_ANGLE);
  waitMs(500);
}

void clawClose() {
  RoboticArm3.setArmAngle(CLAW_CLOSED_ANGLE);
  waitMs(500);
}


/*============================================================================*/
/*  SENSOR DETECTION                                                          */
/*============================================================================*/

bool isMineralDetected() {
  return (Optical7.brightness() >= MINERAL_BRIGHTNESS_THRESHOLD);
}

bool isRockDetected() {
  double dist = Distance6.objectDistance(inches);
  return (dist > 0 && dist <= ROCK_DETECT_DISTANCE_IN);
}


/*============================================================================*/
/*  LAWNMOWER TRAVERSE                                                        */
/*============================================================================*/

enum DetectionType { NOTHING, MINERAL, ROCK };

int mineralsFound = 0;
int rocksFound    = 0;

DetectionType driveScanningDirection(bool goForward) {
  Drivetrain.setDriveVelocity(DRIVE_SPEED_DEFAULT, percent);

  if (goForward) {
    Drivetrain.drive(forward);
  } else {
    Drivetrain.drive(reverse);
  }

  int elapsed = 0;
  int maxTimeMs = (int)((LANE_DISTANCE_IN / 12.0) * 1000);
  while (elapsed < maxTimeMs) {
    if (isRockDetected()) {
      Drivetrain.stop();
      return ROCK;
    }
    if (isMineralDetected()) {
      Drivetrain.stop();
      return MINERAL;
    }
    waitMs(20);
    elapsed += 20;
  }

  Drivetrain.stop();
  return NOTHING;
}

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

void handleRockFound() {
  rocksFound++;
  clearScreen();
  printLine("ROCK DETECTED!");
  Brain.Screen.setCursor(3, 1);
  Brain.Screen.print("Rocks found: %d", rocksFound);

  printLine("Arm down...");
  armToPickupPosition();

  printLine("Grabbing...");
  clawClose();

  printLine("Lifting...");
  armToHoldPosition();

  clearScreen();
  Brain.Screen.setCursor(3, 1);
  Brain.Screen.setFont(mono40);
  Brain.Screen.print("ROCK HELD!");
  Brain.Screen.setFont(mono20);
  Brain.Screen.setCursor(5, 1);
  Brain.Screen.print("Total: %d", rocksFound);
  waitMs(5000);

  printLine("Setting down...");
  armToPickupPosition();

  clawOpen();

  armToTravelPosition();

  clearScreen();
  printLine("Rock set down. Done.");
}

void shiftToNextLane(bool shiftRight) {
  if (shiftRight) {
    Drivetrain.turnFor(right, TURN_90_DEGREES, degrees);
  } else {
    Drivetrain.turnFor(left, TURN_90_DEGREES, degrees);
  }
  waitMs(300);

  Drivetrain.driveFor(forward, SHIFT_DISTANCE_IN, inches);
  waitMs(300);

  if (shiftRight) {
    Drivetrain.turnFor(right, TURN_90_DEGREES, degrees);
  } else {
    Drivetrain.turnFor(left, TURN_90_DEGREES, degrees);
  }
  waitMs(300);
}

void runLawnmower() {
  clearScreen();
  printLine("== LAWNMOWER ==");

  mineralsFound = 0;
  rocksFound = 0;

  setupDrivetrain();
  Optical7.setLight(ledState::on);
  waitMs(300);

  for (int lane = 0; lane < NUM_LANES; lane++) {
    bool goForward = (lane % 2 == 0);

    Brain.Screen.setCursor(2, 1);
    Brain.Screen.clearLine();
    Brain.Screen.print("Lane %d/%d %s", lane + 1, NUM_LANES,
                       goForward ? ">>>" : "<<<");

    DetectionType result = driveScanningDirection(goForward);

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

  clearScreen();
  printLine("== DONE ==");
  Brain.Screen.setCursor(3, 1);
  Brain.Screen.print("Minerals: %d", mineralsFound);
  Brain.Screen.setCursor(4, 1);
  Brain.Screen.print("Rocks: %d", rocksFound);

  Optical7.setLight(ledState::off);
}


/*============================================================================*/
/*  SAFE SHUTDOWN                                                             */
/*============================================================================*/

void stopEverything() {
  Drivetrain.stop();
  RoboticArm3.emergencyStop();
}


/*============================================================================*/
/*  COMPETITION CALLBACKS                                                     */
/*============================================================================*/

void pre_auton(void) {
  vexcodeInit();
  clearScreen();
  printLine("Lumena");

  printLine("Opening claw...");
  clawOpen();
  printLine("Raising arm...");
  armToTravelPosition();

  printLine("Ready.");
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
  calibrateDrivetrain();
  pre_auton();

  // Run autonomous immediately
  autonomous();

  // Idle forever after
  while (true) {
    wait(100, msec);
  }
}
