//VC-Win32
#include <windows.h>
#include <windowsx.h>
#include "time.h"

#include "FuShu.h"//复数类
#include "FuShu4.h"//四元数类
#include "GongShi.h"

//资源定义文件
#include "Resource.h"
#include "commctrl.h"//控件头文件
#pragma comment(lib,"comctl32.lib")//加载通用控件库
#pragma comment(lib,"comdlg32.lib")//加

#define WINDOW_CLASS_NAME "Fractal"//窗口类名
#define WINDOW_TITLE "分形画板 1.0"//窗口标题
#define WINDOW_WIDTH 640//窗口尺寸
#define WINDOW_HEIGHT 480

//T0分形类型属性
#define FRACTAL_T0_MANDELBROT	1//Mandelbrot集
#define FRACTAL_T0_JULIA		2//Julia集
#define FRACTAL_T0_NEWTON		4//牛顿分形

//全局变量
HWND          MAIN_HANDLE        =NULL;//保存主窗口句柄
HINSTANCE     MAIN_INSTANCE      =NULL;//保存窗口实例句柄

HWND          MAIN_DIALOG		 =NULL;//主对话框句柄
HWND          hT0		         =NULL;//Tab子对话框句柄
HWND          hT1		         =NULL;//Tab子对话框句柄
HWND          hT2		         =NULL;//Tab子对话框句柄
HWND          hT3		         =NULL;//Tab子对话框句柄
HWND          hT4		         =NULL;//Tab子对话框句柄
HWND          hT10		         =NULL;//Tab子对话框句柄
HDC           hMemDC             =NULL;//兼容DC

HANDLE        hDrawThreadT0      =NULL;//画图线程句柄
HANDLE        hDrawThreadT1      =NULL;//画图线程句柄
HANDLE        hDrawThreadT2      =NULL;//画图线程句柄
HANDLE        hDrawThreadT3      =NULL;//画图线程句柄
HANDLE        hDrawThreadT4      =NULL;//画图线程句柄
HANDLE        hDrawThreadT10     =NULL;//画图线程句柄
HWND		  hTab               =NULL;//Tab控件句柄

HINSTANCE     hStockDll          =NULL;//库存公式动态连接库句柄
RECT		  MyRect			 ={0,0,0,0};//鼠标所选矩形区域
RECT		  MyOldRect			 ={0,0,0,0};//保存鼠标原来矩形区域
POINT         OldPoint		     ={0,0};//保存鼠标原来的坐标点
BOOL		  IsCapture			 =FALSE;//鼠标是否捕获
BOOL		  IsFangSuo			 =FALSE;//是否在放缩

BOOL		  IsScreen			 =FALSE;//是否全屏

COLORREF	  BackGroundColor	 =0;//画板背景色
COLORREF	  T3_Color			 =RGB(255,255,255);//T3对话框画线颜色

WNDPROC		  OldT3ColorMsgProc  =NULL;//记录原来T3对话框颜色静态控件消息处理函数的全局变量

long double kx=0.006,ky=0.006,fx1=0,fx2=0,fy1=0,fy2=0;//放大参数

//参数属性
#define PARAMS_ATTR_T0			1
#define PARAMS_ATTR_T1			2
#define PARAMS_ATTR_T3			4
#define PARAMS_ATTR_T2			8
#define PARAMS_ATTR_T4			16
struct PARAMS//参数结构体(用于DLL中)
{
	int attr;//属性
	HWND main_handle;//主窗口句柄
	HWND main_dlg;//主对话框句柄
	HINSTANCE main_instance;//保存窗口实例句柄
	HWND hTabDlg;//Tab子对话框句柄
	HDC  hmemdc;//兼容DC
	vector<string> gongshi_array;//保存公式字符串的数组
	RECT myrect;//鼠标所选矩形区域
	BOOL isfangsuo;//是否在放缩
};

//IFS码结构体
struct IFS
{
	double a,b,c,d,e,f;//变换系数
	double p;//概率
	double psum;//前面所有ifs的概率和，用于计算概率空间
	double x0,y0;//起始坐标
	COLORREF color;//颜色
};

//LS文法规则结构体
struct LS
{
	string head;//规则头
	string body;//规则体
	double p;//概率
};

//LS节点
struct LSDOT
{
	double x,y,z;//坐标
	//double xr,yr,zr;//单位向量
	double xdeg,ydeg,zdeg;//离轴夹角
};

#define SIGN(t) (t>=0?1:-1)

typedef int (*StockDllProc)(LPVOID lpParam);
StockDllProc StartDrawByDLL=NULL;//动态连接库函数(画库存图)
StockDllProc GetAllGongShiFromDLL=NULL;//动态连接库函数(得到所有库存函数) 

string        rules_str;//LS规则序列
string		  wenfastr;//LS文法文本
long		  ls_count=0;//LS迭代次数

//T2设置名称对话框消息处理(保存文法及参数配置)
BOOL WINAPI Dlg_T2_SetName(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	HWND hName=GetDlgItem(hWnd,IDC_T4_SETNAME);
	HWND hN=GetDlgItem(hT2,IDC_T2_N);
	HWND hLen=GetDlgItem(hT2,IDC_T2_LEN);
	HWND hX0=GetDlgItem(hT2,IDC_T2_X0);
	HWND hY0=GetDlgItem(hT2,IDC_T2_Y0);
	HWND hZ0=GetDlgItem(hT2,IDC_T2_Z0);
	HWND hAngle=GetDlgItem(hT2,IDC_T2_DEGREE);
	HWND hWenfa=GetDlgItem(hT2,IDC_T2_WENFA_EDIT);

	//得到配置文件路径
	char filename[260];
	string buffer;
	GetModuleFileName(NULL,filename,260);
	buffer=filename;
	buffer.assign(buffer,0,buffer.find_last_of('\\'));
	buffer+="\\LS.ini";

	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
		

			return TRUE;
		}
		break;
	case WM_COMMAND:
		{
			int ControlID=LOWORD(wParam);
			int ControlNotify=HIWORD(wParam);
			HWND ControlHandle=(HWND)lParam;

			if(ControlID==IDC_SETNAME_OK)//确定
			{
				char name[256],str[256];
				GetWindowText(hName,name,256);//得到名称

				int weishu;//得到维数
				if(IsDlgButtonChecked(hT2,IDC_RBN_T2_2)&&!IsDlgButtonChecked(hT2,IDC_RBN_T2_3))
					weishu=2;
				else if(!IsDlgButtonChecked(hT2,IDC_RBN_T2_2)&&IsDlgButtonChecked(hT2,IDC_RBN_T2_3))
					weishu=3;
				else weishu=2;

				//写入配置文件
				sprintf(str,"%d",weishu);//维数
				WritePrivateProfileString(name,"WeiShu",str,buffer.c_str());
				GetWindowText(hN,str,256);//迭代次数
				WritePrivateProfileString(name,"N",str,buffer.c_str());
				GetWindowText(hAngle,str,256);//缺省角度
				WritePrivateProfileString(name,"Angle",str,buffer.c_str());
				GetWindowText(hLen,str,256);//单位长度
				WritePrivateProfileString(name,"Len",str,buffer.c_str());
				GetWindowText(hX0,str,256);
				WritePrivateProfileString(name,"X0",str,buffer.c_str());
				GetWindowText(hY0,str,256);
				WritePrivateProfileString(name,"Y0",str,buffer.c_str());
				GetWindowText(hZ0,str,256);
				WritePrivateProfileString(name,"Z0",str,buffer.c_str());
				

				char wenfa[10000];
				GetWindowText(hWenfa,wenfa,10000);
				string wenfastr(wenfa);
				int index=wenfastr.find_first_of("\r\n");
				int endindex=wenfastr.find_first_of('@');
				int oldindex=0;
				int i=0;
				while(index!=-1&&oldindex<index&&index<endindex)//写入规则
				{
					sprintf(str,"Rule_%d",i++);
					string strbuf(wenfa,oldindex,index-oldindex);
					WritePrivateProfileString(name,str,strbuf.c_str(),buffer.c_str());
					oldindex=index+2;
					index=wenfastr.find_first_of("\r\n",oldindex);
				}
				//==========================

				EndDialog(hWnd,NULL);
			}
			else if(ControlID==IDC_SETNAME_CANCEL)//取消
			{
			
				EndDialog(hWnd,NULL);
			}
	
			return TRUE;
		}
		break;
	case WM_CLOSE:
		{
			EndDialog(hWnd,NULL);
			return TRUE;
		}
		break;
	default:
		return FALSE;
	}
	return FALSE;

}
//T2库存公式对话框消息处理
BOOL WINAPI DlgT2Stock(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	HWND hN=GetDlgItem(hT2,IDC_T2_N);
	HWND hLen=GetDlgItem(hT2,IDC_T2_LEN);
	HWND hX0=GetDlgItem(hT2,IDC_T2_X0);
	HWND hY0=GetDlgItem(hT2,IDC_T2_Y0);
	HWND hZ0=GetDlgItem(hT2,IDC_T2_Z0);
	HWND hAngle=GetDlgItem(hT2,IDC_T2_DEGREE);
	HWND hWenfa=GetDlgItem(hT2,IDC_T2_WENFA_EDIT);

	//得到配置文件路径
	char filename[260];
	string buffer;
	GetModuleFileName(NULL,filename,260);
	buffer=filename;
	buffer.assign(buffer,0,buffer.find_last_of('\\'));
	buffer+="\\LS.ini";

	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
		
			//设置列表框
			char inibuffer[1000];
			//得到所有节名到inibuffer,每个节名以0间隔，最后以2个0结束
			if(GetPrivateProfileSectionNames(inibuffer,1000,buffer.c_str()))//若打开文件成功
			{
				char *p=inibuffer;
				while(*(p))
				{
					SendDlgItemMessage(hWnd,IDC_LST_STOCK,LB_ADDSTRING,0,(LPARAM)p);//保存到列表框
					p+=strlen(p)+1;//递增到下一个节名
				
				}
			}
			
			return TRUE;
		}
		break;
	case WM_COMMAND:
		{
			int ControlID=LOWORD(wParam);
			int ControlNotify=HIWORD(wParam);
			HWND ControlHandle=(HWND)lParam;
			if(ControlID==IDC_LST_STOCK)
			{
				if(ControlNotify==LBN_DBLCLK)//若为双击
				{
					char name[260];
					int sel=SendMessage(ControlHandle,LB_GETCURSEL,0,0);//得到当前索引
					SendMessage(ControlHandle,LB_GETTEXT,(WPARAM)sel,(LPARAM)name);//得到文法名

					char str[256];
					GetPrivateProfileString(name,"WeiShu","2",str,256,buffer.c_str());//得到维数
					int select=atol(str);
					if(select==3)
					{
						CheckDlgButton(hT2,IDC_RBN_T2_3,BST_CHECKED);
						CheckDlgButton(hT2,IDC_RBN_T2_2,BST_UNCHECKED);
					}
					else
					{
						CheckDlgButton(hT2,IDC_RBN_T2_2,BST_CHECKED);
						CheckDlgButton(hT2,IDC_RBN_T2_3,BST_UNCHECKED);
					
					}
					GetPrivateProfileString(name,"N","0",str,256,buffer.c_str());//得到迭代次数
					SetWindowText(hN,str);
					GetPrivateProfileString(name,"Angle","0",str,256,buffer.c_str());//得到缺省角度
					SetWindowText(hAngle,str);
					GetPrivateProfileString(name,"Len","1",str,256,buffer.c_str());
					SetWindowText(hLen,str);
					GetPrivateProfileString(name,"X0","0",str,256,buffer.c_str());
					SetWindowText(hX0,str);
					GetPrivateProfileString(name,"Y0","0",str,256,buffer.c_str());
					SetWindowText(hY0,str);
					GetPrivateProfileString(name,"Z0","0",str,256,buffer.c_str());
					SetWindowText(hZ0,str);

					string wenfa,strbuf;
					GetPrivateProfileString(name,"Rule_0","",str,256,buffer.c_str());
					strbuf=str;
					int i=1;
					char indexstr[20];
					while(strbuf.size()>0)
					{
						wenfa+=strbuf;
						wenfa+="\r\n";
						sprintf(indexstr,"Rule_%d",i++);
						GetPrivateProfileString(name,indexstr,"",str,256,buffer.c_str());
						strbuf=str;
					}
					wenfa+="@\r\n";
					SetWindowText(hWenfa,wenfa.c_str());

					EndDialog(hWnd,NULL);//关闭对话框
				}
			
			}
	
			return TRUE;
		}
		break;
	case WM_CLOSE:
		{
			EndDialog(hWnd,NULL);
			return TRUE;
		}
		break;
	default:
		return FALSE;
	}
	return FALSE;
	
}

//T3对话框颜色静态控件新消息处理函数
LRESULT MyT3ColorMsgProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_PAINT:
		{
			PAINTSTRUCT p;
			HDC hDC=BeginPaint(hWnd,&p);
			//填从控件颜色
			HBRUSH hbrush=CreateSolidBrush(T3_Color);
			RECT cr;
			GetClientRect(hWnd,&cr);
			FillRect(hDC,&cr,hbrush);

			EndPaint(hWnd,&p);
			
			return TRUE;
		}
		break;

	}
	//调用控件原来的消息处理函数
	return CallWindowProc(OldT3ColorMsgProc,hWnd,uMsg,wParam,lParam);
}

COLORREF Edit_Color=0;//该对话框专用全局变量，显示控件的颜色
//T4设置名称对话框消息处理
BOOL WINAPI Dlg_T4_SetName(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	HWND hName=GetDlgItem(hWnd,IDC_T4_SETNAME);
	HWND hN=GetDlgItem(hT4,IDC_T4_N);
	HWND hXmax=GetDlgItem(hT4,IDC_T4_XMAX);
	HWND hYmax=GetDlgItem(hT4,IDC_T4_YMAX);
	HWND hX0=GetDlgItem(hT4,IDC_T4_X0);
	HWND hY0=GetDlgItem(hT4,IDC_T4_Y0);
	HWND hListView=GetDlgItem(hT4,IDC_T4_LISTVIEW);
	HWND hList=GetDlgItem(hT4,IDC_T4_STOCKLIST);

	//得到配置文件路径
	char filename[260];
	string buffer;
	GetModuleFileName(NULL,filename,260);
	buffer=filename;
	buffer.assign(buffer,0,buffer.find_last_of('\\'));
	buffer+="\\IFS.ini";

	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			char name[256];
			int sel=SendMessage(hList,LB_GETCURSEL,0,0);//得到当前索引
			if(sel>=0)
			{
				SendMessage(hList,LB_GETTEXT,(WPARAM)sel,(LPARAM)name);//得到当前选中字符串
				SetWindowText(hName,name);
			}
			else
			{
				SetWindowText(hName,"noname");
			}

			return TRUE;
		}
		break;
	case WM_COMMAND:
		{
			int ControlID=LOWORD(wParam);
			int ControlNotify=HIWORD(wParam);
			HWND ControlHandle=(HWND)lParam;

			if(ControlID==IDC_SETNAME_OK)//确定
			{
				char name[256],str[256];
				GetWindowText(hName,name,256);//得到名称

				//写入配置文件
				GetWindowText(hN,str,256);//迭代次数
				WritePrivateProfileString(name,"N",str,buffer.c_str());
				GetWindowText(hXmax,str,256);//Xmax
				WritePrivateProfileString(name,"Xmax",str,buffer.c_str());
				GetWindowText(hYmax,str,256);//Ymax
				WritePrivateProfileString(name,"Ymax",str,buffer.c_str());
				GetWindowText(hX0,str,256);//X0
				WritePrivateProfileString(name,"X0",str,buffer.c_str());
				GetWindowText(hY0,str,256);//Y0
				WritePrivateProfileString(name,"Y0",str,buffer.c_str());

				int itemcounts=ListView_GetItemCount(hListView);//得到当前项数
				sprintf(str,"%d",itemcounts);
				WritePrivateProfileString(name,"Counts",str,buffer.c_str());//写入ifs项数

				int i,j;
				//写入IFS码
				for(i=0;i<itemcounts;i++)
				{
					string ifs_str;
					for(j=1;j<=8;j++)
					{
						ListView_GetItemText(hListView,i,j,str,256);
						ifs_str+=str;
						ifs_str+="|";//分隔符
					}
					sprintf(str,"IFS_%d",i);
					WritePrivateProfileString(name,str,ifs_str.c_str(),buffer.c_str());
				}

				//刷新列表框
				SendDlgItemMessage(hT4,IDC_T4_STOCKLIST,LB_RESETCONTENT,0,0);//清空列表框

				char inibuffer[1000];
				//得到所有节名到inibuffer,每个节名以0间隔，最后以2个0结束
				if(GetPrivateProfileSectionNames(inibuffer,1000,buffer.c_str()))//若打开文件成功
				{
					char *p=inibuffer;
			
					while(*(p))
					{
						SendDlgItemMessage(hT4,IDC_T4_STOCKLIST,LB_ADDSTRING,0,(LPARAM)p);//保存到列表框
						p+=strlen(p)+1;//递增到下一个节名
					
					}
				}

				EndDialog(hWnd,NULL);
			}
			else if(ControlID==IDC_SETNAME_CANCEL)//取消
			{
			
				EndDialog(hWnd,NULL);
			}
	
			return TRUE;
		}
		break;
	case WM_CLOSE://鼠标左键消息
		{
			EndDialog(hWnd,NULL);
			return TRUE;
		}
		break;
	default:
		return FALSE;
	}
	return FALSE;

}
//T4设置值对话框消息处理
BOOL WINAPI Dlg_T4_SetValue(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	HWND hListView=GetDlgItem(hT4,IDC_T4_LISTVIEW);
	HWND hA=GetDlgItem(hWnd,IDC_ST_T4_A);
	HWND hB=GetDlgItem(hWnd,IDC_ST_T4_B);
	HWND hC=GetDlgItem(hWnd,IDC_ST_T4_C);
	HWND hD=GetDlgItem(hWnd,IDC_ST_T4_D);
	HWND hE=GetDlgItem(hWnd,IDC_ST_T4_E);
	HWND hF=GetDlgItem(hWnd,IDC_ST_T4_F);
	HWND hP=GetDlgItem(hWnd,IDC_ST_T4_P);
	HWND hColor=GetDlgItem(hWnd,IDC_ST_T4_COLOR);
	char buffer[256];
	int curitem;
	
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			curitem=ListView_GetNextItem(hListView,-1,LVNI_FOCUSED);//得到当前项
			//得到当前项文本进行初始化编辑框
			ListView_GetItemText(hListView,curitem,1,buffer,256);
			SetWindowText(hA,buffer);
			ListView_GetItemText(hListView,curitem,2,buffer,256);
			SetWindowText(hB,buffer);
			ListView_GetItemText(hListView,curitem,3,buffer,256);
			SetWindowText(hC,buffer);
			ListView_GetItemText(hListView,curitem,4,buffer,256);
			SetWindowText(hD,buffer);
			ListView_GetItemText(hListView,curitem,5,buffer,256);
			SetWindowText(hE,buffer);
			ListView_GetItemText(hListView,curitem,6,buffer,256);
			SetWindowText(hF,buffer);
			ListView_GetItemText(hListView,curitem,7,buffer,256);
			SetWindowText(hP,buffer);

			// 得到颜色
			ListView_GetItemText(hListView,curitem,8,buffer,256);
			Edit_Color=atol(buffer);
			
			//发送控件消息，设置IDC_ST_T4_COLOR颜色
			PostMessage(hWnd,WM_CTLCOLOR,(WPARAM)0,(LPARAM)IDC_ST_T4_COLOR);

			return TRUE;
		}
		break;
	case WM_COMMAND:
		{
			int ControlID=LOWORD(wParam);
			int ControlNotify=HIWORD(wParam);
			HWND ControlHandle=(HWND)lParam;
			if(ControlID==IDC_ST_T4_OK)//若为确定按钮，则设回值
			{
				curitem=ListView_GetNextItem(hListView,-1,LVNI_FOCUSED);//得到当前项
				//设置子项文本
				GetWindowText(hA,buffer,256);
				ListView_SetItemText(hListView,curitem,1,buffer);
				GetWindowText(hB,buffer,256);
				ListView_SetItemText(hListView,curitem,2,buffer);
				GetWindowText(hC,buffer,256);
				ListView_SetItemText(hListView,curitem,3,buffer);
				GetWindowText(hD,buffer,256);
				ListView_SetItemText(hListView,curitem,4,buffer);
				GetWindowText(hE,buffer,256);
				ListView_SetItemText(hListView,curitem,5,buffer);
				GetWindowText(hF,buffer,256);
				ListView_SetItemText(hListView,curitem,6,buffer);
				GetWindowText(hP,buffer,256);
				ListView_SetItemText(hListView,curitem,7,buffer);
				sprintf(buffer,"%ld",Edit_Color);
				ListView_SetItemText(hListView,curitem,8,buffer);
				
				EndDialog(hWnd,NULL);
			}
			else if(ControlID==IDC_ST_T4_CANCEL)//若为取消按钮
			{
				EndDialog(hWnd,NULL);
			}
			else if(ControlID==IDC_ST_T4_RESET)//若为重置按钮
			{
				SetWindowText(hA,"0");
				SetWindowText(hB,"0");
				SetWindowText(hC,"0");
				SetWindowText(hD,"0");
				SetWindowText(hE,"0");
				SetWindowText(hF,"0");
				SetWindowText(hP,"0");
				
			}
			else if(ControlID==IDC_ST_T4_CHOOSECOLOR)//若为颜色选择按钮
			{
				//打开通用颜色对话框
				COLORREF cusColor[16];

				CHOOSECOLOR lcc;
				lcc.lStructSize=sizeof(CHOOSECOLOR);
				lcc.hwndOwner=hWnd;
				lcc.hInstance=NULL;
				lcc.rgbResult=RGB(0,0,0);
				lcc.lpCustColors=cusColor; 
				lcc.Flags=CC_RGBINIT|CC_FULLOPEN| CC_ANYCOLOR;
				lcc.lCustData=NULL;
				lcc.lpfnHook=NULL;
				lcc.lpTemplateName=NULL;

				if(ChooseColor(&lcc))// 得到颜色
				{
					Edit_Color=lcc.rgbResult;
					//将颜色填充到静态框显示
					HDC hdc=GetDC(hColor);
					HBRUSH hbrush=CreateSolidBrush(Edit_Color);
					RECT r;
					GetClientRect(hColor,&r);
					FillRect(hdc,&r,hbrush);
					ReleaseDC(hColor,hdc);
				
				}
			}

			return TRUE;
		}
		break;
	case WM_CTLCOLOR://控件消息
		{
			
			int ControlID=LOWORD(lParam);
			if(ControlID==IDC_ST_T4_COLOR)
			{
				//将颜色填充到静态框显示
				HDC hdc=GetDC(hColor);
				HBRUSH hbrush=CreateSolidBrush(Edit_Color);
				RECT r;
				GetClientRect(hColor,&r);
				FillRect(hdc,&r,hbrush);
				ReleaseDC(hColor,hdc);
			
			}

			return TRUE;
		}
		break;
	case WM_CLOSE://鼠标左键消息
		{
			EndDialog(hWnd,NULL);
			return TRUE;
		}
		break;
	default:
		return FALSE;
	}
	return FALSE;
	
}
//T1库存公式对话框消息处理
BOOL WINAPI DlgT1Stock(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			//获取所有库存公式，并将所有公式添加到列表
			if(GetAllGongShiFromDLL)
			{
				PARAMS param;
				param.attr=PARAMS_ATTR_T0;
				if(GetAllGongShiFromDLL((LPVOID)&param))//得到库存公式
				{
					vector<string> gongshi_array=param.gongshi_array;
					if(gongshi_array.size())//将所有公式添加到列表
					{
						for(int i=0;i<gongshi_array.size();i++)
						{
							SendDlgItemMessage(hWnd,IDC_LST_STOCK,\
								LB_ADDSTRING,0,(LPARAM)gongshi_array[i].c_str());
						
						}
					
					}
				}
			}
			return TRUE;
		}
		break;
	case WM_COMMAND:
		{
			int ControlID=LOWORD(wParam);
			int ControlNotify=HIWORD(wParam);
			HWND ControlHandle=(HWND)lParam;
			if(ControlID==IDC_LST_STOCK)
			{
				if(ControlNotify==LBN_DBLCLK)//若为双击
				{
					char gongshi[260];
					int sel=SendMessage(ControlHandle,LB_GETCURSEL,0,0);//得到当前索引
					SendMessage(ControlHandle,LB_GETTEXT,(WPARAM)sel,(LPARAM)gongshi);//得到公式
			
					HWND hEdit=GetDlgItem(hT0,IDC_T0_GONGSHI_EDT);
					SetWindowText(hEdit,gongshi);//将公式设回编辑框

					EndDialog(hWnd,NULL);//关闭对话框
				}
			
			}
	
			return TRUE;
		}
		break;
	case WM_CLOSE://鼠标左键消息
		{
			EndDialog(hWnd,NULL);
			return TRUE;
		}
		break;
	default:
		return FALSE;
	}
	return FALSE;
	
}

