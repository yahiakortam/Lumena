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
/*  TUNING CONSTANTS — all magic numbers live here                            */
/*============================================================================*/

// Speeds
const double DRIVE_SPEED     = 30;   // percent — main driving speed
const double TURN_SPEED      = 10;   // percent — turning speed
const double ARM_RAISE_SPEED = 30;
const double ARM_HOLD_SPEED  = 15;
const double ARM_LOWER_SPEED = 20;
const double CLAW_SPEED      = 40;

// Lawnmower dimensions (inches)
const double LANE_FORWARD    = 44.0;  // first lane length
const double LANE_SHIFT      = 12.0;  // sideways shift between lanes
const double LANE_RETURN     = 44.0;  // return lane length

// Sensor thresholds
const double ROCK_DETECT_MM  = 240.0;
const double YELLOW_HUE_MIN  = 40.0;
const double YELLOW_HUE_MAX  = 75.0;
const double GREEN_HUE_MIN   = 80.0;
const double GREEN_HUE_MAX   = 160.0;
const double BRIGHT_MIN      = 10.0;

// Arm/claw timing (ms)
const int ARM_RAISE_MS       = 6000;
const int ARM_LOWER_MS       = 5000;
const int CLAW_OPEN_MS       = 4000;
const int CLAW_CLOSE_MS      = 1000;


/*============================================================================*/
/*  SMALL HELPERS                                                             */
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
/*  MINERAL + ROCK HANDLERS                                                   */
/*============================================================================*/

int mineralsFound = 0;

void handleMineral() {
  mineralsFound++;
  Drivetrain.stop();
  lcdBig("MINERAL!", color::green);
  Brain.Screen.setCursor(5, 1);
  Brain.Screen.setFont(mono20);
  Brain.Screen.print("Total: %d", mineralsFound);
  wait(2000, msec);
  lcdClear();
}

void handleRock() {
  Drivetrain.stop();

  // Lower arm to rock level FIRST, then open claw around it, then grab
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
  wait(1500, msec);
  lcdClear();
}


/*============================================================================*/
/*  DRIVING — straight line with continuous sensor scanning                   */
/*============================================================================*/

void driveInches(double inches) {
  LeftDriveSmart.resetPosition();

  double wheelCircIn = 319.19 / 25.4;          // ~12.57 inches
  double targetDeg   = (inches / wheelCircIn) * 360.0;

  Drivetrain.drive(forward);

  while (fabs(LeftDriveSmart.position(degrees)) < targetDeg) {
    if (isMineralDetected()) {
      Drivetrain.stop();
      handleMineral();
      Drivetrain.drive(forward);
    }
    if (isRockDetected()) {
      Drivetrain.stop();
      handleRock();
      Drivetrain.drive(forward);
    }
    wait(20, msec);
  }

  Drivetrain.stop();
}


/*============================================================================*/
/*  TURNING — simple drivetrain turns                                        */
/*============================================================================*/

void turnRight90() {
  Drivetrain.turnFor(right, 90, degrees, true);
  //wait(200, msec);
}

void turnLeft90() {
  Drivetrain.turnFor(left, 90, degrees, true);
  // wait(200, msec);
}


/*============================================================================*/
/*  LAWNMOWER PATTERN                                                         */
/*                                                                            */
/*  Each cycle:                                                               */
/*    1. Drive forward  44 in   (LANE_FORWARD)                                */
/*    2. Turn right 90°                                                       */
/*    3. Drive forward  12 in   (LANE_SHIFT)                                  */
/*    4. Turn right 90°         → now facing back                             */
/*    5. Drive forward  44 in   (LANE_RETURN)                                 */
/*    6. Turn left  90°                                                       */
/*    7. Drive forward  12 in   (LANE_SHIFT)                                  */
/*    8. Turn left  90°         → now facing forward again                    */
/*    9. Repeat                                                               */
/*                                                                            */
/*============================================================================*/

void lawnmower() {
  int lane = 0;

  while (true) {
    /* ---- Forward lane (44 in) ---- */
    lane++;
    Brain.Screen.setCursor(1, 1);
    Brain.Screen.print("Lane %d  FWD 44in  ", lane);
    driveInches(LANE_FORWARD);
    wait(300, msec);

    /* ---- Shift right into next lane ---- */
    turnRight90();
    driveInches(LANE_SHIFT);
    wait(200, msec);
    turnRight90();

    /* ---- Return lane (44 in) ---- */
    lane++;
    Brain.Screen.setCursor(1, 1);
    Brain.Screen.print("Lane %d  RET 44in  ", lane);
    driveInches(LANE_RETURN);
    wait(300, msec);

    /* ---- Shift left into next lane ---- */
    turnLeft90();
    driveInches(LANE_SHIFT);
    wait(200, msec);
    turnLeft90();
  }
}


/*============================================================================*/
/*  STARTUP + MAIN                                                            */
/*============================================================================*/

void pre_auton() {
  vexcodeInit();
  lcdClear();
  lcdPrint("Lumena");
  lcdPrint("Calibrating inertial...");

  // Single inertial calibration — only happens once here
  Inertial11.calibrate();
  waitUntil(!Inertial11.isCalibrating());
  lcdPrint("Inertial OK.");

  lcdClear();
  lcdPrint("Lumena ready.");
  Optical7.setLight(ledState::on);

  // Set drive speeds
  Drivetrain.setDriveVelocity(DRIVE_SPEED, percent);
  Drivetrain.setTurnVelocity(TURN_SPEED, percent);
  Drivetrain.setStopping(brake);
}

void autonomous() {
  armHold();   // arm starts raised — just hold it in place
  wait(500, msec);
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
