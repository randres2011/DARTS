#include "forwardPropagation.h"
#include "../layers/Layers.h"
#include "actionFunctions.h"

forwardPropagation::forwardPropagation(double *output, Codelet * toSig_, execTimes_t *execTimes, uint32_t frameworkId) :
    Framework(toSig_, execTimes, frameworkId),
    Feature0(INPUT1,INPUT1), Feature6(output)
    {
        // Create the trig codelets to connect the layers
        // for (int i = 0; i < LAYERS_PER_FRAMEWORK - 1; i++) 
        //    createTrig(i);
        CREATE_TRIGS(myTrigs,LAYERS_PER_FRAMEWORK);

        // Initialize matrices
        for(int i=0;i<CH0*CH1;i++)
            Weight1[i].Resize(KERNEL1,KERNEL1,i);
        for(int i=0;i<CH1;i++)
            Feature1[i].Resize(INPUT2,INPUT2,0);
        for(int i=0;i<CH2;i++)
            Feature2[i].Resize(OUTPUT2,OUTPUT2,0);
        for(int i=0;i<CH3;i++)
            Feature3[i].Resize(OUTPUT3,OUTPUT3,0);
        for(int i=0;i<CH2*CH3;i++)
            Weight3[i].Resize(KERNEL3,KERNEL3,i);
        for(int i=0;i<CH4;i++)
            Feature4[i].Resize(OUTPUT4,OUTPUT4,0);
        for(int i=0;i<CH4*OUTPUT5;i++)
            Weight5[i].Resize(KERNEL5,KERNEL5,1);
        for(int i=0;i<INPUT6*OUTPUT6;i++)
            Weight6[i]=1;

        for(int i=0;i<CH1;i++)
            Bias1[i]=1;
        for(int i=0;i<CH3;i++)
            Bias3[i]=1;
        for(int i=0;i<OUTPUT5;i++)
            Bias5[i]=1;
        for(int i=0;i<OUTPUT6;i++)
            Bias6[i]=1;
        
        CREATE_INVOKE_LAYER(frameworkId, InputLayer, myTrigs, execTimes,
                            &Feature0);

        CREATE_INVOKE_LAYER(frameworkId, OneConvLayer, myTrigs, execTimes,
                             &Feature0, 
                             Weight1,
                             Feature1,
                             Bias1,
                             relu,
                             INPUT1,
                             INPUT1,
                             KERNEL1,
                             STRIDE1,
                             IC1,
                             JC1,
                             CH0,
                             CH1);
        
        CREATE_INVOKE_LAYER(frameworkId, PoolLayer, myTrigs, execTimes,
                            Feature1,
                            Feature2,
                            KERNEL2,
                            CH1);//std::cout<<"Pooling Layer Invoked\n";
        
        CREATE_INVOKE_LAYER(frameworkId, OneConvLayer, myTrigs, execTimes,
                            Feature2,
                            Weight3,
                            Feature3,
                            Bias3,
                            relu,
                            INPUT3,
                            INPUT3,
                            KERNEL3,
                            STRIDE3,
                            IC3,
                            JC3,
                            CH2,
                            CH3);

        CREATE_INVOKE_LAYER(frameworkId, PoolLayer, myTrigs, execTimes,
                            Feature3,
                            Feature4,
                            KERNEL4,
                            CH4);
        
        CREATE_INVOKE_LAYER(frameworkId, Conv2Full, myTrigs, execTimes,
                            Feature4,
                            Feature5,
                            Weight5,
                            Bias5,
                            relu,
                            CUT5,
                            CH4,
                            OUTPUT5);

        CREATE_INVOKE_LAYER(frameworkId, FullConnect, myTrigs, execTimes,
                            Feature5,
                            Feature6,
                            Weight6,
                            Bias6,
                            relu,
                            INPUT6,
                            OUTPUT6,
                            CUT6);
        
        CREATE_INVOKE_LAYER(frameworkId, FinishLayer, myTrigs, execTimes,
                            &resetLayers, 
                            &layersFinished, 
                            CUT6,
                            NUM_REPETITIONS); 


        CHECK_LAYERS_CREATION(myTrigs);

        // INPUT LAYER
//        invoke<InputLayer>(this,
//                            (trig_cd *) NULL,
//                            myTrigs[INPUT_LAYER_ID], 
//                            INPUT_LAYER_ID, 
//                            &execTimes->layerStartTime[frameworkId][INPUT_LAYER_ID],
//                            &execTimes->layerStopTime[frameworkId][INPUT_LAYER_ID], 
//                            &Feature0);
        // FIRST CONVOLUTION LAYER
//        invoke<OneConvLayer>(this,
//                             myTrigs[INPUT_LAYER_ID],
//                             myTrigs[FIRST_CONV_LAYER_ID],
//                             FIRST_CONV_LAYER_ID,
//                             &execTimes->layerStartTime[frameworkId][FIRST_CONV_LAYER_ID],
//                             &execTimes->layerStopTime[frameworkId][FIRST_CONV_LAYER_ID], 
//                             &Feature0, 
//                             Weight1,
//                             Feature1,
//                             Bias1,
//                             relu,
//                             INPUT1,
//                             INPUT1,
//                             KERNEL1,
//                             STRIDE1,
//                             IC1,
//                             JC1,
//                             CH0,
//                             CH1);
//        invoke<FinishLayer>(this, 
//                            myTrigs[1], 
//                            (trig_cd *) NULL,  
//                            2, 
//                            &execTimes->layerStartTime[frameworkId][2],
//                            &execTimes->layerStopTime[frameworkId][2], 
//                            &resetLayers, 
//                            &layersFinished, 
//                            CH1,//CHANGE THIS 
//                            0); 
        //    invoke<InputLayer>(this,&Feature0,&address[0],&address[1],&trig);//std::cout<<"InputLayer Invoked.\n";
        //    invoke<OneConvLayer>(this,&Feature0,Weight1,Feature1,Bias1,relu,INPUT1,INPUT1,KERNEL1,STRIDE1,IC1,JC1,CH0,CH1,&address[1],map2,map1,&trig,1);//std::cout<<"ConvLayer Invoked.\n";
        //    invoke<PoolLayer>(this,Feature1,Feature2,KERNEL2,CH1,&address[2],map3,map2,&trig);//std::cout<<"Pooling Layer Invoked\n";
        //    invoke<OneConvLayer>(this,Feature2,Weight3,Feature3,Bias3,relu,INPUT3,INPUT3,KERNEL3,STRIDE3,IC3,JC3,CH2,CH3,&address[3],map4,map3,&trig,0);
        //    invoke<PoolLayer>(this,Feature3,Feature4,KERNEL4,CH4,&address[4],map5,map4,&trig);//std::cout<<"Pooling Layer Invoked\n";
        //    invoke<Conv2Full>(this,Feature4,Feature5,Weight5,Bias5,relu,KERNEL5,CUT5,CH4,OUTPUT5,&address[5],map6,map5,&trig);
        //    invoke<FullConnect>(this,Feature5,Feature6,Weight6,Bias6,relu,INPUT6,OUTPUT6,CUT6,&address[6],map7,map6,&trig);
        //    invoke<FinishLayer>(this,&address[7],map7,toSig_,OUTPUT,&trig,endTime,8,address);//std::cout<<"FinishLayer Invoked\n";

    }


forwardPropagationFrame::forwardPropagationFrame(double * output, Codelet * toSig_, execTimes_t * execTimes, int numFrames_) :
    Frame(toSig_, execTimes, numFrames_)
{

    for(uint32_t i = 0; i < Frame::numFrames; i++)
    {
        invoke<forwardPropagation>(this, &output[i*OUTPUT], &frameworksFinished, execTimes,i);
    }
}
