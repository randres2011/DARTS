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


#pragma once
#ifndef MATRIX_H
#define	MATRIX_H
#include "Atomics.h"
#include <stdint.h>
#include <cstring>
#include <assert.h>

#ifdef MKL
#include <mkl_cblas.h>
#define INT MKL_INT
#define DGEMM(i, j, k, alpha, aPtr, aSize, bPtr, bSize, beta, cPtr, cSize) cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, i, j, k, alpha, aPtr, aSize, bPtr, bSize, beta, cPtr, cSize)
#define DGEMM_COLMAJOR(i, j, k, alpha, aPtr, aSize, bPtr, bSize, beta, cPtr, cSize) cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, i, j, k, alpha, aPtr, aSize, bPtr, bSize, beta, cPtr, cSize)
#define DGEMV(m, n, alpha, aPtr, aSize, xPtr, incx, beta, yPtr, incy) cblas_dgemm(CblasRowMajor, CblasNoTrans, m, n, alpha, aPtr, aSize, xPtr, incx, beta, yPtr, incy)
#define DDOT(size_vector, xPtr, incx, yPtr, incy) cblas_ddot(size_vector, xPtr, incx, yPtr, incy)
#else
//#include "/opt/shared/ACML/5.3.0/gfortran64_fma4/include/acml.h"
#define INT int
#define DGEMM(i, j, k, alpha, aPtr, aSize, bPtr, bSize, beta, cPtr, cSize) dgemm('T', 'T', i, j, k, alpha, aPtr, aSize, bPtr, bSize, beta, cPtr, cSize)
#endif

class matrix
{
private:
    double * ptr;
    INT M; //Columns
    INT N; //Row
    bool save;
public:
    matrix(void):
    ptr(0),
    M(0),
    N(0),
    save(true){ }
    
    matrix(INT m, INT n, double * pt):
    ptr(pt),
    M(m),
    N(n),
    save(true) { }
    
    matrix(INT m, INT n, bool set = true):
    M(m),
    N(n),
    save(false)
    {
        ptr = new double[M*N];
    if(set)
    memset(ptr,0,sizeof(double)*M*N);
    else
    ptr[0]=0;
    }
    
    matrix(matrix * source, int x, int y,INT m, INT n):
    M(m),
    N(n),
    save(false)
    {
        ptr = new double[M*N];
        
        double * sPtr = source->getPtr();
        INT sN = source->getN();
        
        for(int i=0;i<M;i++)
        {
            memcpy(&ptr[i*N],&sPtr[(i+x)*sN + y],sizeof(double)*N);       
        }
    }
    
    inline double * getPtr() { return ptr; }
    inline INT getM() { return M; }
    inline INT getN() { return N; }
    
    void resetMatrix(void)
    {
        memset(ptr,0,sizeof(double)*M*N);
    }

    void Resize(int m, int n,int p)
    {
        if(ptr!=0)
        {
            std::cout<<"Matrix already be initialized.\n";

        }
        delete[] ptr;
        ptr=new double[m*n];
        memset(ptr,0,sizeof(double)*m*n);
        M=m;N=n;
        for(int i = 0; i < M; i++)
        {
            for(int j = 0; j < N; j++)
            {
                //Aptr[i*aN + j] = rand()%100;
                ptr[i*N + j] = (i*N+j)*p;
            }
        }
    }
    void printMatrix(void)
    {
        for(int i = 0; i < M; i++)
        {
            for(int j = 0; j < N; j++)
            {
                std::cout << ptr[i*N + j] << " ";
            }
            std::cout << "\n" << std::endl;
        }
    }
    // i = col; j = row
    double GetElement(int i,int j)
    {
        return ptr[i*N+j];
    }

    // TODO Change the name of this method 
    // it is confusing with the next method
    // this should be setAllElements, the other one 
    // should be setSingleElement
    void SetElement(matrix * source, int x, int y)
    {
        double * sPtr = source->getPtr();
        int sN = source->getN();
        
        for(int i=0;i<M;i++)
        {
            memcpy(&ptr[i*N],&sPtr[(i+x)*sN + y],sizeof(double)*N);       
        }
    }

    void SetElement(int i, int j, double element)
    {
        int index=i*N+j;

        if(index>=M*N)
        {
            std::cout<<"i="<<i<<" j="<<j<<" M="<<M<<" N="<<N<<std::endl;
            return;
        }
        ptr[i*N+j] = element;	 
    }

    void AddElement(int i,int j,double element)
    {
        int index=1*N+j;	
        if(index>=M*N)
        {
            std::cout<<"i="<<i<<" j="<<j<<" M="<<M<<" N="<<N<<std::endl;
            return;
        }

        ptr[i*N+j] += element;
    }

    ~matrix(void)
    {
        if(ptr != NULL)
            delete [] ptr;
    }
};

struct mmArgs
{
    matrix * a;
    matrix * b;
    matrix * c;
    
    int input_size_x;
    int input_size_y;
    int kernel_size;
    int stride;
    
    int output_size_x;
    int output_size_y;

    int xCut;
    int yCut;
    
    int xTile;
    int yTile;

    int xMod;
    int yMod;
    
    mmArgs(matrix * A, matrix * B, matrix * C, int MSI, int MSJ, int MSK, int STRIDE, int XC, int YC):
    a(A), b(B), c(C),
    input_size_x(MSI), input_size_y(MSJ), kernel_size(MSK), stride(STRIDE),
    output_size_x((input_size_x - kernel_size) / stride + 1),output_size_y((input_size_y - kernel_size) / stride + 1),
    xCut(XC), yCut(YC),
    xTile(output_size_x/xCut), yTile(output_size_y/yCut),
    xMod(output_size_x%xCut), yMod(output_size_y%yCut)
    {//std::cout<<"args initialized.\n";
    }    
};


void multiply(matrix * c, matrix * a, matrix * b);
bool compare(matrix * a, matrix * b);
void copyMatrix(matrix * dest, matrix * source, int x, int y);
void acumulateMatrix(matrix * dest, matrix * a);
double DotProduct(matrix *a,matrix*b);

void initRandomMatrix(matrix * A);
void initOneMatrix(matrix * A);
void initCountMatrix(matrix * A);
void initIdMatrix(matrix * A);


#endif	/* MATRIX_H */

