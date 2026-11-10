/*--------------------------------------------------------------------------------
    AmpMux.cpp
    Library to send commands to the maxim 9744 chip through the i2c mux.
    When writing to one amp, we write to the others with the stored value.
    We do this because, when the amps come out of shutdown, they need to be
    silenced.
    
    Author: Jon Norenberg
    Date: 11/21/2015 - 3/20/2016
--------------------------------------------------------------------------------*/

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Wire.h>
#include "AmpMux.h"

//--------------------------------------------------------------------------------
// Macros
//--------------------------------------------------------------------------------

    // amp protocol: i2c device address, volume ( 0 to 63 )
#define MAXIM9744_I2CADDR           0x4B
    // mux protocol: i2c device address, mux output number ( 1 << i ) ; 0 <= i <= 7
#define MUX_I2CADDR                 0x70

#define DEFAULT_OFF_VOLUME          0

#define SHUTDOWN_GPIO               8
#define AMP_GOTO_SHUTDOWN           LOW
#define AMP_GOTO_RUNNING            HIGH

//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------
AmpMux::AmpMux() 
{

}

//--------------------------------------------------------------------------------
// Talk to each amp and set the volume to 0, then shutdown the amps
//--------------------------------------------------------------------------------
int8_t AmpMux::begin( void ) 
{
    int8_t errorCode;
    
    pinMode( SHUTDOWN_GPIO, OUTPUT );
    digitalWrite( SHUTDOWN_GPIO, AMP_GOTO_RUNNING );       
    
    Wire.begin();
    
    int8_t ampNum;
    
    for ( ampNum = 0; ampNum < AMP_TOTAL_NUM; ++ampNum )
    {
        _ampVolume[ ampNum ] = DEFAULT_OFF_VOLUME; 
    }
    
    // just really a test to see if we can talk to the amps
    errorCode = setAllAmpVolumes();
    
    // turn off the amps and they will lose their volume settings
    digitalWrite( SHUTDOWN_GPIO, AMP_GOTO_SHUTDOWN );       

    return errorCode;
} 

//--------------------------------------------------------------------------------
// Return the static variable, which is the last volume setting for this amp
// Our internalvolume is 0 to 63, but we return a 0 to 100 value for the user
//--------------------------------------------------------------------------------
int8_t AmpMux::getVolume( uint8_t ampNum ) 
{
    if ( ampNum < AMP_TOTAL_NUM )
    {

        // convert 0 to 63 value to a 0 to 100 value for the user
        return ( _ampVolume[ ampNum ] / AMP_MAX_VOLUME ) * 100;
    }

    return 0;
}

//--------------------------------------------------------------------------------
// Setting the volume is very simple! Just write the 6-bit
// volume to the i2c bus. That's it!
// 
// The input is a 0 to 100 value, which we convert to a 0 to 63 value for the amp.
//--------------------------------------------------------------------------------
int8_t AmpMux::setVolume( uint8_t ampNum, int8_t newVolume0to100 ) 
{
    int8_t errorCode;
    
    if ( ampNum >= AMP_TOTAL_NUM )
    {
        return -1;
    }

    // convert the 0 to 100 value to a 0 to 63 value for the amp
    int8_t tempVolume0to63 = (newVolume0to100 * AMP_MAX_VOLUME) / 100;

    digitalWrite( SHUTDOWN_GPIO, AMP_GOTO_RUNNING );       

    // can't be higher than 63 or lower than 0
    if ( tempVolume0to63 > AMP_MAX_VOLUME )
    {
        tempVolume0to63 = AMP_MAX_VOLUME;
    }
    else if ( tempVolume0to63 < 0 ) 
    {
        tempVolume0to63 = 0;
    }
    
    // save the new amp volume
    _ampVolume[ ampNum ] = tempVolume0to63;
    
    errorCode = setAllAmpVolumes();
        
    return errorCode;
} 
//--------------------------------------------------------------------------------
// Update all the amp volumes.  Write to the mux with the bus number
// then write to the amp through the mux. 
//--------------------------------------------------------------------------------
int8_t AmpMux::setAllAmpVolumes( void ) 
{
    int8_t errorCode = 0;
    int8_t ampNum;
    bool bAllVolumesAreZero = true;
    
    // set volume for each amp, most will be 0
    for ( ampNum = 0; ampNum < AMP_TOTAL_NUM; ++ampNum )
    {
        // SEND I2C TRANSACTION TO THE I2C MUX BEFORE SENDING NEW VOLUME
        errorCode = setMux( ampNum );
        if ( errorCode != 0 )
        {
            return errorCode;
        }
        
        // send new volume setting over i2c
        Wire.beginTransmission( MAXIM9744_I2CADDR );
        Wire.write( _ampVolume[ ampNum ] );
        
        if ( Wire.endTransmission() != 0 )
        {
            return -2;
        }

        if ( _ampVolume[ ampNum ] != 0 )
        {
            bAllVolumesAreZero = false;
        }
    }

    // if all the volumes are zero, then shutdown the amps
    if ( bAllVolumesAreZero )
    {
        digitalWrite( SHUTDOWN_GPIO, AMP_GOTO_SHUTDOWN );       
    }
    else
    {
        digitalWrite( SHUTDOWN_GPIO, AMP_GOTO_RUNNING );       
    }

    return errorCode;

}

//--------------------------------------------------------------------------------
// Configure the mux for sending an i2c to a certain amp
//--------------------------------------------------------------------------------
int8_t AmpMux::setMux( uint8_t ampNum ) 
{
    int8_t errorCode = 0;
    
    Wire.beginTransmission( MUX_I2CADDR );
    // amp number needs to be shifted a little before sending to the mux
    Wire.write( 1 << ampNum );
    
    if ( Wire.endTransmission() != 0 )
    {
        errorCode = -3;
    }

    return errorCode;
}

//--------------------------------------------------------------------------------
// Set the shutdown gpio high means to shutdown the amp
//--------------------------------------------------------------------------------
int8_t AmpMux::powerDown( void ) 
{
    int8_t errorCode = 0;

    digitalWrite( SHUTDOWN_GPIO, AMP_GOTO_SHUTDOWN ); 
    
    int8_t ampNum;
    
    // put all the volumes down to off
    for ( ampNum = 0; ampNum < AMP_TOTAL_NUM; ++ampNum )
    {
        _ampVolume[ ampNum ] = DEFAULT_OFF_VOLUME; 
    }

    errorCode = setAllAmpVolumes();    
    return errorCode;
}