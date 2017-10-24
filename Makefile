GCCPARAMS = -Iinclude -g -lncurses

OBJECTS=obj/main.o \
	obj/buffer.o \
	obj/red_black_tree.o \
	obj/tui.o \
	obj/common.o \
	obj/loop.o \

obj/%.o: src/%.c
	mkdir -p $(@D)
	gcc $(GCCPARAMS) -c -o $@ $<

all: editor

editor: $(OBJECTS)
	gcc $(OBJECTS) -o editor $(GCCPARAMS)

clean:
	rm -rf editor obj
