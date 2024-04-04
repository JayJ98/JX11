/*
  ==============================================================================

    Envelope.h
    Created: 3 Apr 2024 4:08:48pm
    Author:  Jayson

  ==============================================================================
*/

#pragma once

class Envelope{
public:
    float nextValue(){
        level = multiplier * (level - target) + target;
        
        if (level + target > 3.0f) {
            multiplier = decayMultiplier;
            target = sustainLevel;
        }
        
        return level;
    }
    
    void release(){
        target = 0.0f;
        multiplier = releaseMultiplier;
    }
    
    inline bool isActive() const{
        return level > SILENCE;
    }
    
    inline bool isInAttack() const{
        return target >= 2.0f;
    }
    
    void attack(){
        level += SILENCE + SILENCE;
        target = 2.0f;
        multiplier = attackMultiplier;
    }
    
    void reset(){
        level = 0.0f;
        target = 0.0f;
        multiplier = 0.0f;
    }
    
    float level;
    const float SILENCE = 0.0001f;
    
    float attackMultiplier;
    float decayMultiplier;
    float sustainLevel;
    float releaseMultiplier;

private:
    float multiplier;
    float target;
};
