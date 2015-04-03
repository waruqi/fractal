#include "iostream.h"
#include "ctype.h"
#include "stdio.h"
#include "math.h"
#include "stdlib.h"
#include "stack"
#include "vector"
#include "string"
#include "windows.h"

using std::string;
using std::stack;
using std::vector;


#define E 2.71828182845904//自然对数
#define PI 3.14159265358979//圆周率

//属性(数，运算符)
#define ATTR_NUM 1//树值
#define ATTR_SIGN 2//运算符
#define ATTR_VAL 3//变量

//优先级(运算符)
#define PRIORITY_ADD 1//四则运算符
#define PRIORITY_SUB 1
#define PRIORITY_MUL 2
#define PRIORITY_DIV 2

//三角函数
#define PRIORITY_SIN 2
#define PRIORITY_COS 2
#define PRIORITY_SINH 2
#define PRIORITY_COSH 2
#define PRIORITY_TAN 2
#define PRIORITY_TANH 2
#define PRIORITY_COT 2
#define PRIORITY_COTH 2
#define PRIORITY_SEC 2
#define PRIORITY_CSC 2
#define PRIORITY_SECH 2
#define PRIORITY_CSCH 2
#define PRIORITY_EXP 2

#define PRIORITY_ASIN 2
#define PRIORITY_ACOS 2
#define PRIORITY_ATAN 2
#define PRIORITY_ACOT 2

#define PRIORITY_LN 2//自然对数
#define PRIORITY_LOG 2//对数运算
#define PRIORITY_MI 3//幂运算
#define PRIORITY_LEFT 10//左括号
#define PRIORITY_RIGHT 10//右括号

#define PRIORITY_FU 11//负号
#define PRIORITY_ZHENG 11//正号

//结合顺序(运算符)
#define COMBINE_LEFT 1//左结合
#define COMBINE_RIGHT 2//右结合

//公式项
template<class T>
struct TOKEN
{
    int attr;//属性(数，运算符)
    struct 
    {
        int priority;//优先级(运算符)
        int count;//count元运算符
        int combine;//结合顺序
    }sign;
    T num;//数值
    string str;//公式每项字符串
};

