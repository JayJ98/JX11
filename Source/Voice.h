/*
  ==============================================================================

    Voice.h
    Created: 1 Mar 2024 10:09:44pm
    Author:  Jayson

  ==============================================================================
*/

#pragma once
#include "Oscillator.h"
#include "Envelope.h"

struct Voice{
    int note;
    Oscillator osc1;
    Oscillator osc2;
    float saw;
    Envelope env;
    float period;
    
    void reset(){
        note = 0;
        saw = 0.0f;
        env.reset();
        
        osc1.reset();
        osc2.reset();
    }

    float render(float input){
        float sample1 = osc1.nextSample();
        float sample2 = osc2.nextSample();
        saw = saw * 0.997f + sample1 - sample2;
        
        float output = saw + input;
        
        float envelope = env.nextValue();
        
        return output * envelope;
    }
    
    void release(){
        env.release();
    }
    
};
