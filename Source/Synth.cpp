/*
  ==============================================================================

    Synth.cpp
    Created: 1 Mar 2024 10:10:05pm
    Author:  Jayson

  ==============================================================================
*/

#include "Synth.h"
#include "Utils.h"

Synth::Synth(){
    sampleRate = 44100.0f;
}

void Synth::allocateResources(double sampleRate_, int /*samplesPerBlock*/){
    sampleRate = static_cast<float>(sampleRate_);
}

void Synth::deallocateResources(){
    //TODO
}

void Synth::reset(){
    voice.reset();
    noiseGen.reset();
    pitchBend = 1.0f;
}

void Synth::render(float** outputBuffers, int sampleCount){
    float* outputBufferLeft = outputBuffers[0];
    float* outputBufferRight = outputBuffers[1];
    
    voice.osc1.period = voice.period * pitchBend;
    voice.osc2.period = voice.osc1.period * detune;
    
    for (int sample = 0; sample < sampleCount; ++sample) {
        float noise = noiseGen.nextValue() * noiseMix;
        
        float outputLeft = 0.0f;
        float outputRight = 0.0f;
        
        if(voice.env.isActive()){
            float output = voice.render(noise);
            outputLeft += output * voice.panLeft;
            outputRight += output * voice.panRight;
        }
        
        if (outputBufferRight != nullptr) {
            outputBufferLeft[sample] = outputLeft;
            outputBufferRight[sample] = outputRight;
        }else{
            outputBufferLeft[sample] = (outputLeft + outputRight) * 0.5f;
        }
        
    }
    
    if (!voice.env.isActive()) {
        voice.env.reset();
    }
    
    protectYourears(outputBufferLeft, sampleCount);
    protectYourears(outputBufferRight, sampleCount);
}

void Synth::midiMessage(uint8_t data0, uint8_t data1, uint8_t data2){
//    char s[16];
//    snprintf(s, 16, "%d %d %d", (int)data0, (int)data1, (int)data2);
//    DBG(s);

    
    switch (data0 & 0xF0) {
        case 0x80:{
            noteOff(data1 & 0x7F);
            break;
        }
        case 0x90:{
            uint8_t note = data1 & 0x7F;
            uint8_t velocity = data2 & 0x7F;
            if(velocity > 0){
                noteOn(note, velocity);
            }else{
                noteOff(note);
            }
            break;
        }
        case 0xE0:
            pitchBend = std::exp(-0.000014102f * float(data1 + 128 * data2 - 8192));
            break;
    }
    
}

void Synth::noteOn(int note, int velocity){
    voice.note = note;

    voice.updatePanning(noteStereoSpread);
    
    //activate the first oscillator
    float period = calcPeriod(note);
    voice.period = period;
    
    voice.osc1.amplitude = (velocity / 127.0f) * 0.5f;
    voice.osc2.amplitude = voice.osc1.amplitude * oscMix;
    
    voice.osc1.reset();
    voice.osc2.reset();
    
    Envelope& env = voice.env;
    env.attackMultiplier = envAttack;
    env.decayMultiplier = envDecay;
    env.sustainLevel = envSustain;
    env.releaseMultiplier = envRelease;
    env.attack();
    
}

void Synth::noteOff(int note){
    if(voice.note == note){
        voice.release();
    }
}

float Synth::calcPeriod(int note) const{
    // moved std::pow(2, (note - 69)/12) to this function and
    // replaced frequency/period calculation std:pow(x, y)
    // with std::exp(y * log(x)) for the speed improvement.
    
    float period = tune * std::exp(-0.05776226505f * float(note));
    while (period < 6.0f || (period * detune) < 6.0f) {
        period += period;
    }
    return period;
}
