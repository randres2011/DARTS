#pragma once
#include "darts.h"
#include <vector>

#include "../matrix/matrix.h"
#include "../Parameters.h"
#include "../infrastructure/Framework.h"
#include "../infrastructure/Frame.h"

class forwardPropagation: public Framework
{
    public:

        //Input Layer
        matrix Feature0;

        //Convolution Layer
        matrix Feature1[CH1];
        matrix Weight1[CH0*CH1];
        double Bias1[CH1];
        std::vector<Codelet*> map1[INPUT1*INPUT1*CH0];

        //Pooling Layer
        matrix Feature2[CH2];
        std::vector<Codelet*> map2[INPUT2*INPUT2*CH1];

        //Conv Layer
        matrix Feature3[CH3];
        matrix Weight3[CH2*CH3];
        double Bias3[CH3];
        std::vector<Codelet*> map3[INPUT3*INPUT3*CH2];

        //Pooling Layer
        matrix Feature4[CH4];
        std::vector<Codelet*> map4[INPUT4*INPUT4*CH3];

        //Conv2Full
        double Feature5[OUTPUT5];
        matrix Weight5[CH4*OUTPUT5];
        double Bias5[OUTPUT5];
        std::vector<Codelet*> map5[INPUT5*INPUT5*CH4];

        //Full
        double *Feature6;
        double Weight6[INPUT6*OUTPUT6];
        double Bias6[OUTPUT6];
        std::vector<Codelet*> map6[INPUT6];


        //Finish Layer
        std::vector<Codelet*> map7[OUTPUT];

        forwardPropagation(double *output, Codelet * toSig_, execTimes_t *execTimes, uint32_t frameworkId);

};

class forwardPropagationFrame: public Frame
{
    public:
        forwardPropagationFrame(double * output, Codelet * toSig_, execTimes_t * execTimes, int numFrames_);

};
