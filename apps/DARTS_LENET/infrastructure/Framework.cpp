#include "Framework.h"

Framework::Framework(Codelet * toSig_, execTimes_t *execTimes, uint32_t frameworkId):
    ThreadedProcedure(),
    frameworkId(frameworkId),
    startTime(&execTimes->frameworksStartTime[frameworkId]),
    endTime(&execTimes->frameworksStopTime[frameworkId]),
    endCodeletSignal(toSig_),
    resetLayers(1,1, this, SHORTWAIT),
    layersFinished(1, 1, this, SHORTWAIT)
{
    DEBUG_MESSAGE("Creating Framework %lx", (uint64_t)this); 
    *startTime = getTime();
    // Initialize exec times
    execTimes->layerStartTime[frameworkId] = new uint64_t[LAYERS_PER_FRAMEWORK];
    execTimes->layerStopTime[frameworkId] = new uint64_t[LAYERS_PER_FRAMEWORK];
    for (int i = 0; i < LAYERS_PER_FRAMEWORK; i++) {
        myLayers[i] = NULL;
        if (i != LAYERS_PER_FRAMEWORK - 1) myTrigs[i] = NULL;
    } 
}

void frameworkFinishCodelet::fire(void)
{
    DEBUG_MESSAGE("Framework finish fired %lx", (uint64_t)myTP_); 
    Framework * myTP = static_cast<Framework*>(myTP_); // We obtain our TP

    for (int i = 0; i < LAYERS_PER_FRAMEWORK; i++) {
        myTP->myLayers[i]->ThreadedProcedure::decRef();// Make sure they are deleted
    }
    *myTP->endTime = getTime();
    myTP->endCodeletSignal->decDep();
}

void resetAllLayers::fire(void)
{
    DEBUG_MESSAGE("reset all fired %lx", (uint64_t)myTP_); 
    // TODO Think if this is thread safe
    this->resetCodelet();
    Framework * myTP = static_cast<Framework*>(myTP_); // We obtain our TP

    // Reset all the layers
    for (int i = 0; i < LAYERS_PER_FRAMEWORK; i++) {
        myTP->myLayers[i]->reset();
    }
}

void trig_cd::fire(void)
{
    DEBUG_MESSAGE("trig_cd fired %lx", (uint64_t)myTP_); 
    Framework *myTP = static_cast<Framework*>(myTP_); // We obtain our TP

    // Obtain next and prev layers
    Layer * previousLayer = myTP->getLayer(prevLayerId);
    Layer * nextLayer = myTP->getLayer(nextLayerId);

    // Connect next and previous layers
    previousLayer->nextLayer = nextLayer;
    nextLayer->previousLayer = previousLayer;

    // Method to do things after connection
    nextLayer->onAfterConnectPrevLayer();
    previousLayer->onAfterConnectNextLayer();
    
    // signal the layers. They should be ready to go
    previousLayer->connectionReadySig.decDep(); // Start output Layer
    nextLayer->connectionReadySig.decDep(); // Start output Layer

    // Boundary conditions initial layer and final layer
    if (prevLayerId == 0) previousLayer->connectionReadySig.decDep();
    if (nextLayerId == LAYERS_PER_FRAMEWORK - 1) nextLayer->connectionReadySig.decDep();
    //inputLayerSig->decDep(); // Start input Layer
}
