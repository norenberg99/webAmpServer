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

#define AMP_FIRST_NUM       0
#define AMP_LAST_NUM        5
#define AMP_TOTAL_NUM       ( AMP_LAST_NUM - AMP_FIRST_NUM + 1 )


class AmpMux
{
public:
    // Create an instance of the library
    AmpMux();
    
    // init the library, 0 = success
    int8_t begin( void );

    // 0 to max volume
    int8_t getVolume( uint8_t ampNum );

    // 0 to max volume
    int8_t setVolume( uint8_t ampNum, int8_t newVolume );

    // 0 = success
    int8_t powerDown( void ); 

protected:
    int8_t setMux( uint8_t ampNum );
    int8_t setAllAmpVolumes( void ); 
    

    // We'll track the volume level in this variable.
    int8_t _ampVolume[ AMP_TOTAL_NUM ];
        
};

 
