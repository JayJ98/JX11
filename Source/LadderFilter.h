/*
  ==============================================================================

    LadderFilter.h
    Created: 29 Apr 2024 12:10:53pm
    Author:  Jayson

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class Filter : public juce::dsp::LadderFilter<float>{
public:
    void updateCoefficients(float cutoff, float Q, float drive){
        setCutoffFrequencyHz(cutoff);
        setResonance(std::clamp(Q / 30.0f, 0.0f, 1.0f));
        setDrive(drive);
    }
    
    float render(float x){
        updateSmoothers();
        return processSample(x, 0);
    }
};