//T0对话框绘图线程
DWORD WINAPI OnDrawT0(LPVOID lpParam)
{
	RECT r;
	GetClientRect(MAIN_HANDLE,&r);//得到窗口客户端大小

	HDC hDC=GetDC(MAIN_HANDLE);//得到屏幕DC

	//擦除背景
	HBRUSH hbrush=CreateSolidBrush(BackGroundColor);
	//FillRect(hDC,&r,hbrush);//填充擦除色
	//得到参数
	//========================================================
	//颜色参数
	int Ar=(int)GetDlgItemInt(hT0,IDC_T0_AR,NULL,FALSE);
	int Ag=(int)GetDlgItemInt(hT0,IDC_T0_AG,NULL,FALSE);
	int Ab=(int)GetDlgItemInt(hT0,IDC_T0_AB,NULL,FALSE);
	int Br=(int)GetDlgItemInt(hT0,IDC_T0_BR,NULL,FALSE);
	int Bg=(int)GetDlgItemInt(hT0,IDC_T0_BG,NULL,FALSE);
	int Bb=(int)GetDlgItemInt(hT0,IDC_T0_BB,NULL,FALSE);

	unsigned long Rmax=(unsigned long)GetDlgItemInt(hT0,IDC_T0_RMAX,NULL,FALSE);//Rmax
	unsigned long Nmax=(unsigned long)GetDlgItemInt(hT0,IDC_T0_NMAX,NULL,FALSE);//Nmax

	long double Newton_Rmax=(long double)1/Rmax;//Newton逃逸半径

	//开始计算并绘制(在内存中绘制)
	//========================================================
	long double width=r.right-r.left;
	long double height=r.bottom-r.top;
	unsigned long n;
	long double i,j;
	int R,G,B;
	long double dx,dy;

	//===========================================================
	//放大算法
	//===================================================
	RECT myrect={0,0,width,height};//缺省放大矩形区域
	if(IsFangSuo)
	{
		myrect=MyRect;//得到放大矩形区域
	}
	else //若第一次画，设置初始复平面区域
	{
		kx=0.006;
		ky=0.006;
		fx1=-(width/2)*kx;
		fy1=-(height/2)*ky;
		fx2=(width/2)*kx;
		fy2=(height/2)*ky;
	}

	fx1=fx1+(myrect.left-0)*kx;//逐次放大
	fy1=fy1+(myrect.top-0)*ky;
	fx2=fx2+(myrect.right-width)*kx;
	fy2=fy2+(myrect.bottom-height)*ky;

	kx*=(myrect.right-myrect.left)/width;//保存放大比例
	ky*=(myrect.bottom-myrect.top)/height;

	//显示放大比例
	char bitstr[20];
	sprintf(bitstr,"分形画板1.0(放大比例: x轴 1:%ld,y轴 1:%ld)",(long)(1/kx),(long)(1/ky));
	SetWindowText(MAIN_DIALOG,bitstr);

	char gongshi[260];
	vector<TOKEN<FuShu> > arrays;
	vector<TOKEN<FuShu> > val(2);
	val[0].str="z";//变量z
	val[1].str="c";//常量c

	//选择绘制的集合
	int select;
	if(IsDlgButtonChecked(hT0,IDC_RBN_T0_MANDELBROT)\
	&&!IsDlgButtonChecked(hT0,IDC_RBN_T0_JULIA)\
	&&!IsDlgButtonChecked(hT0,IDC_RBN_T0_NEWTON))
	{
		select=FRACTAL_T0_MANDELBROT;//Mandelbrot
	}
	else if(!IsDlgButtonChecked(hT0,IDC_RBN_T0_MANDELBROT)\
	&&!IsDlgButtonChecked(hT0,IDC_RBN_T0_NEWTON)\
	&&IsDlgButtonChecked(hT0,IDC_RBN_T0_JULIA))
	{
		select=FRACTAL_T0_JULIA;//Julia
	}
	else if(!IsDlgButtonChecked(hT0,IDC_RBN_T0_MANDELBROT)\
	&&!IsDlgButtonChecked(hT0,IDC_RBN_T0_JULIA)\
	&&IsDlgButtonChecked(hT0,IDC_RBN_T0_NEWTON))
	{
		select=FRACTAL_T0_NEWTON;//Newton
	}
	else select=0;

	//得到维数
	int weishu=2;
	if(IsDlgButtonChecked(hT0,IDC_RBN_T0_3))weishu=3;

	FuShu z;
	FuShu z1,t1,t2;
	if(select==FRACTAL_T0_JULIA||select==FRACTAL_T0_NEWTON)//若为Julia集，或Newton分形
	{
		//得到常数C
		HWND hReal=GetDlgItem(hT0,IDC_T0_C_REAL);
		HWND hImag=GetDlgItem(hT0,IDC_T0_C_IMAG);
		char real[20],imag[20];
		GetWindowText(hReal,real,20);
		GetWindowText(hImag,imag,20);

		//将字符串转为double值
		char *st;
		long double Real=(long double)strtod(real,&st);
		long double Imag=(long double)strtod(imag,&st);

		val[1].num.Get(Real,Imag);
	
	}

	//得到公式
	HWND hEdit=GetDlgItem(hT0,IDC_T0_GONGSHI_EDT);
	GetWindowText(hEdit,gongshi,260);

	if(!GongShi_Change(gongshi,val,arrays))//公式转换
	{
		MessageBox(MAIN_HANDLE,"无效公式!","错误",MB_OK);
		goto end;
	}

	//画图
	//========================================
	//先失真放大
	StretchBlt(hDC,0,0,width,height,hMemDC,myrect.left,myrect.top,\
		myrect.right-myrect.left,myrect.bottom-myrect.top,SRCCOPY);

	FillRect(hMemDC,&r,hbrush);//填充擦除色
	//=========================================
	dx=(fx2-fx1)/width;
	dy=(fy2-fy1)/height;
	
	for(i=0;i<height;i++)
	{
		for(j=0;j<width;j++)
		{
			if(select==FRACTAL_T0_MANDELBROT)//Mandelbrot
			{
				val[0].num.Get(0);//z=(0,0)
				val[1].num.Get(fx1+j*dx,fy1+i*dy);//c=(fx1+j*dx,fy1+i*dy)
			}
			else if(select==FRACTAL_T0_JULIA)//Julia
			{
				val[0].num.Get(fx1+j*dx,fy1+i*dy);//z=(fx1+j*dx,fy1+i*dy)
				//c=(Real,Imag)
			}
			else if(select==FRACTAL_T0_NEWTON)//Newton
			{
				z1.Get(fx1+j*dx,fy1+i*dy);//z1=(fx1+j*dx,fy1+i*dy)
			}
			else goto end;

			n=0;
			while (1)
			{
				if(select==FRACTAL_T0_MANDELBROT||select==FRACTAL_T0_JULIA)
				{
					if(!GongShi_Compute(arrays,val,val[0].num))//计算公式
					{
						MessageBox(MAIN_HANDLE,"无效公式!","错误",MB_OK);
						goto end;
		
					}
					if((val[0].num.m2())>=Rmax)
					{
						R = n*Ar+Br;
						G = n*Ag+Bg;
						B = n*Ab+Bb;
						if ((R & 0x1FF) > 0xFF) R = R ^ 0xFF;
						if ((G & 0x1FF) > 0xFF) G = G ^ 0xFF;
						if ((B & 0x1FF) > 0xFF) B = B ^ 0xFF;
					
						SetPixel(hMemDC,(int)(j),(int)(i),RGB(R,G,B));
						break;
					}
					if(n++>=Nmax)
					{
						break;
					}
				}
				else if(select==FRACTAL_T0_NEWTON)
				{
					val[0].num=z1;
					if(!GongShi_Compute(arrays,val,t1))//计算原公式
					{
						MessageBox(MAIN_HANDLE,"无效公式!","错误",MB_OK);
						goto end;
		
					}
					FuShu h(0.0000001,0.0000001);
					val[0].num=z1+h;
					if(!GongShi_Compute(arrays,val,t2))//计算公式(用于求导)
					{
						MessageBox(MAIN_HANDLE,"无效公式!","错误",MB_OK);
						goto end;
		
					}
					z=z1-(t1*h)/(t2-t1);//Newton矢代
					z1=z-z1;//计算与上次值的距离
					if(z1.m2()>Newton_Rmax)//若趋于0
					{
						R = n*Ar+Br;
						G = n*Ag+Bg;
						B = n*Ab+Bb;
						if ((R & 0x1FF) > 0xFF) R = R ^ 0xFF;
						if ((G & 0x1FF) > 0xFF) G = G ^ 0xFF;
						if ((B & 0x1FF) > 0xFF) B = B ^ 0xFF;
						SetPixel(hMemDC,(int)(j),(int)(i),RGB(R,G,B));
						
					}
					if(n++>=Nmax)//若逃逸
					{
						break;
					}
					z1=z;//矢代
				}
			}
			
			
		}
	BitBlt(hDC,r.left,r.top,r.right-r.left,i,hMemDC,0,0,SRCCOPY);//逐渐清晰
	}
	//========================================================

	//显示图像
	//BitBlt(hDC,r.left,r.top,r.right-r.left,r.bottom-r.top,hMemDC,0,0,SRCCOPY);
end:
	ReleaseDC(MAIN_HANDLE,hDC);

	if(!IsFangSuo)//若在放缩状态下，不启用
	{
		//启用绘图按钮
		HWND hDraw=GetDlgItem(MAIN_DIALOG,IDC_DRAW);
		EnableWindow(hDraw,TRUE);
		//启用擦除按钮
		HWND hClear=GetDlgItem(MAIN_DIALOG,IDC_CLEAR);
		EnableWindow(hClear,TRUE);
	}
	else
	{
		//放大后矩形区域清零
		MyRect.left=0;
		MyRect.top=0;
		MyRect.right=0;
		MyRect.bottom=0;
	
	}

	//取消显示放大比例
	SetWindowText(MAIN_DIALOG,"分形画板1.0");
	
	return 1;
}
//T1对话框绘图线程
DWORD WINAPI OnDrawT1(LPVOID lpParam)
{
	RECT r;
	GetClientRect(MAIN_HANDLE,&r);//得到窗口客户端大小

	HDC hDC=GetDC(MAIN_HANDLE);//得到屏幕DC

	//擦除背景
	HBRUSH hbrush=CreateSolidBrush(BackGroundColor);
	FillRect(hDC,&r,hbrush);//填充擦除色

	//开始计算并绘制(在内存中绘制)
	//========================================================
	int width=r.right-r.left;
	int height=r.bottom-r.top;


	//========================================================

	//显示图像
	//BitBlt(hDC,r.left,r.top,r.right-r.left,r.bottom-r.top,hMemDC,0,0,SRCCOPY);

	ReleaseDC(MAIN_HANDLE,hDC);

	//启用绘图按钮
	HWND hDraw=GetDlgItem(MAIN_DIALOG,IDC_DRAW);
	EnableWindow(hDraw,TRUE);
	//启用擦除按钮
	HWND hClear=GetDlgItem(MAIN_DIALOG,IDC_CLEAR);
	EnableWindow(hClear,TRUE);
	
	return 1;
}
//T2对话框绘图线程
DWORD WINAPI OnDrawT2(LPVOID lpParam)
{
	RECT r;
	GetClientRect(MAIN_HANDLE,&r);//得到窗口客户端大小

	HDC hDC=GetDC(MAIN_HANDLE);//得到屏幕DC

	//擦除背景
	HBRUSH hbrush=CreateSolidBrush(BackGroundColor);
	FillRect(hDC,&r,hbrush);//填充擦除色
	FillRect(hMemDC,&r,hbrush);//填充擦除色


	//开始计算并绘制(在内存中绘制)
	//========================================================
	double width=r.right-r.left;
	double height=r.bottom-r.top;
	double scrsize2=sqrt(width*width+height*height);//得到2维屏幕尺寸
	double x0=width/2;
	double y0=height/2;
	int i;
	string bufrules;
	//得到参数
	//=======================================================
	char len[20],*st;
	//得到单位长度
	HWND hLen=GetDlgItem(hT2,IDC_T2_LEN);
	GetWindowText(hLen,len,20);
	double lenght=strtod(len,&st);

	//得到初始坐标
	double X_0,Y_0,Z_0;
	char x_0[20],y_0[20],z_0[20];
	HWND hX0=GetDlgItem(hT2,IDC_T2_X0);
	GetWindowText(hX0,x_0,20);
	X_0=strtod(x_0,&st);
	HWND hY0=GetDlgItem(hT2,IDC_T2_Y0);
	GetWindowText(hY0,y_0,20);
	Y_0=strtod(y_0,&st);
	HWND hZ0=GetDlgItem(hT2,IDC_T2_Z0);
	GetWindowText(hZ0,z_0,20);
	Z_0=strtod(z_0,&st);

	int weishu;//得到维数
	if(IsDlgButtonChecked(hT2,IDC_RBN_T2_2)&&!IsDlgButtonChecked(hT2,IDC_RBN_T2_3))
		weishu=2;
	else if(!IsDlgButtonChecked(hT2,IDC_RBN_T2_2)&&IsDlgButtonChecked(hT2,IDC_RBN_T2_3))
		weishu=3;
	else weishu=2;

	long n;//得到迭代次数
	char nstr[20];
	HWND hN=GetDlgItem(hT2,IDC_T2_N);
	GetWindowText(hN,nstr,20);
	n=atol(nstr);

	//得到旋转角度
	double angle;
	char anglestr[20];
	HWND hAngle=GetDlgItem(hT2,IDC_T2_DEGREE);
	GetWindowText(hAngle,anglestr,20);
	angle=strtod(anglestr,&st);
	
	//==========================

	char wenfa[10000];//得到文法文本
	HWND hWenfa=GetDlgItem(hT2,IDC_T2_WENFA_EDIT);
	GetWindowText(hWenfa,wenfa,10000);

	vector<LS> Rules;//转为规则数组(分解规则)
	if(wenfastr.compare(wenfa)!=0||ls_count!=n)//判断文法是否改变
	{
		wenfastr.assign(wenfa);
		ls_count=n;
		
		string str;
		char *p=wenfa;

		//得到初始规则
		string initrule;
		int init_index=wenfastr.find_first_of("\r\n");
		initrule.assign(wenfa,init_index);
		p+=init_index+2;
		rules_str=initrule;

		if(rules_str.size()==0||rules_str.find_first_of('=')!=-1)
		{
			MessageBox(MAIN_HANDLE,"请输入初始规则!","错误",MB_OK);
			goto end;
		}
	 
	
		if(wenfastr.find_first_of('@')==-1)
		{
			MessageBox(MAIN_HANDLE,"请输入文法结束符:'@'!","错误",MB_OK);
			goto end;

		}
		while(*p)
		{
			if(*p=='\r'||*p=='#')
			{
				//分解规则
				int index=str.find_first_of('=');
				if(index!=-1)
				{
					LS rule;
					if(index!=1)
					{
						rule.head.assign(str,0,index);
						char errstr[100];
						sprintf(errstr,"生成元'%s'不是字符变量!",rule.head.c_str());
						MessageBox(MAIN_HANDLE,errstr,"错误",MB_OK);
						goto end;
					}
				
					rule.head.assign(str,0,1);
					rule.body.assign(str,2,str.length()-2);
					Rules.push_back(rule);
					if(rule.head.size()==0)
					{
						MessageBox(MAIN_HANDLE,"LS语法错误!","错误",MB_OK);
						goto end;
					}
				}
				str.resize(0);
				//忽略注释部分
				if(*p=='#')
				{
					while(*(p+1))
					{
						p++;
						if(*p=='\n')
						{
							break;
						}
				
					}
				}
			}
			else if(!isspace(*p))//过滤掉空白字符
			{
				str+=*p;
			}
			if(*p=='@')break;//文件结束符
		
			p++;
		}
	//for(i=0;i<Rules.size();i++)
		//DebugBox("head:%s,body:%s",Rules[i].head.c_str(),Rules[i].body.c_str());
	//========================================================
		//规则迭代
		while(n-->0)
		{
			bufrules=rules_str;
			char *p=(char *)bufrules.c_str();
			rules_str.resize(0);
			while(*p)
			{
				string s;
				s.assign(p,1);
				for(i=0;i<Rules.size();i++)
				{
					if(Rules[i].head.compare(s)==0)
					{
						rules_str+=Rules[i].body;
						break;
					}
			
			
				}
				if(i==Rules.size())
				{
					rules_str+=s;
				}
				p++;
		
			}
		}
		if(rules_str.compare(initrule)==0)
		{
			MessageBox(MAIN_HANDLE,"无匹配生成元!","错误",MB_OK);
			goto end;
		}
		//DebugBox("%s",rules_str.c_str());
	}
	//========================================================
	
	//按规则画图
	if(weishu==2)//画二维文法图
	{
		LSDOT CurDot;
		stack<LSDOT> DotStack;

		CurDot.x=X_0;//设置初始坐标
		CurDot.y=Y_0;
		CurDot.xdeg=0;

		char *p=(char *)rules_str.c_str();

		HPEN hPen=CreatePen(PS_SOLID,1,RGB(255,255,255));
		SelectObject(hMemDC,hPen);

		MoveToEx(hMemDC,x0+CurDot.x,y0-CurDot.y,NULL);

		while(*p)
		{
			if(*p=='F')//向前走一步，画线(1个单位长度)
			{
				CurDot.x+=lenght*cos(CurDot.xdeg*PI/180);
				CurDot.y+=lenght*sin(CurDot.xdeg*PI/180);
				LineTo(hMemDC,x0+CurDot.x,y0-CurDot.y);
			}
			else if(*p=='L')
			{
			}
			else if(*p=='R')
			{
			}
			else if(*p=='+')//左转
			{
				string bufstr(p);
				int begin_index=bufstr.find_first_of('(');
				int end_index=bufstr.find_first_of(')');

				if(begin_index==1&&end_index>begin_index)//旋转自设角度
				{
					//取括号中的值
					bufstr.assign(p,begin_index+1,end_index-begin_index-1);
					double ang=strtod(bufstr.c_str(),&st);
					CurDot.xdeg+=ang;

					p+=end_index-begin_index+2;
					continue;
				}
				else//旋转默认角度
				{
					CurDot.xdeg+=angle;
				}
			
			}
			else if(*p=='-')//右转
			{
				string bufstr(p);
				int begin_index=bufstr.find_first_of('(');
				int end_index=bufstr.find_first_of(')');

				if(begin_index==1&&end_index>begin_index)//旋转自设角度
				{
					bufstr.assign(p,begin_index+1,end_index-begin_index-1);
					double ang=strtod(bufstr.c_str(),&st);
					CurDot.xdeg-=ang;

					p+=end_index-begin_index+2;
					continue;
				}
				else//旋转默认角度
				{
					CurDot.xdeg-=angle;
				}
			}
			else if(*p=='[')//压栈
			{
				DotStack.push(CurDot);
			
			}
			else if(*p==']')//出栈
			{
				if(DotStack.size()>0)
				{
					CurDot=DotStack.top();
					DotStack.pop();
					MoveToEx(hMemDC,x0+CurDot.x,y0-CurDot.y,NULL);
				}
			
			}
			else ;
			p++;
		}
	
	
	}
	else if(weishu==3)//画三维文法图
	{

	
	}

	//显示图像
	BitBlt(hDC,r.left,r.top,r.right-r.left,r.bottom-r.top,hMemDC,0,0,SRCCOPY);
end:
	ReleaseDC(MAIN_HANDLE,hDC);

	//启用绘图按钮
	HWND hDraw=GetDlgItem(MAIN_DIALOG,IDC_DRAW);
	EnableWindow(hDraw,TRUE);
	//启用擦除按钮
	HWND hClear=GetDlgItem(MAIN_DIALOG,IDC_CLEAR);
	EnableWindow(hClear,TRUE);
	
	return 1;
}
//T3对话框绘图线程
DWORD WINAPI OnDrawT3(LPVOID lpParam)
{
	RECT r;
	GetClientRect(MAIN_HANDLE,&r);//得到窗口客户端大小

	HDC hDC=GetDC(MAIN_HANDLE);//得到屏幕DC
	//擦除背景
	HBRUSH hbrush=CreateSolidBrush(BackGroundColor);
	FillRect(hMemDC,&r,hbrush);//填充擦除色

	//得到参数值
	//==================================
	double Xmax=2*(double)GetDlgItemInt(hT3,IDC_T3_XMAX,NULL,FALSE);//x轴比例
	double Ymax=2*(double)GetDlgItemInt(hT3,IDC_T3_YMAX,NULL,FALSE);//y轴比例
	double Zmax=2*(double)GetDlgItemInt(hT3,IDC_T3_ZMAX,NULL,FALSE);//z轴比例

	
	int P_Range=(int)GetDlgItemInt(hT3,IDC_EDT_T3_PMAX,NULL,FALSE);//参数P范围
	int Q_Range=(int)GetDlgItemInt(hT3,IDC_EDT_T3_QMAX,NULL,FALSE);//参数P范围

	int select;
	if(IsDlgButtonChecked(hT3,IDC_T3_RBN1)&&!IsDlgButtonChecked(hT3,IDC_T3_RBN2))
		select=1;
	else if(!IsDlgButtonChecked(hT3,IDC_T3_RBN1)&&IsDlgButtonChecked(hT3,IDC_T3_RBN2))
		select=2;
	else select=0;

	//========================================================
	double width=(double)r.right-r.left;
	double height=(double)r.bottom-r.top;
	double xr=(Xmax)/width;
	double yr=(Ymax)/height;
	double x0=(double)r.left+width/2;
	double y0=(double)r.top+height/2;
	double i,j;

	//画坐标
	for(i=0;i<height;i++)
		SetPixel(hMemDC,x0,int(i),RGB(255,255,255));
	
	for(j=0;j<width;j++)
		SetPixel(hMemDC,int(j),y0,RGB(255,255,255));

	//显示坐标
	BitBlt(hDC,r.left,r.top,r.right-r.left,r.bottom-r.top,hMemDC,0,0,SRCCOPY);
	
	//开始计算并绘制(在内存中绘制)
	//========================================================
	if(select==1)//表达式绘图
	{
		char gongshi[260];
		vector<TOKEN<double> > arrays;
		vector<TOKEN<double> > val(2);
		val[0].str="x";//变量x
		val[1].str="y";//变量y

		HWND hEdit=GetDlgItem(hT3,IDC_T3_GONGSHI);
		GetWindowText(hEdit,gongshi,260);//得到公式

		double result=0;
		string str=gongshi;
		if(gongshi[0]=='y'&&gongshi[1]=='='&&str.find_first_of('x',1)!=-1&&str.find_first_of('y',1)==-1)
		{
			if(!GongShi_Change(gongshi,val,arrays))//公式转换
			{
				MessageBox(MAIN_HANDLE,"无效公式!","错误",MB_OK);
				goto end;
			}
			for(j=-width/2;j<width/2;j+=0.05)
			{	
				val[0].num=j*xr;//x轴比例缩放

				if(!GongShi_Compute(arrays,val,result))//计算公式
				{
					MessageBox(MAIN_HANDLE,"无效公式!","错误",MB_OK);
					goto end;
		
				}
		
				SetPixel(hMemDC,int(x0+j),int(y0-result/yr),T3_Color);
				
			}
		}
		else if(gongshi[0]=='x'&&gongshi[1]=='='&&str.find_first_of('y',1)!=-1&&str.find_first_of('x',1)==-1)
		{
			if(!GongShi_Change(gongshi,val,arrays))//公式转换
			{
				MessageBox(MAIN_HANDLE,"无效公式!","错误",MB_OK);
				goto end;
			}
			for(i=-height/2;i<height/2;i+=0.05)
			{	
				val[1].num=i*yr;//x轴比例缩放

				if(!GongShi_Compute(arrays,val,result))//计算公式
				{
					MessageBox(MAIN_HANDLE,"无效公式!","错误",MB_OK);
					goto end;
		
				}
				SetPixel(hMemDC,int(x0+result/xr),int(y0+i),T3_Color);
				
			}
	
		}
		else//若=在公式中间，则转换公式，并用Newton矢代法求值来画
		{
			string gstr(gongshi);
			string s;
			
			int index=gstr.find_first_of('=');
			if(index>0)
			{
				gstr.at(index)='-';//转换公式，如:x+y=1 ==> x+y-1
				gstr.insert(index+1,"(");
				gstr+=")";
				if(!GongShi_Change((char *)gstr.c_str(),val,arrays))//公式转换,后缀表达式
				{
					MessageBox(MAIN_HANDLE,"无效公式!","错误",MB_OK);
					goto end;
				}

				double dd=height/Ymax;//两个逼近值之间的间隔点(根据y轴范围进行调整)
				double yd,y1,preyd;
				long n;
				
				//遍历x轴求y值
				for(j=-width/2;j<width/2;j+=0.05)
				{	
					val[0].num=j*xr;//x轴比例缩放
					for(i=-height/2;i<height/2;i+=dd)//隔点设置以逼近值(用于x对应y轴有多个值的情况)
					{
						y1=i*yr;//y轴逼近值y1
						n=0;
						preyd=height*yr;
						while(1)//Newton矢代求y值
						{
							val[1].num=y1;
							
							if(!GongShi_Compute(arrays,val,result))//计算公式
							{
								MessageBox(MAIN_HANDLE,"无效公式!","错误",MB_OK);
								goto end;
		
							}
							val[1].num=y1+0.00001;
							if(!GongShi_Compute(arrays,val,yd))//计算公式
							{
								MessageBox(MAIN_HANDLE,"无效公式!","错误",MB_OK);
								goto end;
		
							}
							result=y1-(result*0.00001)/(yd-result);//得结果

							if(fabs(preyd)<=fabs(result-y1))//若无解(若上次的差值小于这次的差值，则趋于无穷)
							{
								result=-1;
								break;
							}

							if(fabs(result-y1)<0.005)//若逼近零点，则求得解
							{
								result=y0-result/yr;//计算实际屏幕坐标
								break;
						
							}
							preyd=result-y1;//保存上次的差值

							y1=result;
							
						}
						if(result>=0&&result<height)
						{
							SetPixel(hDC,int(x0+j),int(result),T3_Color);
							SetPixel(hMemDC,int(x0+j),int(result),T3_Color);
						}
						
					}
					SetPixel(hDC,int(x0+j),0,RGB(255,255,0));//显示进度
				}
			
			}
			else
			{
				MessageBox(MAIN_HANDLE,"无效公式!","错误",MB_OK);
				goto end;
			
			}

		
		}
	}
	else if(select==2)//参数公式绘图
	{
		
		//若为2维
		if(IsDlgButtonChecked(hT3,IDC_RBN_T3_2)&&!IsDlgButtonChecked(hT3,IDC_RBN_T3_3))
		{
			char gongshiX[260];
			char gongshiY[260];
			vector<TOKEN<double> > arraysX;
			vector<TOKEN<double> > arraysY;
			vector<TOKEN<double> > val(1);
			val[0].str="p";//变量p
		
			HWND hEditX=GetDlgItem(hT3,IDC_T3_EDIT_X);
			HWND hEditY=GetDlgItem(hT3,IDC_T3_EDIT_Y);
			GetWindowText(hEditX,gongshiX,260);//得到x轴公式
			GetWindowText(hEditY,gongshiY,260);//得到y轴公式

			if(!GongShi_Change(gongshiX,val,arraysX))//公式转换
			{
				MessageBox(MAIN_HANDLE,"无效公式!","错误",MB_OK);
				goto end;
			}
			if(!GongShi_Change(gongshiY,val,arraysY))//公式转换
			{
				MessageBox(MAIN_HANDLE,"无效公式!","错误",MB_OK);
				goto end;
			}
			double resultX=0;
			double resultY=0;

			for(j=-P_Range;j<P_Range;j+=0.005)
			{	
				val[0].num=j;

				if(!GongShi_Compute(arraysX,val,resultX))//计算x轴公式
				{
					MessageBox(MAIN_HANDLE,"无效公式!","错误",MB_OK);
					goto end;
		
				}
				if(!GongShi_Compute(arraysY,val,resultY))//计算y轴公式
				{
					MessageBox(MAIN_HANDLE,"无效公式!","错误",MB_OK);
					goto end;
		
				}
				//在有效区域内绘制
				if(resultX/xr>(-width/2)&&resultX<(width/2)&&resultY/yr>(-height/2)&&resultY<(height/2))
				SetPixel(hMemDC,int(x0+resultX/xr),int(y0-resultY/yr),T3_Color);
			
			}
		}
		else if(!IsDlgButtonChecked(hT3,IDC_RBN_T3_2)&&IsDlgButtonChecked(hT3,IDC_RBN_T3_3))//若为3维
		{
			char gongshiX[260];
			char gongshiY[260];
			char gongshiZ[260];
			vector<TOKEN<double> > arraysX;
			vector<TOKEN<double> > arraysY;
			vector<TOKEN<double> > arraysZ;
			vector<TOKEN<double> > val(2);
			val[0].str="p";//变量p
			val[1].str="q";//变量q
		
			HWND hEditX=GetDlgItem(hT3,IDC_T3_EDIT_X);
			HWND hEditY=GetDlgItem(hT3,IDC_T3_EDIT_Y);
			HWND hEditZ=GetDlgItem(hT3,IDC_T3_EDIT_Z);
			GetWindowText(hEditX,gongshiX,260);//得到x轴公式
			GetWindowText(hEditY,gongshiY,260);//得到y轴公式
			GetWindowText(hEditZ,gongshiZ,260);//得到z轴公式

			//画y轴坐标
			for(i=-width/2;i<width/2;i++)
				SetPixel(hMemDC,int(x0+i),int(y0-i),RGB(255,255,255));
			//显示坐标
			BitBlt(hDC,r.left,r.top,r.right-r.left,r.bottom-r.top,hMemDC,0,0,SRCCOPY);

			
			if(!GongShi_Change(gongshiX,val,arraysX))//公式转换
			{
				MessageBox(MAIN_HANDLE,"无效公式!","错误",MB_OK);
				goto end;
			}
			if(!GongShi_Change(gongshiY,val,arraysY))//公式转换
			{
				MessageBox(MAIN_HANDLE,"无效公式!","错误",MB_OK);
				goto end;
			}
			if(!GongShi_Change(gongshiZ,val,arraysZ))//公式转换
			{
				MessageBox(MAIN_HANDLE,"无效公式!","错误",MB_OK);
				goto end;
			}
			double resultX=0;
			double resultY=0;
			double resultZ=0;
			double xx,yy;

			for(i=-P_Range;i<P_Range;i+=0.025)
			for(j=-Q_Range;j<Q_Range;j+=0.025)
			{	
				val[0].num=i;
				val[1].num=j;
				
				if(!GongShi_Compute(arraysX,val,resultX))//计算x轴公式
				{
					MessageBox(MAIN_HANDLE,"无效公式!","错误",MB_OK);
					goto end;
		
				}
				if(!GongShi_Compute(arraysY,val,resultY))//计算y轴公式
				{
					MessageBox(MAIN_HANDLE,"无效公式!","错误",MB_OK);
					goto end;
		
				}
				if(!GongShi_Compute(arraysZ,val,resultZ))//计算z轴公式
				{
					MessageBox(MAIN_HANDLE,"无效公式!","错误",MB_OK);
					goto end;
		
				}
		
				resultX=resultX+0.7071067811865*resultY;
				resultY=resultZ+0.7071067811865*resultY;
				xx=resultX/xr;
				yy=resultY/yr;
				//在有效区域内绘制
			
				if(xx>(-width/2)&&resultX<(width/2)&&yy>(-height/2)&&resultY<(height/2))
				{
					SetPixel(hMemDC,int(x0+xx),int(y0-yy),T3_Color);
					SetPixel(hDC,int(x0+xx),int(y0-yy),T3_Color);
				}
				
			
			}
			
		}
		else ;
	
	
	}
	else MessageBox(MAIN_HANDLE,"请选择公式类型公式!","错误",MB_OK);
	//========================================================
end:
	//显示图像
	BitBlt(hDC,r.left,r.top,r.right-r.left,r.bottom-r.top,hMemDC,0,0,SRCCOPY);

	ReleaseDC(MAIN_HANDLE,hDC);

	//启用绘图按钮
	HWND hDraw=GetDlgItem(MAIN_DIALOG,IDC_DRAW);
	EnableWindow(hDraw,TRUE);
	//启用擦除按钮
	HWND hClear=GetDlgItem(MAIN_DIALOG,IDC_CLEAR);
	EnableWindow(hClear,TRUE);
	
	return 1;
}
//T4对话框绘图线程
DWORD WINAPI OnDrawT4(LPVOID lpParam)
{
	RECT r;
	GetClientRect(MAIN_HANDLE,&r);//得到窗口客户端大小

	HDC hDC=GetDC(MAIN_HANDLE);//得到屏幕DC

	//擦除背景
	HBRUSH hbrush=CreateSolidBrush(BackGroundColor);
	FillRect(hDC,&r,hbrush);//填充擦除色
	FillRect(hMemDC,&r,hbrush);//填充擦除色

	//开始计算并绘制(在内存中绘制)
	//========================================================
	double width=r.right-r.left;
	double height=r.bottom-r.top;
	double x0=width/2;
	double y0=height/2;
	double xr,yr;
	//========================================================
	//得到参数
	vector<IFS > ifs_array;
	int i;

	HWND hListView=GetDlgItem(hT4,IDC_T4_LISTVIEW);
	int itemcounts=ListView_GetItemCount(hListView);//得到当前项数

	ifs_array.resize(itemcounts);
	char buffer[256];
	char *st;
	double ps=0;
	for(i=0;i<itemcounts;i++)
	{

			//得到当前项文本进行初始化编辑框
			ListView_GetItemText(hListView,i,1,buffer,256);
			ifs_array[i].a=strtod(buffer,&st);
			ListView_GetItemText(hListView,i,2,buffer,256);
			ifs_array[i].b=strtod(buffer,&st);
			ListView_GetItemText(hListView,i,3,buffer,256);
			ifs_array[i].c=strtod(buffer,&st);
			ListView_GetItemText(hListView,i,4,buffer,256);
			ifs_array[i].d=strtod(buffer,&st);
			ListView_GetItemText(hListView,i,5,buffer,256);
			ifs_array[i].e=strtod(buffer,&st);
			ListView_GetItemText(hListView,i,6,buffer,256);
			ifs_array[i].f=strtod(buffer,&st);
			ListView_GetItemText(hListView,i,7,buffer,256);
			ifs_array[i].p=strtod(buffer,&st);

			ifs_array[i].psum=ps;//保存概率空间起始值
			ps+=ifs_array[i].p;

			// 得到颜色
			ListView_GetItemText(hListView,i,8,buffer,256);
			ifs_array[i].color=atol(buffer);
			
	}
	//得到迭代次数
	HWND hN=GetDlgItem(hT4,IDC_T4_N);
	GetWindowText(hN,buffer,256);
	long n=atol(buffer);
	//得到x,y轴最大值
	HWND hXmax=GetDlgItem(hT4,IDC_T4_XMAX);
	GetWindowText(hXmax,buffer,256);
	double xmax=strtod(buffer,&st);
	HWND hYmax=GetDlgItem(hT4,IDC_T4_YMAX);
	GetWindowText(hYmax,buffer,256);
	double ymax=strtod(buffer,&st);

	//计算缩放比例
	xr=xmax/width;
	yr=ymax/height;

	HWND hX0=GetDlgItem(hT4,IDC_T4_X0);//得到坐标偏移
	GetWindowText(hX0,buffer,256);
	double xd=strtod(buffer,&st);
	HWND hY0=GetDlgItem(hT4,IDC_T4_Y0);
	GetWindowText(hY0,buffer,256);
	double yd=strtod(buffer,&st);

	//========================================================
	//画图
	double pr;//随机量
	double x=0,y=0,newx,newy;
	srand(time(0));
	while((n--)>0)
	{
		pr=(double)rand()/RAND_MAX;//产生0-1的随机量
		
		for(i=0;i<ifs_array.size();i++)
		{	
			//若在该概率空间内，则绘制该ifs变换
			if(pr>=ifs_array[i].psum&&pr<(ifs_array[i].psum+ifs_array[i].p))
			{
				newx=ifs_array[i].a*x+ifs_array[i].b*y+ifs_array[i].e;
				newy=ifs_array[i].c*x+ifs_array[i].d*y+ifs_array[i].f;

				x=newx;
				y=newy;
				
				if((int)(x0+(xd+x)/xr)>=0&&(int)(x0+(xd+x)/xr)<=width&&\
					(int)(y0-(yd+y)/yr)>=0&&(int)(y0-(yd+y)/yr)<=height)
				{
					SetPixel(hMemDC,(int)(x0+(xd+x)/xr),(int)(y0-(yd+y)/yr),ifs_array[i].color);
				}

				break;
			
			}
		
		}
	}
	//========================================================

	//显示图像
	BitBlt(hDC,r.left,r.top,r.right-r.left,r.bottom-r.top,hMemDC,0,0,SRCCOPY);

	ReleaseDC(MAIN_HANDLE,hDC);

	//启用绘图按钮
	HWND hDraw=GetDlgItem(MAIN_DIALOG,IDC_DRAW);
	EnableWindow(hDraw,TRUE);
	//启用擦除按钮
	HWND hClear=GetDlgItem(MAIN_DIALOG,IDC_CLEAR);
	EnableWindow(hClear,TRUE);
	
	return 1;
}


