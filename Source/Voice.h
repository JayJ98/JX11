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
    float saw;
    
    void reset(){
        note = 0;
        saw = 0.0f;
    }

    float render(){
        float sample = osc.nextSample();
        saw = saw * 0.997f - sample;
        return saw;
    }
    
};
