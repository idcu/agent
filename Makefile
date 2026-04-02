CC = gcc
CFLAGS = -Wall -Icore/include
LDFLAGS = -lkernel32

SRC = \
main.c \
core/src/core.c \
core/src/module.c \
core/src/context.c \
modules/base/json/json.c \
modules/base/yaml/yaml.c \
modules/base/platform/platform.c \
modules/base/config/config.c \
modules/biz/file/file.c \
modules/biz/editor/editor.c \
modules/biz/scanner/scanner.c

all:
	$(CC) $(SRC) -o agent.exe $(CFLAGS) $(LDFLAGS)

clean:
	del /f /q *.exe *.o
