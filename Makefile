# 跨平台兼容Makefile
CC ?= gcc
AR ?= ar
RM = rm -rf

# 平台检测
ifeq ($(OS),Windows_NT)
    TARGET = out/idcu_agent.exe
    CFLAGS += -DWIN32=1 -Ilib
    LDFLAGS += -lkernel32
    RM = del /f /q
else
    TARGET = out/idcu_agent
    CFLAGS += -DLINUX=1 -fPIC -pthread
    LDFLAGS += -lm
endif

# 编译选项
ifeq ($(DEBUG),1)
    CFLAGS += -g -O0 -Wall -Wextra -DDEBUG=1
else
    CFLAGS += -O2 -Wall -Wextra -DNDEBUG=1
endif

# 包含目录
CFLAGS += -Iinclude -Ikernel -Imodules -Iconfig

# 源文件
CORE_SRCS = main.c \
            kernel/context.c \
            kernel/msg_bus.c \
            kernel/micro_kernel.c \
            kernel/sandbox.c \
            kernel/coroutine.c \
            kernel/log.c

MODULE_SRCS = modules/base/log.c \
              modules/core/module.c \
              modules/biz/collect.c

# 目标文件
OBJS = $(CORE_SRCS:.c=.o) $(MODULE_SRCS:.c=.o)

# 构建目录
$(shell mkdir -p out)

# 主目标
all: $(TARGET)

# 链接
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "Build success: $@"

# 编译
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# 清理
clean:
	$(RM) $(OBJS) $(TARGET)
	$(RM) build out
	@echo "Clean done"

# 安装
install: $(TARGET)
	install -d /usr/local/bin
	install $(TARGET) /usr/local/bin
	install -d /usr/local/etc/idcu
	install config/agent.cfg /usr/local/etc/idcu

.PHONY: all clean install