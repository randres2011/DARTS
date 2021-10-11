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
#include "../infrastructure/layerTemplate.h"
#include <vector>

#include "../matrix/matrix.h"
#include "../Parameters.h"
#include "../infrastructure/Framework.h"

using namespace darts;

class LoadInput : public Codelet
{
    public:

        LoadInput(uint32_t dep, uint32_t res, ThreadedProcedure * myTP, uint32_t stat):
            Codelet(dep, res, myTP, stat) { }

        virtual void fire(void);

};

class InputLayer : public Layer
{
    public:
        matrix *input;
        LoadInput input_cd;

        InputLayer(trig_cd *prevLayerTrig, trig_cd *nextLayerTrig, uint32_t layerId, uint64_t *startTime, uint64_t *stopTime, matrix *input_) :
            Layer(prevLayerTrig, nextLayerTrig, layerId, startTime, stopTime),
            input(input_), input_cd(1, 1, this, SHORTWAIT)
        {
            Layer::insertElementToMap((Codelet *)&input_cd);
            Layer::registerCodelet((Codelet *)&input_cd);
            Layer::layerReadyToStart();
        }


};

void
LoadInput::fire(void)
{
    InputLayer *myTP = static_cast<InputLayer*>(myTP_); // We obtain our TP
    
    initOneMatrix(myTP->input);
    for (uint32_t i = 0; i < myTP->Layer::nextLayerMaps->size(); i++)
    {			
        myTP->Layer::nextLayerMaps->at(i)->decDep();
    }
    //std::cout<<next_map->size()<<" LoadInput\n";
    //resetCodelet();
}
