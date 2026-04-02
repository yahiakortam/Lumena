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
  initializeRandomSeed();
}

void playVexcodeSound(const char *soundName) {
  printf("VEXPlaySound:%s\n", soundName);
  wait(5, msec);
}

#pragma endregion VEXcode Generated Robot Configuration

/*----------------------------------------------------------------------------*/
/*    Module:       main.cpp                                                  */
/*    Author:       Team Lumena                                               */
/*    Created:      2026                                                      */
/*    Description:  NASA NCAS Rover Challenge                                 */
/*----------------------------------------------------------------------------*/

competition Competition;

/*============================================================================*/
/*  TUNING CONSTANTS                                                          */
/*============================================================================*/

const double DRIVE_SPEED_DEFAULT       = 30;
const double TURN_SPEED_DEFAULT        = 5;
const double TURN_90_DEGREES           = 97.0;

const double SEG_A_IN                  = 44.0;
const double SEG_B_IN                  = 20.0;

const double ROCK_DETECT_DISTANCE_MM   = 250.0;

const int ARM_RAISE_TIME_MS            = 2000;
const int ARM_LOWER_TIME_MS            = 5000;
const int CLAW_OPEN_TIME_MS            = 3000;
const int CLAW_CLOSE_TIME_MS           = 800;

const double ARM_RAISE_SPEED           = 20;
const double ARM_HOLD_SPEED            = 12;
const double ARM_LOWER_SPEED           = 20;  // actively drives arm down
const double CLAW_MOTOR_SPEED          = 25;

// Hue ranges for mineral detection (0-360 degrees)
// Green hue: ~80-160, Yellow hue: ~40-75
// TUNE these if detections are off
const double YELLOW_HUE_MIN            = 40.0;
const double YELLOW_HUE_MAX            = 75.0;
const double GREEN_HUE_MIN             = 80.0;
const double GREEN_HUE_MAX             = 160.0;
const double MINERAL_BRIGHTNESS_MIN    = 10.0; // ignore readings in near-darkness

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
  if (screenLine > 12) screenLine = 1;
}

/*============================================================================*/
/*  DRIVE HELPERS                                                             */
/*============================================================================*/

void setupDrivetrain() {
  Drivetrain.setDriveVelocity(DRIVE_SPEED_DEFAULT, percent);
  Drivetrain.setTurnVelocity(TURN_SPEED_DEFAULT, percent);
  Drivetrain.setStopping(brake);
}

/*============================================================================*/
/*  SENSOR DETECTION                                                          */
/*============================================================================*/

bool isMineralDetected() {
  // Use hue values instead of color enum for more reliable detection
  // Ignore if brightness is too low (sensor not over anything)
  if (Optical7.brightness() < MINERAL_BRIGHTNESS_MIN) return false;

  double hue = Optical7.hue();
  bool isYellow = (hue >= YELLOW_HUE_MIN && hue <= YELLOW_HUE_MAX);
  bool isGreen  = (hue >= GREEN_HUE_MIN  && hue <= GREEN_HUE_MAX);
  return (isYellow || isGreen);
}

bool isRockDetected() {
  return (Distance6.objectDistance(mm) <= ROCK_DETECT_DISTANCE_MM);
}

/*============================================================================*/
/*  MINERAL DISPLAY                                                           */
/*============================================================================*/

int mineralsFound = 0;

void handleMineralFound() {
  mineralsFound++;
  Drivetrain.stop();

  clearScreen();
  Brain.Screen.clearScreen(color::green);
  Brain.Screen.setCursor(2, 1);
  Brain.Screen.setFont(mono40);
  Brain.Screen.print("MINERAL!");
  Brain.Screen.setCursor(5, 1);
  Brain.Screen.setFont(mono20);
  Brain.Screen.print("Total: %d", mineralsFound);
  waitMs(2000);

  Brain.Screen.setFont(mono20);
  Brain.Screen.clearScreen(color::black);
  clearScreen();
}

/*============================================================================*/
/*  ARM CONTROL                                                               */
/*============================================================================*/

