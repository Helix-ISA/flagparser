TARGET := flagparser.so

CC := clang

CFLAGS := -Wall -Wextra -Werror -Wpedantic -g -O0 -fPIC -fvisibility=hidden
DFLAGS := -MMD -MP
LDFLAGS := -shared

INCS := -Iinclude/

SRCS := $(shell find src -type f -name "*.c")
OBJS := $(SRCS:src/%.c=bin-int/%.o)
DEPS := $(OBJS:.o=.d)

.PHONY: all build clean dirs lexer parser symbol symbol-pic

all: build

build: dirs $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o bin/$@ $(OBJS)

bin-int/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DFLAGS) $(INCS) -c -o $@ $<

clean:
	$(RM) -r bin bin-int

dirs:
	mkdir -p bin bin-int

-include $(DEPS)
