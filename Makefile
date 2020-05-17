CC := gcc
CFLAGS := -shared -fPIC `pkg-config gtk+-x11-3.0 --cflags --libs`
ifdef ALT_SPACE
CFLAGS += -DALT_SPACE_INSTEAD_OF_ESC
endif

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
