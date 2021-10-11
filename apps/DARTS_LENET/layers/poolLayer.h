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
#include "convLayer.h"
#include "conv2FullLayer.h"
using namespace darts;


class PoolCompute : public Codelet
{
private:

    matrix * input;
    matrix * output;
    int ks;
    int myChannel;

public:
    PoolCompute(matrix *input_,matrix * output_,int ks_, int myChannel):
    Codelet(),
    input(input_),
    output(output_),
    ks(ks_),
    myChannel(myChannel)
    { }

    PoolCompute(uint32_t dep, uint32_t res, ThreadedProcedure * myTP, uint32_t stat, matrix *input_,matrix * output_,int ks_, int myChannel):
    Codelet(dep,res,myTP,stat),
    input(input_),
    output(output_),
    ks(ks_),
    myChannel(myChannel)
    { }

    virtual void fire(void);
};

class PoolLayer : public Layer
{
    public:
        OneConvLayer * prevConvLayer; 
        OneConvLayer * nextConvLayer; 
        PoolLayer(trig_cd *prevLayerTrig, trig_cd *nextLayerTrig, 
                uint32_t layerId, uint64_t *startTime, uint64_t *stopTime, 
                matrix *input, matrix *output, int ks, int input_ch) :
            Layer(prevLayerTrig, nextLayerTrig, layerId, startTime, stopTime)
        {
            for(int i = 0; i < input_ch; i++)
            {
                //Codelet* temp=new PoolCompute(isx*isy+1,isx*isy,this,SHORTWAIT,&input[i],&output[i],ks,&next_map[isx*isy*i/ks/ks]);
                Codelet* temp = new PoolCompute(&input[i], &output[i], ks, i);

                Layer::insertElementToMap(temp);
                Layer::registerCodelet(temp);
            }

            // Signal previous and next layer
            Layer::layerReadyToStart();
        }
        virtual void onAfterConnectPrevLayer();
        virtual void onAfterConnectNextLayer();

};

void
PoolLayer::onAfterConnectPrevLayer() {
    prevConvLayer = static_cast<OneConvLayer*> (previousLayer);
    uint32_t numInputSignals = prevConvLayer->numOutputSignals();
    for (uint32_t i = 0; i < allMyCodelets.size(); ++i) {
        allMyCodelets[i]->initCodelet(numInputSignals + 1, numInputSignals + 1, this, SHORTWAIT);
    }
}

void 
PoolLayer::onAfterConnectNextLayer() {
    nextConvLayer = static_cast<OneConvLayer*> (nextLayer);
}


    void
PoolCompute::fire(void)
{
    DEBUG_MESSAGE("poolCompute fired %lx", (uint64_t)myTP_); 
    PoolLayer *myTP = static_cast<PoolLayer *>(myTP_); // We obtain our TP

    int isx=input->getM();
    int isy=input->getN();
    int max=-214748364;
    double *s=input->getPtr();
    for (int i=0;i<isx;i+=ks)
    {
        for (int j=0;j<isy;j+=ks)
        {
            max=-214748364;
            for(int ii=0;ii<ks;ii++)
            {
                for(int jj=0;jj<ks;jj++)
                {
                    if(max<s[(i+ii)*isy+j+jj])
                    {
                        max=s[(i+ii)*isy+j+jj];
                    }
                }

            }
            output->SetElement(i/ks,j/ks,max);
        }
    }
    // Check if the next layer is conv2Full
    if (myTP->nextLayer->layerId == CONV2FULL_LAYER_ID) {
        Conv2Full* nextConv2FullLayer = static_cast<Conv2Full*> (myTP->nextLayer);
        nextConv2FullLayer->signalAll(myChannel);
    } else {
        myTP->nextConvLayer->signalOutputsChannel(myChannel);
    }
}
