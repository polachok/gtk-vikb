CC := gcc
GTK2_CFLAGS := -shared -fPIC `pkg-config gtk+-x11-2.0 --cflags --libs`
GTK3_CFLAGS := -shared -fPIC `pkg-config gtk+-x11-3.0 --cflags --libs`
ifdef ALT_SPACE
CFLAGS += -DALT_SPACE_INSTEAD_OF_ESC
endif

RM := rm

TARGET: libvi-2 libvi-3
all: $(TARGET)

SRC := vi.c

libvi-2: libvi-2.so
libvi-2.so: $(SRC)
	$(CC) $(CFLAGS) $(GTK2_CFLAGS) -o $@ $(SRC)

libvi-3: libvi-3.so
libvi-3.so: $(SRC)
	$(CC) $(CFLAGS) $(GTK3_CFLAGS) -o $@ $(SRC)

clean:
	$(RM) *.o *.so

force-clean:
	$(RM) -f *.o $(TARGET)
