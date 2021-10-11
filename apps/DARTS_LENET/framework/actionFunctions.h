#ifndef __ACTION_FCN_GUARD_H__
#define __ACTION_FCN_GUARD_H__

double relu(double x)
{
	return x*(x>0);
}

double drelu(double x);

void softmax(double *input,double *output,int n);

double sigmoid(double x);

double dsig(double x);

#endif /* __ACTION_FCN_GUARD_H__ */
