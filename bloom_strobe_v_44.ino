/*
  Bloom Strobe – Demo Mode v4.4
  -------------------------------
  Manueller Modus: Potentiometer steuert 20–80 Hz
  Demo nach 10 s Inaktivität:
    Demo1: 36 Hz konstant, 10 s
    Demo2: 72 Hz konstant, 10 s
    Wechsel 3x → Pause 10 s
  Nach Pause: Zyklus wiederholt sich
  Poti-Bewegung bricht Demo sofort ab
*/

const byte LED_strip = 9;
const byte potPin = A0;

const bool INVERT_POT = true;
const int OFF_THRESHOLD = 30;
const float minHz = 20.0;
const float maxHz = 80.0;

// Demo Parameter
const float demo1Hz = 36.0;
const float demo2Hz = 72.0;
const unsigned long demoStepTime = 10000; // 10 s pro Demo
const int demoCycles = 3;                  // Demo1+Demo2 3x
const unsigned long pauseDuration = 10000; // 10 s Pause

// State variables
unsigned long lastPotChange = 0;
int lastPotValue = -1;
bool demoMode = false;
bool inPause = false;
int demoStep = 0;
int demoIteration = 0;
unsigned long demoStartTime = 0;

void setup() {
  pinMode(LED_strip, OUTPUT);
  Serial.begin(9600);
  Serial.println("Bloom Strobe – Demo Mode v4.4 Ready");
}

void loop() {
  int raw = analogRead(potPin);
  if (INVERT_POT) raw = 1023 - raw;

  unsigned long now = millis();

  // Poti-Bewegung -> Demo abbrechen
  if (abs(raw - lastPotValue) > 5) {
    lastPotChange = now;
    demoMode = false;
    inPause = false;
    demoStep = 0;
    demoIteration = 0;
    demoStartTime = 0;
    lastPotValue = raw;
  }

  // Demo starten nach 10 s Inaktivität
  if ((now - lastPotChange > 10000) && !demoMode) {
    demoMode = true;
    inPause = false;
    demoStep = 0;
    demoIteration = 0;
    demoStartTime = now;
    Serial.println("DEMO MODE START");
  }

  float strobeHz = 0.0;

  if (demoMode) {
    if (inPause) {
      if (now - demoStartTime >= pauseDuration) {
        inPause = false;
        demoStep = 0;
        demoIteration = 0;
        demoStartTime = now;
        Serial.println("DEMO RESTART");
      } else {
        digitalWrite(LED_strip, LOW);
        delay(10);
        return;
      }
    } else {
      // Demo Step
      if (demoStep % 2 == 0) strobeHz = demo1Hz;
      else strobeHz = demo2Hz;

      if (now - demoStartTime >= demoStepTime) {
        demoStep++;
        demoStartTime = now;

        if (demoStep >= demoCycles * 2) {
          inPause = true;
          demoStartTime = now;
          Serial.println("PAUSE LED OFF");
        }
      }
    }
  } else {
    // Manueller Modus
    if (raw <= OFF_THRESHOLD) {
      digitalWrite(LED_strip, LOW);
      delay(10);
      Serial.println("OFF");
      return;
    }
    strobeHz = map(raw, OFF_THRESHOLD, 1023, (int)(minHz * 10), (int)(maxHz * 10)) / 10.0;
    strobeHz = round(strobeHz * 2.0) / 2.0;
  }

  // LED Strobe
  float periodMs = 1000.0 / strobeHz;
  int onMs = 2; // kurzer Blitz

  digitalWrite(LED_strip, HIGH);
  delay(onMs);
  digitalWrite(LED_strip, LOW);
  delay((int)(periodMs - onMs));

  Serial.print("Hz: ");
  Serial.println(strobeHz, 1);
}