bool rockGrabbed = false;

void clawOpen() {
  RoboticArm3_mJ4.spin(reverse, CLAW_MOTOR_SPEED, percent);
  waitMs(CLAW_OPEN_TIME_MS);
  RoboticArm3_mJ4.stop();
}

void clawClose() {
  RoboticArm3_mJ4.spin(forward, CLAW_MOTOR_SPEED, percent);
  waitMs(CLAW_CLOSE_TIME_MS);
  RoboticArm3_mJ4.stop();
}

void armHoldUp() {
  // Keep motor spinning at low speed to hold arm up against gravity
  RoboticArm3_mJ2.spin(forward, ARM_HOLD_SPEED, percent);
}

void armRaise() {
  RoboticArm3_mJ2.spin(forward, ARM_RAISE_SPEED, percent);
  waitMs(ARM_RAISE_TIME_MS);
  armHoldUp();
}

void armLower() {
  // Actively drive arm DOWN using reverse direction
  // If arm goes up instead, swap reverse to forward here
  RoboticArm3_mJ2.spin(reverse, ARM_LOWER_SPEED, percent);
  waitMs(ARM_LOWER_TIME_MS);
  RoboticArm3_mJ2.stop();
}

void grabRock() {
  clearScreen();
  printLine("Opening claw...");
  clawOpen();
  waitMs(300);

  clearScreen();
  printLine("Lowering arm...");
  armLower();

  clearScreen();
  printLine("Grabbing...");
  clawClose();

  clearScreen();
  printLine("Raising arm...");
  armRaise();

  rockGrabbed = true;

  clearScreen();
  Brain.Screen.clearScreen(color::orange);
  Brain.Screen.setCursor(3, 1);
  Brain.Screen.setFont(mono40);
  Brain.Screen.print("GRABBED!");
  waitMs(1500);
  Brain.Screen.setFont(mono20);
  clearScreen();
}

/*============================================================================*/
/*  DRIVE SEGMENT                                                             */
/*============================================================================*/

void driveSegment(double distanceInches) {
  int steps = (int)(distanceInches / 1.0);
  for (int i = 0; i < steps; i++) {
    if (isMineralDetected()) {
      handleMineralFound();
    }
    if (!rockGrabbed && isRockDetected()) {
      Drivetrain.stop();
      grabRock();
    }
    Drivetrain.driveFor(forward, 1, inches);
  }
}

/*============================================================================*/
/*  SAFE SHUTDOWN                                                             */
/*============================================================================*/

void stopEverything() {
  Drivetrain.stop();
  RoboticArm3_mJ2.stop();
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

  clearScreen();
  printLine("Lumena");
  printLine("Raising arm...");
  armRaise();

  clearScreen();
  printLine("Lumena");
  printLine("Arm up. Ready.");

  Optical7.setLight(ledState::on);
}

void autonomous(void) {
  setupDrivetrain();
  waitMs(500);

  while (true) {
    Brain.Screen.setCursor(1, 1);
    Brain.Screen.print("Seg 1: 44in fwd");
    driveSegment(SEG_A_IN);
    waitMs(300);

    Drivetrain.turnFor(right, TURN_90_DEGREES, degrees);
    waitMs(300);

    Brain.Screen.setCursor(1, 1);
    Brain.Screen.print("Seg 2: 20in fwd");
    driveSegment(SEG_B_IN);
    waitMs(300);

    Drivetrain.turnFor(right, TURN_90_DEGREES, degrees);
    waitMs(300);

    Brain.Screen.setCursor(1, 1);
    Brain.Screen.print("Seg 3: 44in fwd");
    driveSegment(SEG_A_IN);
    waitMs(300);

    Drivetrain.turnFor(right, TURN_90_DEGREES, degrees);
    waitMs(300);
    Drivetrain.turnFor(right, TURN_90_DEGREES, degrees);
    waitMs(300);
  }
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
  autonomous();

  while (true) {
    wait(100, msec);
  }
}
