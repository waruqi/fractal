#主文件名
NAME=NewProject

#搜索路径
PATH=../../IDE\VC80\

LIB_PATH=../../IDE\VC80\Lib
INCLUDE_PATH=../../IDE\VC80\Include
BIN_PATH=../../Bin

#对象文件
OBJS=$(NAME).obj

#链接标志
LINK_FLAG=/libpath:$(LIB_PATH) /libpath:$(BIN_PATH) kernel32.lib user32.lib gdi32.lib winspool.lib \
comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib \
uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:no /machine:I386 \
/out:"$(BIN_PATH)/$(NAME).exe"

#编译标志
CL_FLAG=/X /I $(INCLUDE_PATH) /nologo /W3 /EHsc /O2 /D "WIN32" \
/D "NDEBUG" /D "_WINDOWS" /Fo"$(BIN_PATH)/" /c


#链接
$(NAME).exe:$(OBJS)
	Link $(LINK_FLAG) $(OBJS)

#编译
$(NAME).obj:$(NAME).cpp

#隐含规则
.cpp.obj:
	Cl $(CL_FLAG) $<

#清除文件
clean:
