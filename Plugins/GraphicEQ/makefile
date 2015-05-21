# BINARY NAME
OUTPUT_BIN = ../bin/libGraphicEQ.dylib

# COMPILER OPTIONS
CFLAGS = -Wall -std=gnu99 -fPIC -g

# INCLUDE SEARCH FOLDERS
CPPFLAGS = -I../../AudioBase/inc/

# OBJECT FILES
OBJS = graphiceq.o

all: $(OBJS)
	@$(CC) $(CFLAGS) -dynamiclib -o $(OUTPUT_BIN) $^
	@echo "  LD\t$(OUTPUT_BIN)"

%.o: %.c
	@$(CC) $(CFLAGS) -c -o $@ $^ $(CPPFLAGS)
	@echo "  CC\t$@"

clean:
	@rm -rf $(OBJS)
	@rm -f $(OUTPUT_BIN)

rebuild: clean all