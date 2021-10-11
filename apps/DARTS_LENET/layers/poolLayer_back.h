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

class Poolback_cd : public Codelet
{
public:
    matrix *input,*inerror,*outerror;
    int osx,osy,ks;
    std::vector<Codelet*> *toSig;

    Poolback_cd(uint32_t dep, uint32_t res, ThreadedProcedure * myTP, uint32_t stat,
            matrix *input_,matrix *inerror_,matrix *outerror_,int osx_,int osy_,int ks_,
            std::vector<Codelet*> *toSig_):
        Codelet(dep,res,myTP,stat),input(input_),inerror(inerror_),outerror(outerror),
        osx(osx_),osy(osy_),ks(ks_),toSig(toSig_){}

    virtual void fire(void);
};

void Poolback_cd::fire(void)
{
    int i,j,ii,jj,max,site1,site2,k;
    for(i=0;i<osx;i++)
    {
        for(j=0;j<osy;j++)
        {
            max=-214748364;
            for(ii=0;ii<ks;ii++)
            {
                for(jj=0;jj<ks;jj++)
                {
                    if(max<input->GetElement(i*ks+ii,j*ks+jj))
                    {
                        max=input->GetElement(i*ks+ii,j*ks+jj);
                        site1=i*ks+ii;site2=j*ks+jj;
                    }
                }
            }
            inerror->SetElement(site1,site2,outerror->GetElement(i,j));
            for(ii=0;ii<ks;ii++)
            {
                for(jj=0;jj<ks;jj++)
                {
                    for(k=0;k<toSig[(i*ks+ii)*osy*ks+j*ks+jj].size();k++)
                        toSig[(i*ks+ii)*osy*ks+j*ks+jj][k]->decDep();
                }
            }
        }
    }
    resetCodelet();
}

class Poolback : public ThreadedProcedure
{
public:
    Poolback(matrix *input,matrix *inerror,matrix *outerror,int ch,
            std::vector<Codelet*> *address,std::vector<Codelet*> *next_map,
            std::vector<Codelet*> *cur_map,Codelet *trig_sig)
    {
            incRef();
            Codelet *temp;
            int osx=outerror->getM();
            int osy=outerror->getN();
            int ks=inerror->getM()/osx;
            for(int i=0;i<ch;i++)
            {
                temp=new Poolback_cd(osx*osy+1,osx*osy,this,SHORTWAIT,&input[i],
                        &inerror[i],&outerror[i],osx,osy,ks,next_map);
                address->push_back(temp);
                for(int j=0;j<osx*osy;j++)
                {
                    cur_map[i*osx*osy+j].push_back(temp);
                }
            }
            trig_sig->decDep();
    }
};

