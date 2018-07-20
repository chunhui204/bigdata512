OBJS = zvmon.o
PROGRAM = zv-monitor
LIBS = -L/usr/X11R6/lib -lX11 -lXv -lXext

CFLAGS = -O2

$(PROGRAM): $(OBJS)
	$(CC) -o $(PROGRAM) $(OBJS) $(LIBS)

clean:
	/bin/rm $(OBJS)
