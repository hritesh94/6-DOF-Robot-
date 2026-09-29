// TW2KA 6-axis arm firmware — Teensy 4.1 + 6x TMC2209 (STEP/DIR) + 5 NC endstops
// Library: AccelStepper by Mike McCauley (Arduino Library Manager)
//
// Serial protocol (USB, one command per line):
//   M j1 j2 j3 j4 j5 j6   move to joint angles in degrees (absolute, coordinated)
//   V deg_per_s           set max joint speed
//   H                     home J1–J5 using the endstops, then go to the zero pose
//   H n                   home only joint n (1–5)
//   Z                     declare the current position as zero on all axes (no homing)
//   S                     smooth stop (decelerate)
//   E                     emergency stop: halt instantly and disable all drivers
//   R                     re-enable all drivers
//   N n 1 / N n 0         enable / disable joint n only (1–6)
//   J n +1 / J n -1       start jogging joint n (hold-to-jog, like the old GUI)
//   J 0 0                 stop jogging
//   D n +1 / D n -1       set which way joint n drives to find its switch (saved in EEPROM)
//   I n 1 / I n 0         reverse / normal motor direction for joint n (saved in EEPROM)
//   O n                   make joint n's current position its new 0 (saved; applied after every homing)
//   K n min max           set joint n's soft limits in degrees (saved)
//   G n spd               set joint n's steps per degree (saved), e.g. G 5 35.556
//   X                     forget saved directions, zeros and limits (then power-cycle)
//   T n                   hardware test: bit-bang 1600 steps forward and back on joint n
//   ?                     report position, switches and status
// Replies:
//   P j1..j6              joint positions (deg)
//   L s1..s5              endstop states, 1 = pressed
//   N e1..e6              driver enable states, 1 = enabled
//   D d1..d5              homing directions (+1/-1)
//   LIM a1 b1 ... a6 b6   soft limits in degrees
//   INV i1..i6            motor direction, 1 = reversed
//   SPD s1..s6            steps per degree in use
//   OK ..., ERR ..., HOMED

#include <AccelStepper.h>
#include <EEPROM.h>

// ============================ PINS (your board) ============================
const int N = 6;
const uint8_t STEP_PIN[N] = { 3,  6, 12, 23, 15, 37};
const uint8_t DIR_PIN[N]  = { 2,  5, 11, 22, 14, 36};
const uint8_t EN_PIN[N]   = { 4,  9, 26, 19, 13, 38};   // active LOW
const int8_t  SW_PIN[N]   = {32, 31, 30, 29, 28, -1};   // J6: no switch fitted (pin 27 reserved)
// Endstops: NC contact to signal, COM to GND, INPUT_PULLUP.
// Not pressed = LOW. Pressed = HIGH. A broken wire also reads HIGH, which fails safe.

// ======================= MECHANICS: CHECK EVERY LINE =======================
const float MOTOR_STEPS = 200.0;          // 1.8° motors
const float MICROSTEPS  = 8.0;            // TMC2209 standalone with MS1=MS2=GND
// Reductions from your drive table. Steps/deg = 200 x 8 x GEAR / 360:
//   J1 27.778, J2 115.111, J3 27.778, J4 16.667, J5 8.889, J6 8.889
float GEAR[N]     = {6.25, 25.9, 6.25, 3.75, 2.0, 2.0};   // J2 = 5.18:1 planetary x 80T/16T belt
bool  INVERT[N]   = {0, 1, 0, 0, 0, 0};   // J2 inverted so '+' matches the old Data_receive sketch
// Soft limits (deg). J1–J3 converted from the old sketch's J*minSteps/J*maxSteps.
// J4–J6 were placeholders in the old sketch too: measure real travel and update. Keep in sync with the app's LIMITS.
float LIM_MIN[N]  = {-144, -13, -54, -180, -20, -180};
float LIM_MAX[N]  = { 144,  87,  68,   10, 120,  180};
float speedDeg    = 30.0;                 // deg/s (app overrides)
float accelDeg    = 90.0;                 // deg/s^2
// Top speed per joint (deg/s), whatever speed the app asks for. J2 has the weakest
// motor (0.4 A) behind a 25.9:1 reduction, so it stalls and grinds if driven fast.
float MAX_DEG_S[N] = {45, 12, 40, 60, 90, 90};
// Jog speeds in STEPS per second, copied from the old Data_receive sketch so jogging
// looks the same even before the gear ratios above are filled in.
float JOG_DEG[N]  = {30, 10, 25, 40, 60, 60};        // jog speed, deg/s
float JOG_ACC_DEG[N] = {120, 40, 100, 160, 240, 240}; // jog ramp, deg/s^2 (stops within ~2-4 deg)

