#include <Arduino.h>
#include <stdint.h>
#include <avr/sleep.h>

// #define ZC PB2
// #define PUMP PB1
// #define INPUT PB3

static volatile uint8_t value = 0;
static volatile uint8_t a = 0;

void updateValue(uint16_t);
void calculateSkip(void);

void setup()
{
  // ADC configuration
  ADMUX = _BV(MUX0) | _BV(MUX1); // Select ADC input channel 3 (PB3)
  ADCSRA = _BV(ADEN) |           // Enable ADC
           _BV(ADPS1) |          // Set prescaler to 8 (125kHz with 1MHz clock)
           _BV(ADPS0);

  // Setup PB2 and PB3 as inputs
  DDRB &= ~((1 << PB2) | (1 << PB3));
  PCMSK |= (1 << PCINT2); // Enable pin change interrupt for PB2
  GIMSK |= (1 << PCIE);   // Enable general pin change interrupts

  // Setup PB1 as output (push-pull is the default output configuration)
  DDRB |= (1 << PB1);
  PORTB &= ~(1 << PB1);

  // Configure sleep mode
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();

  sei(); // Enable global interrupts

  // stop timer 0 to save power, we don't need it
  TCCR0A = 0;
  TCCR0B = 0;
  // stop timer 1 to save power, we don't need it
  TCCR1 = 0;
}

void loop()
{
  static unsigned long adcReadTime = 0;
  if (adcReadTime++ > 10)
  {
    adcReadTime = 0;

    ADCSRA |= _BV(ADEN); // ADC on (we turned it off the save power)

    // Start ADC conversion
    ADCSRA |= _BV(ADSC);

    // Wait for conversion to complete
    while (ADCSRA & _BV(ADSC))
      ;

    // Read ADC value (10-bit value)
    uint16_t adcValue = ADC;

    ADCSRA &= ~_BV(ADEN); // ADC off (saves power)

    updateValue(adcValue);
  }

  sleep_mode(); // Enter low power sleep mode
}

// Pin Change Interrupt Service Routine for ATtiny85
ISR(PCINT0_vect)
{
  unsigned short debounceCnt = 0;
  while (debounceCnt++ < 5)
  {
    if ((PINB & (1 << PB2))) // Proceed only if PB2 is low 5 reads in a row (debouncing)
    {
      return;
    }
  }
  calculateSkip();
}

void calculateSkip(void)
{
  a += value;

  const uint8_t range = 127U;
  static volatile bool skip = false;

  if (a >= range)
  {
    a -= range;
    skip = false;
  }
  else
  {
    skip = true;
  }

  if (a > range)
  {
    a = 0;
    skip = false;
  }

  if (skip)
  {
    PORTB &= ~(1 << PB1);
  }
  else
  {
    PORTB |= (1 << PB1);
  }
}

const uint8_t valueFactor = 4U; // precalculated constant 1024 [10 bit ADC resolution] / 128 [range + 1] / 2

void updateValue(uint16_t newValue)
{
  uint16_t oldValue = value * valueFactor * 2;

  if (newValue > (oldValue + valueFactor * 3) || (newValue + valueFactor) < oldValue)
  { // add some hysteresis to filter out noise
    value = newValue / valueFactor / 2;
    a = 0;
  }
}