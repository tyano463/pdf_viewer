CC		:= gcc
CFLAGS	:= -g -O0
LDFLAGS	:= -lX11
EXE		:= pdf_viewer
OBJS	:= main.o dlog.o

all:$(EXE)

$(EXE):$(OBJS)
	$(CC) $^ $(LDFLAGS) -o $@

.c.o:
	$(CC) $(CFLAGS) -c $<