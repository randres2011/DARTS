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

typedef struct convArgs_s {
    // Arguments
    matrix * inputMatrix;
    matrix * kernelMatrix;
    matrix * outputMatrix;
    int kernelSize;
    int input_size_x;
    int input_size_y;
    int stride;
    int output_size_x;
    int output_size_y;

    double bias;
    double (*action)(double);
    int codeletOutputCh;
    int numInputChannels;
    int codeletNumId;
    int output_xstart;
    int output_xend;
    int output_ystart;
    int output_yend;

} convArgs_t;

class TileCompute : public Codelet
{
    private:
        convArgs_t *myArgs;

        // Private methods
        void fillInputFeaturesMatrix(matrix *inputFeaturesMatrix, matrix *sourceInputMatrix, int currentChannel);

    public:
        TileCompute(uint32_t dep, uint32_t res, ThreadedProcedure * myTP, uint32_t stat, convArgs_t *myNewArgs):
            Codelet(dep, res, myTP, stat),
            myArgs(myNewArgs){ }
        
        void decDep(){
            Codelet::decDep();
            DEBUG_MESSAGE("DecDep Codelet %lx, remaining: %d", (uint64_t)this, this->getCounter());
        }
        virtual void fire(void);

        void mklConvolution(void);
};

inline void 
TileCompute::fillInputFeaturesMatrix(matrix *inputFeaturesMatrix, matrix *sourceInputMatrix, int currentChannel) {
    int size_x = myArgs->output_xend - myArgs->output_xstart;
    int size_y = myArgs->output_yend - myArgs->output_ystart;
    int numElementsKernel = myArgs->kernelSize*myArgs->kernelSize;
    int currentRowInputFeatures, initialColInput, initialRowInput;
    for (int input_row = 0; input_row < size_y; ++input_row) {
        for (int input_col = 0; input_col < size_x; ++input_col) {
            currentRowInputFeatures = input_row*size_x + input_col;
            initialColInput = (input_col + myArgs->output_xstart)*myArgs->stride;
            initialRowInput = (input_row + myArgs->output_ystart)*myArgs->stride;
            for (int k_row = 0; k_row < myArgs->kernelSize; ++k_row) {
                for (int k_col = 0; k_col < myArgs->kernelSize; ++k_col) {
                    inputFeaturesMatrix->SetElement(currentChannel*numElementsKernel + k_row*myArgs->kernelSize + k_col,
                            currentRowInputFeatures,
                            sourceInputMatrix->GetElement(initialColInput + k_col,initialRowInput + k_row));
                }

            }
        }
    }

}

class OneConvLayer : public Layer
{
    private:
        void fillKernelMatrix(matrix *kernelMatrix, matrix *sourceKernel, int currentChannel);
    public:
        double * spResults;
        int resultStep;
        double * spInput;
        int inputStep;
        matrix * kernel_matrix;
        std::vector<convArgs_t*> codeletArgs;

        // For keeping track of the channel codelets within myMap
        uint32_t numInputChannels;
        uint32_t numOutputChannels;
        uint32_t *outChannelIndexes;
        uint32_t numICuts;
        uint32_t numJCuts;

        OneConvLayer(trig_cd *prevLayerTrig, trig_cd *nextLayerTrig, uint32_t layerId, uint64_t *startTime, uint64_t *stopTime,
                matrix *input, matrix *kernel, matrix *output, double *bias,
                double (*action)(double), int isx, int isy, int ks, int strd,
                int IC, int JC, int input_ch, int output_ch) :
                Layer(prevLayerTrig, nextLayerTrig, layerId, startTime, stopTime),
                numInputChannels(input_ch), numOutputChannels(output_ch),
                outChannelIndexes(new uint32_t[output_ch]), numICuts(IC), numJCuts(JC)

