#pragma once
#include "darts.h"
#include <vector>

#include "../Parameters.h"
#include "layerTemplate.h"

using namespace darts;

typedef struct execTimes_s {
    // Frame Execution
    uint64_t frameStartTime;
    uint64_t frameStopTime;

    // Frameworks execution
    uint64_t *frameworksStartTime;
    uint64_t *frameworksStopTime;

    // Layers execution time
    uint64_t **layerStartTime;
    uint64_t **layerStopTime;
} execTimes_t;

class trig_cd : public Codelet
{
    public:
        int prevLayerId;
        int nextLayerId;
        trig_cd(uint32_t dep, uint32_t res, ThreadedProcedure * myTP, uint32_t stat, int pLayerId, int nLayerId):
            Codelet(dep,res,myTP,stat),
            prevLayerId(pLayerId), nextLayerId(nLayerId){ }

        void fire(void);
};

class frameworkFinishCodelet: public Codelet
{
    private:
        int slowDown;
    public:
        frameworkFinishCodelet(uint32_t dep, uint32_t res, ThreadedProcedure * myTP, uint32_t stat):
            Codelet(dep,res,myTP,stat), slowDown(0) {}
        virtual void fire(void);
};

class resetAllLayers: public Codelet
{
    public:
        resetAllLayers(uint32_t dep, uint32_t res, ThreadedProcedure * myTP, uint32_t stat):
            Codelet(dep,res,myTP,stat) {}
        virtual void fire(void);
};

class Framework : public ThreadedProcedure
{
    public:
        uint32_t frameworkId;

        Layer* myLayers[LAYERS_PER_FRAMEWORK];
        trig_cd* myTrigs[LAYERS_PER_FRAMEWORK - 1];

        //Trig Cd
        std::vector<Codelet*> address[8]; // One per layer

        // ExecutionTime
        uint64_t *startTime;
        uint64_t *endTime;

        // Ending execution
        Codelet *endCodeletSignal;
        resetAllLayers resetLayers;
        frameworkFinishCodelet layersFinished;

        Framework(Codelet * toSig_, execTimes_t *execTimes, uint32_t frameworkId);

        Layer * getLayer(int layerID) {
            return this->myLayers[layerID];
        }

        void setLayer(int layerID, Layer * layerPtr) {
            this->myLayers[layerID] = layerPtr;
        }

        trig_cd *createTrig(int trigPosition) {
            return myTrigs[trigPosition] = new trig_cd(2,2,this, SHORTWAIT, trigPosition, trigPosition + 1);
        }

        ~Framework() {
            // Memory deallocation for codelets
            for (int i = 0; i < LAYERS_PER_FRAMEWORK; i++) {
                if (i != LAYERS_PER_FRAMEWORK - 1 && myTrigs[i] != NULL) {
                    delete (myTrigs[i]);
                }
            } 
            DEBUG_MESSAGE("Deleteing Framework %lx", (uint64_t)this);
        }
};

