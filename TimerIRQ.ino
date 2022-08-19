/*******************************************************************************
*
*  The Cognitive Power Meter Project (c-meter)
*
*  Coopyright (C) 2013 by Stgephen Makonin. All rights reserved.
*
*
*  Timers IRQ module file.
*
*******************************************************************************/

struct 
{
  byte flag;
  byte divisor;
} clock_config[] = { { TC_CMR_TCCLKS_TIMER_CLOCK1,   2 },
         	     { TC_CMR_TCCLKS_TIMER_CLOCK2,   8 },
		     { TC_CMR_TCCLKS_TIMER_CLOCK3,  32 },
		     { TC_CMR_TCCLKS_TIMER_CLOCK4, 128 } };

void start_timer(Tc *tc, uint32_t channel, IRQn_Type irq, uint32_t frequency)
{
  pmc_set_writeprotect(false);
  pmc_enable_periph_clk((uint32_t)irq);
  
  int best_clock = 3;
  float best_err = 1.0;
  for(int i = 3; i > -1; i--)
  {
    float ticks = (float)VARIANT_MCK / (float)frequency / (float)clock_config[i].divisor;
    float err = abs(ticks - round(ticks));
    if(err < best_err) 
    {
      best_clock = i;
      best_err = err;
    }
  }
  
  TC_Configure(tc, channel, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | clock_config[best_clock].flag);  
  uint32_t rc = (float)VARIANT_MCK / (float)frequency / (float)clock_config[best_clock].divisor;
  TC_SetRA(tc, channel, rc / 2);
  TC_SetRC(tc, channel, rc);
  TC_Start(tc, channel);
  tc->TC_CHANNEL[channel].TC_IER = TC_IER_CPCS;
  tc->TC_CHANNEL[channel].TC_IDR = ~TC_IER_CPCS;
  NVIC_EnableIRQ(irq);
}