        {
            int outsize_i=(isx-ks+1)/strd;
            int outsize_j=(isy-ks+1)/strd;
            int xtile=outsize_i/IC;
            int ytile=outsize_j/JC;
            int xmod=outsize_i%IC;
            int ymod=outsize_j%JC;

            spResults = new double[xtile*ytile*IC*JC*output_ch];
            resultStep = xtile*ytile;
            spInput = new double[ks*ks*input_ch*xtile*ytile*IC*JC*output_ch];
            inputStep = ks*ks*input_ch*xtile*ytile;

            // We only need a single kernel matrix. All of them are the same
            // Col major was much more easier
            int kernel_matrix_xdim = 1;
            int kernel_matrix_ydim = ks * ks * input_ch;
            kernel_matrix = new matrix[output_ch];


            //std::cout<<"Single Conv Initializing.\n";
            for(int ch=0;ch<output_ch;ch++)
            {
                outChannelIndexes[ch] = myMaps.size();
                kernel_matrix[ch].Resize(kernel_matrix_xdim, kernel_matrix_ydim, 0);
                // we fill the kernel and the input features matrix
                for (int current_ch = 0; current_ch < input_ch; current_ch++) {
                    fillKernelMatrix(&kernel_matrix[ch], &kernel[ch*input_ch + current_ch], current_ch);
                }
                for(int num=0;num<IC*JC;num++)
                {
                    int i=num/JC;
                    int j=num%JC;

                    int output_tempxSize = (i + 1 == IC) ? (xtile + xmod) : (xtile);
                    int output_tempySize = (j + 1 == JC) ? (ytile + ymod) : (ytile);

                    int output_xstart = i * xtile;
                    int output_xend = output_xstart + output_tempxSize;
                    int output_ystart = j * ytile;
                    int output_yend = output_ystart + output_tempySize;

                    convArgs_t * argsToSend = new convArgs_t;
                    // Old mmArgs
                    argsToSend->inputMatrix = input;
                    argsToSend->kernelMatrix = &(kernel[ch*input_ch]);
                    argsToSend->outputMatrix = &output[ch];
                    argsToSend->kernelSize = ks;
                    argsToSend->input_size_x = isx;
                    argsToSend->input_size_y = isy;
                    argsToSend->stride = strd;
                    argsToSend->output_size_x = (isx - ks) / strd + 1;
                    argsToSend->output_size_y = (isy - ks) / strd + 1;


                    // Old constructor args
                    argsToSend->bias = bias[ch];
                    argsToSend->action = action;
                    argsToSend->codeletOutputCh = ch;
                    argsToSend->numInputChannels = input_ch;
                    argsToSend->codeletNumId = num;
                    argsToSend->output_xstart = output_xstart;
                    argsToSend->output_xend = output_xend;
                    argsToSend->output_ystart = output_ystart;
                    argsToSend->output_yend = output_yend;

                    codeletArgs.push_back(argsToSend);
                    Codelet* temp_address = new TileCompute(input_ch + 1, 
                            input_ch + 1,
                            this,
                            SHORTWAIT,
                            argsToSend);
                    
                    
                    Layer::insertElementToMap(temp_address);
                    Layer::registerCodelet(temp_address);
                }
            }
            // Signal previous and next layer
            Layer::layerReadyToStart();
        }

