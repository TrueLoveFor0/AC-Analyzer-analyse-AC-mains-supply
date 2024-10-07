//ESP32 super mini (Makergo used)

// Defines
const int onboardLedPin = 8;  // Onboard LED  GPIO8/PWM

#define PULSE_PIN_1 9   // IO9
#define PULSE_PIN_2 10  // IO10
#define ANALOG_PIN 1    // Analog input on IO1 (Analog 1)
//??#define ANALOG_PIN 3  // Analog input on GPIO3 (A1)

unsigned long pulseWidth1Min = 1000000;  // Initialize to a large value
unsigned long pulseWidth1Max = 0;        // Initialize to a small value
unsigned long pulseWidth2Min = 1000000;  // Same for IO9
unsigned long pulseWidth2Max = 0;
unsigned long InvalidLowPulses = 0, InvalidHighPulses = 0;
unsigned long pulseWidth1;
unsigned long pulseWidth2;
unsigned long pulsePeriod;
float freq = 0;
float minFreq = 50, maxFreq = 50;

float emaPulsePeriod = 20000.00, emaFreq = 0;  // 50Hz Exponential moving average change to your period if needed

float alpha = 0.05;  // Average smoothing factor (between 0 and 1)

int statPrint = 0;  // statistic print counter (internal)

void setup() {

  pinMode(PULSE_PIN_1, INPUT);
  pinMode(PULSE_PIN_2, INPUT);
  pinMode(ANALOG_PIN, INPUT);

  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB
  }

  Serial.print("Sketch name: ");
  Serial.println(__FILE__);  // Prints the filename of the sketch

  Serial.print("Compiled on: ");
  Serial.print(__DATE__);  // Prints the date of the compilation
  Serial.print(" at ");
  Serial.println(__TIME__);  // Prints the time of the compilation

  // Set onboard LED as output
  pinMode(onboardLedPin, OUTPUT);
}

int CheckMinMax(void) {  // Check and update min and max valus
  int validPeriod = 0, minPulseWidht = 10;
  if (pulseWidth1 > minPulseWidht) {  // Valid pulse
    if (pulseWidth1 < pulseWidth1Min) pulseWidth1Min = pulseWidth1;
    if (pulseWidth1 > pulseWidth1Max) pulseWidth1Max = pulseWidth1;
    validPeriod = 1;
  } else {
    InvalidHighPulses++;
  }
  // Update min and max
  if (pulseWidth2 > minPulseWidht) {  // Valid pulse
    if (pulseWidth2 < pulseWidth2Min) pulseWidth2Min = pulseWidth2;
    if (pulseWidth2 > pulseWidth2Max) pulseWidth2Max = pulseWidth2;
  } else {
    InvalidLowPulses++;
    validPeriod = 0;
  }

  if (validPeriod) {
    pulsePeriod = pulseWidth1 + pulseWidth2;

    freq = (1.0 / pulsePeriod) * 1000000;
    emaPulsePeriod = (alpha * pulsePeriod) + ((1 - alpha) * emaPulsePeriod);  // update exponential moving average count
    emaFreq = (1.0 / emaPulsePeriod) * 1000000;

    if (freq < minFreq) minFreq = freq;
    if (freq > maxFreq) maxFreq = freq;
  }

  return validPeriod;
}

void printFreq(void) {

  Serial.print("AC main min: ");
  Serial.print(minFreq);

  Serial.print(" max: ");
  Serial.print(maxFreq);

  Serial.print(" actual: ");
  Serial.print(freq);

  Serial.print(" avg: ");
  Serial.print(emaFreq);

  Serial.println(" Hz ");
}

void printMinMax(void) {
  // Print the measured pulse width
  Serial.print("High / Low, period, exp average (us): H + L ");
  Serial.print(pulseWidth1);

  Serial.print(" + ");
  Serial.print(pulseWidth2);

  Serial.print(" = ");
  Serial.print(pulsePeriod);

  Serial.print(" - Avg ");
  Serial.print(emaPulsePeriod);
  Serial.print("us = ");

  Serial.print(freq);
  Serial.println(" Hz ");

  Serial.print("High min / max (us) = ");
  Serial.print(pulseWidth1Min);
  Serial.print(" / ");
  Serial.print(pulseWidth1Max);
  Serial.print(" invalid high (too short) # ");
  Serial.println(InvalidHighPulses);

  Serial.print("Low min / max (us) = ");
  Serial.print(pulseWidth2Min);
  Serial.print(" / ");
  Serial.print(pulseWidth2Max);
  Serial.print(" invalid low (too short) # ");
  Serial.println(InvalidLowPulses);
}

void loop() {

  digitalWrite(onboardLedPin, LOW);  // onboard LED shows measure cycle start

  // Measure the pulse width on IO8 (PULSE_PIN_1)
  // pulseIn(PULSE_PIN_1, HIGH);                              // wait on pulse start
  pulseWidth1 = pulseIn(PULSE_PIN_1, HIGH);  // For HIGH pulse (not in zero crossing point so pulse widht are not real)
  pulseIn(PULSE_PIN_2, LOW);
  pulseWidth2 = pulseIn(PULSE_PIN_2, LOW);  // For low pulse alt measure time between two pulse triggers

  if (CheckMinMax()) {  // Valid measuement

    printFreq();


    //if (statPrint++ % 20 == 0) {  // Arduino plotter output each % value
      Serial.print("Freq(Hz):");
      Serial.print(freq);
      Serial.print("\t");
      Serial.print("Avg:");
      Serial.print(emaFreq);
      Serial.print("\t");
      Serial.print("Min:");
      Serial.print(minFreq);
      Serial.print("\t");
      Serial.print("Max:");
      Serial.print(maxFreq);
      Serial.println("");
      
      //printMinMax();
    //}
  }

  // Wave output works barely with arduino plotter
  /*for (int i = 0; i < 100; i++) {
    // Read the analog value on GPIO3 (A1)
    int analogValue = analogRead(ANALOG_PIN);
    Serial.println(analogValue);
 
    /* Print the analog value
    Serial.print("Analog Value ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(analogValue);
  
    // Wait for 0.2 ms (200 microseconds) to complete 100 readings in 20 ms
    delayMicroseconds(200);
  } */

  digitalWrite(onboardLedPin, HIGH);  // Turn off onboard LED
  delay(900);                         // Delay between measurements
}
