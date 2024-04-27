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
#include "Filter.h"


struct Voice{
    int note;
    Oscillator osc1;
    Oscillator osc2;
    float saw;
    Envelope env;
    float period;
    
    float target;
    
    float glideRate;
    
    float panLeft, panRight;
    
    Filter filter;
    float cutoff;
    float filterMod;
    float filterQ;
    
    float pitcheBend;
    
    Envelope filterEnv;
    float filterEnvDepth;
    
    void reset(){
        note = 0;
        saw = 0.0f;
        env.reset();
        
        osc1.reset();
        osc2.reset();
        
        panLeft = 0.707f;
        panRight = 0.707f;
        
        filter.reset();
        
        filterEnv.reset();
    }

    float render(float input){
        float sample1 = osc1.nextSample();
        float sample2 = osc2.nextSample();
        saw = saw * 0.997f + sample1 - sample2;
        
        float output = saw + input;
        
        output = filter.render(output);
        
        float envelope = env.nextValue();
        
        return output * envelope;
    }
    
    void release(){
        env.release();
        filterEnv.release();
    }
    
    void updatePanning(float noteStereoSpread){
        
        float stereoSpread = 100 - noteStereoSpread;
        
        float panning = std::clamp((note - 60.0f) / stereoSpread, -1.0f, 1.0f);
        panLeft = std::sin(PI_OVER_4 * (1.0f - panning));
        panRight = std::sin(PI_OVER_4 * (1.0f + panning));
        
    }
    
    void updateLFO(){
        period += glideRate * (target - period);
        float fenv = filterEnv.nextValue();
        float modulatedCutoff = cutoff * std::exp(filterMod + filterEnvDepth * fenv) / pitcheBend;
        modulatedCutoff = std::clamp(modulatedCutoff, 30.0f, 20000.0f);
        filter.updateCoefficients(modulatedCutoff, filterQ);
    }
    
    
    
};
