/*******************************************************************************
*
*  The Cognitive Power Meter Project (c-meter)
*
*  Coopyright (C) 2013 by Stgephen Makonin. All rights reserved.
*
*
*  The ammeter module file.
*
*******************************************************************************/

const word SAMPLE_FREQ = 2000;                      // The frequency at with to sample ADC
const word CT_COUNT = 4;                            // The number of CTs that can be connected
const double CT_MVOLTS = 333.0;                     // The max CT secondary output in mV
const double REF_MVOLTS = 3300.0;                   // The refernce V of the Due board
const double MAX_ADC_VAL = 4095.0;                  // The max bits of the ADC, 4095 for 12-bits
const double FINAL_PRECISION = 1000.0;              // 1000 is milli-Amperes
const double OPAMP_GAIN = 4.2;                      // The gain of the opamp amplifier adjusted for RMS(3.2 if peak) 
                                                    // The max ADC value that the ammeter will ever reach due to rectification, etc.
const word MAX_RECT_VAL = CT_MVOLTS * OPAMP_GAIN / REF_MVOLTS * MAX_ADC_VAL;

const word CT_PINS[] = { 0, 1, 3, 2 };              // What analog pin is assigned to wich CT
const word CT_PRIMARY[] = { 200, 200, 100, -1 };    // The CT primary A, -1 means no CT is connected

volatile word sample_idx;                           // Where in the sampling array are we
volatile word ct_samples[CT_COUNT][SAMPLE_FREQ];    // Store a number of ADC sample to do averaging
volatile uint32_t ct_totals[CT_COUNT];              // The sum of the samples array 
volatile uint32_t ct_readings[CT_COUNT];            // The final mA value, DO NOT ACCESS, use  make_local_copy() and local_copy[]

uint32_t local_copy[CT_COUNT];                      // A non volitle copy of ct_readings to by used by non-timer code

void make_local_copy()
{
  noInterrupts();
  memcpy(local_copy, (const void *)ct_readings, sizeof(uint32_t) * CT_COUNT);
  interrupts();
}

//TC1 ch 0
void TC3_Handler()
{
  // Clear clags and per MCU datasheet
  TC_GetStatus(TC1, 0);  
  noInterrupts();
    
  // Set ADC sampling to 12-bit
  analogReadResolution(12);

  // For each CT, sample and calc final value
  for(word i = 0; i < CT_COUNT; i++)
  {
    // Use sliding window averaging, optimized so that the whole array not needed to be summed each time
    ct_totals[i] -= ct_samples[i][sample_idx];
    ct_samples[i][sample_idx] = (CT_PRIMARY[i] == -1) ? 0: analogRead(CT_PINS[i]);
    ct_totals[i] += ct_samples[i][sample_idx];
    double ct_average = (double)ct_totals[i] / (double)SAMPLE_FREQ;
        
    // Convert ADC value to mA reading
    ct_readings[i] = round(ct_average * (double)CT_PRIMARY[i] * FINAL_PRECISION / (double)MAX_RECT_VAL);
  }
  
  // Incroment index, wrap to 0 when max is reached
  ++sample_idx %= SAMPLE_FREQ;
  
  interrupts();
}
   
void ammeter_setup() 
{
  sample_idx = 0;
  memset((void *)ct_samples, 0, sizeof(uint32_t) * CT_COUNT * SAMPLE_FREQ);
  memset((void *)ct_totals, 0, sizeof(uint32_t) * CT_COUNT);
  memset((void *)ct_readings, 0, sizeof(uint32_t) * CT_COUNT);
  
  start_timer(TC1, 0, TC3_IRQn, SAMPLE_FREQ);
}

double get_amp_reading()
{
  make_local_copy();
  
  Serial.print("* ");
  for(word i = 0; i < CT_COUNT; i++)
  {
    Serial.print("CT ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(local_copy[i]);
    Serial.print(", ");
  }
  Serial.println();  
  
  return (double)((local_copy[0] + local_copy[1]) - (local_copy[2] + local_copy[3])) / 1000.0;
}














