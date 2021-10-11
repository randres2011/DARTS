#include "layerTemplate.h"
#include "Framework.h"

Layer::Layer(trig_cd * previousLayerTrig,  trig_cd * nextLayerTrig, int layerId, uint64_t *startTime, uint64_t *endTime):
    ThreadedProcedure(),
    layerId(layerId),
    connectionReadySig(2, 2, this, SHORTWAIT),
    resetLayerSig(1, 1, this, SHORTWAIT),
    startLayerSig(2, 2, this, SHORTWAIT),
    nextLayerTrig(nextLayerTrig),
    prevLayerTrig(previousLayerTrig),
    previousLayer(NULL), nextLayer(NULL),
    startLayerTime(startTime), endLayerTime(endTime)
{
    DEBUG_MESSAGE("Creating Layer %lx", (uint64_t)this); 
    incRef();
    // Start the timer
    *startLayerTime = getTime();

    // Tell Framework trigs codelets who to call whenever next and previous are ready
    Framework * myFramework = static_cast<Framework*>(this->parentTP_);
    myFramework->setLayer(layerId, this);

} // End of constructor


void 
Layer::layerReadyToStart() {
    // The if checks for boundry condition
    if (prevLayerTrig != NULL) prevLayerTrig->decDep();
    if (nextLayerTrig != NULL) nextLayerTrig->decDep();
}

void
layerIsConnected::fire(void)
{
    DEBUG_MESSAGE("Layer Is Connected fired %lx", (uint64_t) myTP_); 
    Layer *myTP = static_cast<Layer*>(myTP_); // We obtain our TP

    // Tell previous layer where my Maps are. They should be filled already
    if (myTP->nextLayer != NULL)
        myTP->nextLayerMaps = &myTP->nextLayer->myMaps;

    // This codelet is called whenever my previous and next layer are ready
    // I can enable all my codelets
    for (uint32_t i = 0; i < myTP->allMyCodelets.size(); ++i) {
        myTP->allMyCodelets[i]->decDep();
    }

}

void
resetLayerCodelet::fire(void)
{
    DEBUG_MESSAGE("reset Layer Fired %lx", (uint64_t)myTP_); 
    Layer *myTP = static_cast<Layer*>(myTP_); // We obtain our TP

    // Call onReset method
    myTP->onReset();

    // This codelet is called whenever my previous and next layer are ready
    // I can enable all my codelets
    for (uint32_t i = 0; i < myTP->allMyCodelets.size(); ++i) {
        myTP->allMyCodelets[i]->resetCodelet();
    }
    // TODO: This might not be thread safe operation
    this->resetCodelet();
    if(myTP->previousLayer != NULL) myTP->previousLayer->start();
    if(myTP->nextLayer == NULL) myTP->start();
    myTP->start();
}

void
startLayer::fire(void)
{
    DEBUG_MESSAGE("start layer Fired %lx", (uint64_t)myTP_); 
    // TODO: This might not be thread safe operation
    this->resetCodelet();
    Layer *myTP = static_cast<Layer*>(myTP_); // We obtain our TP

    // This codelet is called whenever my previous and next layer are ready
    // I can enable all my codelets
    for (uint32_t i = 0; i < myTP->allMyCodelets.size(); ++i) {
        myTP->allMyCodelets[i]->decDep();
    }

}