typedef struct DOT_COLOR_TAG
{
	int color;//颜色
	int grade;//灰度值

}DOT_COLOR;

//T10对话框绘图线程
DWORD WINAPI OnDrawT10(LPVOID lpParam)
{
	RECT r;
	GetClientRect(MAIN_HANDLE,&r);//得到窗口客户端大小

	HDC hDC=GetDC(MAIN_HANDLE);//得到屏幕DC

	//擦除背景
	HBRUSH hbrush=CreateSolidBrush(BackGroundColor);
	FillRect(hDC,&r,hbrush);//填充擦除色

	//开始计算并绘制(在内存中绘制)
	//========================================================
	double width=r.right-r.left;
	double height=r.bottom-r.top;
	double x,y,z,w,b=0.999,a=0;
	int i,j,n=0,color=0;
	
	//========================================================
	//得到参数


	//========================================================
	//DebugTestException();
	//开始绘图
	vector<vector<DOT_COLOR> > dots;//用于保存屏幕已画得像素点
	vector<vector<COLORREF> > ColorArray;//色彩数组
	int R,G,B;

	ColorArray.resize(192);
	//调色，192种颜色渐变
	for(i=0;i<192;i++)
	{
		if(i<32)
		{
			R=255;
			G=i*8;
			B=0;
		}
		else if(i<64&&i>=32)
		{
			R=255-(i-32)*8;
			G=255;
			B=0;
		
		}
		else if(i<96&&i>=64)
		{
			R=0;
			G=255;
			B=(i-64)*8;
		
		}
		else if(i<128&&i>=96)
		{
			R=0;
			G=255-(i-96)*8;
			B=255;
		
		}
		else if(i<160&&i>=128)
		{
			R=(i-128)*8;
			G=0;
			B=255;
		
		}
		else if(i<192&&i>=160)
		{
			R=255;
			G=0;
			B=255-(i-160)*8;
		
		}
		ColorArray[i].resize(128);
		ColorArray[i][0]=RGB(R,G,B);
		for(j=0;j<128;j++)
		{
			ColorArray[i][127-j]=RGB((((0-R)*j*2)>>8)+R,(((0-G)*j*2)>>8)+G,(((0-B)*j*2)>>8)+B);
			
		}
		//测试调色板
		//for(j=0;j<128;j++)
		//SetPixel(hDC,i,100+j,ColorArray[i][j]);
	
	}
	//初始化屏幕色彩点
	dots.resize(height);
	for(i=0;i<dots.size();i++)
	{
		dots[i].resize(width);
		for(j=0;j<dots[i].size();j++)
		{
			dots[i][j].color=-1;
			dots[i][j].grade=127;
		}
	}
	color=0;
	while(1)
	{
	if(a>=1) 
	{
		a=0;
		b=0.5+(double)rand()/(RAND_MAX);
		//b=1.011;
		
	}
			
	/*if(!(int(a*10000)%100))
	{
		b-=0.005;
		if(b<=0)b=0.999;
	}*/
	
	
	a+=0.0005;
	
	x=0.004;
	y=0.004;
	w=1;
	//================
	//实现色彩点逐渐消失的效果，并画出保存的点
	int size=dots.size();
	for(i=0;i<height;i++)
		for(j=0;j<width;j++)
		{
			if(dots[i][j].color>=0)
			{
				if(--dots[i][j].grade>=0)//递减亮度
				{
					SetPixel(hMemDC,j,i,ColorArray[dots[i][j].color][dots[i][j].grade]);

				}
				else //直到消失
				{
					dots[i][j].color=-1;
					dots[i][j].grade=127;
				
				}
			}
		}

	//================
	color++;//色彩循环
	if(color==ColorArray.size())color=0;

	for(n=0;n<100;n++)//迭代过程
	{
		z=x;
		x=b*y+w;
		//w=a*x+(2*(1-a)*x*x)/(1+x*x);
		w=-0.05*a*x+a*(PI-a*x)*x*x/(1+x*x);
		//w=SIGN(x)*(a*x+2*(1-a)*x*x/(1+x*x))+sin(a+x);
		//w=2*PI*a*5*SIGN(x)+PI+cos(x)+sin(PI+sin(PI+x));
		
		y=w-z;

		//计算屏幕坐标点
		int hh=(int)(height/2-y*10);
		int ww=(int)(width/2+x*10);

		if(ww>0&&ww<width&&hh>0&&hh<height)
		{
			//保存该点
			dots[hh][ww].color=color;
			dots[hh][ww].grade=127;
			
		}
	
	}
	BitBlt(hDC,r.left,r.top,(int)width,(int)height,hMemDC,0,0,SRCCOPY);
	}
	//========================================================
	

	//显示图像
	//BitBlt(hDC,r.left,r.top,r.right-r.left,r.bottom-r.top,hMemDC,0,0,SRCCOPY);

	ReleaseDC(MAIN_HANDLE,hDC);

	//启用绘图按钮
	HWND hDraw=GetDlgItem(MAIN_DIALOG,IDC_DRAW);
	EnableWindow(hDraw,TRUE);
	//启用擦除按钮
	HWND hClear=GetDlgItem(MAIN_DIALOG,IDC_CLEAR);
	EnableWindow(hClear,TRUE);
	
	return 1;
}
WNDPROC OldAboutStaticMsgProc=NULL;
//T3对话框颜色静态控件新消息处理函数
LRESULT MyAboutStaticMsgProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_PAINT:
		{
			PAINTSTRUCT p;
			HDC hDC=BeginPaint(hWnd,&p);
			//填充控件位图
			HBITMAP hbitmap=LoadBitmap(MAIN_INSTANCE,MAKEINTRESOURCE(IDB_APP_BITMAP));
			HBRUSH hbrush=CreatePatternBrush(hbitmap);
			/*HDC hmemdc=CreateCompatibleDC(hDC);

			RECT cr;
			GetClientRect(hWnd,&cr);

			HBITMAP bm;//创建内存兼容位图,否则只能是单色兼容DC
			bm=CreateCompatibleBitmap(hDC,cr.right-cr.left,cr.bottom-cr.top);
			SelectObject(hmemdc,bm);

			FillRect(hmemdc,&cr,hbrush);
			BitBlt(hDC,0,0,cr.right-cr.left,cr.bottom-cr.top,hmemdc,0,0,SRCPAINT);*/
			RECT cr;
			GetClientRect(hWnd,&cr);
			FillRect(hDC,&cr,hbrush);

			EndPaint(hWnd,&p);
			
			return TRUE;
		}
		break;

	}
	//调用控件原来的消息处理函数
	return CallWindowProc(OldT3ColorMsgProc,hWnd,uMsg,wParam,lParam);
}
//关于对话框消息处理
BOOL WINAPI DlgAboutProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			HICON hicon=LoadIcon(MAIN_INSTANCE,MAKEINTRESOURCE(IDI_APP));
			//设置图标，两种方法
			//SetClassLong(hWnd,GCL_HICON,(long)hicon);
			SendMessage(hWnd,WM_SETICON,ICON_BIG,(LPARAM)hicon);

			HWND hStatic=GetDlgItem(hWnd,IDC_APP_ICON);
			//嵌入新的静态控件消息处理函数
			OldAboutStaticMsgProc=(WNDPROC)SetWindowLong(hStatic,GWL_WNDPROC,(LONG)MyAboutStaticMsgProc);
			return TRUE;
		}
		break;
	case WM_CLOSE://鼠标左键消息
		{
			EndDialog(hWnd,NULL);
			return TRUE;
		}
		break;
	default:
		return FALSE;
	}
	return FALSE;
	
}
//Tab子对话框T0消息处理
BOOL WINAPI DlgT0(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	HWND hReal_w=GetDlgItem(hWnd,IDC_T0_C_REAL);
	HWND hImag_i=GetDlgItem(hWnd,IDC_T0_C_IMAG);
	HWND hImag_j=GetDlgItem(hWnd,IDC_T0_C_IMAG_J);
	HWND hImag_k=GetDlgItem(hWnd,IDC_T0_C_IMAG_K);
	HWND hMandelbrot=GetDlgItem(hWnd,IDC_RBN_T0_MANDELBROT);
	HWND hNewton=GetDlgItem(hWnd,IDC_RBN_T0_NEWTON);

	//得到配置文件路径
	char filename[260];
	string buffer;
	GetModuleFileName(NULL,filename,260);
	buffer=filename;
	buffer.assign(buffer,0,buffer.find_last_of('\\'));
	buffer+="\\Fractal.ini";

	int Ar,Ag,Ab,Br,Bg,Bb;
	unsigned long Rmax,Nmax;

	switch(uMsg)
	{
	case WM_INITDIALOG://打开程序时，列出所有保存的公式
		{
			//=================================================
			//初始化T0对话框
			//=================================================
			//设置列表框
			char inibuffer[1000];
			//得到所有节名到inibuffer,每个节名以0间隔，最后以2个0结束
			if(GetPrivateProfileSectionNames(inibuffer,1000,buffer.c_str()))//若打开文件成功
			{
				char *p=inibuffer;
			
				while(*(p))
				{
					SendDlgItemMessage(hWnd,IDC_T0_LIST,LB_ADDSTRING,0,(LPARAM)p);//保存到列表框
					p+=strlen(p)+1;//递增到下一个节名
				
				}
			}

			//设置微调旋钮(关联编辑框)
			HWND hUdn_Rmax=GetDlgItem(hWnd,IDC_UDN_T0_RMAX);//RMAX
			HWND hRmax=GetDlgItem(hWnd,IDC_T0_RMAX);
			SendMessage(hUdn_Rmax,UDM_SETBUDDY,(WPARAM)hRmax,(LPARAM)0);
			SendMessage(hUdn_Rmax,UDM_SETRANGE,(WPARAM)0,(LPARAM)10000);

			HWND hUdn_Nmax=GetDlgItem(hWnd,IDC_UDN_T0_NMAX);//NMAX
			HWND hNmax=GetDlgItem(hWnd,IDC_T0_NMAX);
			SendMessage(hUdn_Nmax,UDM_SETBUDDY,(WPARAM)hNmax,(LPARAM)0);
			SendMessage(hUdn_Nmax,UDM_SETRANGE,(WPARAM)0,(LPARAM)10000);

			HWND hUdn_Ar=GetDlgItem(hWnd,IDC_UDN_T0_AR);//Ar
			HWND hAr=GetDlgItem(hWnd,IDC_T0_AR);
			SendMessage(hUdn_Ar,UDM_SETBUDDY,(WPARAM)hAr,(LPARAM)0);
			SendMessage(hUdn_Ar,UDM_SETRANGE,(WPARAM)0,(LPARAM)255);

			HWND hUdn_Br=GetDlgItem(hWnd,IDC_UDN_T0_BR);//Br
			HWND hBr=GetDlgItem(hWnd,IDC_T0_BR);
			SendMessage(hUdn_Br,UDM_SETBUDDY,(WPARAM)hBr,(LPARAM)0);
			SendMessage(hUdn_Br,UDM_SETRANGE,(WPARAM)0,(LPARAM)255);

			HWND hUdn_Ag=GetDlgItem(hWnd,IDC_UDN_T0_AG);//Ag
			HWND hAg=GetDlgItem(hWnd,IDC_T0_AG);
			SendMessage(hUdn_Ag,UDM_SETBUDDY,(WPARAM)hAg,(LPARAM)0);
			SendMessage(hUdn_Ag,UDM_SETRANGE,(WPARAM)0,(LPARAM)255);

			HWND hUdn_Bg=GetDlgItem(hWnd,IDC_UDN_T0_BG);//Bg
			HWND hBg=GetDlgItem(hWnd,IDC_T0_BG);
			SendMessage(hUdn_Bg,UDM_SETBUDDY,(WPARAM)hBg,(LPARAM)0);
			SendMessage(hUdn_Bg,UDM_SETRANGE,(WPARAM)0,(LPARAM)255);

			HWND hUdn_Ab=GetDlgItem(hWnd,IDC_UDN_T0_AB);//Ab
			HWND hAb=GetDlgItem(hWnd,IDC_T0_AB);
			SendMessage(hUdn_Ab,UDM_SETBUDDY,(WPARAM)hAb,(LPARAM)0);
			SendMessage(hUdn_Ab,UDM_SETRANGE,(WPARAM)0,(LPARAM)255);

			HWND hUdn_Bb=GetDlgItem(hWnd,IDC_UDN_T0_BB);//Bb
			HWND hBb=GetDlgItem(hWnd,IDC_T0_BB);
			SendMessage(hUdn_Bb,UDM_SETBUDDY,(WPARAM)hBb,(LPARAM)0);
			SendMessage(hUdn_Bb,UDM_SETRANGE,(WPARAM)0,(LPARAM)255);

			//=================================================
			//设置单选按钮初始状态
			CheckDlgButton(hWnd,IDC_RBN_T0_MANDELBROT,BST_CHECKED);//Mandelbrot
			CheckDlgButton(hWnd,IDC_RBN_T0_2,BST_CHECKED);//使用二维单选按钮
			//初始禁用常数c的编辑框
			EnableWindow(hReal_w,FALSE);
			EnableWindow(hImag_i,FALSE);
			EnableWindow(hImag_j,FALSE);
			EnableWindow(hImag_k,FALSE);
			//==================================================
			//设置编辑框初值
			HWND hGongshi=GetDlgItem(hWnd,IDC_T0_GONGSHI_EDT);
			SetWindowText(hGongshi,"z*z+c");//公式
			SetWindowText(hRmax,"50");//逃逸半径
			SetWindowText(hNmax,"50");//逃逸时间
			SetWindowText(hAr,"10");//颜色参数
			SetWindowText(hBr,"5");
			SetWindowText(hAg,"10");
			SetWindowText(hBg,"10");
			SetWindowText(hAb,"16");
			SetWindowText(hBb,"10");
			SetWindowText(hReal_w,"0");//常数C
			SetWindowText(hImag_i,"1");
			SetWindowText(hImag_j,"0");
			SetWindowText(hImag_k,"0");
			
			
			return TRUE;
		}
		break;
	case WM_COMMAND:
		{
			int ControlID=LOWORD(wParam);//控件ID
			int ControlNotify=HIWORD(wParam);//控件通知码
			HWND ControlHandle=(HWND)lParam;//控件句柄

			if(ControlID==IDC_T0_SAVE)//保存公式
			{
				char gongshi[260];
				HWND hEdit=GetDlgItem(hWnd,IDC_T0_GONGSHI_EDT);
				GetWindowText(hEdit,gongshi,260);//得到公式

				SendDlgItemMessage(hWnd,IDC_T0_LIST,LB_ADDSTRING,0,(LPARAM)gongshi);//保存到列表框

				//得到参数
				//========================================================
				//颜色参数
				char canshu[10];
				Ar=(int)GetDlgItemInt(hT0,IDC_T0_AR,NULL,FALSE);
				Ag=(int)GetDlgItemInt(hT0,IDC_T0_AG,NULL,FALSE);
				Ab=(int)GetDlgItemInt(hT0,IDC_T0_AB,NULL,FALSE);
				Br=(int)GetDlgItemInt(hT0,IDC_T0_BR,NULL,FALSE);
				Bg=(int)GetDlgItemInt(hT0,IDC_T0_BG,NULL,FALSE);
				Bb=(int)GetDlgItemInt(hT0,IDC_T0_BB,NULL,FALSE);

				Rmax=(unsigned long)GetDlgItemInt(hT0,IDC_T0_RMAX,NULL,FALSE);//Rmax
				Nmax=(unsigned long)GetDlgItemInt(hT0,IDC_T0_NMAX,NULL,FALSE);//Nmax
				
				//选择绘制的集合
				int select;
				if(IsDlgButtonChecked(hT0,IDC_RBN_T0_MANDELBROT)\
				&&!IsDlgButtonChecked(hT0,IDC_RBN_T0_JULIA)\
				&&!IsDlgButtonChecked(hT0,IDC_RBN_T0_NEWTON))
				{
					select=FRACTAL_T0_MANDELBROT;//Mandelbrot
				}
				else if(!IsDlgButtonChecked(hT0,IDC_RBN_T0_MANDELBROT)\
				&&!IsDlgButtonChecked(hT0,IDC_RBN_T0_NEWTON)\
				&&IsDlgButtonChecked(hT0,IDC_RBN_T0_JULIA))
				{
					select=FRACTAL_T0_JULIA;//Julia
				}
				else if(!IsDlgButtonChecked(hT0,IDC_RBN_T0_MANDELBROT)\
				&&!IsDlgButtonChecked(hT0,IDC_RBN_T0_JULIA)\
				&&IsDlgButtonChecked(hT0,IDC_RBN_T0_NEWTON))
				{
					select=FRACTAL_T0_NEWTON;//Newton
				}
				else select=0;

				//常数C,B
				char real[20],imag_i[20],imag_j[20],imag_k[20];
				GetWindowText(hReal_w,real,20);
				GetWindowText(hImag_i,imag_i,20);
				GetWindowText(hImag_j,imag_j,20);
				GetWindowText(hImag_k,imag_k,20);

				//得到维数
				int weishu=2;
				if(IsDlgButtonChecked(hWnd,IDC_RBN_T0_3))weishu=3;
				

				//写入配置文件
				sprintf(canshu,"%d",weishu);//维数
				WritePrivateProfileString(gongshi,"WeiShu",canshu,buffer.c_str());

				sprintf(canshu,"%d",select);//选择集合
				WritePrivateProfileString(gongshi,"Select",canshu,buffer.c_str());

				WritePrivateProfileString(gongshi,"C_Real",real,buffer.c_str());//常数C
				WritePrivateProfileString(gongshi,"C_Imag_i",imag_i,buffer.c_str());
				WritePrivateProfileString(gongshi,"C_Imag_j",imag_j,buffer.c_str());
				WritePrivateProfileString(gongshi,"C_Imag_k",imag_k,buffer.c_str());

				sprintf(canshu,"%d",Ar);//颜色
				WritePrivateProfileString(gongshi,"Ar",canshu,buffer.c_str());
				sprintf(canshu,"%d",Ag);
				WritePrivateProfileString(gongshi,"Ag",canshu,buffer.c_str());
				sprintf(canshu,"%d",Ab);
				WritePrivateProfileString(gongshi,"Ab",canshu,buffer.c_str());
				sprintf(canshu,"%d",Br);
				WritePrivateProfileString(gongshi,"Br",canshu,buffer.c_str());
				sprintf(canshu,"%d",Bg);
				WritePrivateProfileString(gongshi,"Bg",canshu,buffer.c_str());
				sprintf(canshu,"%d",Bb);
				WritePrivateProfileString(gongshi,"Bb",canshu,buffer.c_str());

				sprintf(canshu,"%d",Rmax);//逃逸半径与时间
				WritePrivateProfileString(gongshi,"Rmax",canshu,buffer.c_str());
				sprintf(canshu,"%d",Nmax);
				WritePrivateProfileString(gongshi,"Nmax",canshu,buffer.c_str());

			}
			else if(ControlID==IDC_T0_CLEAR)//清除公式
			{
				int sel=SendDlgItemMessage(hWnd,IDC_T0_LIST,LB_GETCURSEL,0,0);//得到当前索引

				if(sel>=0)
				{
					//从配置文件中清除该公式
					char gongshi[260];
					SendDlgItemMessage(hWnd,IDC_T0_LIST,LB_GETTEXT,(WPARAM)sel,(LPARAM)gongshi);
					WritePrivateProfileString(gongshi,0,0,buffer.c_str());
	
					SendDlgItemMessage(hWnd,IDC_T0_LIST,LB_DELETESTRING,(WPARAM)sel,0);//从列表框中清除公式
				}
			
			}
			else if(ControlID==IDC_T0_CLEAN)//清空公式列表框
			{
				
				int selcounts=SendDlgItemMessage(hWnd,IDC_T0_LIST,LB_GETCOUNT,0,0);//得到列表框中项数
				for(int i=0;i<selcounts;i++)//循环遍历清除小节
				{
					char str[260];
					SendDlgItemMessage(hWnd,IDC_T0_LIST,LB_GETTEXT,(WPARAM)i,(LPARAM)str);
					WritePrivateProfileString(str,NULL,NULL,buffer.c_str());
				
				}
				SendDlgItemMessage(hWnd,IDC_T0_LIST,LB_RESETCONTENT,0,0);//从列表框中清空公式
			
			}
			else if(ControlID==IDC_T0_LIST)//若为列表框
			{
				if(ControlNotify==LBN_DBLCLK)//若为双击
				{
					char gongshi[260];
					int sel=SendMessage(ControlHandle,LB_GETCURSEL,0,0);//得到当前索引
					SendMessage(ControlHandle,LB_GETTEXT,(WPARAM)sel,(LPARAM)gongshi);//得到公式
			
					HWND hEdit=GetDlgItem(hWnd,IDC_T0_GONGSHI_EDT);
					SetWindowText(hEdit,gongshi);//将公式设回编辑框

					//得到参数，并设回编辑框
					int color;
					color=GetPrivateProfileInt(gongshi,"Ar",0,buffer.c_str());
					SetDlgItemInt(hWnd,IDC_T0_AR,color,FALSE);
					color=GetPrivateProfileInt(gongshi,"Ag",0,buffer.c_str());
					SetDlgItemInt(hWnd,IDC_T0_AG,color,FALSE);
					color=GetPrivateProfileInt(gongshi,"Ab",0,buffer.c_str());
					SetDlgItemInt(hWnd,IDC_T0_AB,color,FALSE);
					color=GetPrivateProfileInt(gongshi,"Br",0,buffer.c_str());
					SetDlgItemInt(hWnd,IDC_T0_BR,color,FALSE);
					color=GetPrivateProfileInt(gongshi,"Bg",0,buffer.c_str());
					SetDlgItemInt(hWnd,IDC_T0_BG,color,FALSE);
					color=GetPrivateProfileInt(gongshi,"Bb",0,buffer.c_str());
					SetDlgItemInt(hWnd,IDC_T0_BB,color,FALSE);

					Rmax=GetPrivateProfileInt(gongshi,"Rmax",0,buffer.c_str());
					SetDlgItemInt(hWnd,IDC_T0_RMAX,Rmax,FALSE);
					Nmax=GetPrivateProfileInt(gongshi,"Nmax",0,buffer.c_str());
					SetDlgItemInt(hWnd,IDC_T0_NMAX,Nmax,FALSE);

					//设置常数c,b的编辑框
					char real[20],imag_i[20],imag_j[20],imag_k[20];
					GetPrivateProfileString(gongshi,"C_Real","0",real,20,buffer.c_str());
					GetPrivateProfileString(gongshi,"C_Imag_i","0",imag_i,20,buffer.c_str());
					GetPrivateProfileString(gongshi,"C_Imag_j","0",imag_j,20,buffer.c_str());
					GetPrivateProfileString(gongshi,"C_Imag_k","0",imag_k,20,buffer.c_str());
					SetWindowText(hReal_w,real);
					SetWindowText(hImag_i,imag_i);
					SetWindowText(hImag_j,imag_j);
					SetWindowText(hImag_k,imag_k);

					int weishu=GetPrivateProfileInt(gongshi,"WeiShu",2,buffer.c_str());
					if(weishu==3)
					{
						EnableWindow(hReal_w,TRUE);
						EnableWindow(hImag_i,TRUE);
						EnableWindow(hImag_j,TRUE);
						EnableWindow(hImag_k,TRUE);
						EnableWindow(hMandelbrot,FALSE);
						EnableWindow(hNewton,FALSE);
						CheckDlgButton(hWnd,IDC_RBN_T0_JULIA,BST_CHECKED);
						CheckDlgButton(hWnd,IDC_RBN_T0_MANDELBROT,BST_UNCHECKED);
						CheckDlgButton(hWnd,IDC_RBN_T0_NEWTON,BST_UNCHECKED);
						CheckDlgButton(hWnd,IDC_RBN_T0_3,BST_CHECKED);
						CheckDlgButton(hWnd,IDC_RBN_T0_2,BST_UNCHECKED);
					
					}
					else
					{
						EnableWindow(hReal_w,TRUE);
						EnableWindow(hImag_i,TRUE);
						EnableWindow(hImag_j,FALSE);
						EnableWindow(hImag_k,FALSE);
						EnableWindow(hMandelbrot,TRUE);
						EnableWindow(hNewton,TRUE);
						CheckDlgButton(hWnd,IDC_RBN_T0_2,BST_CHECKED);
						CheckDlgButton(hWnd,IDC_RBN_T0_3,BST_UNCHECKED);
					
					
					}
			

					//选择绘制的集合
					int select=GetPrivateProfileInt(gongshi,"Select",1,buffer.c_str());
					if(select==FRACTAL_T0_MANDELBROT)//Mandelbrot
					{
						CheckDlgButton(hWnd,IDC_RBN_T0_MANDELBROT,BST_CHECKED);
						CheckDlgButton(hWnd,IDC_RBN_T0_JULIA,BST_UNCHECKED);
						CheckDlgButton(hWnd,IDC_RBN_T0_NEWTON,BST_UNCHECKED);
						EnableWindow(hReal_w,FALSE);
						EnableWindow(hImag_i,FALSE);
					}
					else if(select==FRACTAL_T0_JULIA)//Julia
					{
						CheckDlgButton(hWnd,IDC_RBN_T0_MANDELBROT,BST_UNCHECKED);
						CheckDlgButton(hWnd,IDC_RBN_T0_NEWTON,BST_UNCHECKED);
						CheckDlgButton(hWnd,IDC_RBN_T0_JULIA,BST_CHECKED);
						EnableWindow(hReal_w,TRUE);
						EnableWindow(hImag_i,TRUE);
					}
					else if(select==FRACTAL_T0_NEWTON)//Newton
					{
						CheckDlgButton(hWnd,IDC_RBN_T0_MANDELBROT,BST_UNCHECKED);
						CheckDlgButton(hWnd,IDC_RBN_T0_JULIA,BST_UNCHECKED);
						CheckDlgButton(hWnd,IDC_RBN_T0_NEWTON,BST_CHECKED);
						EnableWindow(hReal_w,TRUE);
						EnableWindow(hImag_i,TRUE);
					}
					else //缺省为Mandelbrot
					{
						CheckDlgButton(hWnd,IDC_RBN_T0_MANDELBROT,BST_CHECKED);
						CheckDlgButton(hWnd,IDC_RBN_T0_JULIA,BST_UNCHECKED);
						EnableWindow(hReal_w,FALSE);
						EnableWindow(hImag_i,FALSE);
					
					}
					
				}
				
			
			}
			else if(ControlID==IDC_RBN_T0_MANDELBROT)
			{
				EnableWindow(hReal_w,FALSE);
				EnableWindow(hImag_i,FALSE);
				HWND hGongShi_Edit=GetDlgItem(hWnd,IDC_T0_GONGSHI_EDT);//设置初始公式
				SetWindowText(hGongShi_Edit,"z*z+c");
			
			}
			else if(ControlID==IDC_RBN_T0_JULIA)
			{
				EnableWindow(hReal_w,TRUE);
				EnableWindow(hImag_i,TRUE);
				HWND hGongShi_Edit=GetDlgItem(hWnd,IDC_T0_GONGSHI_EDT);//设置初始公式
				SetWindowText(hGongShi_Edit,"z*z+c");
			
			}
			else if(ControlID==IDC_RBN_T0_NEWTON)
			{
				EnableWindow(hReal_w,TRUE);
				EnableWindow(hImag_i,TRUE);
				HWND hGongShi_Edit=GetDlgItem(hWnd,IDC_T0_GONGSHI_EDT);//设置初始公式
				SetWindowText(hGongShi_Edit,"z*z+c");
			
			}
			else if(ControlID==IDC_T0_STOCK)//选择库存公式
			{
				DialogBoxParam(MAIN_INSTANCE,MAKEINTRESOURCE(IDD_DLG_STOCK),hWnd,DlgT1Stock,0);//库存公式选择对话框
			
			}
			else if(ControlID==IDC_RBN_T0_2)//若为2维
			{
				EnableWindow(hReal_w,TRUE);
				EnableWindow(hImag_i,TRUE);
				EnableWindow(hImag_j,FALSE);
				EnableWindow(hImag_k,FALSE);
				EnableWindow(hMandelbrot,TRUE);
				EnableWindow(hNewton,TRUE);
				CheckDlgButton(hWnd,IDC_RBN_T0_MANDELBROT,BST_CHECKED);
				CheckDlgButton(hWnd,IDC_RBN_T0_JULIA,BST_UNCHECKED);
				CheckDlgButton(hWnd,IDC_RBN_T0_NEWTON,BST_UNCHECKED);
			}
			else if(ControlID==IDC_RBN_T0_3)//若为3维
			{
				EnableWindow(hReal_w,TRUE);
				EnableWindow(hImag_i,TRUE);
				EnableWindow(hImag_j,TRUE);
				EnableWindow(hImag_k,TRUE);
				EnableWindow(hMandelbrot,FALSE);
				EnableWindow(hNewton,FALSE);
				CheckDlgButton(hWnd,IDC_RBN_T0_JULIA,BST_CHECKED);
				CheckDlgButton(hWnd,IDC_RBN_T0_MANDELBROT,BST_UNCHECKED);
				CheckDlgButton(hWnd,IDC_RBN_T0_NEWTON,BST_UNCHECKED);
			}
			return TRUE;
		}
		break;
	case WM_CLOSE:
		{
			return TRUE;
		}
		break;
	default:
		return FALSE;
	}
	return FALSE;
	
}
//Tab子对话框T1消息处理
BOOL WINAPI DlgT1(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			return TRUE;
		}
		break;
	case WM_COMMAND:
		{
			return TRUE;
		}
		break;
	case WM_CLOSE://鼠标左键消息
		{
			return TRUE;
		}
		break;
	default:
		return FALSE;
	}
	return FALSE;
	
}
DWORD WINAPI OnT2Test(LPVOID lpParam)//测试推荐值,计算最佳参数值
{
	RECT r;
	GetClientRect(MAIN_HANDLE,&r);//得到窗口客户端大小
	//========================================================
	double width=r.right-r.left;
	double height=r.bottom-r.top;
	double x0=width/2;
	double y0=height/2;
	double min_x=0,max_x=0,min_y=0,max_y=0;
	int i;
	string bufrules;
	//得到参数
	//=======================================================
	char *st;
	//初始单位长度
	double lenght=1;

	int weishu;//得到维数
	if(IsDlgButtonChecked(hT2,IDC_RBN_T2_2)&&!IsDlgButtonChecked(hT2,IDC_RBN_T2_3))
		weishu=2;
	else if(!IsDlgButtonChecked(hT2,IDC_RBN_T2_2)&&IsDlgButtonChecked(hT2,IDC_RBN_T2_3))
		weishu=3;
	else weishu=2;

	long n;//得到迭代次数
	char nstr[20];
	HWND hN=GetDlgItem(hT2,IDC_T2_N);
	GetWindowText(hN,nstr,20);
	n=atol(nstr);

	//得到旋转角度
	double angle;
	char anglestr[20];
	HWND hAngle=GetDlgItem(hT2,IDC_T2_DEGREE);
	GetWindowText(hAngle,anglestr,20);
	angle=strtod(anglestr,&st);
	
	//==========================

	char wenfa[10000];//得到文法文本
	HWND hWenfa=GetDlgItem(hT2,IDC_T2_WENFA_EDIT);
	GetWindowText(hWenfa,wenfa,10000);

	vector<LS> Rules;//转为规则数组(分解规则)
	if(wenfastr.compare(wenfa)!=0||ls_count!=n)//判断文法是否改变
	{
		wenfastr.assign(wenfa);
		ls_count=n;
		
		string str;
		char *p=wenfa;

		//得到初始规则
		string initrule;
		int init_index=wenfastr.find_first_of("\r\n");
		initrule.assign(wenfa,init_index);
		p+=init_index+2;
		rules_str=initrule;

		if(rules_str.size()==0||rules_str.find_first_of('=')!=-1)
		{
			MessageBox(MAIN_HANDLE,"请输入初始规则!","错误",MB_OK);
			return TRUE;
		}
	 
	
		if(wenfastr.find_first_of('@')==-1)
		{
			MessageBox(MAIN_HANDLE,"请输入文法结束符:'@'!","错误",MB_OK);
			return TRUE;

		}
		while(*p)
		{
			if(*p=='\r'||*p=='#')
			{
				//分解规则
				int index=str.find_first_of('=');
				if(index!=-1)
				{
					LS rule;
					if(index!=1)
					{
						rule.head.assign(str,0,index);
						char errstr[100];
						sprintf(errstr,"生成元'%s'不是字符变量!",rule.head.c_str());
						MessageBox(MAIN_HANDLE,errstr,"错误",MB_OK);
						return TRUE;
					}
				
					rule.head.assign(str,0,1);
					rule.body.assign(str,2,str.length()-2);
					Rules.push_back(rule);
					if(rule.head.size()==0)
					{
						MessageBox(MAIN_HANDLE,"LS语法错误!","错误",MB_OK);
						return TRUE;
					}
				}
				str.resize(0);
				//忽略注释部分
				if(*p=='#')
				{
					while(*(p+1))
					{
						p++;
						if(*p=='\n')
						{
							break;
						}
				
					}
				}
			}
			else if(!isspace(*p))//过滤掉空白字符
			{
				str+=*p;
			}
			if(*p=='@')break;//文件结束符
		
			p++;
		}
	//for(i=0;i<Rules.size();i++)
		//DebugBox("head:%s,body:%s",Rules[i].head.c_str(),Rules[i].body.c_str());
	//========================================================
		//规则迭代
		while(n-->0)
		{
			bufrules=rules_str;
			char *p=(char *)bufrules.c_str();
			rules_str.resize(0);
			while(*p)
			{
				string s;
				s.assign(p,1);
				for(i=0;i<Rules.size();i++)
				{
					if(Rules[i].head.compare(s)==0)
					{
						rules_str+=Rules[i].body;
						break;
					}
			
			
				}
				if(i==Rules.size())
				{
					rules_str+=s;
				}
				p++;
		
			}
		}
		if(rules_str.compare(initrule)==0)
		{
			MessageBox(MAIN_HANDLE,"无匹配生成元!","错误",MB_OK);
			return TRUE;
		}
	}
	if(weishu==2)//画二维文法图
	{
		LSDOT CurDot;
		stack<LSDOT> DotStack;

		CurDot.x=0;//设置初始坐标
		CurDot.y=0;
		CurDot.xdeg=0;

		char *p=(char *)rules_str.c_str();

		while(*p)
		{
			if(*p=='F')//向前走一步，画线(1个单位长度)
			{
				CurDot.x+=lenght*cos(CurDot.xdeg*PI/180);
				CurDot.y+=lenght*sin(CurDot.xdeg*PI/180);

				min_x=CurDot.x>min_x?min_x:CurDot.x;
				max_x=CurDot.x<max_x?max_x:CurDot.x;
				min_y=CurDot.y>min_y?min_y:CurDot.y;
				max_y=CurDot.y<max_y?max_y:CurDot.y;
	
			}
			else if(*p=='L')
			{
			}
			else if(*p=='R')
			{
			}
			else if(*p=='+')//左转
			{
				string bufstr(p);
				int begin_index=bufstr.find_first_of('(');
				int end_index=bufstr.find_first_of(')');

				if(begin_index==1&&end_index>begin_index)//旋转自设角度
				{
					//取括号中的值
					bufstr.assign(p,begin_index+1,end_index-begin_index-1);
					double ang=strtod(bufstr.c_str(),&st);
					CurDot.xdeg+=ang;

					p+=end_index-begin_index+2;
					continue;
				}
				else//旋转默认角度
				{
					CurDot.xdeg+=angle;
				}

			}
			else if(*p=='-')//右转
			{
				string bufstr(p);
				int begin_index=bufstr.find_first_of('(');
				int end_index=bufstr.find_first_of(')');

				if(begin_index==1&&end_index>begin_index)//旋转自设角度
				{
					bufstr.assign(p,begin_index+1,end_index-begin_index-1);
					double ang=strtod(bufstr.c_str(),&st);
					CurDot.xdeg-=ang;

					p+=end_index-begin_index+2;
					continue;
				}
				else//旋转默认角度
				{
					CurDot.xdeg-=angle;
				}
			}
			else if(*p=='[')//压栈
			{
				DotStack.push(CurDot);
			
			}
			else if(*p==']')//出栈
			{
				if(DotStack.size()>0)
				{
					CurDot=DotStack.top();
					DotStack.pop();
				}
			
			}
			else ;
			p++;
		}
		
		//=================
		//比率
		double br=(height/(max_y-min_y))>(width/(max_x-min_x))?(width/(max_x-min_x)):(height/(max_y-min_y));
		//调整矩形区域
		min_x*=br;
		min_y*=br;
		max_x*=br;
		max_y*=br;
		//调整长度
		lenght=br*lenght;
		//显示推荐长度值
		char s[20];
		sprintf(s,"%lf",lenght);
		HWND hLen=GetDlgItem(hT2,IDC_T2_LEN);
		SetWindowText(hLen,s);

		//根据推荐长度值计算初始坐标
		//显示推荐初始坐标
		sprintf(s,"%lf",-(min_x+max_x)/2);
		HWND hX0=GetDlgItem(hT2,IDC_T2_X0);
		SetWindowText(hX0,s);
		sprintf(s,"%lf",-(min_y+max_y)/2);
		HWND hY0=GetDlgItem(hT2,IDC_T2_Y0);
		SetWindowText(hY0,s);

	}
	else if(weishu==3)//画三维文法图
	{

	
	}
						

	return 1;
}
//Tab子对话框T2消息处理
BOOL WINAPI DlgT2(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			CheckDlgButton(hWnd,IDC_RBN_T2_2,BST_CHECKED);//使用二维单选按钮
			HWND hN=GetDlgItem(hWnd,IDC_T2_N);
			SetWindowText(hN,"5");//迭代次数
			HWND hDegree=GetDlgItem(hWnd,IDC_T2_DEGREE);
			SetWindowText(hDegree,"0");//旋转角度
			//初始坐标
			HWND hX0=GetDlgItem(hWnd,IDC_T2_X0);
			SetWindowText(hX0,"0");
			HWND hY0=GetDlgItem(hWnd,IDC_T2_Y0);
			SetWindowText(hY0,"0");
			HWND hZ0=GetDlgItem(hWnd,IDC_T2_Z0);
			SetWindowText(hZ0,"0");
			//灰化控件
			EnableWindow(hZ0,FALSE);
			return TRUE;
		}
		break;
	case WM_COMMAND:
		{
			int ControlID= LOWORD(wParam);//控件ID

			HWND hZ0=GetDlgItem(hWnd,IDC_T2_Z0);
			if(ControlID==IDC_RBN_T2_2)
			{
				EnableWindow(hZ0,FALSE);
			}
			else if(ControlID==IDC_RBN_T2_3)
			{
				EnableWindow(hZ0,TRUE);
			
			}
			else if(ControlID==IDC_T2_SAVE)//保存文法
			{
				DialogBoxParam(MAIN_INSTANCE,MAKEINTRESOURCE\
						(IDD_T4_SETNAME_DLG),hWnd,Dlg_T2_SetName,0);//设置名称对话框
			}
			else if(ControlID==IDC_T2_OPEN_STOCK)//选择库存文法
			{
				DialogBoxParam(MAIN_INSTANCE,MAKEINTRESOURCE(IDD_DLG_STOCK),hWnd,DlgT2Stock,0);//库存公式选择对话框
			
			}
			else if(ControlID==IDC_T2_TEST)//计算最佳参数值
			{
				HANDLE hthread=CreateThread(NULL,0,OnT2Test,0,0,0);
				CloseHandle(hthread);
						
			}

			return TRUE;
		}
		break;
	case WM_CLOSE://鼠标左键消息
		{
			return TRUE;
		}
		break;
	default:
		return FALSE;
	}
	return FALSE;
	
}
//Tab子对话框T3消息处理
BOOL WINAPI DlgT3(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	HWND hColor=GetDlgItem(hWnd,IDC_T3_COLOR);
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			//=================================================
			//初始化T3对话框
			//=================================================
			//设置微调旋钮(关联编辑框)
			HWND hUdn_Xmax=GetDlgItem(hWnd,IDC_UDN_T3_XMAX);//X_MAX
			HWND hXmax=GetDlgItem(hWnd,IDC_T3_XMAX);
			SendMessage(hUdn_Xmax,UDM_SETBUDDY,(WPARAM)hXmax,(LPARAM)0);
			SendMessage(hUdn_Xmax,UDM_SETRANGE,(WPARAM)0,(LPARAM)10000);

			HWND hUdn_Ymax=GetDlgItem(hWnd,IDC_UDN_T3_YMAX);//Y_MAX
			HWND hYmax=GetDlgItem(hWnd,IDC_T3_YMAX);
			SendMessage(hUdn_Ymax,UDM_SETBUDDY,(WPARAM)hYmax,(LPARAM)0);
			SendMessage(hUdn_Ymax,UDM_SETRANGE,(WPARAM)0,(LPARAM)10000);

			HWND hUdn_Zmax=GetDlgItem(hWnd,IDC_UDN_T3_ZMAX);//Z_MAX
			HWND hZmax=GetDlgItem(hWnd,IDC_T3_ZMAX);
			SendMessage(hUdn_Zmax,UDM_SETBUDDY,(WPARAM)hZmax,(LPARAM)0);
			SendMessage(hUdn_Zmax,UDM_SETRANGE,(WPARAM)0,(LPARAM)10000);

			HWND hUdn_Pmax=GetDlgItem(hWnd,IDC_UDN_T3_PMAX);
			HWND hPmax=GetDlgItem(hWnd,IDC_EDT_T3_PMAX);//P_MAX编辑框
			SendMessage(hUdn_Pmax,UDM_SETBUDDY,(WPARAM)hPmax,(LPARAM)0);
			SendMessage(hUdn_Pmax,UDM_SETRANGE,(WPARAM)-10000,(LPARAM)10000);

			HWND hUdn_Qmax=GetDlgItem(hWnd,IDC_UDN_T3_QMAX);
			HWND hQmax=GetDlgItem(hWnd,IDC_EDT_T3_QMAX);//Q_MAX编辑框
			SendMessage(hUdn_Qmax,UDM_SETBUDDY,(WPARAM)hQmax,(LPARAM)0);
			SendMessage(hUdn_Qmax,UDM_SETRANGE,(WPARAM)-10000,(LPARAM)10000);
			
			//初始化单选按钮
			//CheckRadioButton(hWnd,IDC_T3_RBN1,IDC_T3_RBN1,IDC_T3_RBN1);
			CheckDlgButton(hWnd,IDC_T3_RBN1,BST_CHECKED);//默认使用表达式
			CheckDlgButton(hWnd,IDC_RBN_T3_2,BST_CHECKED);//使用二维单选按钮
			//灰化参数编辑框控件
			HWND hEditX=GetDlgItem(hWnd,IDC_T3_EDIT_X);
			HWND hEditY=GetDlgItem(hWnd,IDC_T3_EDIT_Y);
			HWND hEditZ=GetDlgItem(hWnd,IDC_T3_EDIT_Z);
			EnableWindow(hEditX,FALSE);
			EnableWindow(hEditY,FALSE);
			EnableWindow(hEditZ,FALSE);
			EnableWindow(hPmax,FALSE);
			EnableWindow(hQmax,FALSE);
			EnableWindow(hZmax,FALSE);
			//禁用三维单选按钮
			HWND hRbn3=GetDlgItem(hWnd,IDC_RBN_T3_3);
			HWND hRbn2=GetDlgItem(hWnd,IDC_RBN_T3_2);
			EnableWindow(hRbn3,FALSE);
			EnableWindow(hRbn2,FALSE);

			//=================================================
			//设置编辑框初值
			HWND hgongshi=GetDlgItem(hWnd,IDC_T3_GONGSHI);
			SetWindowText(hgongshi,"y=x^2+2*x+1");
			SetWindowText(hXmax,"7");
			SetWindowText(hYmax,"5");
			SetWindowText(hZmax,"5");
			SetWindowText(hPmax,"3");
			SetWindowText(hQmax,"3");

			//=================================================

			//嵌入新的颜色静态控件消息处理函数
			OldT3ColorMsgProc=(WNDPROC)SetWindowLong(hColor,GWL_WNDPROC,(LONG)MyT3ColorMsgProc);

			return TRUE;
		}
		break;
	case WM_COMMAND:
		{
			int ControlID= LOWORD(wParam);//控件ID

			HWND hEditX=GetDlgItem(hWnd,IDC_T3_EDIT_X);//参数公式编辑框
			HWND hEditY=GetDlgItem(hWnd,IDC_T3_EDIT_Y);
			HWND hEditZ=GetDlgItem(hWnd,IDC_T3_EDIT_Z);
			HWND hEdit=GetDlgItem(hWnd,IDC_T3_GONGSHI);//普通公式编辑框
			HWND hPmax=GetDlgItem(hWnd,IDC_EDT_T3_PMAX);//P_MAX编辑框
			HWND hQmax=GetDlgItem(hWnd,IDC_EDT_T3_QMAX);//Q_MAX编辑框
			HWND hZmax=GetDlgItem(hWnd,IDC_T3_ZMAX);//Z_MAX编辑框
			HWND hRbn2=GetDlgItem(hWnd,IDC_RBN_T3_2);//2维单选按钮
			HWND hRbn3=GetDlgItem(hWnd,IDC_RBN_T3_3);//3维单选按钮

			//禁止和使用控件
			if(ControlID==IDC_T3_RBN1)//若公式类型为表达式
			{
				EnableWindow(hEditX,FALSE);
				EnableWindow(hEditY,FALSE);
				EnableWindow(hEditZ,FALSE);
				EnableWindow(hPmax,FALSE);
				EnableWindow(hQmax,FALSE);
				EnableWindow(hZmax,FALSE);
				EnableWindow(hRbn2,FALSE);
				EnableWindow(hRbn3,FALSE);
				EnableWindow(hEdit,TRUE);
			


			}
			else if(ControlID==IDC_T3_RBN2)//若公式类型为参数表达式
			{
				EnableWindow(hEdit,FALSE);
				EnableWindow(hEditX,TRUE);
				EnableWindow(hEditY,TRUE);
				EnableWindow(hRbn3,TRUE);
				EnableWindow(hRbn2,TRUE);
				EnableWindow(hPmax,TRUE);
				
				//根据维数启用编辑框
				if(IsDlgButtonChecked(hWnd,IDC_RBN_T3_2)&&!IsDlgButtonChecked(hWnd,IDC_RBN_T3_3))//2维
				{
					EnableWindow(hEditZ,FALSE);
					EnableWindow(hZmax,FALSE);
					EnableWindow(hQmax,FALSE);
				}
				else if(!IsDlgButtonChecked(hWnd,IDC_RBN_T3_2)&&IsDlgButtonChecked(hWnd,IDC_RBN_T3_3))//3维
				{
					EnableWindow(hEditZ,TRUE);
					EnableWindow(hZmax,TRUE);
					EnableWindow(hQmax,TRUE);
				}
				else //缺省2维
				{
					EnableWindow(hEditZ,FALSE);
					EnableWindow(hZmax,FALSE);
					EnableWindow(hQmax,FALSE);
				}
			
			}
			else if(ControlID==IDC_RBN_T3_2)//若为2维单选按钮
			{
				EnableWindow(hEditZ,FALSE);
				EnableWindow(hQmax,FALSE);
				EnableWindow(hZmax,FALSE);
			
			}
			else if(ControlID==IDC_RBN_T3_3)//若为3维单选按钮
			{
				EnableWindow(hEditZ,TRUE);
				EnableWindow(hQmax,TRUE);
				EnableWindow(hZmax,TRUE);
			
			}
			else if(ControlID==IDC_T3_SETCOLOR)//若为设置颜色对话框
			{
				//打开通用颜色对话框
				COLORREF cusColor[16];

				CHOOSECOLOR lcc;
				lcc.lStructSize=sizeof(CHOOSECOLOR);
				lcc.hwndOwner=hWnd;
				lcc.hInstance=NULL;
				lcc.rgbResult=RGB(0,0,0);
				lcc.lpCustColors=cusColor; 
				lcc.Flags=CC_RGBINIT|CC_FULLOPEN| CC_ANYCOLOR;
				lcc.lCustData=NULL;
				lcc.lpfnHook=NULL;
				lcc.lpTemplateName=NULL;

				if(ChooseColor(&lcc))
				{
					T3_Color=lcc.rgbResult;//保存画线颜色

					//显示静态控件颜色
					HDC hdc=GetDC(hColor);
					RECT cr;
					GetClientRect(hColor,&cr);
					HBRUSH hbrush=CreateSolidBrush(T3_Color);
					FillRect(hdc,&cr,hbrush);
					ReleaseDC(hColor,hdc);
				
				}
			
			}
			return TRUE;
		}
		break;
	case WM_NOTIFY://控件通知消息
		{
			int ControlID= LOWORD(wParam);//控件ID
			LPNMHDR pNmhdr=(LPNMHDR)lParam;//NMHDR结构体(控件消息头)
		}
		break;
	case WM_CLOSE://鼠标左键消息
		{
			return TRUE;
		}
		break;
	default:
		return FALSE;
	}
	return FALSE;
	
}
//Tab子对话框T4消息处理
BOOL WINAPI DlgT4(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	HWND hListView=GetDlgItem(hWnd,IDC_T4_LISTVIEW);

	LVCOLUMN lvc;//listview列结构体
	lvc.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_FMT|LVCF_SUBITEM;
	lvc.fmt=LVCFMT_CENTER;//格式居中
	lvc.cchTextMax=256;

	LVITEM litem;//listview项结构体
	litem.mask=LVIF_TEXT;
	litem.cchTextMax=256;

	//得到配置文件路径
	char filename[260];
	string inistr;
	GetModuleFileName(NULL,filename,260);
	inistr=filename;
	inistr.assign(inistr,0,inistr.find_last_of('\\'));
	inistr+="\\IFS.ini";

	int i;
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			//=================================================
			//初始化T4对话框
			//=================================================
			//初始化ListView
			//插入8列
			//------------------------------------
			lvc.iSubItem=0;
			lvc.cx=24;//列宽
			lvc.pszText="i";//列的标题
			ListView_InsertColumn(hListView,0,&lvc);
			lvc.cx=38;//列宽
			lvc.pszText="a";//列的标题
			ListView_InsertColumn(hListView,1,&lvc);
			lvc.pszText="b";
			ListView_InsertColumn(hListView,2,&lvc);
			lvc.pszText="c";
			ListView_InsertColumn(hListView,3,&lvc);
			lvc.pszText="d";
			ListView_InsertColumn(hListView,4,&lvc);
			lvc.pszText="e";
			ListView_InsertColumn(hListView,5,&lvc);
			lvc.pszText="f";
			ListView_InsertColumn(hListView,6,&lvc);
			lvc.pszText="p";
			ListView_InsertColumn(hListView,7,&lvc);
			lvc.cx=65;//列宽
			lvc.pszText="color";
			ListView_InsertColumn(hListView,8,&lvc);
			//---------------------------------------
			//插入第1项
			litem.iItem=0;//第itemcounts项
			litem.iSubItem=0;
			litem.pszText="1";//项文本
			ListView_InsertItem(hListView,&litem);//插入项

			//设置项初值
			ListView_SetItemText(hListView,0,1,"0");
			ListView_SetItemText(hListView,0,2,"0");
			ListView_SetItemText(hListView,0,3,"0");
			ListView_SetItemText(hListView,0,4,"0");
			ListView_SetItemText(hListView,0,5,"0");
			ListView_SetItemText(hListView,0,6,"0");
			ListView_SetItemText(hListView,0,7,"0");
			ListView_SetItemText(hListView,0,8,"0");
			//=================================================
			//=================================================

			HWND hN=GetDlgItem(hWnd,IDC_T4_N);//N
			HWND hXmax=GetDlgItem(hWnd,IDC_T4_XMAX);//xmax
			HWND hYmax=GetDlgItem(hWnd,IDC_T4_YMAX);//ymax
			HWND hX0=GetDlgItem(hWnd,IDC_T4_X0);//x0
			HWND hY0=GetDlgItem(hWnd,IDC_T4_Y0);//y0

			//设置编辑框初始值
			SetWindowText(hXmax,"5");
			SetWindowText(hYmax,"4");
			SetWindowText(hX0,"0");
			SetWindowText(hY0,"0");
			SetWindowText(hN,"10000");

			//设置列表框,得到所有ifs
			char inibuffer[1000];
			//得到所有节名到inibuffer,每个节名以0间隔，最后以2个0结束
			if(GetPrivateProfileSectionNames(inibuffer,1000,inistr.c_str()))//若打开文件成功
			{
				char *p=inibuffer;
			
				while(*(p))
				{
					SendDlgItemMessage(hWnd,IDC_T4_STOCKLIST,LB_ADDSTRING,0,(LPARAM)p);//保存到列表框
					p+=strlen(p)+1;//递增到下一个节名
					
				}
			}

			return TRUE;
		}
		break;
	case WM_COMMAND:
		{
			int ControlID= LOWORD(wParam);//控件ID
			HWND ControlHandle=(HWND)lParam;//控件句柄
			int ControlNotify=HIWORD(wParam);//控件通知码

			if(ControlID==IDC_T4_SETVALUE)//设置值
			{
				int curitem= ListView_GetNextItem(hListView,-1,LVNI_SELECTED);//得到当前项
				if(curitem!=-1)
				{
					DialogBoxParam(MAIN_INSTANCE,MAKEINTRESOURCE\
					(IDD_T4_SETVALUE_DLG),hWnd,Dlg_T4_SetValue,0);//设置值选择对话框
				}
				else
				{
					MessageBox(MAIN_HANDLE,"请选择所要修改项!","错误",MB_OK);
				
				}
			
			}
			else if(ControlID==IDC_T4_ADD_LINE)//插入项
			{
				int itemcounts=ListView_GetItemCount(hListView);//得到当前项数

				//插入项
				litem.iItem=itemcounts;//第itemcounts项
				litem.iSubItem=0;
				char buffer[256];
				sprintf(buffer,"%d",itemcounts+1);
				litem.pszText=buffer;//项文本
				ListView_InsertItem(hListView,&litem);//插入项

				//设置项初值
				ListView_SetItemText(hListView,itemcounts,1,"0");
				ListView_SetItemText(hListView,itemcounts,2,"0");
				ListView_SetItemText(hListView,itemcounts,3,"0");
				ListView_SetItemText(hListView,itemcounts,4,"0");
				ListView_SetItemText(hListView,itemcounts,5,"0");
				ListView_SetItemText(hListView,itemcounts,6,"0");
				ListView_SetItemText(hListView,itemcounts,7,"0");
				ListView_SetItemText(hListView,itemcounts,8,"0");
			
			}
			else if(ControlID==IDC_T4_DELETE_LINE)//删除所有选中项
			{
				while(1)
				{
					int curitem= ListView_GetNextItem(hListView,-1,LVNI_SELECTED);//得到当前项
					ListView_DeleteItem(hListView,curitem);//删除项
					if(curitem<0)break;
				}

				//重新排序
				int itemcounts=ListView_GetItemCount(hListView);//得到当前项数
				char buffer[10];
				for(i=1;i<=itemcounts;i++)
				{
					sprintf(buffer,"%d",i);
					ListView_SetItemText(hListView,i-1,0,buffer);
				}
			
			}
			else if(ControlID==IDC_T4_CLEAN_ALLLINE)//清空ListView
			{
				ListView_DeleteAllItems(hListView);
			}
			else if(ControlID==IDC_T4_SAVE)//保存IFS码
			{
				int itemcounts=ListView_GetItemCount(hListView);//得到当前项数
				if(itemcounts>0)
				{
					DialogBoxParam(MAIN_INSTANCE,MAKEINTRESOURCE\
						(IDD_T4_SETNAME_DLG),hWnd,Dlg_T4_SetName,0);//设置名称对话框
				}
				else
				{
					MessageBox(MAIN_HANDLE,"没有所要保存的IFS码!","错误",MB_OK);
				
				}
				

			}
			else if(ControlID==IDC_T4_STOCKLIST)//若为列表框
			{
				if(ControlNotify==LBN_DBLCLK)//若为双击
				{
					char name[260];
					int sel=SendMessage(ControlHandle,LB_GETCURSEL,0,0);//得到当前索引
					SendMessage(ControlHandle,LB_GETTEXT,(WPARAM)sel,(LPARAM)name);//得到列表项字符串

					//得到控件句柄
					HWND hN=GetDlgItem(hWnd,IDC_T4_N);
					HWND hXmax=GetDlgItem(hWnd,IDC_T4_XMAX);
					HWND hYmax=GetDlgItem(hWnd,IDC_T4_YMAX);
					HWND hX0=GetDlgItem(hWnd,IDC_T4_X0);
					HWND hY0=GetDlgItem(hWnd,IDC_T4_Y0);
					HWND hListView=GetDlgItem(hWnd,IDC_T4_LISTVIEW);

					//从ini文件得到对应项参数与ifs码
					char str[256];
					GetPrivateProfileString(name,"N","0",str,256,inistr.c_str());
					SetWindowText(hN,str);
					GetPrivateProfileString(name,"Xmax","0",str,256,inistr.c_str());
					SetWindowText(hXmax,str);
					GetPrivateProfileString(name,"Ymax","0",str,256,inistr.c_str());
					SetWindowText(hYmax,str);
					GetPrivateProfileString(name,"X0","0",str,256,inistr.c_str());
					SetWindowText(hX0,str);
					GetPrivateProfileString(name,"Y0","0",str,256,inistr.c_str());
					SetWindowText(hY0,str);
					//得到ifs
					long counts,i,j;
					char ifsname[20];

					GetPrivateProfileString(name,"Counts","0",str,256,inistr.c_str());
					counts=atol(str);//得到ifs项数

					ListView_DeleteAllItems(hListView);//清空ListView
					
					//重新设置ListView
					for(i=0;i<counts;i++)
					{
						//插入第i项
						litem.iItem=i;
						litem.iSubItem=0;
						char buf[20];
						sprintf(buf,"%d",i+1);
						litem.pszText=buf;//项文本(序号)
						ListView_InsertItem(hListView,&litem);

						sprintf(ifsname,"IFS_%ld",i);//得到ifs码序列
						GetPrivateProfileString(name,ifsname,"0",str,256,inistr.c_str());

						
						char *p=str;
						for(j=1;j<=8;j++)
						{
							//分解ifs码序列
							string ps;
							while((*p))
							{
								if((*p)!='|')
								{
									ps+=*p;
									p++;
								}
								else
								{
									p++;
									break;
								
								}
							}
							//设回ListView
							if(ps.length()>0)
							{
								ListView_SetItemText(hListView,i,j,(char *)ps.c_str());
							}
							else
							{
								ListView_SetItemText(hListView,i,j,"0");
							}
						
						
						}
					
					}
				}
		
			}
			else if(ControlID==IDC_T4_CLEAN_LIST)//清空列表与ini文件
			{
				int selcounts=SendDlgItemMessage(hWnd,IDC_T4_STOCKLIST,LB_GETCOUNT,0,0);//得到列表框中项数
				for(int i=0;i<selcounts;i++)//循环遍历清除小节
				{
					char str[260];
					SendDlgItemMessage(hWnd,IDC_T4_STOCKLIST,LB_GETTEXT,(WPARAM)i,(LPARAM)str);
					WritePrivateProfileString(str,NULL,NULL,inistr.c_str());
				
				}
				SendDlgItemMessage(hWnd,IDC_T4_STOCKLIST,LB_RESETCONTENT,0,0);//从列表框中清空
			
			
			}
			else if(ControlID==IDC_T4_DELETE_LIST)//删除列表项并刷新ini文件
			{
				int sel=SendDlgItemMessage(hWnd,IDC_T4_STOCKLIST,LB_GETCURSEL,0,0);//得到当前索引

				if(sel>=0)
				{
					//从配置文件中清除
					char str[260];
					SendDlgItemMessage(hWnd,IDC_T4_STOCKLIST,LB_GETTEXT,(WPARAM)sel,(LPARAM)str);
					WritePrivateProfileString(str,0,0,inistr.c_str());
	
					SendDlgItemMessage(hWnd,IDC_T4_STOCKLIST,LB_DELETESTRING,(WPARAM)sel,0);//从列表框中清除
				}
			}
			return TRUE;
		}
		break;
	case WM_CLOSE://鼠标左键消息
		{
			return TRUE;
		}
		break;
	default:
		return FALSE;
	}
	return FALSE;
	
}