// ================================ HOMING ==================================
// Directions and back-off distances copied from the old Data_receive homing:
//   J1 seeks DIR LOW, J2 DIR HIGH (inverted joint), J3 LOW, J4 HIGH, J5 LOW
int8_t HOME_DIR[N]  = {-1, -1, -1, 1, -1, 0};
// Old sketch backed off J*calibrationReturn steps from the switch and called that 0.
// Using the same numbers puts 0 exactly where your old GUI had it.
const long OLD_RETURN_STEPS[N] = {4300, 1650, 1620, 200, 200, 0};
float  HOME_POS[N];                       // angle at the switch, computed in calcHome()
float  HOME_OFF[N] = {0, 0, 0, 0, 0, 0};  // your zero correction from "Set zero here" (saved)
const uint8_t HOME_ORDER[] = {4, 3, 2, 1, 0};   // J5, J4, J3, J2, J1 (wrist first, keeps it tucked)
float HOME_FAST[N] = {9, 4, 9, 20, 30, 0};   // search speed deg/s (old sketch's calibration speeds)
float HOME_SLOW[N] = {2, 1, 2, 5, 8, 0};     // precise second touch
float backoffDeg  = 5.0;                  // retreat after first touch
float SEARCH_MAX[N] = {360, 120, 150, 360, 200, 0}; // give up if no switch within this travel (deg)
// Before homing, jogging is allowed but soft limits are OFF (angles are relative to power-up).
// Endstops still stop the arm. Home before using Cartesian/IK moves.
bool warnedUnhomed = false;
// ==========================================================================

AccelStepper *m[N];
float spd[N];              // steps per degree
bool enabled = true, ready = false, aborted = false;
bool homedJ[N] = {0, 0, 0, 0, 0, 1};              // J6 has no switch   // enabled = at least one driver on
bool jEn[N] = {1, 1, 1, 1, 1, 1};                      // per-joint enable
bool swState[N], swRaw[N]; uint32_t swSince[N];
String rx;

// ------------------------------- helpers ---------------------------------
void reportEnable() {
  Serial.print("N");
  for (int i = 0; i < N; i++) { Serial.print(' '); Serial.print(jEn[i] ? 1 : 0); }
  Serial.println();
}
void setJoint(int i, bool on) {
  if (!on) m[i]->setCurrentPosition(m[i]->currentPosition());   // stop this joint first
  jEn[i] = on;
  digitalWrite(EN_PIN[i], on ? LOW : HIGH);
  enabled = false; for (int k = 0; k < N; k++) enabled |= jEn[k];
}
void setEnable(bool on) { for (int i = 0; i < N; i++) setJoint(i, on); reportEnable(); }

void report() {
  Serial.print("P");
  for (int i = 0; i < N; i++) { Serial.print(' '); Serial.print(m[i]->currentPosition() / spd[i], 2); }
  Serial.println();
}
void reportHomeDir() {
  Serial.print("D");
  for (int i = 0; i < 5; i++) { Serial.print(' '); Serial.print((int)HOME_DIR[i]); }
  Serial.println();
  Serial.print("INV");
  for (int i = 0; i < N; i++) { Serial.print(' '); Serial.print(INVERT[i] ? 1 : 0); }
  Serial.println();
  Serial.print("SPD");
  for (int i = 0; i < N; i++) { Serial.print(' '); Serial.print(spd[i], 3); }
  Serial.println();
  Serial.print("LIM");
  for (int i = 0; i < N; i++) { Serial.print(' '); Serial.print(LIM_MIN[i], 1); Serial.print(' '); Serial.print(LIM_MAX[i], 1); }
  Serial.println();
}

