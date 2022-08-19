/*******************************************************************************
*
*  The Cognitive Power Meter Project (c-meter)
*
*  Coopyright (C) 2013 by Stgephen Makonin. All rights reserved.
*
*
*  Main project file.
*
*******************************************************************************/

const boolean OUTOUT_TO_SERIAL = true;

word guess_load_amps(word load_id, word Z)
{
  word amps = 0;
  double p = -999999.999;
  
  for(word u = 0; u <= Z; u++)
  {
    if(zero_unprop(load_id, u))
      continue;
      
    word d = Z - u;
    if(zero_prop(load_id, d))
      continue;
      
    double pn = log(get_prop(load_id, d)) + log(get_unprop(load_id, u));
    if(p < pn)
    {
      amps = d;
      p = pn;
    }
  }
  
  return amps;
}
   
void setup() 
{
  if(OUTOUT_TO_SERIAL)
    Serial.begin(19200);
  
  ammeter_setup();
  
  delay(3000);
}

void loop() 
{  
  if(OUTOUT_TO_SERIAL)
  {
      uint32_t stime = millis();
      double ZZ = get_amp_reading();
      word Z = round(ZZ);
      
      Serial.print("Whole-House reading is ");
      Serial.print(ZZ);
      Serial.println(" A, appliances running are:");
      
      for(word i = 0; i < get_load_count(); i++)
      {
        word amps = guess_load_amps(i, Z);
        
        if(amps > 0)
        {
          Serial.print("\t");
          Serial.print(amps);
          Serial.print(" A - ");
          Serial.print(get_load_name(i));
          Serial.println();
        }
      }
      
      uint32_t etime = millis();
      
      Serial.print("\t*** Elapsed Time: ");
      Serial.print((double)(etime - stime) / 1000.0);
      Serial.println(" msec.");
      Serial.println();
      
      delay(10000);
  }
}














