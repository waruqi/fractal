#include "FuShu.h"

FuShu::FuShu()
{
	a=0;
	b=0;

}
FuShu::FuShu(long double m)
{
	a=m;
	b=0;
}
FuShu::FuShu(long double m,long double n)
{
	a=m;
	b=n;

}
FuShu::~FuShu()
{

}

void FuShu::Get(long double m,long double n)
{
	a=m;
	b=n;

}
void FuShu::Get(long double m)
{
	a=m;
	b=0;

}
void FuShu::Get(FuShu f)
{
	a=f.a;
	b=f.b;

}
FuShu FuShu::operator +(FuShu f)
{
	FuShu r;
	r.a=a+f.a;
	r.b=b+f.b;

	return r;

}

FuShu FuShu::operator -(FuShu f)
{

	FuShu r;
	r.a=a-f.a;
	r.b=b-f.b;

	return r;

}

FuShu FuShu::operator *(FuShu f)
{
	FuShu r;
	r.a=a*f.a-b*f.b;
	r.b=a*f.b+b*f.a;

	return r;

}

FuShu FuShu::operator /(FuShu f)
{
	FuShu r;
	if(f.a==0&&f.b==0)
	{
		//MessageBox(NULL,"除数为0","错误!",MB_OK);
		r.a=0;
		r.b=0;
		return r;
	}
	
	r.a=(a*f.a+b*f.b)/(f.a*f.a+f.b*f.b);
	r.b=(b*f.a-a*f.b)/(f.a*f.a+f.b*f.b);
	
	/*if(fabs(f.a)>=fabs(f.b))
	{
		if(f.a==0)return r;
		long double dc=f.b/f.a;
		r.a=(a+b*dc)/(f.a+f.b*dc);
		r.b=(b-a*dc)/(f.a+f.b*dc);
	}
	else
	{
		if(f.b==0)return r;
		long double cd=f.a/f.b;
		r.a=(a*cd+b)/(f.a*cd+f.b);
		r.b=(b*cd-a)/(f.a*cd+f.b);
	
	}*/
	
	return r;

}

/*FuShu FuShu::operator =(FuShu f)
{
	FuShu r;
	r.a=f.a;
	r.b=f.b;

	a=f.a;
	b=f.b;

	return r;
}*/
FuShu FuShu::operator =(long double m)
{
	FuShu r;
	r.a=m;
	r.b=0;

	a=m;
	b=0;

	return r;

}
FuShu FuShu::operator +(long double m)
{
	FuShu r;
	r.a=a+m;
	r.b=b;

	return r;

}

FuShu FuShu::operator -(long double m)
{

	FuShu r;
	r.a=a-m;
	r.b=b;

	return r;

}

FuShu FuShu::operator *(long double m)
{
	FuShu r;
	r.a=a*m;
	r.b=b*m;

	return r;

}

FuShu FuShu::operator /(long double m)
{
	FuShu r;
	if(m==0)
	{
		//MessageBox(NULL,"除数为0","错误!",MB_OK);
		r.a=0;
		r.b=0;
		return r;
	}
	
	r.a=a/m;
	r.b=b/m;
	
	return r;

}
//前缀自增
FuShu FuShu::operator ++()
{
	a++;//先自增
	FuShu r;

	r.a=a;//后赋值
	r.b=b;

	return r;

}
//后缀自增
FuShu FuShu::operator ++(int)
{
	FuShu r;
	r.a=a;//先赋值
	r.b=b;

	a++;//后自增
	return r;

}
FuShu operator +(long double m,FuShu f)
{
	FuShu r;
	r.a=m+f.a;
	r.b=f.b;

	return r;

}
FuShu operator -(long double m,FuShu f)
{
	FuShu r;
	r.a=m-f.a;
	r.b=-1*f.b;

	return r;

}
FuShu operator *(long double m,FuShu f)
{
	FuShu r;
	r.a=m*f.a;
	r.b=m*f.b;

	return r;

}
FuShu operator /(long double m,FuShu f)
{
	FuShu r;
	if(f.a==0&&f.b==0)
	{
		//MessageBox(NULL,"除数为0","错误!",MB_OK);
		r.a=0;
		r.b=0;
		return r;
	}
	
	r.a=(f.a*m)/(f.a*f.a+f.b*f.b);
	r.b=-1*(f.b*m)/(f.a*f.a+f.b*f.b);

	return r;

}

