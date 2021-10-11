
#define GET_TRIGS_COUNTER(trigsVar) macro_  ##  trigsVar  ##  Counter
#define GET_TRIGS_TOTAL(trigsVar) macro_ ## trigsVar ## Total
#define GET_LAYERS_COUNTER(trigsVar) macro_ ## trigsVar ## LayersCounter
#define GET_LAYERS_TOTAL(trigsVar) macro_ ## trigsVar ## NumLayers
#define MARK_TRIGS_CREATED(trigsVar, numLayers) \
    uint32_t GET_TRIGS_COUNTER(trigsVar) = 0; \
    const uint32_t GET_TRIGS_TOTAL(trigsVar) = numLayers - 1; \
    uint32_t GET_LAYERS_COUNTER(trigsVar) = 0; \
    const uint32_t GET_LAYERS_TOTAL(trigsVar) = numLayers;

#define CREATE_TRIGS(trigsVar, numLayers) \
    MARK_TRIGS_CREATED(trigsVar, numLayers) \
    { \
        for (uint32_t i = 0; i < GET_TRIGS_TOTAL(trigsVar); i++) \
            createTrig(i); \
    } 

#define CREATE_INVOKE_LAYER(frameworkId, layerType, trigsVar, execTimes, ...) { \
        trig_cd * _inputTrig = NULL, *_outputTrig = NULL; \
        if (GET_LAYERS_COUNTER(trigsVar) == 0) { \
            _outputTrig = trigsVar[GET_TRIGS_COUNTER(trigsVar)]; \
        } else if (GET_LAYERS_COUNTER(trigsVar) == GET_LAYERS_TOTAL(trigsVar) - 1) { \
            _inputTrig = trigsVar[GET_TRIGS_COUNTER(trigsVar) - 1]; \
        } else { \
            _inputTrig = trigsVar[GET_TRIGS_COUNTER(trigsVar) - 1]; \
            _outputTrig = trigsVar[GET_TRIGS_COUNTER(trigsVar)]; \
        } \
        invoke<layerType> (this, \
                           _inputTrig, \
                           _outputTrig, \
                           GET_LAYERS_COUNTER(trigsVar), \
                           &execTimes->layerStartTime[frameworkId][GET_LAYERS_COUNTER(trigsVar)], \
                           &execTimes->layerStopTime[frameworkId][GET_LAYERS_COUNTER(trigsVar)], \
                            ## __VA_ARGS__); \
        GET_LAYERS_COUNTER(trigsVar)++; \
        GET_TRIGS_COUNTER(trigsVar)++; \
        DEBUG_MESSAGE("Layer " layerType " invoked. framework ID = %d, inputTrig = %lx, outputTrig = %lx, trigsCount = %d, layerCouter = %d out of %d ", \
                      frameworkId, (uint64_t) _inputTrig, (uint64_t) _outputTrig, GET_TRIGS_COUNTER(trigsVar), GET_LAYERS_COUNTER(trigsVar), GET_LAYERS_TOTAL(trigsVar));\
}

#define CHECK_LAYERS_CREATION(trigsVar) { \
    if (GET_LAYERS_COUNTER(trigsVar) != GET_LAYERS_TOTAL(trigsVar)) \
        ERROR_MESSAGE("Number of layers created [%d] is not equal to the total specified [%d]",GET_LAYERS_COUNTER(trigsVar), GET_LAYERS_TOTAL(trigsVar)); \
}