        void signalOutputsTile(uint32_t ICutIndex, uint32_t JCutIndex) {
            // Maps are divided by output channels, inside each output channels there are IC x JC 
            // codelets. Once a IC x JC input is ready, it can be signaled.
            for ( uint32_t ch = 0; ch < this->numOutputChannels; ++ch) {
                myMaps[outChannelIndexes[ch] + ICutIndex*numJCuts + JCutIndex]->decDep();
            }
        }
        void signalOutputsChannel(uint32_t inputChannel) {
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

        uint32_t numOutputSignals() {
            return numICuts * numJCuts;
        }

        uint32_t getNumICuts() {
            return numICuts;
        }
        uint32_t getNumJCuts() {
            return numJCuts;
        }

        ~OneConvLayer() {
            for (uint32_t i = 0; i < allMyCodelets.size(); i++)
                delete allMyCodelets[i];
            for (unsigned int i = 0; i < codeletArgs.size(); i++)
                delete codeletArgs[i];
            delete kernel_matrix;
            delete spInput;
            delete spResults;
            delete outChannelIndexes;
        }
};

inline void 
OneConvLayer::fillKernelMatrix(matrix *kernelMatrix, matrix *sourceKernel, int currentChannel) {
    int initialPos = currentChannel*sourceKernel->getN()*sourceKernel->getN();
    for (int k_row = 0; k_row < sourceKernel->getM(); ++k_row) {
        for (int k_col = 0; k_col < sourceKernel->getM(); ++k_col) {
            kernelMatrix->SetElement(0, initialPos + k_row*sourceKernel->getM() + k_col, 
                    sourceKernel->GetElement(k_col, k_row));
        }

    }

}

void TileCompute::mklConvolution(void) {
    OneConvLayer * myTP = static_cast<OneConvLayer*>(myTP_);

    int size_x = myArgs->output_xend - myArgs->output_xstart;
    int size_y = myArgs->output_yend - myArgs->output_ystart;

    // We create the input features matrix
    int input_features_xdim = myArgs->kernelSize * myArgs->kernelSize * myArgs->numInputChannels;
    int input_features_ydim = size_x*size_y;
    double * thisSPInput = myTP->spInput + myTP->inputStep*myArgs->codeletNumId*myArgs->codeletOutputCh;
    matrix input_features_mat(input_features_xdim, input_features_ydim, thisSPInput);

    // We create the result matrix
    double *result = myTP->spResults + myTP->resultStep*myArgs->codeletNumId*myArgs->codeletOutputCh;
    memset(result, 0, sizeof(double)*size_x*size_y);

    int kernel_matrix_xdim = 1;
    int kernel_matrix_ydim = myArgs->kernelSize * myArgs->kernelSize * myArgs->numInputChannels;
    double *kernel_matrix = myTP->kernel_matrix[myArgs->codeletOutputCh].getPtr();

    for (int current_ch = 0; current_ch < myArgs->numInputChannels; current_ch++) {
        fillInputFeaturesMatrix(&input_features_mat, &(myArgs->inputMatrix[current_ch]), current_ch);
    }

    DGEMM_COLMAJOR(input_features_ydim, kernel_matrix_xdim, input_features_xdim, 1,
            input_features_mat.getPtr(), input_features_ydim, kernel_matrix, kernel_matrix_ydim, 0, result, size_x*size_y);

    // assing result to c and signal
    for (int output_col = 0; output_col < size_x; ++output_col) {
        for (int output_row = 0; output_row < size_y; ++output_row) {
            double temp_result = myArgs->action(result[output_col*size_y + output_row] + myArgs->bias);
            myArgs->outputMatrix->SetElement(myArgs->output_xstart + output_col, myArgs->output_ystart + output_row, temp_result);
        }
    }
    // TODO JOSE myTP->nextLayerMaps->at(myArgs->codeletOutputCh)->decDep();
    //myTP->nextLayerMaps->at(0)->decDep();
}

    void
TileCompute::fire(void)
{
    DEBUG_MESSAGE("tileCompute fired %lx", (uint64_t)myTP_); 


#if ENABLE_MKL == MKL_MATRIX_CONV
    mklConvolution();
#else
    
    OneConvLayer * myTP = static_cast<OneConvLayer*>(myTP_);
    
    matrix temp(myArgs->kernelSize, myArgs->kernelSize);
    int size_x = myArgs->output_xend - myArgs->output_xstart;
    int size_y = myArgs->output_yend - myArgs->output_ystart;
    double result[size_x*size_y];
    memset(result, 0, sizeof(double)*size_x*size_y);
    double temp_result;

    for (int k = 0; k < myArgs->numInputChannels; k++) {
        for (int output_i = myArgs->output_xstart ; output_i < myArgs->output_xend; output_i++)
        {
            for (int output_j = myArgs->output_ystart; output_j < myArgs->output_yend; output_j++)
            {
                temp.SetElement(&(myArgs->inputMatrix[k]), output_i*myArgs->stride, output_j*myArgs->stride);
#if ENABLE_MKL == MKL_DOTPRODUCT_CONV
                result[(output_i - myArgs->output_xstart)*size_y + output_j - myArgs->output_ystart] += 
                    DDOT(myArgs->kernelSize*myArgs->kernelSize, 
                            temp.getPtr(), 0, 
                            myArgs->kernelMatrix[k].getPtr(), 0);
#else
                result[(output_i - myArgs->output_xstart)*size_y + output_j - myArgs->output_ystart] += DotProduct(&temp,&(myArgs->kernelMatrix[k]));
#endif

                if (k == (myArgs->numInputChannels - 1))
                {                                       
                    temp_result = myArgs->action(result[(output_i - myArgs->output_xstart)*size_y + output_j - myArgs->output_ystart] + myArgs->bias);
                    myArgs->outputMatrix->SetElement(output_i, output_j, temp_result);
                    
                }

            }
        }
    }

    // Signal next layer
    myTP->nextLayerMaps->at(myArgs->codeletOutputCh)->decDep();
#endif

}

