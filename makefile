CC		:= $(CROSS_COMPILE)gcc
CFLAGS	:= -g -O0 -Wall
LDFLAGS	:= -lX11 -lmupdf
EXE		:= pdf_viewer
OBJS	:= main.o \
	dlog.o \
	mc_button.o \
	mc_menu.o \
	mc_pdf.o \
	misc.o \
	file_open.o \
	config.o \


all:$(EXE)

$(EXE):$(OBJS)
	$(CC) $^ $(LDFLAGS) -o $@

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(EXE)
