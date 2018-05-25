TARGET ?= datastore
SRC_DIRS ?= ./ ../../lib

SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.s')
OBJS := $(addsuffix .o,$(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS=-g -O -Wall -Weffc++ -pedantic  \
		 -pedantic-errors -Wextra -Wcast-align \
		 -Wcast-qual -Wconversion \
		 -Wdisabled-optimization \
		 -Werror -Wfloat-equal -Wformat=2 \
		 -Wformat-nonliteral -Wformat-security  \
		 -Wformat-y2k \
		 -Wimport  -Winit-self  -Winline \
		 -Winvalid-pch   \
		 -Wlong-long \
		 -Wmissing-field-initializers -Wmissing-format-attribute   \
		 -Wmissing-include-dirs -Wmissing-noreturn \
		 -Wpacked -Wpointer-arith \
		 -Wredundant-decls \
		 -Wshadow -Wstack-protector \
		 -Wstrict-aliasing=2 -Wswitch-default \
		 -Wswitch-enum \
		 -Wunreachable-code -Wunused \
		 -Wunused-parameter \
		 -Wvariadic-macros \
		 -Wwrite-strings \
		 $(INC_FLAGS)
#-Waggregate-return
#-Wpadded  

LDFLAGS=-g
LDLIBS=

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

-include $(DEPS)
