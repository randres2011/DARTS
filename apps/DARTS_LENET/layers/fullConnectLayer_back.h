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
using namespace darts;

class Fullback_cd1 : public Codelet
{
public:
    double *input,*inerror,*outerror,*weight;  //no diff_bias, it's equal to outerror
    double (*actgrad)(double);
    int in_num,out_num,start,end;
    std::vector<Codelet*> *toSig;

    //start - the start index of input corresponding to the cd
    //end - ...

    Fullback_cd1(uint32_t dep, uint32_t res, ThreadedProcedure * myTP, uint32_t stat,
        double *input_,double *inerror_,double *outerror_,double *weight_,
        double(*actgrad_)(double),int in_num_,int out_num_,int start_, int end_,std::vector<Codelet*> *toSig_):
        Codelet(dep,res,myTP,stat),input(input_),inerror(inerror_),outerror(outerror_),weight(weight_),
        actgrad(actgrad_),in_num(in_num_),out_num(out_num_),start(start_),end(end_),toSig(toSig_){}

    virtual void fire(void);
};

void Fullback_cd1::fire(void)
{
    double temp_inerror[in_num];
    memset(temp_inerror,0,sizeof(double)*in_num);
    std::cout<<"Fire\tFullback_cd1\n";
    for(int j=0;j<out_num;j++)
    {
    for(int i=start;i<end;i++)
    {
        temp_inerror[i]+=outerror[j]*weight[j*in_num+i];
    }
    }
    for(int i=start;i<end;i++)
    {
        inerror[i]=temp_inerror[i]*actgrad(input[i]);
        for(unsigned int j=0;j<toSig[i].size();j++)
            toSig[i][j]->decDep();
    }
}

class Fullback_cd2 : public Codelet
{
public:
    double *input,*inerror,*outerror,*weight,*dw;  //no diff_bias, it's equal to outerror
    double (*actgrad)(double);
    int in_num,out_num,start,end;
    Codelet *toSig;

    //start - the start index of input corresponding to the cd
    //end - ...

    Fullback_cd2(uint32_t dep, uint32_t res, ThreadedProcedure * myTP, uint32_t stat,
        double *input_,double *inerror_,double *outerror_,double *weight_,double *dw_,
        double(*actgrad_)(double),int in_num_,int out_num_,int start_, int end_,Codelet *toSig_):
        Codelet(dep,res,myTP,stat),input(input_),inerror(inerror_),outerror(outerror_),weight(weight_),dw(dw_),
        actgrad(actgrad_),in_num(in_num_),out_num(out_num_),start(start_),end(end_),toSig(toSig_){}

    virtual void fire(void);
};

void Fullback_cd2::fire(void)
{
    std::cout<<"Fire\tFullback_cd1\n";
    for(int j=0;j<out_num;j++)
    {
    for(int i=start;i<end;i++)
    {
        dw[j*in_num+i]+=outerror[j]*input[i];
    }
    }
    toSig->decDep();
}

class Full_back : public ThreadedProcedure
{
    public:

    Full_back(double *input,double *inerror,double *outerror,double *weight,double *bias,double *dw,double *db,
            double (*gradaction)(double),int in_ch,int out_ch,int cut, std::vector<Codelet*> *address,
            std::vector<Codelet*> *next_map,
            std::vector<Codelet*> *cur_map,Codelet *trig_sig,Codelet *Back_Sig)
    {
        incRef();
        int tile=in_ch/cut;
        int mod=in_ch%cut;
        Codelet *temp1,*temp2;

        for(int i=0;i<cut;i++)
        {

            int Size = (i + 1 == cut) ? (tile + mod) : (tile);
            int start=i*tile;
            int end=start+Size;
        temp1=new Fullback_cd1(Size+1,Size,this,SHORTWAIT,input,inerror,
                outerror,weight,gradaction,in_ch,out_ch,start,end,*next_map[i*tile]);
        temp2=new Fullback_cd2(Size+1,Size,this,SHORTWAIT,input,inerror,
                outerror,weight,dw,gradaction,in_ch,out_ch,start,end,Back_Sig);
        address->push_back(temp1);
        address->push_back(temp2);
        for(int k=start;k<end;k++)
        {
            cur_map[k].push_back(temp1);
            cur_map[k].push_back(temp2);
        }
        }
        trig_sig->decDep();
    }
};
