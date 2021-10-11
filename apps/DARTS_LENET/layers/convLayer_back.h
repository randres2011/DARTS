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


class Convback_cd1 : public Codelet
{
public:
    matrix *input,*inerror,*outerror,*weight;  //no diff_bias, it's equal to outerror
    double (*actgrad)(double);
    int ks,in_ch,out_ch;
    ConvArgs *args;
    std::vector<Codelet*> *toSig;
    Convback_cd1(uint32_t dep, uint32_t res, ThreadedProcedure * myTP, uint32_t stat,
        matrix *input_,matrix *inerror_,matrix *outerror_,matrix *weight_,
        double(*actgrad_)(double),int ks_,int in_num_,int out_num_,ConvArgs *args_,std::vector<Codelet*> *toSig_):
        Codelet(dep,res,myTP,stat),input(input_),inerror(inerror_),outerror(outerror_),weight(weight_),
        actgrad(actgrad_),ks(ks_),in_ch(in_num_),out_ch(out_num_),args(args_),toSig(toSig_){}

    virtual void fire(void);
};



void Convback_cd1::fire(void)
{
    int kernal_i,kernal_j,ch,i,j,ii,jj,k,index;
    int in_size_x=args->in_yend-args->in_ystart;
    int in_size_y=args->in_xend-args->in_xstart;
    double *result = new double[in_size_x*in_size_y];
    memset(result,0,sizeof(double)*in_size_x*in_size_y);
    double *out;
    int out_N;
    double *kernal;
    std::cout<<"Fired\tonvback_cd1\n";
    for(ch=0;ch<out_ch;ch++)
    {
        out=outerror[ch].getPtr();
        out_N=outerror[ch].getN();
        kernal=weight[ch].getPtr();
        for(i=args->out_xstart;i<args->out_xend;i++)
        {
            for(j=args->out_ystart;j<args->out_yend;i++)
            {
                for(ii=MAX(i-ks,args->in_xstart);ii<MIN(i+ks,args->in_xend);ii++)
                {
                    for(jj=MAX(j-ks,args->in_ystart);ii<MIN(j+ks,args->in_yend);ii++)
                    {
                        kernal_i=ii-i;
                        kernal_j=jj-j;
                        index=(ii-args->in_xstart)*in_size_y+jj-args->in_ystart;
                        result[index]+=out[i*out_N+j]*kernal[kernal_i*ks+kernal_j];
                        if(ch==(out_ch-1))
                        {
                            result[index]=result[index]*actgrad(input->GetElement(ii,jj));
                            inerror->SetElement(ii,jj,result[index]);
                            for(k=0;k<toSig[index].size();k++)
                            {
                                toSig[index][k]->decDep();
                            }
                        }
                    }
                }
            }
        }
    }
    resetCodelet();
}


class Convback_bias : public Codelet
{
public:
    matrix *outerror;  //no diff_bias, it's equal to outerror
    double *db;
    Codelet *toSig;
    Convback_bias(uint32_t dep, uint32_t res, ThreadedProcedure * myTP, uint32_t stat,
        matrix *outerror_,double *db_,Codelet *toSig_):
        Codelet(dep,res,myTP,stat),outerror(outerror_),db(db_),toSig(toSig_){}

    virtual void fire(void);
};


void Convback_bias::fire(void)
{

    int i,j,k;
    double *out=outerror->getPtr();

    int N=outerror->getN();
    int M=outerror->getM();
    for(i=0;i<M;i++)
    {
        for(j=0;j<N;j++)
        {
            *db+=out[i*N+j];
        }
    }
    toSig->decDep();

    resetCodelet();
}


class Convback_weight : public Codelet
{
public:
    matrix *input,*outerror,*dw;  //no diff_bias, it's equal to outerror
    int ks;
    Codelet *toSig;
    Convback_weight(uint32_t dep, uint32_t res, ThreadedProcedure * myTP, uint32_t stat,
        matrix *input_,matrix *outerror_,matrix *dw_,int ks_,Codelet *toSig_):
        Codelet(dep,res,myTP,stat),input(input_),outerror(outerror_),dw(dw_),
        ks(ks_),toSig(toSig_){}

    virtual void fire(void);
};