// Flip which way a joint seeks its switch. The soft limits were written for the
// original direction, so mirror them too (the switch moves to the other end).
// EEPROM layout: [0] magic, [1..5] HOME_DIR+1, [6..11] INVERT
const uint8_t EE_MAGIC = 0xB8, EE_MAGIC_OLD = 0xB7;   // 0xB7 = older firmware (directions only)
// [15] second magic, [16..] zeros and limits you set from the app
const uint8_t EE_CAL_MAGIC = 0xC2, EE_CAL_MAGIC_OLD = 0xC1;   // C1 = older block without steps/deg
struct Cal { float lmin[N], lmax[N], off[N], spd[N]; };
void saveEE() {
  EEPROM.write(0, EE_MAGIC);
  for (int k = 0; k < 5; k++) EEPROM.write(1 + k, (uint8_t)(HOME_DIR[k] + 1));
  for (int k = 0; k < N; k++) EEPROM.write(6 + k, INVERT[k] ? 1 : 0);
  Cal c; for (int k = 0; k < N; k++) { c.lmin[k] = LIM_MIN[k]; c.lmax[k] = LIM_MAX[k]; c.off[k] = HOME_OFF[k]; c.spd[k] = spd[k]; }
  EEPROM.put(16, c); EEPROM.write(15, EE_CAL_MAGIC);
}
bool okNum(float x) { return x == x && fabs(x) < 10000; }   // rejects NaN and blank EEPROM
void calcHome(int i) { HOME_POS[i] = HOME_DIR[i] * OLD_RETURN_STEPS[i] / spd[i] + HOME_OFF[i]; }
void mirrorLimits(int i) { float a = LIM_MIN[i], b = LIM_MAX[i]; LIM_MIN[i] = -b; LIM_MAX[i] = -a; }
void setHomeDir(int i, int d, bool save) {
  if (d == HOME_DIR[i]) return;
  mirrorLimits(i);
  HOME_DIR[i] = d;
  HOME_OFF[i] = 0;                         // other switch side: old zero correction no longer applies
  calcHome(i);
  if (save) saveEE();
}
// Reverse a motor so its '+' matches the 3D model. The arm doesn't move: its angle
// just changes sign, and homing still drives the same physical way (toward the switch).
void setInvert(int i, bool inv, bool save) {
  if (inv == INVERT[i]) return;
  INVERT[i] = inv;
  m[i]->setPinsInverted(inv, false, false);
  m[i]->setCurrentPosition(-m[i]->currentPosition());
  mirrorLimits(i);
  HOME_DIR[i] = -HOME_DIR[i];
  HOME_OFF[i] = -HOME_OFF[i];
  calcHome(i);
  if (save) saveEE();
}

void reportSwitches() {
  Serial.print("L");
  for (int i = 0; i < N; i++) if (SW_PIN[i] >= 0) { Serial.print(' '); Serial.print(swState[i] ? 1 : 0); }
  Serial.println();
}

// 2 ms debounce; returns true if any switch changed
bool updateSwitches() {
  bool changed = false; uint32_t now = micros();
  for (int i = 0; i < N; i++) {
    if (SW_PIN[i] < 0) continue;
    bool r = digitalRead(SW_PIN[i]) == HIGH;
    if (r != swRaw[i]) { swRaw[i] = r; swSince[i] = now; }
    else if (r != swState[i] && now - swSince[i] > 2000) { swState[i] = r; changed = true; }
  }
  return changed;
}

void haltAll() { for (int i = 0; i < N; i++) m[i]->setCurrentPosition(m[i]->currentPosition()); }

// Read serial without blocking; returns true when a full line is in `out`
bool readLine(String &out) {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') { out = rx; rx = ""; out.trim(); return true; }
    if (c != '\r' && rx.length() < 120) rx += c;
  }
  return false;
}

// Called inside blocking homing loops: only E and S are accepted
bool checkAbort() {
  String s;
  if (readLine(s) && s.length()) {
    if (s[0] == 'E') { haltAll(); setEnable(false); Serial.println("OK E-STOP drivers disabled"); aborted = true; }
    else if (s[0] == 'S') { haltAll(); Serial.println("OK homing cancelled"); aborted = true; }
    else Serial.println("ERR busy homing");
  }
  return aborted;
}