//分解公式,gongshi:字符串公式 str_array:分解后的公式 vals:指定变量字符串
template<class T>
int fenjie(char *gongshi,vector<TOKEN<T> > &str_array,vector<TOKEN<T> > &vals)
{
    char *p=gongshi;
    string s,str=gongshi;
    int t=0;
    if(t=str.find_first_of('='))p+=(t+1);

    if(*p=='+')//若开头就为+则为正号
    {
        TOKEN<T> token;
        token.str="+";
        token.attr=ATTR_SIGN;
        token.sign.priority=PRIORITY_ZHENG;
        token.sign.count=1;
        token.sign.combine=COMBINE_LEFT;
        str_array.push_back(token);
        p++;
    }
    if(*p=='-')//若开头就为-则为负号
    {
        TOKEN<T> token;
        token.str="-";
        token.attr=ATTR_SIGN;
        token.sign.priority=PRIORITY_FU;
        token.sign.count=1;
        token.sign.combine=COMBINE_LEFT;
        str_array.push_back(token);
        p++;
    }
    while(*p)
    {
        TOKEN<T> token;
        if(*p=='+')
        {
            if(*(p+1)=='-'||*(p+1)=='+')//若后面还是+或-则为正号
            {
                token.str="+";
                token.attr=ATTR_SIGN;
                token.sign.priority=PRIORITY_ZHENG;
                token.sign.count=1;
                token.sign.combine=COMBINE_LEFT;
                str_array.push_back(token);
                p++;
                continue;
            }
            int b=0;
            for(int i=0;i<vals.size();i++)//判断后一个字符串是否为变量
            {
                if(*(p+1+vals[i].str.length()-1))
                {
                    s.assign(p+1,vals[i].str.length());
                    if(s.compare(vals[i].str)==0)
                    {
                        b=1;
                        break;
                    }
                }
                
            }
            if(isdigit(*(p+1))||b)//若后一个字符串是变量或数字
            {
                b=0;
                for(int i=0;i<vals.size();i++)//判断前一个字串是否为变量
                {
                    if(*(p+vals[i].str.length()-2))
                    {
                        s.assign(p-1,vals[i].str.length());
                        if(s.compare(vals[i].str)==0)
                        {
                            b=1;
                            break;
                        }
                    }
                
                }
                //若后一个字符串是变量或数字且前一个字串不为变量和数字和右括号，则为正号
                if(!isdigit(*(p-1))&&!b&&*(p-1)!=')')
                {
                    token.str="+";
                    token.attr=ATTR_SIGN;
                    token.sign.priority=PRIORITY_ZHENG;
                    token.sign.count=1;
                    token.sign.combine=COMBINE_LEFT;
                    str_array.push_back(token);
                    p++;
                }
                else//否则为加号
                {
                    token.str="+";
                    token.attr=ATTR_SIGN;
                    token.sign.priority=PRIORITY_ADD;
                    token.sign.count=2;
                    token.sign.combine=COMBINE_RIGHT;
                    str_array.push_back(token);
                    p++;

                }
            
            }
            else
            {
                token.str="+";
                token.attr=ATTR_SIGN;
                token.sign.priority=PRIORITY_ADD;
                token.sign.count=2;
                token.sign.combine=COMBINE_RIGHT;
                str_array.push_back(token);
                p++;
            }
            continue;
        }
        if(*p=='-')
        {
            if(*(p+1)=='-'||*(p+1)=='+')//若后面还是+或-则为负号
            {
                token.str="-";
                token.attr=ATTR_SIGN;
                token.sign.priority=PRIORITY_FU;
                token.sign.count=1;
                token.sign.combine=COMBINE_LEFT;
                str_array.push_back(token);
                p++;
                continue;
            }
            int b=0;
            for(int i=0;i<vals.size();i++)//判断后一个字符串是否为变量
            {
                if(*(p+1+vals[i].str.length()-1))
                {
                    s.assign(p+1,vals[i].str.length());
                    if(s.compare(vals[i].str)==0)
                    {
                        b=1;
                        break;
                    }
                }
                
            }
            if(isdigit(*(p+1))||b)//若后一个字符串是变量或数字
            {
                b=0;
                for(int i=0;i<vals.size();i++)//判断前一个字串是否为变量
                {
                    if(*(p+vals[i].str.length()-2))
                    {
                        s.assign(p-1,vals[i].str.length());
                        if(s.compare(vals[i].str)==0)
                        {
                            b=1;
                            break;
                        }
                    }
                
                }
                //若后一个字符串是变量或数字且前一个字串不为变量和数字和右括号，则为负号
                if(!isdigit(*(p-1))&&!b&&*(p-1)!=')')
                {
                    token.str="-";
                    token.attr=ATTR_SIGN;
                    token.sign.priority=PRIORITY_FU;
                    token.sign.count=1;
                    token.sign.combine=COMBINE_LEFT;
                    str_array.push_back(token);
                    p++;
                }
                else//否则为减号
                {
                    token.str="-";
                    token.attr=ATTR_SIGN;
                    token.sign.priority=PRIORITY_SUB;
                    token.sign.count=2;
                    token.sign.combine=COMBINE_RIGHT;
                    str_array.push_back(token);
                    p++;

                }
            
            }
            else
            {
                token.str="-";
                token.attr=ATTR_SIGN;
                token.sign.priority=PRIORITY_SUB;
                token.sign.count=2;
                token.sign.combine=COMBINE_RIGHT;
                str_array.push_back(token);
                p++;
            }
            continue;
            
        }
        if(*p=='*')
        {
            token.str="*";
            token.attr=ATTR_SIGN;
            token.sign.priority=PRIORITY_MUL;
            token.sign.count=2;
            token.sign.combine=COMBINE_RIGHT;
            str_array.push_back(token);
            p++;
            continue;
            
        }
        if(*p=='/')
        {
            token.str="/";
            token.attr=ATTR_SIGN;
            token.sign.priority=PRIORITY_DIV;
            token.sign.count=2;
            token.sign.combine=COMBINE_RIGHT;
            str_array.push_back(token);
            p++;
            continue;
            
        }
        if(*p=='^')
        {
            token.str="^";
            token.attr=ATTR_SIGN;
            token.sign.priority=PRIORITY_MI;
            token.sign.count=2;
            token.sign.combine=COMBINE_LEFT;//左结合
            str_array.push_back(token);
            p++;
            continue;
            
        }
        if(*p=='(')
        {
            token.str="(";
            token.attr=ATTR_SIGN;
            token.sign.priority=PRIORITY_LEFT;
            token.sign.count=0;
            token.sign.combine=COMBINE_LEFT;
            str_array.push_back(token);
            p++;
            continue;
            
        }
        if(*p==')')
        {
            token.str=")";
            token.attr=ATTR_SIGN;
            token.sign.priority=PRIORITY_RIGHT;
            token.sign.count=0;
            token.sign.combine=COMBINE_RIGHT;
            str_array.push_back(token);
            p++;
            continue;
            
        }
        if(*(p+3))
        {
            s.assign(p,4);
            if(s.compare("sinh")==0)
            {
                token.str="sinh";
                token.attr=ATTR_SIGN;
                token.sign.priority=PRIORITY_SINH;
                token.sign.count=1;
                token.sign.combine=COMBINE_LEFT;
                str_array.push_back(token);
                p+=4;
                continue;
                
            }
            else if(s.compare("cosh")==0)
            {
                token.str="cosh";
                token.attr=ATTR_SIGN;
                token.sign.priority=PRIORITY_COSH;
                token.sign.count=1;
                token.sign.combine=COMBINE_LEFT;
                str_array.push_back(token);
                p+=4;
                continue;
                
            }
            else if(s.compare("tanh")==0)
            {
                token.str="tanh";
                token.attr=ATTR_SIGN;
                token.sign.priority=PRIORITY_TANH;
                token.sign.count=1;
                token.sign.combine=COMBINE_LEFT;
                str_array.push_back(token);
                p+=4;
                continue;
                
            }
            else if(s.compare("coth")==0)
            {
                token.str="coth";
                token.attr=ATTR_SIGN;
                token.sign.priority=PRIORITY_COTH;
                token.sign.count=1;
                token.sign.combine=COMBINE_LEFT;
                str_array.push_back(token);
                p+=4;
                continue;
                
            }
            else if(s.compare("sech")==0)
            {
                token.str="sech";
                token.attr=ATTR_SIGN;
                token.sign.priority=PRIORITY_SECH;
                token.sign.count=1;
                token.sign.combine=COMBINE_LEFT;
                str_array.push_back(token);
                p+=4;
                continue;
                
            }
            else if(s.compare("csch")==0)
            {
                token.str="csch";
                token.attr=ATTR_SIGN;
                token.sign.priority=PRIORITY_CSCH;
                token.sign.count=1;
                token.sign.combine=COMBINE_LEFT;
                str_array.push_back(token);
                p+=4;
                continue;
                
            }
            else if(s.compare("asin")==0)
            {
                token.str="asin";
                token.attr=ATTR_SIGN;
                token.sign.priority=PRIORITY_ASIN;
                token.sign.count=1;
                token.sign.combine=COMBINE_LEFT;
                str_array.push_back(token);
                p+=4;
                continue;
                
            }
            else if(s.compare("acos")==0)
            {
                token.str="acos";
                token.attr=ATTR_SIGN;
                token.sign.priority=PRIORITY_ACOS;
                token.sign.count=1;
                token.sign.combine=COMBINE_LEFT;
                str_array.push_back(token);
                p+=4;
                continue;
                
            }
            else if(s.compare("atan")==0)
            {
                token.str="atan";
                token.attr=ATTR_SIGN;
                token.sign.priority=PRIORITY_ATAN;
                token.sign.count=1;
                token.sign.combine=COMBINE_LEFT;
                str_array.push_back(token);
                p+=4;
                continue;
                
            }
            else if(s.compare("acot")==0)
            {
                token.str="acot";
                token.attr=ATTR_SIGN;
                token.sign.priority=PRIORITY_ACOT;
                token.sign.count=1;
                token.sign.combine=COMBINE_LEFT;
                str_array.push_back(token);
                p+=4;
                continue;
                
            }
            else ;
            
        }
        if(*(p+2))
        {
            s.assign(p,3);
            if(s.compare("sin")==0)
            {
                token.str="sin";
                token.attr=ATTR_SIGN;
                token.sign.priority=PRIORITY_SIN;
                token.sign.count=1;
                token.sign.combine=COMBINE_LEFT;
                str_array.push_back(token);
                p+=3;
                continue;
                
            }
            else if(s.compare("cos")==0)
            {
                token.str="cos";
                token.attr=ATTR_SIGN;
                token.sign.priority=PRIORITY_COS;
                token.sign.count=1;
                token.sign.combine=COMBINE_LEFT;
                str_array.push_back(token);
                p+=3;
                continue;
                
            }
            else if(s.compare("tan")==0)
            {
                token.str="tan";
                token.attr=ATTR_SIGN;
                token.sign.priority=PRIORITY_TAN;
                token.sign.count=1;
                token.sign.combine=COMBINE_LEFT;
                str_array.push_back(token);
                p+=3;
                continue;
                
            }
            else if(s.compare("cot")==0)
            {
                token.str="cot";
                token.attr=ATTR_SIGN;
                token.sign.priority=PRIORITY_COT;
                token.sign.count=1;
                token.sign.combine=COMBINE_LEFT;
                str_array.push_back(token);
                p+=3;
                continue;
                
            }
            else if(s.compare("sec")==0)
            {
                token.str="sec";
                token.attr=ATTR_SIGN;
                token.sign.priority=PRIORITY_SEC;
                token.sign.count=1;
                token.sign.combine=COMBINE_LEFT;
                str_array.push_back(token);
                p+=3;
                continue;
                
            }
            else if(s.compare("csc")==0)
            {
                token.str="csc";
                token.attr=ATTR_SIGN;
                token.sign.priority=PRIORITY_CSC;
                token.sign.count=1;
                token.sign.combine=COMBINE_LEFT;
                str_array.push_back(token);
                p+=3;
                continue;
                
            }
            else if(s.compare("log")==0)
            {
                token.str="log";
                token.attr=ATTR_SIGN;
                token.sign.priority=PRIORITY_LOG;
                token.sign.count=1;
                token.sign.combine=COMBINE_LEFT;
                str_array.push_back(token);
                p+=3;
                continue;
                
            }
            else if(s.compare("exp")==0)
            {
                token.str="exp";
                token.attr=ATTR_SIGN;
                token.sign.priority=PRIORITY_EXP;
                token.sign.count=1;
                token.sign.combine=COMBINE_LEFT;
                str_array.push_back(token);
                p+=3;
                continue;
                
            }
            else ;
            
        }
        if(*(p+1))
        {
            s.assign(p,2);
            if(s.compare("ln")==0)
            {
                token.str="ln";
                token.attr=ATTR_SIGN;
                token.sign.priority=PRIORITY_LN;
                token.sign.count=1;
                token.sign.combine=COMBINE_LEFT;
                str_array.push_back(token);
                p+=2;
                continue;
                
            }
            else ;
            
        }
        if(isdigit(*p))//若为数值
        {
            string str_num;
            while(1)
            {
                str_num+=*p;
                p++;
                if(!isdigit(*p)&&*p!='.')break;
                
            }
            char *st;
            token.str=str_num;
            token.attr=ATTR_NUM;
            token.num=(double)strtod(str_num.c_str(),&st);//将字符串转为double值
            str_array.push_back(token);
            continue;
        }
        if(isalpha(*p))//若为变量字符串
        {
            for(int i=0;i<vals.size();i++)
            {
                if(*(p+vals[i].str.length()-1))
                {
                    s.assign(p,vals[i].str.length());
                    if(s.compare(vals[i].str)==0)
                    {
                        token.str=vals[i].str;
                        token.attr=ATTR_VAL;
                        vals[i].attr=ATTR_VAL;//为以后计算做优化
                        token.num=0;
                        str_array.push_back(token);
                        p+=vals[i].str.length();
                        break;
                    }
                }
                
            }
            continue;
        }
        if(isspace(*p))//若为空格
        {
            p++;
            continue;
        }
        return 0;//没有匹配的项,返回失败
        
 }
 if(str_array.size()==0)return 0;//返回失败
 
 return 1;
}
//转为后缀表达式 str_array:分解后的公式,并返回转换后的式子
template<class T>
int after_gongshi(vector<TOKEN<T> > &str_array)
{
    stack<TOKEN<T> > str_stack;//堆栈
    vector<TOKEN<T> > temp_array;//临时公式数组
    int i;
    TOKEN<T> token;
    for(i=0;i<str_array.size();i++)
    {
        
        if(str_array[i].attr==ATTR_SIGN)//若为运算符
        {
            if(str_array[i].str.compare("(")==0)//若为左括号直接压入
            {
                str_stack.push(str_array[i]);
                continue;
            }
            if(str_array[i].str.compare(")")==0)//若右括号，弹出左括号后面的所有运算符
            {
                
                while(1)
                {
                    if(str_stack.size()>0)
                    {
                        token=str_stack.top();
                        if(token.str.compare("(")==0)
                        {
                            str_stack.pop();
                            break;
                        }
                        else
                        {
                            temp_array.push_back(token);
                            str_stack.pop();
                            
                        }
                    }
                    else break;
                    
                }
                continue;
            }
            if(str_stack.size()==0)//若堆栈为空，直接压入
            {
                str_stack.push(str_array[i]);
                continue;
            }
            while(str_stack.size()>0)//若堆栈不为空
            {
                token=str_stack.top();
                if(token.str.compare("(")!=0)//且堆栈顶不为左括号
                {
                    //根据优先级判断是否入栈
                    if(str_array[i].sign.priority>token.sign.priority)//若大于堆栈顶运算符优先级，直接压入
                    {
                        str_stack.push(str_array[i]);
                        break;
                        
                    }
                    else if(str_array[i].sign.priority<token.sign.priority)//若小于堆栈顶运算符优先级
                    {
                        //弹出所有优先级比它大的运算符
                        temp_array.push_back(token);
                        str_stack.pop();
                        
                    }
                    else//若优先级相同，则根据结合方向判断
                    {
                        if(str_array[i].sign.combine==COMBINE_RIGHT)//若为右结合
                        {
                            temp_array.push_back(token);//弹出堆栈
                            str_stack.pop();
                        }
                        else if(str_array[i].sign.combine==COMBINE_LEFT)//若为左结合
                        {
                            str_stack.push(str_array[i]);//继续压入堆栈
                            break;
                        
                        }
                        else return 0;
                    }
                }
                else //若堆栈顶为左括号，直接压入
                {
                    str_stack.push(str_array[i]);
                    break;
                    
                }
            }
            if(str_stack.size()==0)//弹出完所有优先级比它大的运算符后，若堆栈恰为空则将其压入
            {
                str_stack.push(str_array[i]);
            }
            continue;
            
        }
        else if(str_array[i].attr==ATTR_NUM)//若为数值，直接压入
        {
            temp_array.push_back(str_array[i]);
            
        }
        else if(str_array[i].attr==ATTR_VAL)//若为变量，直接压入
        {
            temp_array.push_back(str_array[i]);
        }
        else return 0;
        
    }
    if(i==str_array.size())//遍历完后，弹出所有
    {
        while(1)
        {
            if(str_stack.size()>0)
            {
                token=str_stack.top();
                if(token.str.compare("(")==0)
                {
                    str_stack.pop();
                }
                else
                {
                    temp_array.push_back(token);
                    str_stack.pop();
                    
                }
            }
            else break;
            
        }
        
    }
    //返回数组
    str_array.empty();
    str_array.resize(0);
    for(i=0;i<temp_array.size();i++)
    {
        str_array.push_back(temp_array[i]);

    }
    return 1;
}
//将变量数组vals中的值来替换公式，并将公式返回
template<class T>
int fuzhi_gongshi(vector<TOKEN<T> > &str_array,vector<TOKEN<T> > vals)
{
    int i,j;
    for(i=0;i<str_array.size();i++)
    {
        if(str_array[i].attr==ATTR_VAL)
        {
            for(j=0;j<vals.size();j++)
            {
                if(vals[j].str.compare(str_array[i].str)==0)
                {
                    str_array[i].num=vals[j].num;
                    break;
                }
            
            }
        }
    }
    return 1;
}

