#主文件名
NAME=fractal

#搜索路径
PATH=../../IDE\VC60\Bin

LIB_PATH=../../IDE\VC60\Lib
INCLUDE_PATH=../../IDE\VC60\Include
BIN_PATH=../../Bin

#对象文件
OBJS=$(NAME).obj FuShu.obj FuShu4.obj

#资源文件
RES=$(NAME).res

#链接标志
LINK_FLAG=/libpath:$(LIB_PATH) /libpath:$(BIN_PATH) kernel32.lib user32.lib gdi32.lib winspool.lib \
comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib \
uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:no /machine:I386 \
/out:"$(BIN_PATH)/$(NAME).exe"

#编译标志
CL_FLAG=/X /I $(INCLUDE_PATH) /nologo /ML /W3 /GX /O2 /D "WIN32" \
/D "NDEBUG" /D "_WINDOWS" /Fo"$(BIN_PATH)/" /c

#资源标志
RC_FLAG=/l 0x804 /fo"$(BIN_PATH)/$(NAME).res" /d "NDEBUG"

#链接
$(NAME).exe:$(OBJS) $(RES)
	@Link $(LINK_FLAG) $(OBJS) $(RES)

#编译
$(OBJS):$(NAME).cpp
FuShu.obj:FuShu.cpp FuShu.h
FuShu4.obj:FuShu4.cpp FuShu4.h

#资源编译
$(RES):$(NAME).rc


#隐含规则
.cpp.obj:
	@Cl $(CL_FLAG) $<

.rc.res:
	@Rc $(RC_FLAG) $<

#清除文件
clean:
