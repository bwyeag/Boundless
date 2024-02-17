# 定义使用的编译器
CXX = g++

# 定义编译器选项
CXXFLAGS:= -std=c++20 -Wall -Wextra -g 2> errorout.txt -fopenmp

# 定义附加库选项
LFLAGS   = 

# 定义输出目录
OUTPUT	:= ./

# 定义源文件目录
SRC		:= ./
# 定义包含目录
INCLUDE	:= includes

# 定义库目录
LIB		:= libraries
LIBRARIES := -lglad -lglfw3dll -lzlib -lassimp

# 预定义宏
DEFINES = _DEBUG
DEFS = $(patsubst %,-D%, $(DEFINES:%/=%))

# 检查系统类型
ifeq ($(OS),Windows_NT)
# 设置输出可执行文件名称
MAIN	:= boundless.exe
# 设置源文件目录
SOURCEDIRS	:= $(SRC)
# 设置包含目录
INCLUDEDIRS	:= $(INCLUDE)
# 设置库目录
LIBDIRS		:= $(LIB)
FIXPATH = $(subst /,\,$1)
RM			:= del /q /f
MD	:= mkdir
else
MAIN	:= main
SOURCEDIRS	:= $(shell find $(SRC) -type d)
INCLUDEDIRS	:= $(shell find $(INCLUDE) -type d)
LIBDIRS		:= $(shell find $(LIB) -type d)
FIXPATH = $1
RM = rm -f
MD	:= mkdir -p
endif

# 定义包含目录选项信息
INCLUDES	:= $(patsubst %,-I%, $(INCLUDEDIRS:%/=%))

# 定义库文件选项信息
LIBS		:= $(patsubst %,-L%, $(LIBDIRS:%/=%))

# 定义源文件选项信息
SOURCES		:= $(wildcard $(patsubst %,%/*.cpp, $(SOURCEDIRS)))

# 定义.o文件选项信息(*.cpp => *.o)
OBJECTS		:= $(SOURCES:.cpp=.o)

# 定义依赖输出文件选项信息(*.o => *.d)
DEPS		:= $(OBJECTS:.o=.d)

#
# 生成文件的以下部分是通用的
# 只需更改上述定义即可构建任何可执行文件
# 从“制作依赖”中删除附加到文件的依赖项
#

OUTPUTMAIN	:= $(call FIXPATH,$(OUTPUT)/$(MAIN))

all: $(OUTPUT) $(MAIN)
	@echo Executing 'all' complete!

# 创建output目录
$(OUTPUT):
	$(MD) $(OUTPUT)
# 
$(MAIN): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(DEFS) -o $(OUTPUTMAIN) $(OBJECTS) $(LFLAGS) $(LIBS) $(LIBRARIES)

# 包括所有 .d 文件
-include $(DEPS)

# 这是一个后缀替换规则，用于从 .cpp 构建 .o 和 .d。
# 它使用自动变量 $<：先决条件的名称
# 规则（一个 .cpp 文件）和 $@：规则目标的名称（一个 .o 文件）
# -MMD 生成与 .o 文件同名的依赖输出文件
# （参见 GNU make 手册中关于自动变量的部分）
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(DEFS) -c -MMD $<  -o $@

.PHONY: clean
clean:
	$(RM) $(OUTPUTMAIN)
	$(RM) $(call FIXPATH,$(OBJECTS))
	$(RM) $(call FIXPATH,$(DEPS))
	@echo Cleanup complete!

run: all
	./$(OUTPUTMAIN)
	@echo Executing 'run: all' complete!

console:
	$(CXX) bl_console.cpp bl_resource_load.cpp -oconsole -O3 -Llibraries -lglad -lassimp -lzlib