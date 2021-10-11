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



class TileConv2Full : public Codelet
{
    public:
        int ch, num;
        matrix *input;
        double *output;
    matrix *kernel;
    double *bias;
    double (*action)(double);
    TileConv2Full(uint32_t dep, uint32_t res, ThreadedProcedure * myTP, uint32_t stat,
            matrix *input_, double * output_, matrix *kernel_, double *bias_, double (*action_)(double),
            int ch_, int num_):
            Codelet(dep,res,myTP,stat),
            ch(ch_), num(num_), input(input_), output(output_),
            kernel(kernel_), bias(bias_), action(action_)
    {}

    virtual void fire(void);
};

class Conv2Full : public Layer
{
private:
    uint32_t numOfCuts;
public:

    Conv2Full(trig_cd *prevLayerTrig, trig_cd *nextLayerTrig, uint32_t layerId, uint64_t *startTime, uint64_t *stopTime,
              matrix *input, double *output, matrix *kernel, double *bias,
              double (*action)(double), int cut, int input_ch, int output_num):
                Layer(prevLayerTrig, nextLayerTrig, layerId, startTime, stopTime), 
                numOfCuts(cut)
    {
        int tile = output_num / cut;
        int mod = output_num % cut;
        for(int i = 0; i < cut; i++)
        {
            int output_Size = (i + 1 == cut) ? (tile + mod) : (tile);
            Codelet *temp=new TileConv2Full(input_ch + 1, input_ch + 1, this, SHORTWAIT, input,
                    &output[i*tile], &kernel[i*tile*input_ch], &bias[i*tile], action,
                    input_ch, output_Size);
            Layer::insertElementToMap(temp);
            Layer::registerCodelet(temp);
        }
        Layer::layerReadyToStart();
    }
        
    void signalAll(uint32_t inputChannel) {
        DEBUG_MESSAGE("Signaling input channel %d.", inputChannel);
        (void)(inputChannel);
        // TODO, there is no real advantage of this. There must be more overlapping
        //for ( uint32_t = outChannelIndexes[outChannel]; compCodelet < numICuts*numJCuts; ++compCodelet) {
        //    myMaps[compCodelet]->decDep();
        //}
        for ( uint32_t codelet_i = 0; codelet_i < myMaps.size(); ++codelet_i) {
            myMaps[codelet_i]->decDep();
        }
    }
    uint32_t getNumCuts() {
        return numOfCuts;
    }
};

void
TileConv2Full::fire(void)
{
    DEBUG_MESSAGE("Tileconv2Full fired %lx", (uint64_t)myTP_); 
    Conv2Full * myTP = static_cast<Conv2Full*>(myTP_);
    for (int j = 0; j < num; j++)
    {
        double sum = 0;
        for(int i = 0; i < ch; i++)
        {
            sum += DotProduct(&input[i], &kernel[i]);
        }
        output[j] = action(sum + bias[j]);
    }
    
    // Signal next layer
    for ( uint32_t codelet_i = 0; codelet_i < myTP->nextLayerMaps->size(); ++codelet_i) {
        myTP->nextLayerMaps->at(codelet_i)->decDep();
    }
}


