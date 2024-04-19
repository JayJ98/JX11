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
    void startVoice(int v, int note, int velocity);
    void controlChange(uint8_t data1, uint8_t data2);
    
    float noiseMix;
    float envAttack;
    float envDecay;
    float envSustain;
    float envRelease;
    
    float oscMix;
    float detune;
    
    float tune;
    
    float noteStereoSpread;
    
    static constexpr int MAX_VOICES = 16;
    int numVoices;
    bool sustainPedaPressed;
    
    float volumeTrim;
    juce::LinearSmoothedValue<float> outputLevelSmoother;
    
    float velocitySensitivity;
    bool ignoreVelocity;
    
private:
    void noteOn(int note, int velocity);
    void noteOff(int note);
    float calcPeriod(int v, int note) const;
    int findFreeVoice();
    void restartMonoVoice(int note, int velocity);
    void shiftQueuedNotes();
    int nextQueuedNote();
    
    float sampleRate;
    std::array<Voice, MAX_VOICES> voices;
    float pitchBend;
    
    NoiseGenerator noiseGen;
    
};
