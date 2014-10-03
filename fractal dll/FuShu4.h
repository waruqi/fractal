#include <windows.h>
#include "math.h"

#define PI 3.14159265358979//圆周率
#define E 2.71828182845904//自然对数

#define LN(a) (log(a)/log(E))

//复数类
class FuShu4  
{
public:
	long double w;//w+xi+yj+zk
	long double x;
	long double y;
	long double z;
public:
	friend FuShu4 operator+(long double m,FuShu4 f);
	friend FuShu4 operator-(long double m,FuShu4 f);
	friend FuShu4 operator*(long double m,FuShu4 f);
	friend FuShu4 operator/(long double m,FuShu4 f);

	FuShu4 operator++();//前缀自增
	FuShu4 operator++(int);//后缀自增

	FuShu4 operator=(long double m);
	FuShu4 operator*(long double m);
	FuShu4 operator-(long double m);
	FuShu4 operator+(long double m);
	FuShu4 operator/(long double m);

	//FuShu4 operator=(FuShu4 f);
	FuShu4 operator*(FuShu4 f);
	FuShu4 operator-(FuShu4 f);
	FuShu4 operator+(FuShu4 f);
	FuShu4 operator/(FuShu4 f);
	
public:
	FuShu4();
	FuShu4(long double m);
	FuShu4(long double m,long double a,long double b,long double c);
	virtual ~FuShu4();

public:
	long double m();//复数模
	long double m2();//复数模的平方
	void Get(long double m,long double a,long double b,long double c);
	void Get(long double m);
	void Get(FuShu4 f);

};

FuShu4 cos(FuShu4 f);
FuShu4 sin(FuShu4 f);
FuShu4 tan(FuShu4 f);
FuShu4 cot(FuShu4 f);
FuShu4 cosh(FuShu4 f);
FuShu4 sinh(FuShu4 f);
FuShu4 tanh(FuShu4 f);
FuShu4 coth(FuShu4 f);
FuShu4 exp(FuShu4 f);
FuShu4 ln(FuShu4 f);
FuShu4 log(FuShu4 f);//以十为底

FuShu4 csc(FuShu4 f);
FuShu4 sec(FuShu4 f);
FuShu4 csch(FuShu4 f);
FuShu4 sech(FuShu4 f);

FuShu4 sqrt(FuShu4 f);
FuShu4 asin(FuShu4 f);
FuShu4 acos(FuShu4 f);
FuShu4 atan(FuShu4 f);

FuShu4 pow(FuShu4 f,long double n);//复数n次方幂
FuShu4 pow(FuShu4 f,FuShu4 n);//复数n次方幂