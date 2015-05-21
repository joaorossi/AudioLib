# BINARY NAME
OUTPUT_BIN = bin/testunit

# COMPILER OPTIONS
CFLAGS  = -Wall -std=gnu99 -g

# INCLUDE SEARCH FOLDERS
CPPFLAGS += -Iinc/
CPPFLAGS += -I../AudioBase/inc
CPPFLAGS += -I../pa_build/include

# LIBS
LDFLAGS  = -L../pa_build/lib
LDFLAGS += -lportaudio

# OBJECT FILES
OBJS += src/testunit.o
OBJS += src/plugin_ctrl.o

all: $(OBJS)
	@$(CC) $(CFLAGS) -o $(OUTPUT_BIN) $^ $(LDFLAGS) $(CPPFLAGS)
	@echo "  LD\t$(OUTPUT_BIN)"

%.o: %.c
	@$(CC) $(CFLAGS) -c -o $@ $^ $(CPPFLAGS)
	@echo "  CC\t$@"

clean:
	@rm -rf $(OBJS)
	@rm -f  $(OUTPUT_BIN)

rebuild: clean all