// Split "M 1 2 3" style arguments without sscanf (float sscanf is unreliable on some cores)
int parseArgs(const String &s, float *out, int maxN) {
  int n = 0, i = 1, L = s.length();
  while (n < maxN) {
    while (i < L && s[i] == ' ') i++;
    if (i >= L) break;
    int j = i; while (j < L && s[j] != ' ') j++;
    out[n++] = s.substring(i, j).toFloat();
    i = j;
  }
  return n;
}

// ------------------------------- motion ----------------------------------
void moveJoints(float *tgt) {
  float dDeg[N], maxD = 0;
  for (int i = 0; i < N; i++) {
    if (!jEn[i]) tgt[i] = m[i]->currentPosition() / spd[i];   // disabled joint: hold its count, send no steps
    if (ready) tgt[i] = constrain(tgt[i], LIM_MIN[i], LIM_MAX[i]);   // soft limits only mean something after homing
    dDeg[i] = fabs(tgt[i] - m[i]->currentPosition() / spd[i]);
    maxD = max(maxD, dDeg[i]);
  }
  if (maxD < 0.001) return;
  float T = 0;                                 // move time set by the slowest-allowed joint
  for (int i = 0; i < N; i++) T = max(T, dDeg[i] / min(speedDeg, MAX_DEG_S[i]));
  for (int i = 0; i < N; i++) {               // all joints finish together
    float v = max(dDeg[i] / T, 0.02f * maxD / T);   // this joint's cruise speed, deg/s
    float f = v * T / maxD;                    // share of the move, for a matching ramp
    m[i]->setMaxSpeed(v * spd[i]);
    m[i]->setAcceleration(accelDeg * f * spd[i]);
    m[i]->moveTo(lround(tgt[i] * spd[i]));
  }
}

// Constant-speed jog on one joint until the switch reaches `wantPressed`
bool jogUntil(int i, int dir, float degPerSec, bool wantPressed, float maxDeg) {
  long start = m[i]->currentPosition(), maxSteps = lround(maxDeg * spd[i]);
  m[i]->setMaxSpeed(degPerSec * spd[i] * 1.1);
  m[i]->setSpeed(dir * degPerSec * spd[i]);
  uint32_t t = millis();
  while (true) {
    updateSwitches();
    if (swState[i] == wantPressed) return true;
    if (labs(m[i]->currentPosition() - start) > maxSteps) return false;
    m[i]->runSpeed();
    if (millis() - t > 100) { t = millis(); report(); reportSwitches(); if (checkAbort()) return false; }
  }
}

bool homeAxis(int i) {
  if (!jEn[i]) { Serial.print("ERR J"); Serial.print(i + 1); Serial.println(" is disabled"); return false; }
  if (SW_PIN[i] < 0 || HOME_DIR[i] == 0) { Serial.print("ERR J"); Serial.print(i + 1); Serial.println(" has no endstop"); return false; }
  Serial.print("OK homing J"); Serial.println(i + 1);
  int d = HOME_DIR[i];
  if (swState[i] && !jogUntil(i, -d, HOME_FAST[i], false, 30)) goto fail;          // start off the switch
  if (!jogUntil(i, d, HOME_FAST[i], true, SEARCH_MAX[i])) goto fail;                // fast search
  if (!jogUntil(i, -d, HOME_FAST[i], false, 30)) goto fail;                        // release
  { long p = m[i]->currentPosition() - d * lround(backoffDeg * spd[i]);           // back off a bit more
    m[i]->setMaxSpeed(HOME_FAST[i] * spd[i]); m[i]->setAcceleration(accelDeg * spd[i]); m[i]->moveTo(p);
    while (m[i]->distanceToGo()) { m[i]->run(); if (checkAbort()) goto fail; } }
  if (!jogUntil(i, d, HOME_SLOW[i], true, backoffDeg * 3)) goto fail;              // slow precise touch
  m[i]->setCurrentPosition(lround(HOME_POS[i] * spd[i]));
  homedJ[i] = true;
  delay(5); updateSwitches(); delay(3); updateSwitches(); reportSwitches();   // show the pressed switch in the app
  return true;
fail:
  if (!aborted) { Serial.print("ERR J"); Serial.print(i + 1); Serial.println(" switch not found"); }
  return false;
}

