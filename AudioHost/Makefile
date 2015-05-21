# BINARY NAME
OUTPUT_BIN = bin/AudioHost

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
OBJS += src/audio_chain.o
OBJS += src/audio_node.o
OBJS += src/audio_chain_mgmt.o
OBJS += src/main.o

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
