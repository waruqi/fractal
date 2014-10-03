#include <windows.h>
#include "math.h"

#define PI 3.14159265358979//圆周率
#define E 2.71828182845904//自然对数

#define LN(a) (log(a)/log(E))

//复数类
class FuShu  
{
public:
	long double b;//a+bi
	long double a;
public:
	friend FuShu operator+(long double m,FuShu f);
	friend FuShu operator-(long double m,FuShu f);
	friend FuShu operator*(long double m,FuShu f);
	friend FuShu operator/(long double m,FuShu f);

	FuShu operator++();//前缀自增
	FuShu operator++(int);//后缀自增

	FuShu operator=(long double m);
	FuShu operator*(long double m);
	FuShu operator-(long double m);
	FuShu operator+(long double m);
	FuShu operator/(long double m);

	//FuShu operator=(FuShu f);
	FuShu operator*(FuShu f);
	FuShu operator-(FuShu f);
	FuShu operator+(FuShu f);
	FuShu operator/(FuShu f);
	
public:
	FuShu();
	FuShu(long double m);
	FuShu(long double m,long double n);
	virtual ~FuShu();

public:
	long double m();//复数模
	long double m2();//复数模的平方
	void Get(long double m,long double n);
	void Get(long double m);
	void Get(FuShu f);

};

FuShu cos(FuShu f);
FuShu sin(FuShu f);
FuShu tan(FuShu f);
FuShu cot(FuShu f);
FuShu cosh(FuShu f);
FuShu sinh(FuShu f);
FuShu tanh(FuShu f);
FuShu coth(FuShu f);
FuShu exp(FuShu f);
FuShu ln(FuShu f);
FuShu log(FuShu f);//以十为底

FuShu csc(FuShu f);
FuShu sec(FuShu f);
FuShu csch(FuShu f);
FuShu sech(FuShu f);

FuShu sqrt(FuShu f);
FuShu asin(FuShu f);
FuShu acos(FuShu f);
FuShu atan(FuShu f);

FuShu pow(FuShu f,long double n);//复数n次方幂
FuShu pow(FuShu f,FuShu n);//复数n次方幂

