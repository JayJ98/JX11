/*
  ==============================================================================

    Voice.h
    Created: 1 Mar 2024 10:09:44pm
    Author:  Jayson

  ==============================================================================
*/

#pragma once
#include "Oscillator.h"

struct Voice{
    int note;
    Oscillator osc;
    
    void reset(){
        note = 0;
    }

    float render(){
        return osc.nextSample();
    }
    
};
