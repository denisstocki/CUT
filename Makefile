# Zmienne
CC = clang
CFLAGS = -Weverything -std=c99
LDFLAGS = -pthread
SRCDIR = cut/src
BUILDDIR = out
TARGET = cut

# Pliki źródłowe
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))

# Reguły kompilacji
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Reguła docelowa
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# Reguła czyszczenia
clean:
	rm -f $(BUILDDIR)/*.o $(TARGET)

# Domyślna reguła
all: $(TARGET)

.PHONY: all clean
