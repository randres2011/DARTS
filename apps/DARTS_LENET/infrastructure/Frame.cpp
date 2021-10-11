#include "Frame.h"

Frame::Frame(Codelet * toSig_,
             execTimes_t * execTimes, int numFrames_):
    ThreadedProcedure(),
    startFrameTime(&execTimes->frameStartTime),
    endFrameTime(&execTimes->frameStopTime),
    numFrames(numFrames_),
    endCodeletSignal(toSig_), 
    frameworksFinished(numFrames_,numFrames_,this,SHORTWAIT)
{
    // Initialize execution times
    execTimes->frameworksStartTime = new uint64_t[numFrames_];
    execTimes->frameworksStopTime = new uint64_t[numFrames_];
    execTimes->layerStartTime = new uint64_t*[numFrames_];
    execTimes->layerStopTime = new uint64_t*[numFrames_];

    *startFrameTime = getTime();
}

    
void frameFinishCodelet::fire(void)
{
    Frame * myTP = static_cast<Frame*>(myTP_); // We obtain our TP

    *myTP->endFrameTime = getTime();
    myTP->endCodeletSignal->decDep();
}
