/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       Team Lumena                                               */
/*    Created:      2026                                                      */
/*    Description:  NASA NCAS – Competition 2: Rover Rescue                   */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/*============================================================================*/
/*  HARDWARE (defined in pragma region above):                                */
/*    Drivetrain    – ports 1 & 2                                             */
/*    Optical7      – port 7  (facing down for white tape)                    */
/*    Distance6     – port 6  (facing forward for stranded rover)             */
/*    RoboticArm3   – joints on ports 3, 4, 5, 8                             */
/*============================================================================*/

// Competition class instance
competition Competition;


/*============================================================================*/
/*  TUNING CONSTANTS – TUNE ALL OF THESE ON THE REAL ROBOT                    */
/*============================================================================*/

//--- Speed defaults (percent) ---
const double DRIVE_SPEED_DEFAULT = 50;
const double TURN_SPEED_DEFAULT  = 45;
const double TOW_SPEED           = 35;     // slower when towing

//--- Lawnmower traverse (MUST TUNE) ---
const double LANE_DISTANCE_IN    = 72.0;
const double SHIFT_DISTANCE_IN   = 6.0;
const double TURN_90_DEGREES     = 90.0;
const int NUM_LANES              = 12;

//--- Sensor thresholds (MUST TUNE) ---
const double MINERAL_BRIGHTNESS_THRESHOLD = 80;
const double ROVER_DETECT_DISTANCE_IN     = 10;   // stranded rover detection

//--- Robotic arm positions (MUST TUNE on real robot) ---
const int TRAVEL_J1 = 0;
const int TRAVEL_J2 = 0;
const int TRAVEL_J3 = 0;

const int GRAB_J1 = 45;     // arm down to grab stranded rover
const int GRAB_J2 = 30;
const int GRAB_J3 = 20;

const int TOW_J1 = 10;      // arm position while towing (low but off ground)
const int TOW_J2 = 10;
const int TOW_J3 = 10;

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
/*  DRIVE HELPERS                                                             */
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
/*  ARM & CLAW HELPERS                                                        */
/*============================================================================*/

void armToTravelPosition() {
  RoboticArm3.moveToPositionJoint(TRAVEL_J1, TRAVEL_J2, TRAVEL_J3);
  waitMs(1000);
}

void armToGrabPosition() {
  RoboticArm3.moveToPositionJoint(GRAB_J1, GRAB_J2, GRAB_J3);
  waitMs(1000);
}

