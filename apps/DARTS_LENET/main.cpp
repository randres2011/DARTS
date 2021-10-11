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

#include <omp.h>
#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "darts.h"
#include "getClock.h"

#include "Parameters.h"
#include "framework/forwardPropagation.h"
#define INNERLOOP -1
#define OUTERLOOP -1

#define MAX(x,y) (x>y) ? x : y
#define MIN(x,y) (x<y) ? x : y

using namespace darts;

int main(int argc, char * argv[])
{
    if (argc != 3)
    {
        std::cout << "enter number of CDs TPs" << std::endl;
        return 0;
    }
    int cds = atoi(argv[1]);
    int tps = atoi(argv[2]);
    int tpm = 3;
    int cdm = 2;
    int FRAMES = 10;
    
    std::cout<<"Execution Start!\n";
    ThreadAffinity affin(cds, tps, COMPACT, tpm, cdm);
    if (affin.generateMask())
    {
        Runtime * rt = new Runtime(&affin);

        double out[OUTPUT*FRAMES];
    //	uint64_t startTime[FRAMES];
    //	uint64_t endTime[FRAMES];
        execTimes_t execTimes;

        int64_t innerTime = 0;
        uint64_t outerTime = 0;

        uint64_t outerMin = 0-1;
        uint64_t outerMax = 0;
        uint64_t min;
        uint64_t max;

        for(int i=0;i<OUTERLOOP+2;i++)
        {
            rt->run(launch<forwardPropagationFrame>(out,&Runtime::finalSignal,&execTimes,FRAMES));

            uint64_t innerMin = 0-1;
            uint64_t innerMax = 0;
            for(int j=0;j<INNERLOOP+2;j++)
            {                
                rt->run(launch<forwardPropagationFrame>(out,&Runtime::finalSignal,&execTimes,FRAMES));
                min=execTimes.frameStartTime;
                max=execTimes.frameStopTime;
                for(int i=0;i<FRAMES;i++)
                {
                    if(min>execTimes.frameworksStartTime[i])
                        min=execTimes.frameworksStartTime[i];
                }
                for(int i=0;i<FRAMES;i++)
                {
                    if(max<execTimes.frameworksStopTime[i])
                        max=execTimes.frameworksStopTime[i];
                }

                uint64_t tempTime = max-min;

                innerMin = MIN(innerMin,tempTime);
                innerMax = MAX(innerMax,tempTime);
                innerTime+=tempTime;
            }
            innerTime=innerTime-innerMin-innerMax;
            uint64_t tempTime = innerTime/(INNERLOOP+2); 
            outerMin = MIN(outerMin,tempTime);
            outerMax = MAX(outerMax,tempTime);
            outerTime+=tempTime;
            innerTime = 0;
        }
        outerTime=outerTime-outerMin-outerMax;

        for (int frame = 0; frame < FRAMES; frame ++) {
            delete [] execTimes.layerStartTime[frame];
            delete [] execTimes.layerStopTime[frame];
        }
        delete [] execTimes.layerStartTime;
        delete [] execTimes.layerStopTime;
        delete [] execTimes.frameworksStartTime;
        delete [] execTimes.frameworksStopTime;

        delete rt;

        std::cout<<"Execution over!"<<std::endl;
        std::cout<<"Total Execution time\t"<<outerTime/(OUTERLOOP+2)/1e6<<"ms"<<std::endl;
        //std::cout<<out[0]<<std::endl;
        //std::cout<<out[FRAMES-1]<<std::endl;

    }
}

