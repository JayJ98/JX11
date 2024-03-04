/*
  ==============================================================================

    Oscillator.h
    Created: 2 Mar 2024 5:04:02pm
    Author:  Jayson

  ==============================================================================
*/

#pragma once
const float TWO_PI = 6.2831853071795864f;

class Oscillator{
public:
    float amplitude;
    float frequency;
    float sampleRate;
    float phaseOffset;
    int sampleIndex;
    
    void reset(){
        sampleIndex = 0;
    }
    
    float nextSample(){
        float output = amplitude * std::sin( TWO_PI * sampleIndex * frequency / sampleRate + phaseOffset);
        sampleIndex += 1;
        return output;
    }
};
