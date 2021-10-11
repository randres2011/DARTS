#pragma once
#include "darts.h"
#include <vector>
#include "Framework.h"

// FRAME
class frameFinishCodelet: public Codelet
{
    public:
        frameFinishCodelet(uint32_t dep, uint32_t res, ThreadedProcedure * myTP, uint32_t stat):
            Codelet(dep,res,myTP,stat) {}
        virtual void fire(void);
};

class Frame : public ThreadedProcedure
{
    public:
        // Timing: 
        uint64_t *startFrameTime;
        uint64_t *endFrameTime;

        // Framework counts
        uint32_t numFrames;

        // Finish signal codelet
        Codelet *endCodeletSignal;
        frameFinishCodelet frameworksFinished;
        Frame(Codelet * toSig_, execTimes_t * execTimes, int numFrames_);

};