//根据str_array堆栈公式计算值,根据变量数组vals中的值来算,result为返回值
template<class T>
int compute_gongshi(vector<TOKEN<T> > &str_array,T &result)
{
    stack<TOKEN<T> > str_stack;//临时堆栈
    int i=0;
    for(i=0;i<str_array.size();i++)
    {
        if(str_array[i].attr==ATTR_NUM)
        {
            str_stack.push(str_array[i]);
        
        }
        else if(str_array[i].attr==ATTR_VAL)
        {
            str_stack.push(str_array[i]);
        
        }
        else if(str_array[i].attr==ATTR_SIGN)
        {
            if(str_array[i].sign.count==2)
            {
                TOKEN<T> num1,num2,num;
                if(str_stack.size()>=2)//弹出两数
                {
                    num1=str_stack.top();
                    str_stack.pop();
                    num2=str_stack.top();
                    str_stack.pop();

                    num.attr=ATTR_NUM;
                    try//捕获运算异常
                    {
                        if(str_array[i].str=="+")
                            num.num=num2.num+num1.num;
                        else if(str_array[i].str=="-")
                            num.num=num2.num-num1.num;
                        else if(str_array[i].str=="*")
                            num.num=num2.num*num1.num;
                        else if(str_array[i].str=="/")
                            num.num=num2.num/num1.num;
                        else if(str_array[i].str=="^")
                            num.num=pow(num2.num,num1.num);
                        else return 0;
                        
                    }
                    catch (...)
                    {
                        return 0;
                    }
                    str_stack.push(num);
                    continue;
                }
                else if(str_stack.size()==1)
                {
                    continue;
                }
                else return 0;

            }
            else if(str_array[i].sign.count==1)
            {
                TOKEN<T> num;
                if(str_stack.size()>=1)
                {
                    num=str_stack.top();
                    str_stack.pop();
                }
                else return 0;
                num.attr=ATTR_NUM;
                try
                {
                    if(str_array[i].str=="+")//正号
                        num.num=num.num;
                    else if(str_array[i].str=="-")//负号
                        num.num=-1*num.num;
                    else if(str_array[i].str=="sin")
                        num.num=sin(num.num);
                    else if(str_array[i].str=="cos")
                        num.num=cos(num.num);
                    else if(str_array[i].str=="tan")
                        num.num=sin(num.num)/cos(num.num);
                    else if(str_array[i].str=="cot")
                        num.num=cos(num.num)/sin(num.num);
                    /*else if(str_array[i].str=="asin")
                        num.num=asin(num.num);
                    else if(str_array[i].str=="acos")
                        num.num=acos(num.num);
                    else if(str_array[i].str=="atan")
                        num.num=atan(num.num);
                    else if(str_array[i].str=="acot")
                        num.num=acot(num.num);*/
                    else if(str_array[i].str=="sec")
                        num.num=1/cos(num.num);
                    else if(str_array[i].str=="csc")
                        num.num=1/sin(num.num);
                    else if(str_array[i].str=="sinh")
                        num.num=sinh(num.num);
                    else if(str_array[i].str=="cosh")
                        num.num=cosh(num.num);
                    else if(str_array[i].str=="tanh")
                        num.num=tanh(num.num);
                    else if(str_array[i].str=="coth")
                        num.num=1/tanh(num.num);
                    else if(str_array[i].str=="sech")
                        num.num=1/cosh(num.num);
                    else if(str_array[i].str=="csch")
                        num.num=1/sinh(num.num);
                    else if(str_array[i].str=="log")
                        num.num=log(num.num)/log(10);
                    else if(str_array[i].str=="ln")
                        num.num=log(num.num)/log(E);
                    else if(str_array[i].str=="exp")
                        num.num=exp(num.num);
                    else return 0;
                        
                }
                catch (...)
                {
                    return 0;
                }
                    
                str_stack.push(num);
                continue;
            
            }
            else return 0;
        
        }
        else return 0;
    
    }
    if(str_stack.size()==1)
    {
        result=str_stack.top().num;
        str_stack.pop();
    }
    else return 0;

    return 1;

}
//公式转换
template<class T>
int GongShi_Change(char *gongshi,vector<TOKEN<T> > &vals,vector<TOKEN<T> > &str_array)
{
    if(!fenjie(gongshi,str_array,vals))
        return 0;
    if(!after_gongshi(str_array))
        return 0;

    return 1;
}
//计算公式
template<class T>
int GongShi_Compute(vector<TOKEN<T> > &str_array,vector<TOKEN<T> > vals,T &result)
{
    /*if(!fuzhi_gongshi(str_array,vals))
        return 0;
    if(!compute_gongshi(str_array,result))
        return 0;*/
    //公式变量赋值
    int i,j;
    //公式计算
    stack<TOKEN<T> > str_stack;//临时堆栈
    for(i=0;i<str_array.size();i++)
    {
        if(str_array[i].attr==ATTR_NUM)
        {
            str_stack.push(str_array[i]);
            
        }
        else if(str_array[i].attr==ATTR_VAL)
        {
            //公式变量赋值
            for(j=0;j<vals.size();j++)
            {
                if(vals[j].str.compare(str_array[i].str)==0)
                {
                    str_stack.push(vals[j]);//优化，直接压入该变量,省去变量赋值
                    //str_array[i].num=vals[j].num;
                    break;
                }
            
            }
            //str_stack.push(str_array[i]);
        }
        else if(str_array[i].attr==ATTR_SIGN)
        {
            if(str_array[i].sign.count==2)
            {
                TOKEN<T> num1,num2,num;
                if(str_stack.size()>=2)//弹出两数
                {
                    num1=str_stack.top();
                    str_stack.pop();
                    num2=str_stack.top();
                    str_stack.pop();

                    num.attr=ATTR_NUM;
            
                    if(str_array[i].str=="+")
                        num.num=num2.num+num1.num;
                    else if(str_array[i].str=="-")
                        num.num=num2.num-num1.num;
                    else if(str_array[i].str=="*")
                        num.num=num2.num*num1.num;
                    else if(str_array[i].str=="/")
                        num.num=num2.num/num1.num;
                    else if(str_array[i].str=="^")
                        num.num=pow(num2.num,num1.num);
                    else return 0;
        
                    str_stack.push(num);
                    continue;
                }
                else if(str_stack.size()==1)
                {
                    continue;
                }
                else return 0;

            }
            else if(str_array[i].sign.count==1)
            {
                TOKEN<T> num;
                if(str_stack.size()>=1)
                {
                    num=str_stack.top();
                    str_stack.pop();
                }
                else return 0;
                num.attr=ATTR_NUM;
            
                if(str_array[i].str=="+")//正号
                    num.num=num.num;
                else if(str_array[i].str=="-")//负号
                    num.num=-1*num.num;
                else if(str_array[i].str=="sin")
                    num.num=sin(num.num);
                else if(str_array[i].str=="cos")
                    num.num=cos(num.num);
                else if(str_array[i].str=="tan")
                    num.num=sin(num.num)/cos(num.num);
                else if(str_array[i].str=="cot")
                    num.num=cos(num.num)/sin(num.num);
                else if(str_array[i].str=="sec")
                    num.num=1/cos(num.num);
                else if(str_array[i].str=="csc")
                    num.num=1/sin(num.num);
                else if(str_array[i].str=="sinh")
                    num.num=sinh(num.num);
                else if(str_array[i].str=="cosh")
                    num.num=cosh(num.num);
                else if(str_array[i].str=="tanh")
                    num.num=tanh(num.num);
                else if(str_array[i].str=="coth")
                    num.num=1/tanh(num.num);
                else if(str_array[i].str=="sech")
                    num.num=1/cosh(num.num);
                else if(str_array[i].str=="csch")
                    num.num=1/sinh(num.num);
                else if(str_array[i].str=="log")
                    num.num=log(num.num)/log(10);
                else if(str_array[i].str=="ln")
                    num.num=log(num.num)/log(E);
                else if(str_array[i].str=="exp")
                    num.num=exp(num.num);
                else return 0;
            
                str_stack.push(num);
                continue;
            
            }
            else return 0;
        
        }
        else return 0;
    
    }
    if(str_stack.size()==1)
    {
        result=str_stack.top().num;
        //str_stack.pop();
    }
    else return 0;

    return 1;
}