void armToTowPosition() {
  RoboticArm3.moveToPositionJoint(TOW_J1, TOW_J2, TOW_J3);
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

bool isRoverDetected() {
  double dist = Distance6.objectDistance(inches);
  return (dist > 0 && dist <= ROVER_DETECT_DISTANCE_IN);
}


/*============================================================================*/
/*  TRACKING – keeps track of distance driven so we can reverse back          */
/*============================================================================*/

double totalForwardInches = 0;    // how far we've driven forward total
int totalTurnsRight       = 0;    // how many right turns we've made
int currentLane           = 0;    // which lane we found the rover on


/*============================================================================*/
/*  SEARCH, GRAB, AND RETURN                                                  */
/*============================================================================*/

enum DetectionType { NOTHING, MINERAL, ROVER_FOUND };

int mineralsFound = 0;

// Drive in a direction while checking both sensors
DetectionType driveScanningDirection(bool goForward, double *distanceDriven) {
  Drivetrain.setDriveVelocity(DRIVE_SPEED_DEFAULT, percent);

  if (goForward) {
    Drivetrain.drive(forward);
  } else {
    Drivetrain.drive(reverse);
  }

  int elapsed = 0;
  int maxTimeMs = (int)((LANE_DISTANCE_IN / 12.0) * 1000);
  *distanceDriven = 0;

  while (elapsed < maxTimeMs) {
    if (isRoverDetected()) {
      Drivetrain.stop();
      *distanceDriven = (double)elapsed / 1000.0 * 12.0;  // rough estimate
      return ROVER_FOUND;
    }
    if (isMineralDetected()) {
      Drivetrain.stop();
      // Announce mineral but keep going after
      mineralsFound++;
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

      // Resume driving
      if (goForward) {
        Drivetrain.drive(forward);
      } else {
        Drivetrain.drive(reverse);
      }
    }
    waitMs(20);
    elapsed += 20;
  }

  Drivetrain.stop();
  *distanceDriven = LANE_DISTANCE_IN;
  return NOTHING;
}

// Grab the stranded rover
void grabStrandedRover() {
  clearScreen();
  printLine("ROVER FOUND!");
  Brain.Screen.setCursor(3, 1);
  Brain.Screen.setFont(mono40);
  Brain.Screen.print("GRABBING...");
  Brain.Screen.setFont(mono20);

  // Arm down to grab position
  printLine("Arm down...");
  armToGrabPosition();

  // Close claw to secure it
  printLine("Claw close...");
  clawClose();
  waitMs(500);

  // Lift to tow position (low enough to drag, high enough to clear ground)
  printLine("Lifting to tow...");
  armToTowPosition();

  clearScreen();
  printLine("Rover secured!");
  waitMs(1000);
}

// Turn 180 degrees to face back toward start
void turnAround() {
  printLine("Turning around...");
  Drivetrain.turnFor(right, 180, degrees);
  waitMs(500);
}

// Drive back to start area
void returnToStart() {
  clearScreen();
  printLine("== RETURNING ==");

  Drivetrain.setDriveVelocity(TOW_SPEED, percent);
  Drivetrain.setStopping(brake);

  // Turn around to face start
  turnAround();

  // Drive back – rough estimate based on how many lanes we covered
  // Each lane is ~72 inches, we also shifted over between lanes
  // Simple approach: just drive forward for a long time at tow speed
  // The field is 72 inches, so worst case we need to cross the whole thing

  double returnDistance = LANE_DISTANCE_IN;  // at minimum, drive one full lane back

  clearScreen();
  printLine("Towing back...");
  Brain.Screen.setCursor(3, 1);
  Brain.Screen.print("Distance: %.0f in", returnDistance);

  Drivetrain.driveFor(forward, returnDistance, inches);
  waitMs(500);

  // We're back (hopefully)
  clearScreen();
  Brain.Screen.setCursor(3, 1);
  Brain.Screen.setFont(mono40);
  Brain.Screen.print("RESCUED!");
  Brain.Screen.setFont(mono20);
  Brain.Screen.setCursor(5, 1);
  Brain.Screen.print("Minerals: %d", mineralsFound);
  waitMs(3000);
}

// Shift over 6 inches to the next lane
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

// Main routine: search → grab → return
void runRoverRescue() {
  clearScreen();
  printLine("== ROVER RESCUE ==");

  mineralsFound = 0;
  bool roverFound = false;

  setupDrivetrain();
  Optical7.setLight(ledState::on);
  waitMs(300);

  // Phase 1: Lawnmower search
  for (int lane = 0; lane < NUM_LANES; lane++) {
    bool goForward = (lane % 2 == 0);
    currentLane = lane;

    Brain.Screen.setCursor(2, 1);
    Brain.Screen.clearLine();
    Brain.Screen.print("Search %d/%d %s", lane + 1, NUM_LANES,
                       goForward ? ">>>" : "<<<");

    double distDriven = 0;
    DetectionType result = driveScanningDirection(goForward, &distDriven);

    if (result == ROVER_FOUND) {
      roverFound = true;
      break;
    }

    if (lane < NUM_LANES - 1) {
      shiftToNextLane(goForward);
    }
  }

  // Phase 2: Grab stranded rover
  if (roverFound) {
    grabStrandedRover();

    // Phase 3: Return to start
    returnToStart();
  } else {
    clearScreen();
    printLine("Rover not found.");
    printLine("Search complete.");
  }

  // Summary
  clearScreen();
  printLine("== DONE ==");
  Brain.Screen.setCursor(3, 1);
  Brain.Screen.print("Minerals: %d", mineralsFound);
  Brain.Screen.setCursor(4, 1);
  Brain.Screen.print("Rescued: %s", roverFound ? "YES" : "NO");

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
  printLine("Lumena - Rescue");

  printLine("Opening claw...");
  clawOpen();
  printLine("Raising arm...");
  armToTravelPosition();

  printLine("Ready.");
}

void autonomous(void) {
  runRoverRescue();
  stopEverything();
  printLine("Mission complete.");
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

  // Run autonomous immediately
  autonomous();

  while (true) {
    wait(100, msec);
  }
}
