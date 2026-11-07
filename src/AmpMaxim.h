/*--------------------------------------------------------------------------------
  Amp
  Library to send commands to the maxim 9744 chip.
    
  Author: Jon Norenberg
  Date: 10/10/2015
--------------------------------------------------------------------------------*/

#pragma once

#include <inttypes.h>

#define AMP_MIN_VOLUME      0
#define AMP_MAX_VOLUME      63

class AmpMaxim
{
public:
    // Create an instance of the library
    AmpMaxim();

    // int the library, 0 = success
    int8_t begin( void );

    // int the library, 0 = success
    int8_t getCurrentVolume( void );

    // int the library, 0 = success
    bool setVolume( int8_t newVolume ); 

protected:
    // We'll track the volume level in this variable.
    int8_t _currentVolume;
    
};

 