void homeAll(int only) {
  if (!enabled) { Serial.println("ERR disabled, send R"); return; }
  aborted = false; haltAll();
  bool ok = true;
  if (only >= 0) ok = homeAxis(only);
  else for (uint8_t k = 0; k < sizeof(HOME_ORDER) && ok; k++) ok = homeAxis(HOME_ORDER[k]);
  if (!ok) { Serial.println("ERR homing failed"); report(); return; }
  if (only < 0) {                                  // J6 has no switch: its power-up position counts as 0
    ready = true;
    float zero[N] = {0, 0, 0, 0, 0, 0}; moveJoints(zero);
    Serial.println("HOMED");
  } else {
    Serial.print("OK J"); Serial.print(only + 1); Serial.println(" homed");
    bool all = true; for (int k = 0; k < N; k++) all &= homedJ[k];
    if (all && !ready) { ready = true; Serial.println("OK homed"); }
  }
  report();
}

// ------------------------------- commands --------------------------------
void handle(String s) {
  if (s.length() == 0) return;
  char c = s[0];
  if (c == 'M') {
    if (!enabled) { Serial.println("ERR disabled, send R"); return; }
    if (!ready && !warnedUnhomed) { Serial.println("WARN not homed: jogging with soft limits OFF"); warnedUnhomed = true; }
    float t[N];
    if (parseArgs(s, t, N) == N) { moveJoints(t); Serial.println("OK M"); }
    else Serial.println("ERR need 6 angles");
  } else if (c == 'H') {
    int j = s.substring(1).toInt();
    homeAll(j >= 1 && j <= 5 ? j - 1 : -1);
  } else if (c == 'V') {
    speedDeg = constrain(s.substring(1).toFloat(), 1.0f, 360.0f);
    Serial.print("OK speed "); Serial.println(speedDeg);
  } else if (c == 'Z') {
    // Zero every joint here. For homed joints this works like "O": the zero is saved
    // and the soft limits move with it, so they stay at the same physical place.
    bool saved = false;
    for (int i = 0; i < N; i++) {
      m[i]->setCurrentPosition(m[i]->currentPosition());
      float cur = m[i]->currentPosition() / spd[i];
      if (homedJ[i]) { HOME_OFF[i] -= cur; LIM_MIN[i] -= cur; LIM_MAX[i] -= cur; calcHome(i); saved = true; }
      m[i]->setCurrentPosition(0);
    }
    if (saved) saveEE();
    ready = true; Serial.println("OK zero set"); reportHomeDir(); report();
  } else if (c == 'S') {
    for (int i = 0; i < N; i++) m[i]->stop();
    Serial.println("OK stopping");
  } else if (c == 'E') {
    haltAll(); setEnable(false);
    Serial.println("OK E-STOP drivers disabled"); report();
  } else if (c == 'R') {
    setEnable(true); Serial.println("OK enabled");
  } else if (c == 'J') {
    float a[2];
    if (parseArgs(s, a, 2) != 2) { Serial.println("ERR use: J <joint 1-6> <+1|-1>, or J 0 0 to stop"); return; }
    int j = (int)a[0], d = (a[1] > 0) - (a[1] < 0);
    if (j == 0 || d == 0) { for (int i = 0; i < N; i++) m[i]->stop(); Serial.println("OK jog stop"); return; }
    if (j < 1 || j > N) { Serial.println("ERR bad joint"); return; }
    int i = j - 1;
    if (!jEn[i]) { Serial.print("ERR J"); Serial.print(j); Serial.println(" is disabled"); return; }
    if (!ready && !warnedUnhomed) { Serial.println("WARN not homed: jogging with soft limits OFF"); warnedUnhomed = true; }
    long tgt = d > 0 ? (ready ? lround(LIM_MAX[i] * spd[i]) : m[i]->currentPosition() + 100000000L)
                     : (ready ? lround(LIM_MIN[i] * spd[i]) : m[i]->currentPosition() - 100000000L);
    m[i]->setMaxSpeed(JOG_DEG[i] * spd[i]);
    m[i]->setAcceleration(JOG_ACC_DEG[i] * spd[i]);
    m[i]->moveTo(tgt);
    Serial.print("OK jog J"); Serial.print(j); Serial.println(d > 0 ? "+" : "-");
  } else if (c == 'D') {
    float a[2];
    int j = 0, d = 0;
    if (parseArgs(s, a, 2) == 2) { j = (int)a[0]; d = (a[1] > 0) - (a[1] < 0); }
    if (j < 1 || j > 5 || d == 0) { Serial.println("ERR use: D <joint 1-5> <+1|-1>"); return; }
    setHomeDir(j - 1, d, true);
    Serial.print("OK J"); Serial.print(j); Serial.print(" homes toward "); Serial.println(d > 0 ? "+" : "-");
    reportHomeDir();
  } else if (c == 'I') {
    float a[2];
    if (parseArgs(s, a, 2) != 2 || a[0] < 1 || a[0] > N) { Serial.println("ERR use: I <joint 1-6> <0|1>"); return; }
    int i = (int)a[0] - 1;
    for (int k = 0; k < N; k++) m[k]->setCurrentPosition(m[k]->currentPosition());   // stop before flipping
    setInvert(i, a[1] > 0.5, true);
    Serial.print("OK J"); Serial.print(i + 1); Serial.println(INVERT[i] ? " reversed" : " normal");
    reportHomeDir(); report();
  } else if (c == 'O') {
    int j = s.substring(1).toInt();
    if (j < 1 || j > N) { Serial.println("ERR use: O <joint 1-6>"); return; }
    int i = j - 1;
    if (!homedJ[i]) { Serial.print("ERR home J"); Serial.print(j); Serial.println(" first, so the new zero can be tied to its switch"); return; }
    m[i]->setCurrentPosition(m[i]->currentPosition());       // stop this joint
    float cur = m[i]->currentPosition() / spd[i];
    m[i]->setCurrentPosition(0);
    HOME_OFF[i] -= cur; LIM_MIN[i] -= cur; LIM_MAX[i] -= cur;  // limits stay at the same physical place
    calcHome(i); saveEE();
    Serial.print("OK J"); Serial.print(j); Serial.println(" zero set here");
    reportHomeDir(); report();
  } else if (c == 'K') {
    float a[3];
    if (parseArgs(s, a, 3) != 3 || a[0] < 1 || a[0] > N || !(a[1] < a[2])) { Serial.println("ERR use: K <joint 1-6> <min> <max>, min < max"); return; }
    int i = (int)a[0] - 1;
    LIM_MIN[i] = constrain(a[1], -720.0f, 720.0f); LIM_MAX[i] = constrain(a[2], -720.0f, 720.0f);
    saveEE();
    Serial.print("OK J"); Serial.print(i + 1); Serial.println(" limits set");
    reportHomeDir();
  } else if (c == 'G') {
    float a[2];
    if (parseArgs(s, a, 2) != 2 || a[0] < 1 || a[0] > N || a[1] < 0.5 || a[1] > 2000) { Serial.println("ERR use: G <joint 1-6> <steps per degree 0.5-2000>"); return; }
    int i = (int)a[0] - 1;
    m[i]->setCurrentPosition(m[i]->currentPosition());       // stop this joint
    spd[i] = a[1]; calcHome(i); saveEE();
    if (SW_PIN[i] >= 0) { homedJ[i] = false; ready = false; }  // angles changed scale: home again
    Serial.print("OK J"); Serial.print(i + 1); Serial.print(" steps/deg "); Serial.println(spd[i], 3);
    if (SW_PIN[i] >= 0) Serial.println("OK not homed");
    reportHomeDir(); report();
  } else if (c == 'X') {
    EEPROM.write(0, 0); EEPROM.write(15, 0);
    Serial.println("OK calibration cleared: power-cycle the Teensy to load defaults");
  } else if (c == 'T') {
    int j = s.substring(1).toInt();
    if (j < 1 || j > N) { Serial.println("ERR use: T <joint 1-6>"); return; }
    int i = j - 1;
    Serial.print("OK test J"); Serial.print(j); Serial.println(" running");
    digitalWrite(EN_PIN[i], LOW);                        // same raw method as Data_receive
    for (int dir = 1; dir >= 0; dir--) {
      digitalWrite(DIR_PIN[i], dir ? HIGH : LOW);
      delayMicroseconds(50);
      for (int k = 0; k < 1600; k++) {
        digitalWrite(STEP_PIN[i], HIGH); delayMicroseconds(5);
        digitalWrite(STEP_PIN[i], LOW);  delayMicroseconds(495);
      }
      delay(200);
    }
    if (!jEn[i]) digitalWrite(EN_PIN[i], HIGH);
    Serial.print("OK test J"); Serial.print(j); Serial.println(" done");
  } else if (c == 'N') {
    float a[2]; int j = 0, on = -1;
    if (parseArgs(s, a, 2) == 2) { j = (int)a[0]; on = (int)a[1]; }
    if (j >= 1 && j <= N && (on == 0 || on == 1)) {
      setJoint(j - 1, on);
      Serial.print("OK J"); Serial.print(j); Serial.println(on ? " enabled" : " disabled");
      if (!on) Serial.println("WARN a disabled joint can be moved by hand or fall under gravity; re-home before trusting it");
      reportEnable(); report();
    } else Serial.println("ERR use: N <joint 1-6> <0|1>");
  } else if (c == '?') {
    report(); reportSwitches(); reportEnable(); reportHomeDir();
    Serial.println(ready ? "OK homed" : "OK not homed");
  } else {
    Serial.println("ERR unknown command");
  }
}

