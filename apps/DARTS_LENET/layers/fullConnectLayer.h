/* 
 * Copyright (c) 2011-2014, University of Delaware
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

// DARTS PLUS

#pragma once
#include "darts.h"
#include <vector>

#include "../matrix/matrix.h"
#include "../Parameters.h"
#include "../infrastructure/layerTemplate.h"
using namespace darts;


class TileFull : public Codelet
{
public:
    int in_ch,num;
    double *input;
    double *output;
    double *kernel;
    double *bias;
    double (*action)(double);
    TileFull(double *input_,double * output_,double *kernel_,double *bias_,
            double (*action_)(double),int in_ch_,int num_):
        Codelet(),in_ch(in_ch_),num(num_),input(input_),output(output_),
        kernel(kernel_),bias(bias_),action(action_)
    {}
    TileFull(uint32_t dep, uint32_t res, ThreadedProcedure * myTP, uint32_t stat,
            double *input_,double * output_,double *kernel_,double *bias_,
            double (*action_)(double),int in_ch_,int num_):
            Codelet(dep,res,myTP,stat),in_ch(in_ch_),num(num_),input(input_),output(output_),
            kernel(kernel_),bias(bias_),action(action_)
    {}

    virtual void fire(void);
};

class FullConnect : public Layer
{
public:
    FullConnect(trig_cd *prevLayerTrig, trig_cd *nextLayerTrig, uint32_t layerId, uint64_t *startTime, uint64_t *stopTime,
                double *input, double *output, double *kernel,double *bias,
                double (*action)(double),int in_size, int out_size, int cut):
                Layer(prevLayerTrig, nextLayerTrig, layerId, startTime, stopTime)
    {
        for(int i = 0; i < cut; i++)
        {
            int tile = out_size / cut;
            int mod = out_size % cut;
            int tile_size = (i + 1 == cut) ? (tile + mod) : (tile);
            Codelet *temp = new TileFull(input, &output[i*tile],
                &kernel[i*in_size], &bias[i*tile], action, in_size, tile_size);
            Layer::insertElementToMap(temp);
            Layer::registerCodelet(temp);
        }
        Layer::layerReadyToStart();
    }
    virtual void onAfterConnectPrevLayer() {
        Conv2Full * prevConv2Full = static_cast<Conv2Full*> (previousLayer);
        uint32_t numInputSignals = prevConv2Full->getNumCuts();
        for (uint32_t i = 0; i < allMyCodelets.size(); ++i) {
            allMyCodelets[i]->initCodelet(numInputSignals + 1, numInputSignals + 1, this, SHORTWAIT);
        }
    }
};

void
TileFull::fire(void)
{
    DEBUG_MESSAGE("TileFull fired %lx", (uint64_t)myTP_); 
    FullConnect * myTP = static_cast<FullConnect*>(myTP_);
    for (int j = 0; j < num; j++)
    {
        double sum = 0;
        for(int i = 0; i < in_ch; i++)
        {
            sum += input[i] * kernel[num*in_ch + i]; //  TODO JOSE This is just dot product. We can do it better
        }
        output[j] = action(sum + bias[j]);
    }
    myTP->nextLayerMaps->at(0)->decDep();

}