FuShu cos(FuShu f)
{
	FuShu r;
	r.a=cos(f.a)*cosh(f.b);
	r.b=-sin(f.a)*sinh(f.b);

	return r;

}
FuShu sin(FuShu f)
{
	FuShu r;
	r.a=sin(f.a)*cosh(f.b);
	r.b=cos(f.a)*sinh(f.b);

	return r;

}
FuShu tan(FuShu f)
{
	FuShu r;
	r=cos(f);
	if(r.a==0&&r.b==0)
	{
		MessageBox(NULL,"tan(z)失败","错误!",MB_OK);
		return r;
	
	}
	else r=sin(f)/r;

	return r;

}
FuShu cot(FuShu f)
{
	FuShu r;
	r=sin(f);
	if(r.a==0&&r.b==0)
	{
		MessageBox(NULL,"cot(z)失败","错误!",MB_OK);
		return r;
	
	}
	else r=cos(f)/r;
	return r;

}
FuShu exp(FuShu f)
{
	FuShu r;
	r.a=exp(f.a)*cos(f.b);
	r.b=exp(f.a)*sin(f.b);

	return r;

}
FuShu pow(FuShu f,long double n)
{
	FuShu r;
	if(f.a==0&&f.b==0)
	{
		r.a=0;
		r.b=0;
		return r;
	
	}
	long double Atan;
	if(f.a==0)
	{
		Atan=PI/2;
	}
	else 
	{
		Atan=atan(f.b/f.a);
	}
	long double Mo2=f.a*f.a+f.b*f.b;
	r.a=cos(n*Atan)*pow(Mo2,0.5*n);
	r.b=sin(n*Atan)*pow(Mo2,0.5*n);
	return r;

}
FuShu pow(FuShu f,FuShu n)
{
	FuShu r;
	if(f.a==0&&f.b==0)
	{
		r.a=0;
		r.b=0;
		return r;
	
	}
	long double Atan;
	if(f.a==0)
	{
		Atan=PI/2;
	}
	else 
	{
		Atan=atan(f.b/f.a);
	}
	long double Mo2=f.a*f.a+f.b*f.b;
	r.a=cos(n.a*Atan+0.5*n.b*LN(Mo2))*pow(Mo2,0.5*n.a)*exp(-n.b*Atan);
	r.b=sin(n.a*Atan+0.5*n.b*LN(Mo2))*pow(Mo2,0.5*n.a)*exp(-n.b*Atan);
	return r;

}
long double FuShu::m2()
{
	long double m=a*a+b*b;
	return m;

}
long double FuShu::m()
{
	long double m=a*a+b*b;
	return sqrt(m);

}
FuShu cosh(FuShu f)
{
	FuShu r;
	r.a=cosh(f.a)*cos(f.b);
	r.b=sinh(f.a)*sin(f.b);

	return r;

}
FuShu sinh(FuShu f)
{
	FuShu r;
	r.a=sinh(f.a)*cos(f.b);
	r.b=cosh(f.a)*sin(f.b);

	return r;

}
FuShu tanh(FuShu f)
{
	FuShu r;
	r=cosh(f);
	if(r.a==0&&r.b==0)
	{
		MessageBox(NULL,"tanh(z)失败","错误!",MB_OK);
		return r;
	
	}
	else r=sinh(f)/r;

	return r;

}
FuShu coth(FuShu f)
{
	FuShu r;
	r=sinh(f);
	if(r.a==0&&r.b==0)
	{
		MessageBox(NULL,"coth(z)失败","错误!",MB_OK);
		return r;
	
	}
	else r=cosh(f)/r;
	return r;

}
FuShu ln(FuShu f)
{
	FuShu r;
	if(f.a==0&&f.b==0)
	{
		r.a=0;
		r.b=0;
		return r;
	
	}
	long double Atan;
	if(f.a==0)
	{
		Atan=PI/2;
	}
	else 
	{
		Atan=atan(f.b/f.a);
	}
	r.a=0.5*LN(f.a*f.a+f.b*f.b);
	r.b=Atan;
	return r;

}
FuShu log(FuShu f)
{
	FuShu r;
	if(f.a==0&&f.b==0)
	{
		r.a=0;
		r.b=0;
		return r;
	
	}
	r=ln(f)/LN(10);
	return r;

}
FuShu sqrt(FuShu f)
{
	FuShu r;
	long double Atan;
	if(f.a==0)
	{
		Atan=PI/2;
	}
	else 
	{
		Atan=atan(f.b/f.a);
	}
	long double Mo2=f.a*f.a+f.b*f.b;
	r.a=cos(0.5*Atan)*pow(Mo2,0.25);
	r.b=sin(0.5*Atan)*pow(Mo2,0.25);
	return r;

}
FuShu atan(FuShu f)
{
	FuShu r;
	FuShu i(0,1);
	r=ln((1+i*f)/(1-i*f))*i*(-0.5);
	return r;

}
FuShu asin(FuShu f)
{
	FuShu r;
	FuShu i(0,1);
	r=-1*i*ln(i*f+sqrt(1-f*f));
	return r;

}
FuShu acos(FuShu f)
{
	FuShu r;
	FuShu i(0,1);
	r=-1*i*ln(f+i*sqrt(1-f*f));
	return r;

}
FuShu csc(FuShu f)
{
	FuShu r;

	return r;

}
FuShu csch(FuShu f)
{
	FuShu r;

	return r;

}
FuShu sec(FuShu f)
{
	FuShu r;

	return r;

}
FuShu sech(FuShu f)
{
	FuShu r;

	return r;

}


