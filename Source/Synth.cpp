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

void Synth::allocateResources(double sampleRate_, int samplesPerBlock){
    sampleRate = static_cast<float>(sampleRate_);
}

void Synth::deallocateResources(){
    //TODO
}

void Synth::reset(){
    voice.reset();
    noiseGen.reset();
}

void Synth::render(float** outputBuffers, int sampleCount){
    float* outputBufferLeft = outputBuffers[0];
    float* outputBufferRight = outputBuffers[1];
    
    
    for (int sample = 0; sample < sampleCount; ++sample) {
        float noise = noiseGen.nextValue();
        
        float output = 0.0f;
        if(voice.note > 0){
            output = noise * (voice.velocity / 127.0f) * 0.5f;
        }
        outputBufferLeft[sample] = output;
        
        if(outputBufferRight != nullptr){
            outputBufferRight[sample] = output;
        }
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
    }
    
}

void Synth::noteOn(int note, int velocity){
    voice.note = note;
    voice.velocity = velocity;
}

void Synth::noteOff(int note){
    if(voice.note == note){
        voice.note = 0;
        voice.velocity = 0;
    }
}