//Tab子对话框T10消息处理
BOOL WINAPI DlgT10(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			return TRUE;
		}
		break;
	case WM_COMMAND:
		{
			return TRUE;
		}
		break;
	case WM_CLOSE://鼠标左键消息
		{
			return TRUE;
		}
		break;
	default:
		return FALSE;
	}
	return FALSE;
	
}

//主对话框消息处理
BOOL WINAPI DlgProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			
			//创建TAB控件
			//================================================
			RECT r;
			GetClientRect(hWnd,&r);

			/*hTab=CreateWindowEx(0,WC_TABCONTROL,"",\
			TCS_TABS|TCS_FOCUSNEVER|TCS_BOTTOM|WS_VISIBLE|WS_CHILD|WS_TABSTOP,
			r.left,r.top,r.right-r.left-90,r.bottom-r.top,\
			hWnd,(HMENU)MAKEINTRESOURCE(IDC_TAB), //控件ID
			MAIN_INSTANCE, //实例句柄
			NULL);*/
			hTab=GetDlgItem(hWnd,IDC_TAB);
			//插入TAB控件项
			TCITEM tb;
			tb.mask = TCIF_TEXT;
			tb.cchTextMax = 256;
			tb.pszText="逃逸/迭代法";
			TabCtrl_InsertItem(hTab,0,&tb);
			tb.pszText="四元数分形";
			TabCtrl_InsertItem(hTab,1,&tb);
			tb.pszText="LS文法生成";
			TabCtrl_InsertItem(hTab,2,&tb);
			tb.pszText="数学曲线";
			TabCtrl_InsertItem(hTab,3,&tb);
			tb.pszText="IFS仿射变换";
			TabCtrl_InsertItem(hTab,4,&tb);
			tb.pszText="分形动画";
			TabCtrl_InsertItem(hTab,5,&tb);
			TabCtrl_SetCurSel(hTab,0);
	
			//=================================================
			
			//=================================================
			//创建TAB子对话框
			RECT tr;
			GetClientRect(hTab,&tr);
			hT0=CreateDialogParam(MAIN_INSTANCE,MAKEINTRESOURCE(IDD_T0),hTab,DlgT0,0);
			hT1=CreateDialogParam(MAIN_INSTANCE,MAKEINTRESOURCE(IDD_T1),hTab,DlgT1,0);
			hT2=CreateDialogParam(MAIN_INSTANCE,MAKEINTRESOURCE(IDD_T2),hTab,DlgT2,0);
			hT3=CreateDialogParam(MAIN_INSTANCE,MAKEINTRESOURCE(IDD_T3),hTab,DlgT3,0);
			hT4=CreateDialogParam(MAIN_INSTANCE,MAKEINTRESOURCE(IDD_T4),hTab,DlgT4,0);
			hT10=CreateDialogParam(MAIN_INSTANCE,MAKEINTRESOURCE(IDD_T10),hTab,DlgT10,0);
			MoveWindow(hT0,tr.left+5,tr.top+5,tr.right-tr.left-10,tr.bottom-tr.top-30,TRUE);
			MoveWindow(hT1,tr.left+5,tr.top+5,tr.right-tr.left-10,tr.bottom-tr.top-30,TRUE);
			MoveWindow(hT2,tr.left+5,tr.top+5,tr.right-tr.left-10,tr.bottom-tr.top-30,TRUE);
			MoveWindow(hT3,tr.left+5,tr.top+5,tr.right-tr.left-10,tr.bottom-tr.top-30,TRUE);
			MoveWindow(hT4,tr.left+5,tr.top+5,tr.right-tr.left-10,tr.bottom-tr.top-30,TRUE);
			MoveWindow(hT10,tr.left+5,tr.top+5,tr.right-tr.left-10,tr.bottom-tr.top-30,TRUE);
			ShowWindow(hT0,TRUE);
			ShowWindow(hT1,FALSE);
			ShowWindow(hT2,FALSE);
			ShowWindow(hT3,FALSE);
			ShowWindow(hT4,FALSE);
			ShowWindow(hT10,FALSE);
			
			//=================================================

			//=================================================

			//为对话框加载菜单
			HMENU hmenu=LoadMenu(MAIN_INSTANCE,MAKEINTRESOURCE(IDR_MENU));
			SetMenu(hWnd,hmenu);

			//调整主对话框位置
			GetWindowRect(hWnd,&r);
			MoveWindow(hWnd,100,400,r.right-r.left,r.bottom-r.top+25,TRUE);

			//加载图标
			//HICON hicon=LoadIcon(NULL,IDI_APPLICATION);//应用程序图标
			HICON hicon=LoadIcon(MAIN_INSTANCE,MAKEINTRESOURCE(IDI_APP));//应用程序图标
			SendMessage(hWnd,WM_SETICON,ICON_BIG,(LPARAM)hicon);

			return TRUE;
		}
		break;
	case WM_COMMAND:
		{
			int ControlID=LOWORD(wParam);//控件ID

			if(ControlID==IDC_DRAW)//若为画图按钮
			{
				int sel;
				sel=TabCtrl_GetCurSel(hTab);
				switch(sel)//创建线程
				{
					case 0://画复平面集合
					{
						if(StartDrawByDLL)//若DLL函数加载成功
						{
							//=====================================
							
							GetClientRect(MAIN_HANDLE,&MyRect);//只画原图(不放缩)
							
							//传递参数
							PARAMS param;
							param.main_handle=MAIN_HANDLE;
							param.main_dlg=hWnd;
							param.hmemdc=hMemDC;
							param.hTabDlg=hT0;
							param.attr=PARAMS_ATTR_T0;
							param.myrect=MyRect;
							param.isfangsuo=IsFangSuo;
							//若有库存公式匹配，则画库存图
							if(StartDrawByDLL((LPVOID)&param))
							{
							}
							else
							{
								hDrawThreadT0=CreateThread(NULL,0,OnDrawT0,0,0,0);
								CloseHandle(hDrawThreadT0);
							}
					
						}
						else
						{
							hDrawThreadT0=CreateThread(NULL,0,OnDrawT0,0,0,0);
							CloseHandle(hDrawThreadT0);
						}
						
					}
					break;
					case 1:
					{
						hDrawThreadT1=CreateThread(NULL,0,OnDrawT1,0,0,0);
						CloseHandle(hDrawThreadT1);//画Newton/Nova 分形
					}
					break;
					case 2:
					{
						hDrawThreadT2=CreateThread(NULL,0,OnDrawT2,0,0,0);
						CloseHandle(hDrawThreadT2);//画LS文法生成

					}
					break;
					case 3:
					{
						hDrawThreadT3=CreateThread(NULL,0,OnDrawT3,0,0,0);
						CloseHandle(hDrawThreadT3);//画初等函数曲线

					}
					break;
					case 4:
					{
						hDrawThreadT4=CreateThread(NULL,0,OnDrawT4,0,0,0);
						CloseHandle(hDrawThreadT4);//画初等函数曲线

					}
					break;
					case 5:
					{
						hDrawThreadT10=CreateThread(NULL,0,OnDrawT10,0,0,0);
						CloseHandle(hDrawThreadT10);//画初等函数曲线

					}
					break;

					default:
						break;
				
				}

				//禁用绘图按钮
				HWND hDraw=GetDlgItem(hWnd,IDC_DRAW);
				EnableWindow(hDraw,FALSE);

				//禁用擦除按钮
				HWND hClear=GetDlgItem(hWnd,IDC_CLEAR);
				EnableWindow(hClear,FALSE);


				return TRUE;
			}
			else if(ControlID==IDC_CLEAR)//若为擦除按钮
			{
				RECT r;
				GetClientRect(MAIN_HANDLE,&r);//得到窗口客户端大小

				HDC hDC=GetDC(MAIN_HANDLE);//得到屏幕DC

				DeleteDC(hMemDC);//创建内存兼容DC
				hMemDC=CreateCompatibleDC(hDC);
				
				HBITMAP bm;//创建内存兼容位图,否则只能是单色兼容DC
				bm=CreateCompatibleBitmap(hDC,r.right-r.left,r.bottom-r.top);
				SelectObject(hMemDC,bm);

				HBRUSH hbrush=CreateSolidBrush(RGB(0,0,0));
				FillRect(hMemDC,&r,hbrush);//填充擦除色

				//释放资源
				if(bm)DeleteObject(bm);
				ReleaseDC(MAIN_HANDLE,hDC);
				
				//重绘窗口
				InvalidateRect(MAIN_HANDLE,&r,FALSE);

				return TRUE;
			
			}
			else if(ControlID==IDC_EXIT)//若为退出按钮
			{
				DestroyWindow(hWnd);
				DestroyWindow(MAIN_HANDLE);

				return TRUE;
			}
			else if(ControlID==IDC_BIG)//若为放大按钮
			{
				if(!IsFangSuo)//设置放大
				{
					IsFangSuo=TRUE;
					GetClientRect(MAIN_HANDLE,&MyRect);
					//设置十字形光标
					HCURSOR hcursor=LoadCursor(MAIN_INSTANCE,MAKEINTRESOURCE(IDC_MYCROSS));
					SetClassLong(MAIN_HANDLE,GCL_HCURSOR,(LONG)hcursor);

					SetWindowText((HWND)lParam,"还原");

					//禁用绘图按钮
					HWND hDraw=GetDlgItem(hWnd,IDC_DRAW);
					EnableWindow(hDraw,FALSE);

					
				}
				else//还原放大
				{
					IsFangSuo=FALSE;
					//设置箭头光标
					HCURSOR hcursor=LoadCursor(MAIN_INSTANCE,MAKEINTRESOURCE(IDC_MYARROW));
					SetClassLong(MAIN_HANDLE,GCL_HCURSOR,(LONG)hcursor);

					SetWindowText((HWND)lParam,"放大");

					//启用用绘图按钮
					HWND hDraw=GetDlgItem(hWnd,IDC_DRAW);
					EnableWindow(hDraw,TRUE);

					//重画原图
					//======================================
					if(StartDrawByDLL)//若DLL函数加载成功
					{
						//=====================================
						GetClientRect(MAIN_HANDLE,&MyRect);
						//传递参数
						PARAMS param;
						param.main_handle=MAIN_HANDLE;
						param.main_dlg=hWnd;
						param.hmemdc=hMemDC;
						param.hTabDlg=hT0;
						param.attr=PARAMS_ATTR_T0;
						param.myrect=MyRect;
						param.isfangsuo=IsFangSuo;
						//若有库存公式匹配，则画库存图
						if(StartDrawByDLL((LPVOID)&param))
						{
						}
						else
						{
							hDrawThreadT0=CreateThread(NULL,0,OnDrawT0,0,0,0);
							CloseHandle(hDrawThreadT0);
						}
					
					}
					else
					{
						hDrawThreadT0=CreateThread(NULL,0,OnDrawT0,0,0,0);
						CloseHandle(hDrawThreadT0);
					}
					//=========================================================
				
				}

				return TRUE;
			}
			else if(ControlID==IDC_HIDE)//若为隐藏按钮
			{
				ShowWindow(hWnd,FALSE);
			
				return TRUE;
			}
			else if(ControlID==IDM_ABOUT)//若为关于按钮
			{
				DialogBoxParam(MAIN_INSTANCE,MAKEINTRESOURCE(IDD_ABOUT),hWnd,DlgAboutProc,0);
			
				return TRUE;
			}
			else if(ControlID==IDM_BACKGROUND_COLOR)//若为设置背景按钮
			{
				//打开通用颜色对话框
				COLORREF cusColor[16];

				CHOOSECOLOR lcc;
				lcc.lStructSize=sizeof(CHOOSECOLOR);
				lcc.hwndOwner=MAIN_HANDLE;
				lcc.hInstance=NULL;
				lcc.rgbResult=RGB(0,0,0);
				lcc.lpCustColors=cusColor; 
				lcc.Flags=CC_RGBINIT|CC_FULLOPEN| CC_ANYCOLOR;
				lcc.lCustData=NULL;
				lcc.lpfnHook=NULL;
				lcc.lpTemplateName=NULL;

				if(ChooseColor(&lcc))
				{
					/*int r,g,b;
					b=255&(lcc.rgbResult>>16);
					g=255&(lcc.rgbResult>>8);
					r=255&lcc.rgbResult;*/
					BackGroundColor=lcc.rgbResult;//保存背景色
				
				}
				
			
				return TRUE;
			}
			else if(ControlID==IDC_SCREEN)//若为全屏按钮
			{
				LONG style=(LONG)GetWindowLong(MAIN_HANDLE,GWL_STYLE);//得到窗口风格
				if(!IsScreen)
				{
					style &= ~(WS_DLGFRAME | WS_THICKFRAME); //设为全屏风格
					SetWindowLong(MAIN_HANDLE,GWL_STYLE, style); 
					ShowWindow(MAIN_HANDLE,SW_SHOWMAXIMIZED); //最大化显示
					RECT rect; 
					GetWindowRect(MAIN_HANDLE,&rect); 
					SetWindowPos(MAIN_HANDLE,HWND_NOTOPMOST,rect.left-1, rect.top-1, \
						rect.right-rect.left + 3, rect.bottom-rect.top + 3, SWP_FRAMECHANGED); //调整边框

					SetWindowText((HWND)lParam,"窗口");
					IsScreen=TRUE;

					
				}
				else
				{
					style |= WS_DLGFRAME | WS_THICKFRAME; //还原窗口风格
					SetWindowLong(MAIN_HANDLE, GWL_STYLE, style); 
					ShowWindow(MAIN_HANDLE,SW_NORMAL); //正常显示
					SetWindowText((HWND)lParam,"全屏");
					IsScreen=FALSE;
				}
				//重新创建兼容DC，调整大小
				HDC hDC=GetDC(MAIN_HANDLE);//得到屏幕DC
				if(hMemDC)DeleteDC(hMemDC);//创建内存兼容DC
				hMemDC=CreateCompatibleDC(hDC);
				RECT r;
				GetClientRect(MAIN_HANDLE,&r);
				HBITMAP bm;//创建内存兼容位图,否则只能是单色兼容DC
				bm=CreateCompatibleBitmap(hDC,r.right-r.left,r.bottom-r.top);
				SelectObject(hMemDC,bm);
				ReleaseDC(MAIN_HANDLE,hDC);


				return TRUE;
			}
			else return FALSE;

		}
		break;
	case WM_NOTIFY://控件通知消息
		{
			int ControlID= LOWORD(wParam);//控件ID
			LPNMHDR pNmhdr=(LPNMHDR)lParam;//NMHDR结构体(控件消息头)
			if(ControlID==IDC_TAB)
			{
				if(pNmhdr->code==TCN_SELCHANGE)//若为TAB选择改变，显示不同标签页(对话框)
				{
					RECT tr;
					GetClientRect(hTab,&tr);
					int sel;
					sel=TabCtrl_GetCurSel(hTab);
					MoveWindow(hT0,tr.left+5,tr.top+5,tr.right-tr.left-10,tr.bottom-tr.top-30,TRUE);
					MoveWindow(hT1,tr.left+5,tr.top+5,tr.right-tr.left-10,tr.bottom-tr.top-30,TRUE);
					MoveWindow(hT2,tr.left+5,tr.top+5,tr.right-tr.left-10,tr.bottom-tr.top-30,TRUE);
					MoveWindow(hT3,tr.left+5,tr.top+5,tr.right-tr.left-10,tr.bottom-tr.top-30,TRUE);
					MoveWindow(hT4,tr.left+5,tr.top+5,tr.right-tr.left-10,tr.bottom-tr.top-30,TRUE);
					MoveWindow(hT10,tr.left+5,tr.top+5,tr.right-tr.left-10,tr.bottom-tr.top-30,TRUE);
					
					switch(sel)
					{
						case 0:
						{
						ShowWindow(hT1,FALSE);
						ShowWindow(hT2,FALSE);
						ShowWindow(hT3,FALSE);
						ShowWindow(hT4,FALSE);
						ShowWindow(hT10,FALSE);
						ShowWindow(hT0,TRUE);

						//使用放大按钮
						HWND hFangDa=GetDlgItem(hWnd,IDC_BIG);
						EnableWindow(hFangDa,TRUE);

						}

						break;
					case 1:
						{
						ShowWindow(hT0,FALSE);
						ShowWindow(hT2,FALSE);
						ShowWindow(hT3,FALSE);
						ShowWindow(hT4,FALSE);
						ShowWindow(hT10,FALSE);
						ShowWindow(hT1,TRUE);

						//使用放大按钮
						HWND hFangDa=GetDlgItem(hWnd,IDC_BIG);
						EnableWindow(hFangDa,TRUE);

						}

						break;
					case 2:
						{
						ShowWindow(hT0,FALSE);
						ShowWindow(hT1,FALSE);
						ShowWindow(hT3,FALSE);
						ShowWindow(hT4,FALSE);
						ShowWindow(hT10,FALSE);
						ShowWindow(hT2,TRUE);

						//禁用放大按钮
						HWND hFangDa=GetDlgItem(hWnd,IDC_BIG);
						EnableWindow(hFangDa,FALSE);
						}

						break;
					case 3:
						{
						ShowWindow(hT0,FALSE);
						ShowWindow(hT1,FALSE);
						ShowWindow(hT2,FALSE);
						ShowWindow(hT4,FALSE);
						ShowWindow(hT10,FALSE);
						ShowWindow(hT3,TRUE);

						//禁用放大按钮
						HWND hFangDa=GetDlgItem(hWnd,IDC_BIG);
						EnableWindow(hFangDa,FALSE);

						}

						break;
					case 4:
						{
						ShowWindow(hT0,FALSE);
						ShowWindow(hT1,FALSE);
						ShowWindow(hT3,FALSE);
						ShowWindow(hT2,FALSE);
						ShowWindow(hT10,FALSE);
						ShowWindow(hT4,TRUE);

						//禁用放大按钮
						HWND hFangDa=GetDlgItem(hWnd,IDC_BIG);
						EnableWindow(hFangDa,FALSE);
						}
						break;
					case 5:
						{
						ShowWindow(hT0,FALSE);
						ShowWindow(hT1,FALSE);
						ShowWindow(hT3,FALSE);
						ShowWindow(hT2,FALSE);
						ShowWindow(hT4,FALSE);
						ShowWindow(hT10,TRUE);
						
						//禁用放大按钮
						HWND hFangDa=GetDlgItem(hWnd,IDC_BIG);
						EnableWindow(hFangDa,FALSE);
						}
						break;

					default:
						break;
					}

				}
				
			}
	
		}
		break;
	case WM_CLOSE:
		{
			
			DestroyWindow(hWnd);
			DestroyWindow(MAIN_HANDLE);
			return TRUE;
		}
		break;
	case WM_DESTROY://销毁窗口消息
		{
			PostQuitMessage(0);
			return TRUE;
		}
		break;
	default:
		return FALSE;
	}
	return FALSE;
	
}
//窗口处理函数
//------------------------------------------------------------
LRESULT CALLBACK WinProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	
	switch(msg)
	{
	case WM_KEYDOWN://击键消息
		{
			return 0;
		}
		break;
	case WM_CHAR://字符消息
		{
			return 0;	
		}
		break;
	case WM_LBUTTONDOWN://鼠标左键按下消息
		{
			if(!IsCapture)
			{
				//保存鼠标原来的坐标点
				OldPoint.x=LOWORD(lparam);
				OldPoint.y=HIWORD(lparam);

				SetCapture(hwnd);//捕获鼠标
				IsCapture=TRUE;

				if(IsFangSuo)
				{
					//覆盖擦除原矩形
					MyOldRect.left=MyRect.left;
					MyOldRect.right=MyRect.right;
					MyOldRect.top=MyRect.top;
					MyOldRect.bottom=MyRect.bottom;
			
				}
			
			}
			return 0;
		}
		break;
	case WM_LBUTTONUP://鼠标左键弹起消息
		{
			POINT Point;
			
			if(IsCapture)
			{
				Point.x=LOWORD(lparam);
				Point.y=HIWORD(lparam);

				ReleaseCapture();//取消捕获
				IsCapture=FALSE;

				HDC hDC=GetDC(hwnd);

				//显示矩形区域
				char showrect[20];
				sprintf(showrect,"分形画板 1.0"\
					,MyRect.left,MyRect.top,MyRect.right,MyRect.bottom);
				SetWindowText(hwnd,showrect);

			}
			return 0;
		}
		break;
	case WM_LBUTTONDBLCLK://鼠标左键双击消息
		{
			POINT Point;
			Point.x=LOWORD(lparam);
			Point.y=HIWORD(lparam);

			if(IsFangSuo)
			{

				if(Point.x<MyRect.left||Point.x>MyRect.right||Point.y<MyRect.top||Point.y>MyRect.bottom)
					return 0;
				else
				{
					MyRect.left+=1;
					MyRect.top+=1;
					MyRect.right-=1;
					MyRect.bottom-=1;
				
				}
				
				if(StartDrawByDLL)//若DLL函数加载成功
				{
					//=====================================
					//传递参数
					PARAMS param;
					param.myrect=MyRect;
					param.main_handle=hwnd;
					param.main_dlg=MAIN_DIALOG;
					param.hmemdc=hMemDC;
					param.hTabDlg=hT0;
					param.attr=PARAMS_ATTR_T0;
					param.isfangsuo=IsFangSuo;
					//若有库存公式匹配，则画库存图
					if(StartDrawByDLL((LPVOID)&param))
					{
						//放大后矩形区域清零
						MyRect.left=0;
						MyRect.top=0;
						MyRect.right=0;
						MyRect.bottom=0;
					}
					else
					{
						hDrawThreadT0=CreateThread(NULL,0,OnDrawT0,0,0,0);
						CloseHandle(hDrawThreadT0);
					}
					
				}
				else
				{
					hDrawThreadT0=CreateThread(NULL,0,OnDrawT0,0,0,0);
					CloseHandle(hDrawThreadT0);
				}
				
			}
			return 0;
		}
		break;
	case WM_MOUSEMOVE://鼠标移动消息
		{
			POINT Point;

			Point.x=LOWORD(lparam);
			Point.y=HIWORD(lparam);

			if(IsCapture)
			{
				if(IsFangSuo)
				{
					//得到矩形区域
					if(Point.x>=OldPoint.x)
					{
						MyRect.left=OldPoint.x;
						MyRect.right=Point.x;
					}
					else if(Point.x<OldPoint.x)
					{
						MyRect.left=Point.x;
						MyRect.right=OldPoint.x;
				
					}

					if(Point.y>=OldPoint.y)
					{
						MyRect.top=OldPoint.y;
						MyRect.bottom=Point.y;
					}
					else
					{
						MyRect.top=Point.y;
						MyRect.bottom=OldPoint.y;
				
					}
					//减小双击产生的误差
					if(abs(Point.x-OldPoint.x)<=1&&abs(Point.y-OldPoint.y)<=1)
					{
						GetClientRect(hwnd,&MyRect);
					}
				
					HDC hDC=GetDC(hwnd);

					//在物理DC中会矩形
					int oldrop2=SetROP2(hDC,R2_NOT);//设置反色
					HBRUSH hOldBrush=(HBRUSH)SelectObject(hDC,(HBRUSH)GetStockObject(NULL_BRUSH));//设置透明画刷
					Rectangle(hDC,MyOldRect.left,MyOldRect.top,MyOldRect.right,MyOldRect.bottom);//擦除原矩形框
					Rectangle(hDC,MyRect.left,MyRect.top,MyRect.right,MyRect.bottom);//画矩形框
					SetROP2(hDC,oldrop2);//设回ROP2码
					SelectObject(hDC,hOldBrush);//选入原画刷
					
					//在兼容DC中会矩形
					int oldmemrop2=SetROP2(hMemDC,R2_NOT);//设置反色
					HBRUSH hOldMemBrush=(HBRUSH)SelectObject(hMemDC,(HBRUSH)GetStockObject(NULL_BRUSH));//设置透明画刷
					Rectangle(hMemDC,MyOldRect.left,MyOldRect.top,MyOldRect.right,MyOldRect.bottom);//擦除原矩形框
					Rectangle(hMemDC,MyRect.left,MyRect.top,MyRect.right,MyRect.bottom);//画矩形框
					SetROP2(hMemDC,oldmemrop2);//设回ROP2码
					SelectObject(hMemDC,hOldMemBrush);//选入原画刷

					ReleaseDC(hwnd,hDC);

					MyOldRect=MyRect;//保存原矩形

					//显示矩形区域
					char showrect[20];
					sprintf(showrect,"分形画板 1.0 (x1:%d y1:%d x2:%d y2:%d)"\
						,MyRect.left,MyRect.top,MyRect.right,MyRect.bottom);
					SetWindowText(hwnd,showrect);
					}

					

			}
			else
			{
				/*if(IsFangSuo)
				{
					if(Point.x<MyRect.left&&Point.x>MyRect.right&&Point.y<MyRect.top&&Point.y>MyRect.bottom)
					{
						//设置箭头光标
						//HCURSOR hcursor=LoadCursor(MAIN_INSTANCE,MAKEINTRESOURCE(IDC_MYBIG));
						//SetClassLong(hwnd,GCL_HCURSOR,(LONG)hcursor);
				
					}
				}*/
			
			}
			return 0;
		}
		break;
	case WM_RBUTTONDOWN://鼠标右键消息
		{
			//显示主对话框
			ShowWindow(MAIN_DIALOG,TRUE);
			return 0;
		}
		break;
	case WM_CREATE://窗口创建消息
		{
			//创建非模态对话框
			MAIN_DIALOG=CreateDialogParam(MAIN_INSTANCE,MAKEINTRESOURCE(IDD_MAINDLG),hwnd,DlgProc,0);

			//设置箭头光标
			//HCURSOR hcursor=LoadCursor(MAIN_INSTANCE,MAKEINTRESOURCE(IDC_MYARROW));
			//SetClassLong(hwnd,GCL_HCURSOR,(LONG)hcursor);

			//创建兼容DC
			HDC hDC=GetDC(hwnd);//得到屏幕DC
			if(hMemDC)DeleteDC(hMemDC);//创建内存兼容DC
			hMemDC=CreateCompatibleDC(hDC);
			RECT r;
			GetClientRect(hwnd,&r);
			HBITMAP bm;//创建内存兼容位图,否则只能是单色兼容DC
			bm=CreateCompatibleBitmap(hDC,r.right-r.left,r.bottom-r.top);
			SelectObject(hMemDC,bm);
			ReleaseDC(hwnd,hDC);

			//动态加载stock.dll动态连接库
			hStockDll=LoadLibrary("stock.dll");
			if(hStockDll)
			{
				//加载函数
				StartDrawByDLL=(StockDllProc)GetProcAddress(hStockDll,"StartDrawByDLL");//画库存图
				GetAllGongShiFromDLL=(StockDllProc)GetProcAddress(hStockDll,"GetAllGongShi");//得到所有库存公式
			}

			return 0;
		}
		break;
	case WM_PAINT://重绘消息
		{
			RECT r;
			GetClientRect(hwnd,&r);

			hdc=BeginPaint(hwnd,&ps);

			BitBlt(hdc,r.left,r.top,r.right-r.left,r.bottom-r.top,hMemDC,0,0,SRCCOPY);

			EndPaint(hwnd,&ps);
			return 0;
		}
		break;		
	case WM_CLOSE://关闭窗口消息
		{
			
			DestroyWindow(hwnd);
			return 0;
		}
		break;
	case WM_DESTROY://销毁窗口消息
		{
			if(hStockDll)
			{
				FreeLibrary(hStockDll);
			}
			PostQuitMessage(0);
			return 0;
			
		}
		break;
		//其他消息处理
	default: return DefWindowProc(hwnd, msg, wparam, lparam);
	
	}

}


