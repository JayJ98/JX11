/*
  ==============================================================================

    Synth.cpp
    Created: 1 Mar 2024 10:10:05pm
    Author:  Jayson

  ==============================================================================
*/

#include "Synth.h"
#include "Utils.h"
static const float ANALOG = 0.002f;

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
    for (int v = 0; v < MAX_VOICES; ++v) {
        voices[v].reset();
    }
    noiseGen.reset();
    pitchBend = 1.0f;
}


void Synth::render(float** outputBuffers, int sampleCount){
    float* outputBufferLeft = outputBuffers[0];
    float* outputBufferRight = outputBuffers[1];
    
    for (int v = 0; v < numVoices; ++v) {
        Voice& voice = voices[v];
        if (voice.env.isActive()) {
            voice.osc1.period = voice.period * pitchBend;
            voice.osc2.period = voice.osc1.period * detune;
        }
    }
    
    for (int sample = 0; sample < sampleCount; ++sample) {
        const float noise = noiseGen.nextValue() * noiseMix;
        
        float outputLeft = 0.0f;
        float outputRight = 0.0f;
        
        for (int v = 0; v < numVoices; ++v) {
            Voice& voice = voices[v];
            if(voice.env.isActive()){
                float output = voice.render(noise);
                outputLeft += output * voice.panLeft;
                outputRight += output * voice.panRight;
            }
            
        }
        
        if (outputBufferRight != nullptr) {
            outputBufferLeft[sample] = outputLeft;
            outputBufferRight[sample] = outputRight;
        }else{
            outputBufferLeft[sample] = (outputLeft + outputRight) * 0.5f;
        }
        
    }
    
    for (int v = 0; v < numVoices; ++v) {
        Voice& voice = voices[v];
        if (!voice.env.isActive()) {
            voice.env.reset();
        }
    }
    
    
    protectYourears(outputBufferLeft, sampleCount);
    protectYourears(outputBufferRight, sampleCount);
}


void Synth::midiMessage(uint8_t data0, uint8_t data1, uint8_t data2){
    
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
    int v = 0;
    if (numVoices > 1) {
        v = findFreeVoice();
    }
    
    startVoice(v, note, velocity);
    
}


void Synth::noteOff(int note){
    
    for (int v = 0; v < numVoices; ++v) {
        if (voices[v].note == note) {
            voices[v].release();
            voices[v].note = 0;
        }
    }
    
}



void Synth::startVoice(int v, int note, int velocity){
    float period = calcPeriod(v, note);
    
    Voice& voice = voices[v];
    voice.period = period;
    voice.note = note;
    voice.updatePanning(noteStereoSpread);
    
    voice.osc1.amplitude = (velocity / 127.0f) * 0.5f;
    voice.osc2.amplitude = voice.osc1.amplitude * oscMix;
    
    Envelope& env = voice.env;
    env.attackMultiplier = envAttack;
    env.decayMultiplier = envDecay;
    env.sustainLevel = envSustain;
    env.releaseMultiplier = envRelease;
    env.attack();
}


float Synth::calcPeriod(int v, int note) const{
    // moved std::pow(2, (note - 69)/12) to this function and
    // replaced frequency/period calculation std:pow(x, y)
    // with std::exp(y * log(x)) for the speed improvement.
    
    float period = tune * std::exp(-0.05776226505f * (float(note) + ANALOG * float(v)) );
    while (period < 6.0f || (period * detune) < 6.0f) {
        period += period;
    }
    return period;
}


/// implemented a simple voice stealing algorithm
/// 1. loop through all voices
/// 2. find the quietest voice based on envelope level
/// 3. don't steal any voice that is in the attack stage.
/// Note: if all voices are in use and have the same
/// envelope level, it will steal voice 0.
/// Possible solution is to check the note velocity instead of level
int Synth::findFreeVoice(){
    int v = 0;
    float l = 100.0f;
    
    for (int i = 0; i < numVoices; ++i) {
        if (voices[i].env.level < l && !voices[i].env.isInAttack() ) {
            l = voices[i].env.level;
            v = i;
        }
    }
    
    return v;
}
