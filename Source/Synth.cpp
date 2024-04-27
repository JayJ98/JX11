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
static const int SUSTAIN = -1;
const float SILENCE = 0.0001f;

Synth::Synth(){
    sampleRate = 44100.0f;
}


void Synth::allocateResources(double sampleRate_, int /*samplesPerBlock*/){
    sampleRate = static_cast<float>(sampleRate_);
    inverseSampleRate = static_cast<float>(1/sampleRate_);
    
    for(int v = 0; v < MAX_VOICES; ++v){
        voices[v].filter.sampleRate = sampleRate;
    }
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
    
    sustainPedaPressed = false;
    
    outputLevelSmoother.reset(sampleRate, 0.05);
    
    lfo = 0.0f;
    lfoStep = 0;
    
    modWheel = 0.0f;
    
    lastNote = 0;
    
    resonanceCtl = 1.0f;
    
    pressure = 0.0f;
    
    filterCtl = 0.0f;
    
    filterZip = 0.0f;
}


void Synth::render(float** outputBuffers, int sampleCount){
    float* outputBufferLeft = outputBuffers[0];
    float* outputBufferRight = outputBuffers[1];
    
    for (int v = 0; v < MAX_VOICES; ++v) {
        Voice& voice = voices[v];
        if (voice.env.isActive()) {
            updatePeriod(voice);
            voice.glideRate = glideRate;
            voice.filterQ = filterQ * resonanceCtl;
            voice.pitcheBend = pitchBend;
            voice.filterEnvDepth = filterEnvDepth;
        }
    }
    
    for (int sample = 0; sample < sampleCount; ++sample) {
        updateLFO();
        
        const float noise = noiseGen.nextValue() * noiseMix;
        
        float outputLeft = 0.0f;
        float outputRight = 0.0f;
        
        for (int v = 0; v < MAX_VOICES; ++v) {
            Voice& voice = voices[v];
            if(voice.env.isActive()){
                float output = voice.render(noise);
                outputLeft += output * voice.panLeft;
                outputRight += output * voice.panRight;
            }
            
        }
        
        float outputLevel = outputLevelSmoother.getNextValue();
        outputLeft *= outputLevel;
        outputRight *= outputLevel;
        
        if (outputBufferRight != nullptr) {
            outputBufferLeft[sample] = outputLeft;
            outputBufferRight[sample] = outputRight;
        }else{
            outputBufferLeft[sample] = (outputLeft + outputRight) * 0.5f;
        }
    }
    
    
    
    for (int v = 0; v < MAX_VOICES; ++v) {
        Voice& voice = voices[v];
        if (!voice.env.isActive()) {
            voice.env.reset();
            voice.filter.reset();
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
        case 0xB0:
            controlChange(data1, data2);
            break;
        case 0xD0:
            pressure = 0.0001f * float(data1 * data1);
            break;
    }
    
}


void Synth::noteOn(int note, int velocity){
    int v = 0; //index of voice to use (0 is mono voice)
    
    if (ignoreVelocity) {
        velocity = 80;
    }else{
        float sensitivity = std::abs(velocitySensitivity / 0.05f);
        velocity = 80 + (velocity - 80) * sensitivity;
    }
    
    if (numVoices == 1) { // monophonic
        if (voices[0].note > 0) {// legato-style playing
            shiftQueuedNotes();
            restartMonoVoice(note, velocity);
            return;
        }
    }else{ // poloyphonic
        v = findFreeVoice();
    }
    
    startVoice(v, note, velocity);
    
}


void Synth::noteOff(int note){
    if ((numVoices == 1) && (voices[0].note == note)) {
        int queuedNote = nextQueuedNote();
        if (queuedNote > 0) {
            restartMonoVoice(queuedNote, -1);
        }
    }
    
    for (int v = 0; v < MAX_VOICES; ++v) {
        if (voices[v].note == note) {
            if (sustainPedaPressed) {
                voices[v].note = SUSTAIN;
            }else{
                voices[v].release();
                voices[v].note = 0;
            }
        }
    }
    
}



void Synth::startVoice(int v, int note, int velocity){
    float period = calcPeriod(v, note);
    
    Voice& voice = voices[v];
    voice.target = period;
    
    voice.cutoff = sampleRate / (period * PI);
    voice.cutoff *= std::exp(velocitySensitivity * float(velocity - 64));
    
    int noteDistance = 0;
    if (lastNote > 0) {
        if ((glideMode == 2) || ((glideMode == 1) & isPlayingLegatoStyle())) {
            noteDistance = note - lastNote;
        }
    }
    
    voice.period = period * std::pow(1.059463094359f, float(noteDistance) - glideBend);
    
    if (voice.period < 6.0f) {
        voice.period = 6.0f;
    }
    
    lastNote = note;
    voice.note = note;
    voice.updatePanning(noteStereoSpread);
    
    float vel = 0.004f * ((velocity + 64) * (velocity + 64)) - 8;
    voice.osc1.amplitude = volumeTrim * vel;
    voice.osc2.amplitude = voice.osc1.amplitude * oscMix;
    
    if (vibrato == 0.0f && pwmDepth > 0.0f) {
        voice.osc2.squareWave(voice.osc1, voice.period);
    }
    
    Envelope& env = voice.env;
    env.attackMultiplier = envAttack;
    env.decayMultiplier = envDecay;
    env.sustainLevel = envSustain;
    env.releaseMultiplier = envRelease;
    env.attack();
    
    Envelope& filterEnv = voice.filterEnv;
    filterEnv.attackMultiplier = filterAttack;
    filterEnv.decayMultiplier = filterDecay;
    filterEnv.sustainLevel = filterSustain;
    filterEnv.releaseMultiplier = filterRelease;
    filterEnv.attack();
    
}

void Synth::restartMonoVoice(int note, int velocity){
    float period = calcPeriod(0, note);
    
    Voice& voice = voices[0];
    voice.target = period;
    
    voice.cutoff = sampleRate / (period * PI);
    if (velocity > 0) {
        voice.cutoff *= std::exp(velocitySensitivity * float(velocity - 64));
    }
    
    if (glideMode == 0) {
        voice.period = period;
    }
    
    voice.env.level += SILENCE + SILENCE;
    voice.note = note;
    voice.updatePanning(noteStereoSpread);
    
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
    
    for (int i = 0; i < MAX_VOICES; ++i) {
        if (voices[i].env.level < l && !voices[i].env.isInAttack() ) {
            l = voices[i].env.level;
            v = i;
        }
    }
    
    return v;
}


void Synth::controlChange(uint8_t data1, uint8_t data2){
    
    switch (data1){
        case 0x40:
            sustainPedaPressed = (data2 >= 64);
            if (!sustainPedaPressed) {
                noteOff(SUSTAIN);
            }
            break;
        case 0x01:
            modWheel = 0.000005f * float(data2 * data2);
            break;
        case 0x4c:
            resonanceCtl = 154.0f / float(154 - data2);
            break;
        case 0x4b:
            filterCtl = 0.02f * float(data2);
            break;
        case 0x4a:
            filterCtl = -0.03f * float(data2);
            break;
        default:
            if (data1 >= 0x78) {
                for (int v = 0; v < MAX_VOICES; ++v) {
                    voices[v].reset();
                }
                sustainPedaPressed = false;
            }
            
    }
}

void Synth::shiftQueuedNotes(){
    for (int tmp = MAX_VOICES - 1; tmp > 0; tmp--) {
        voices[tmp].note = voices[tmp - 1].note;
        voices[tmp].release();
    }
}

int Synth::nextQueuedNote(){
    int held = 0;
    for (int v = MAX_VOICES - 1; v > 0; v--) {
        if (voices[v].note > 0) {
            held = v;
        }
    }
    
    if (held > 0) {
        int note = voices[held].note;
        voices[held].note = 0;
        return note;
    }
    
    return 0;
}

void Synth::updateLFO(){
    if (--lfoStep <= 0) {
        lfoStep = LFO_MAX;
        
        lfo += lfoInc;
        
        if (lfo > PI) {
            lfo -= TWO_PI;
        }
        
        const float sine = std::sin(lfo);
        
        float vibratoMod = 1.0f + sine * (modWheel + vibrato);
        float pwm = 1.0f + sine * (modWheel + pwmDepth);
        
        float filterMod = filterKeyTracking + filterCtl + (filterLFODepth + pressure) * sine;
        
        filterZip += 0.05f * (filterMod - filterZip);
        
        for (int v = 0; v < MAX_VOICES; ++v) {
            Voice& voice = voices[v];
            if (voice.env.isActive()) {
                voice.osc1.modulation = vibratoMod;
                voice.osc2.modulation = pwm;
                voice.filterMod = filterZip;
                voice.updateLFO();
                updatePeriod(voice);
            }
        }
    }
}

bool Synth::isPlayingLegatoStyle() const{
    int held = 0;
    for (int i = 0; i < MAX_VOICES; ++i) {
        if (voices[i].note > 0) {
            held += 1;
        }
    }
    return held > 0;
}

