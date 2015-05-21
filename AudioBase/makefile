# COMPILER OPTIONS
CFLAGS = -Wall -std=gnu99 -fPIC -g

# INCLUDE SEARCH FOLDERS
CPPFLAGS = -Iinc/

# OBJECT FILES
OBJS += src/iir.o
OBJS += src/delay_line.o
OBJS += src/frac_delay_line.o
OBJS += src/wave_table_osc.o

all: $(OBJS)
	@cp $(OBJS) bin/
	@rm -rf $(OBJS)

%.o: %.c
	@$(CC) $(CFLAGS) -c -o $@ $^ $(CPPFLAGS)
	@echo "  CC\t$@"

clean:
	@rm -rf bin/*.o

rebuild: clean all