//窗口主函数入口
//-------------------------------------------------------------
int WINAPI WinMain(
  HINSTANCE hInstance,      // 当前实例句柄
  HINSTANCE hPrevInstance,  // 先前实例句柄
  LPSTR lpCmdLine,          // 命令行参数
  int nCmdShow              // 显示状态
)
{
	WNDCLASSEX winclass;
	HWND hwnd;
	MSG msg;

	winclass.cbSize=sizeof(WNDCLASSEX);//初始化结构大小，只有WNDCLASSEX有，WNDCLASS没有
	winclass.style=CS_HREDRAW | CS_VREDRAW |CS_DBLCLKS | CS_OWNDC;//窗口风格
	winclass.lpfnWndProc=WinProc;//窗口过程处理函数
	winclass.cbClsExtra=0;//扩展风格
	winclass.cbWndExtra=0;//扩展风格
	winclass.hInstance=hInstance;//窗口实例句柄
	//winclass.hIcon=LoadIcon(NULL,IDI_APPLICATION);//应用程序图标，系统自带
	winclass.hIcon=LoadIcon(hInstance,MAKEINTRESOURCE(IDI_APP));//从外面加载应用程序图标
	winclass.hCursor=LoadCursor(NULL,IDC_ARROW);//鼠标箭头光标
	winclass.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);//窗口背景
	winclass.lpszMenuName=NULL;//窗口菜单
	winclass.lpszClassName=WINDOW_CLASS_NAME;//窗口类名
	//winclass.hIconSm=LoadIcon(NULL,IDI_APPLICATION);
	winclass.hIconSm=LoadIcon(hInstance,MAKEINTRESOURCE(IDI_APP));

	MAIN_INSTANCE=hInstance;//保存窗口实例句柄

	//注册窗口类
	if(!RegisterClassEx(&winclass))
		return 0;
	
	//创建窗口
	if(!(hwnd=CreateWindowEx
		(NULL,//扩展风格
		WINDOW_CLASS_NAME,//窗口类名
		WINDOW_TITLE,//窗口标题
		WS_OVERLAPPED | WS_SYSMENU | WS_VISIBLE,
		0,0,//窗口左上角x,y位置
		WINDOW_WIDTH,WINDOW_HEIGHT,//窗口宽度，高度
		NULL,//父窗口句柄
		NULL,//菜单句柄
		hInstance,////窗口实例句柄
		NULL)))//额外参数
		return 0;

	MAIN_HANDLE=hwnd;//保存窗口句柄
	MAIN_INSTANCE=hInstance;//保存窗口实例句柄

	INITCOMMONCONTROLSEX icex;
	RtlZeroMemory(&icex,sizeof(INITCOMMONCONTROLSEX));
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC=ICC_WIN95_CLASSES;//注册所有控件

	InitCommonControlsEx(&icex);//初始化扩展控件

	//消息循环
	while(1)
	{
		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))//接受并移去消息
		{
			if(msg.message==WM_QUIT)break;//若为WM_QUIT消息则退出
			
			TranslateMessage(&msg);//转换消息
			DispatchMessage(&msg);//分发消息给系统
		}
		Sleep(1);
	}
	
	return msg.wParam;
}