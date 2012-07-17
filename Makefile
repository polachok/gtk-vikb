CC := gcc
CFLAGS := -shared -fPIC `pkg-config gtk+-x11-2.0 --cflags --libs`

RM := rm

TARGET: libvi
all: $(TARGET)

SRC := vi.c
LIB := libvi.so

libvi: $(SRC)
	$(CC) $(CFLAGS) -o $(LIB) $(SRC)

clean:
	$(RM) *.o $(LIB)

force-clean:
	$(RM) -f *.o $(TARGET)
