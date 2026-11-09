/*--------------------------------------------------------------------------------
  Amp
  Library to send commands to the maxim 9744 chip.
    
  Author: Jon Norenberg
  Date: 10/10/2015
--------------------------------------------------------------------------------*/

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Wire.h>
#include "AmpMaxim.h"

// 0x4B is the default i2c address
#define MAXIM9744_I2CADDR           0x4B
#define MAX_VOLUME                  63

boolean gAmpInitialized = false;


//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
AmpMaxim::AmpMaxim() 
{
    _currentVolume = 0;
}

//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
int8_t AmpMaxim::begin( void ) 
{

    Wire.begin();
    gAmpInitialized = true;
    
   if ( setVolume( _currentVolume ) == false ) 
    {
        gAmpInitialized = false;
        return 1;
    }
    
    return 0;
} 

//--------------------------------------------------------------------------------
// Return the static variable
//--------------------------------------------------------------------------------
int8_t AmpMaxim::getCurrentVolume( void ) 
{
    return _currentVolume;
}

//--------------------------------------------------------------------------------
// Setting the volume is very simple! Just write the 6-bit
// volume to the i2c bus. That's it!
// From the web server we can set from 0 to 100, but the amp only takes 0 to 63.  We'll scale it down here.
// new volume * 63 / 100 = new volume for the amp
//
// Can we power down the Maxim amp?  Yes, but we don't have a power down pin connected to the amp.  
//We could set the volume to 0, but that would still draw some current.  The amp is always on in this design.
//
//--------------------------------------------------------------------------------
bool AmpMaxim::setVolume( int8_t newVolume ) 
{
    int adjustedVolume = newVolume * MAX_VOLUME / 100;

    // cant be higher than 63 or lower than 0
    if ( adjustedVolume > MAX_VOLUME )
    {
        newVolume = MAX_VOLUME;
    }
    else if ( adjustedVolume < 0 ) 
    {
        newVolume = 0;
    }
    
    if ( gAmpInitialized == false )
    {
        return false;
    }
    
    // send new volume setting over i2c
    Wire.beginTransmission( MAXIM9744_I2CADDR );
    Wire.write( newVolume );
    
    if ( Wire.endTransmission() == 0 )
    {
        _currentVolume = newVolume;
        return true;
    }
    else
    {
        return false;
    }
} 
