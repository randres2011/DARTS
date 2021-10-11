#ifndef __LAYER_TEMPLATE__
#define __LAYER_TEMPLATE__

#include "darts.h"
#include "../Parameters.h"
#include "layerCreationMacros.h"
using namespace darts;

class Framework;
class trig_cd;

class layerIsConnected : public Codelet
{
public:

    layerIsConnected(uint32_t dep, uint32_t res, ThreadedProcedure * myTP, uint32_t stat) :
        Codelet(dep,res,myTP,stat){ }
    virtual void fire(void);
};

class resetLayerCodelet: public Codelet
{
public:

    resetLayerCodelet(uint32_t dep, uint32_t res, ThreadedProcedure * myTP, uint32_t stat) :
        Codelet(dep,res,myTP,stat){ }
    virtual void fire(void);
};

class startLayer: public Codelet
{
public:

    startLayer(uint32_t dep, uint32_t res, ThreadedProcedure * myTP, uint32_t stat) :
        Codelet(dep,res,myTP,stat){ }
    virtual void fire(void);
};

class Layer : public ThreadedProcedure
{
    
    public:
        int layerId; // ID within Framework
        // Connection signals
        layerIsConnected connectionReadySig;
        resetLayerCodelet resetLayerSig;
        startLayer startLayerSig;

        std::vector <Codelet *> allMyCodelets; // We signal these codelets whenever we are ready to start
        std::vector <Codelet *> myMaps; // Child must fill this in with his maps
        std::vector <Codelet *> *nextLayerMaps; // Where to signal when done

        // Previous and next layer pointers
        trig_cd * nextLayerTrig;
        trig_cd * prevLayerTrig;
        Layer * previousLayer;
        Layer * nextLayer;
        
        // For timing
        uint64_t *startLayerTime;
        uint64_t *endLayerTime;

        // Constructor
        Layer(trig_cd * previousLayerTrig,  trig_cd * nextLayerTrig, int layerId, uint64_t *startTime, uint64_t *endTime);

        void insertElementToMap(Codelet * toInsert) {
            myMaps.push_back(toInsert);
        }

        void registerCodelet(Codelet * toInsert) {
            allMyCodelets.push_back(toInsert);
        }

        // Child must call this method to tell the Framework the layer is ready
        void layerReadyToStart();

        // Override this method if you want something to happen on reset
        virtual void onReset() {}
        virtual void onAfterConnectPrevLayer() {}
        virtual void onAfterConnectNextLayer() {}

        void reset() {
            this->resetLayerSig.decDep();
        }

        void start() {
            this->startLayerSig.decDep();
        }

        uint64_t getRef() {
            return this->ref_;
        }

        // Destructor
        ~Layer() {
            // End the timer
            *endLayerTime = getTime();
            DEBUG_MESSAGE("Deleteing layer %lx", (uint64_t)this);
        }
};

#endif /* __LAYER_TEMPLATE__ */