// --------------------------------- main ----------------------------------
void setup() {
  Serial.begin(115200);
  for (int i = 0; i < N; i++) {
    pinMode(EN_PIN[i], OUTPUT);
    if (SW_PIN[i] >= 0) pinMode(SW_PIN[i], INPUT_PULLUP);
    m[i] = new AccelStepper(AccelStepper::DRIVER, STEP_PIN[i], DIR_PIN[i]);
    m[i]->setMinPulseWidth(5);
    m[i]->setPinsInverted(INVERT[i], false, false);
    spd[i] = MOTOR_STEPS * MICROSTEPS * GEAR[i] / 360.0;
    calcHome(i);
    m[i]->setMaxSpeed(speedDeg * spd[i]);
    m[i]->setAcceleration(accelDeg * spd[i]);
  }
  uint8_t magic = EEPROM.read(0);                      // restore directions you set from the app
  if (magic == EE_MAGIC)
    for (int k = 0; k < N; k++) { uint8_t v = EEPROM.read(6 + k); if (v <= 1) setInvert(k, v == 1, false); }
  if (magic == EE_MAGIC || magic == EE_MAGIC_OLD)
    for (int k = 0; k < 5; k++) { int d = (int)EEPROM.read(1 + k) - 1; if (d == 1 || d == -1) setHomeDir(k, d, false); }
  uint8_t cm = EEPROM.read(15);
  if (magic == EE_MAGIC && (cm == EE_CAL_MAGIC || cm == EE_CAL_MAGIC_OLD)) {   // your zeros, limits, steps/deg
    Cal c; EEPROM.get(16, c);
    if (cm == EE_CAL_MAGIC)
      for (int k = 0; k < N; k++) if (okNum(c.spd[k]) && c.spd[k] > 0.5) { spd[k] = c.spd[k]; calcHome(k); }
    for (int k = 0; k < N; k++)
      if (okNum(c.lmin[k]) && okNum(c.lmax[k]) && okNum(c.off[k]) && c.lmin[k] < c.lmax[k]) {
        LIM_MIN[k] = c.lmin[k]; LIM_MAX[k] = c.lmax[k]; HOME_OFF[k] = c.off[k]; calcHome(k);
      }
  }
  setEnable(true);
  delay(5);
  for (int i = 0; i < N; i++) if (SW_PIN[i] >= 0) swState[i] = swRaw[i] = digitalRead(SW_PIN[i]) == HIGH;
  Serial.println("OK TW2KA ready, send H to home");
}

elapsedMillis sinceReport;
bool wasMoving = false;

void loop() {
  String s;
  if (readLine(s)) handle(s);

  if (updateSwitches()) reportSwitches();

  bool moving = false;
  for (int i = 0; i < N; i++) {
    // Endstop hit while driving toward it: stop everything immediately
    if (SW_PIN[i] >= 0 && swState[i] && m[i]->distanceToGo() * HOME_DIR[i] > 0) {
      haltAll();
      Serial.print("ERR limit J"); Serial.println(i + 1);
      report();
    }
    m[i]->run();
    if (m[i]->distanceToGo() != 0) moving = true;
  }
  if ((moving && sinceReport > 100) || (wasMoving && !moving)) { report(); sinceReport = 0; }
  wasMoving = moving;
}
