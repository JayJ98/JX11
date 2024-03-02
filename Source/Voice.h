/*
  ==============================================================================

    Voice.h
    Created: 1 Mar 2024 10:09:44pm
    Author:  Jayson

  ==============================================================================
*/

#pragma once

struct Voice{
    int note;
    int velocity;
    
    void reset(){
        note = 0;
        velocity = 0;
    }
    
    
};
