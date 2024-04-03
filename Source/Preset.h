/*
  ==============================================================================

    Preset.h
    Created: 1 Apr 2024 8:17:46pm
    Author:  Jayson

  ==============================================================================
*/

#pragma once

#include <cstring>

const int NUM_PARAMS = 26;

struct Preset{
    Preset(const char* name,
           float p0,
           float p1,
           float p2,
           float p3,
           float p4,
           float p5,
           float p6,
           float p7,
           float p8,
           float p9,
           float p10,
           float p11,
           float p12,
           float p13,
           float p14,
           float p15,
           float p16,
           float p17,
           float p18,
           float p19,
           float p20,
           float p21,
           float p22,
           float p23,
           float p24,
           float p25
           ){
        strcpy(this->name, name);
        param[0] = p0;
        param[1] = p1;
        param[2] = p2;
        param[3] = p3;
        param[4] = p4;
        param[5] = p5;
        param[6] = p6;
        param[7] = p7;
        param[8] = p8;
        param[9] = p9;
        param[10] = p10;
        param[11] = p11;
        param[12] = p12;
        param[13] = p13;
        param[14] = p14;
        param[15] = p15;
        param[16] = p16;
        param[17] = p17;
        param[18] = p18;
        param[19] = p19;
        param[20] = p20;
        param[21] = p21;
        param[22] = p22;
        param[23] = p23;
        param[24] = p24;
        param[25] = p25;
    }
    
    char name[40];
    float param[NUM_PARAMS];
};
