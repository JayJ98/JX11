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
    
    const int LFO_MAX = 32;
    float lfoInc;
    
    float vibrato;
    
    float pwmDepth;
    
    int glideMode;
    float glideRate;
    float glideBend;
    
    float filterDrive;
    float filterKeyTracking;
    float filterQ;
    float resonanceCtl;
    float filterLFODepth;
    
    float filterAttack, filterDecay, filterSustain, filterRelease;
    float filterEnvDepth;
    
private:
    void noteOn(int note, int velocity);
    void noteOff(int note);
    float calcPeriod(int v, int note) const;
    int findFreeVoice();
    void restartMonoVoice(int note, int velocity);
    void shiftQueuedNotes();
    int nextQueuedNote();
    
    bool isPlayingLegatoStyle() const;
    
    inline void updatePeriod(Voice& voice){
        voice.osc1.period = voice.period * pitchBend;
        voice.osc2.period = voice.osc1.period * detune;
    }
    
    float sampleRate;
    float inverseSampleRate;
    
    std::array<Voice, MAX_VOICES> voices;
    float pitchBend;
    
    NoiseGenerator noiseGen;
    
    void updateLFO();
    
    int lfoStep;
    float lfo;
    
    float modWheel;
    
    int lastNote;
    
    float pressure;
    
    float filterCtl;
    
    float filterZip;
};
