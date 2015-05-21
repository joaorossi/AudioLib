# BINARY NAME
OUTPUT_BIN = ../bin/libDelay.dylib

# COMPILER OPTIONS
CFLAGS = -Wall -std=gnu99 -fPIC -g

# INCLUDE SEARCH FOLDERS
CPPFLAGS = -I../../AudioBase/inc/

# OBJECT FILES
OBJS = delay.o

# LINKER SEARCH FOLDERS
LIBS = -L../../AudioBase/bin/
LIBS += -ldelay_line.o

all: $(OBJS)
	@$(CC) $(CFLAGS) -dynamiclib -o $(OUTPUT_BIN) $^ $(LIBS)
	@echo "  LD\t$(OUTPUT_BIN)"

%.o: %.c
	@$(CC) $(CFLAGS) -c -o $@ $^ $(CPPFLAGS)
	@echo "  CC\t$@"

clean:
	@rm -rf $(OBJS)
	@rm -f $(OUTPUT_BIN)

rebuild: clean all