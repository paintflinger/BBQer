#ifndef ThermoProbe_h
#define ThermoProbe_h


#define THERMISTORNOMINAL 100000 // resistance at 77 degrees F
#define TEMPERATURENOMINAL 77    // temp. for nominal resistance (almost always 77 F)
#define NUMSAMPLES 5             // how many samples to take and average, more takes longer, but is more 'smooth'
#define SERIESRESISTOR 100000    // the value of the 'other' resistor


class ThermoProbe {
    byte pin;
    uint16_t samples[NUMSAMPLES];
    int sampleIndex = 0;
    float savedTemp = 0.0;
    int savedAverageADC = 0;
    long resistanceAt77 = 100000;
    int betaCoefficient = 3950;
    unsigned long lastMs = 0;
    
  public:

    ThermoProbe(byte attachToPin, long resistanceAt77, int betaCoefficient) :
      pin(attachToPin), resistanceAt77(resistanceAt77), betaCoefficient(betaCoefficient) {
    }

    static int sortDesc(const void *cmp1, const void *cmp2) {
      // Need to cast the void * to int *
      int a = *((int *)cmp1);
      int b = *((int *)cmp2);
      // The comparison
      return a > b ? -1 : (a < b ? 1 : 0);
    }

    void setup() {
        pinMode(pin, INPUT);
        analogReference(EXTERNAL);
    }

    void loop() {

      if (millis() - lastMs > 100) {
        lastMs = millis();
  
        samples[sampleIndex] = analogRead(pin);
        sampleIndex++;
        if (sampleIndex >= NUMSAMPLES) {
          sampleIndex = 0;

          // sort
          qsort(samples, NUMSAMPLES, sizeof(samples[0]), sortDesc);
          
          // average all the samples out without 1 min and 1 max value
          // simple filter
          float average = 0;
          for (int i=1; i<NUMSAMPLES-1; i++) {
             average += samples[i];
          }
          average /= NUMSAMPLES-2;
         
	  savedAverageADC = average;

          // convert the value to resistance
          average = 1023 / average - 1;
          average = SERIESRESISTOR / average;

          float steinhart;
          steinhart = average / resistanceAt77;        // (R/Ro)
          steinhart = log(steinhart);                  // ln(R/Ro)
          steinhart /= betaCoefficient;                // 1/B * ln(R/Ro)
          steinhart += 1.0 / (TEMPERATURENOMINAL + 459.67); // + (1/To)
          steinhart = 1.0 / steinhart;                 // Invert
          steinhart -= 459.67;                         // convert to F

          savedTemp = steinhart < 0 ? 0 : steinhart;
        }
     
      }
    }

    float readFahrenheit() {
      return savedTemp;
    }

    int readADC() {
      return savedAverageADC;
    }
    
    void printState() {
        Serial.print("[PROBE ");
        Serial.print(pin);
        Serial.print("] -> ");
        Serial.print(readFahrenheit());
        Serial.println();
    }

};

#endif
