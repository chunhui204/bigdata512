
CC = gcc
CFLAGS = -g -Wall -O2 -I../../include
LFLAGS = -g -Wall

OBJS = diseqc.o dump-zap.o dump-vdr.o scan.o
SRCS = $(OBJS:.o=.c)

TARGET = scan

$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) -o $(TARGET) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) .depend core core.* *.o $(TARGET)

.depend: $(SRCS)
	$(CC) -M $(CFLAGS) $(SRCS) > .depend

-include .depend

