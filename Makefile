CC 			?= gcc
PKGCONFIG	= $(shell which pkg-config)
CFLAGS		= $(shell $(PKGCONFIG) --cflags --libs gtk+-3.0) -lpthread `sdl2-config --cflags --libs` -lSDL2_image
RM			= rm -f

all: build

build: src/main.c
	$(CC) src/main.c src/codigo/Martian_node.c -o src/main $(CFLAGS)

clean:
	$(RM) src/main