void Convback_weight::fire(void)
{
    int i,j,k;
    matrix temp(input->getM(),input->getN());
    std::cout<<"Fired\tonvback_cd3\n";
    for(i=0;i<ks;i++)
    {
        for(j=0;j<ks;j++)
        {
            temp.SetElement(input,i,j);
            dw->SetElement(i,j,DotProduct(&temp,outerror));
        }
    }
    toSig->decDep();
    resetCodelet();
}

class Conv_back : public ThreadedProcedure
{
    public:

    Conv_back(matrix *input,matrix *inerror,matrix *outerror,matrix *weight,matrix *dw,
            double *db,double (*gradaction)(double),int in_ch,int out_ch,int IC,int JC,
            std::vector<Codelet*> *address,std::vector<Codelet*> *next_map,
            std::vector<Codelet*> *cur_map,Codelet *trig_sig,Codelet *Back_Sig)
    {
        incRef();
        ConvArgs *args;
        Codelet* temp_address;
        int outerror_size_i=outerror->getM();
        int outerror_size_j=outerror->getN();
        int inerror_size_i=inerror->getM();
        int inerror_size_j=inerror->getN();
        int ks=weight->getM();
        for(int ch=0;ch<out_ch;ch++)
        {
            temp_address=new Convback_bias(outerror_size_i*outerror_size_j+1,outerror_size_i*outerror_size_j,
                    this,SHORTWAIT,&outerror[ch],&db[ch],Back_Sig);
            address->push_back(temp_address);
            for(int i=0;i<outerror_size_i;i++)
            {
                for(int j=0;j<outerror_size_j;j++)
                {
                    cur_map[i*outerror_size_j+j+ch*outerror_size_j*outerror_size_i].push_back(temp_address);
                }

            }
        }
        for(int in=0;in<in_ch;in++)
        {
            for(int out=0;out<out_ch;out++)
            {
                temp_address=new Convback_weight(outerror_size_i*outerror_size_j+1,outerror_size_i*outerror_size_j,
                        this,SHORTWAIT,&input[in],&outerror[out],&dw[in*in_ch+out],ks,Back_Sig);
                address->push_back(temp_address);
                for(int i=0;i<outerror_size_i;i++)
                {
                    for(int j=0;j<outerror_size_j;j++)
                    {
                        cur_map[i*outerror_size_j+j+out*outerror_size_j*outerror_size_i].push_back(temp_address);
                    }

                }
            }
        }
        for(int ch=0;ch<in_ch;ch++)
        {
            for(int num=0;num<IC*JC;num++)
            {
                int i=num/JC;
                int j=num%JC;

                int xtile=inerror_size_i/IC;
                int ytile=inerror_size_j/JC;
                int xmod=inerror_size_i%IC;
                int ymod=inerror_size_j%JC;

                int inerror_tempxSize = (i + 1 == IC) ? (xtile + xmod) : (xtile);
                int inerror_tempySize = (j + 1 == JC) ? (ytile + ymod) : (ytile);

                int inerror_xstart = i * xtile;
                int inerror_xend = inerror_xstart + inerror_tempxSize;
                int inerror_ystart = j * ytile;
                int inerror_yend = inerror_ystart + inerror_tempySize;

                int outerror_xstart = MAX(0,inerror_xstart-ks);
                int outerror_xend = MIN(inerror_size_i,inerror_xend+ks);
                int outerror_ystart = MAX(0,inerror_ystart-ks);
                int outerror_yend = MIN(inerror_size_j,inerror_yend+ks);

                args=new ConvArgs(inerror_xstart,inerror_xend,inerror_ystart,inerror_yend,
                        outerror_xstart,outerror_xend,outerror_ystart,outerror_yend);

                int size_x=outerror_xend-outerror_xstart;
                int size_y=outerror_yend-outerror_ystart;

                temp_address=new Convback_cd1(in_ch*size_x*size_y+1, in_ch*size_x*size_y, this, SHORTWAIT,
                        input,inerror,outerror,weight, gradaction, ks,in_ch,out_ch,args,
                        &next_map[ch*inerror_size_i*inerror_size_j]);
                for(int k=0;k<out_ch;k++)
                {
                for(int i=outerror_xstart;i<outerror_xend;i++)
                {
                    for(int j=outerror_ystart;j<outerror_yend;j++)
                    {
                        cur_map[i*outerror_size_j+j+k*outerror_size_j*outerror_size_i].push_back(temp_address);
                    }

                }
                }
                address->push_back(temp_address);
            }
        }
    trig_sig->decDep();
    }
};


