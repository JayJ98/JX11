/*
  ==============================================================================

    Synth.h
    Created: 1 Mar 2024 10:09:52pm
    Author:  Jayson

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Voice.h"
#include "NoiseGenerator.h"

class Synth{
public:
    Synth();
    
    void allocateResources(double sampleRate, int samplesPerBlock);
    void deallocateResources();
    void reset();
    void render(float** outputBuffers, int sampleCount);
    void midiMessage(uint8_t data0, uint8_t data1, uint8_t data2);
    
    float noiseMix;
    float envAttack;
    float envDecay;
    float envSustain;
    float envRelease;
    
    float oscMix;
    float detune;
    
    float tune;
    
    float noteStereoSpread;
    
private:
    void noteOn(int note, int velocity);
    void noteOff(int note);
    float calcPeriod(int note) const;
    
    float sampleRate;
    Voice voice;
    float pitchBend;
    
    NoiseGenerator noiseGen;
    
    
};
