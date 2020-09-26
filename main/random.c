#include "include/random.h"
#include <inttypes.h>


int lastYield = 0;



int randomBitRaw2(void) 
{
    // Software whiten bits using Von Neumann algorithm
    //
    // von Neumann, John (1951). "Various techniques used in connection
    // with random digits". National Bureau of Standards Applied Math Series
    // 12:36.
    //
    for(;;) {
    int a = randomBitRaw() | (randomBitRaw()<<1);
    if (a==1) return 0; // 1 to 0 transition: log a zero bit
    if (a==2) return 1; // 0 to 1 transition: log a one bit
    // For other cases, try again.
    }
    return 0;
}

int randomBitRaw(void)
 {
    uint8_t bit = (int)RANDOM_REG32;

    return bit & 1;
}

int randomBit() {
    // Software whiten bits using Von Neumann algorithm
    //
    // von Neumann, John (1951). "Various techniques used in connection
    // with random digits". National Bureau of Standards Applied Math Series
    // 12:36.
    //
    for(;;) {
        int a = randomBitRaw2() | (randomBitRaw2()<<1);
        if (a==1) return 0; // 1 to 0 transition: log a zero bit
        if (a==2) return 1; // 0 to 1 transition: log a one bit
        // For other cases, try again.
    }
    return 0;
}

long calculateRandom(long howBig) 
{
  long randomValue;
  long topBit;
  long bitPosition;

  if (!howBig) return 0;
  randomValue = 0;
  if (howBig & (howBig-1)) {

    // Range is not a power of 2 - use slow method
    topBit = howBig-1;
    topBit |= topBit>>1;
    topBit |= topBit>>2;
    topBit |= topBit>>4;
    topBit |= topBit>>8;
    topBit |= topBit>>16;
    topBit = (topBit+1) >> 1;

    bitPosition = topBit;
    do {
      // Generate the next bit of the result
      if (randomBit()) randomValue |= bitPosition;

      // Check if bit
      if (randomValue >= howBig) {
        // Number is over the top limit - start again.
        randomValue = 0;
        bitPosition = topBit;
      } else {
        // Repeat for next bit
        bitPosition >>= 1;
      }
    } while (bitPosition);
  } else {
    // Special case, howBig is a power of 2
    bitPosition = howBig >> 1;
    while (bitPosition) {
      if (randomBit()) randomValue |= bitPosition;
      bitPosition >>= 1;
    }
  }
  return randomValue;
}


long random(long min, long max)
{
  if (min >= max) return min;
  long diff = max - min;
  return calculateRandom(diff + min);
}