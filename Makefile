CXXFLAGS = -Wall -Werror -Wextra -g
OBJECTS = obj/bmp.o obj/main.o obj/stego.o

.PHONY: all clean

all: hw-01_bmp

obj:
	mkdir obj
	chmod a=rwx obj

$(OBJECTS): obj/%.o: src/%.c
	gcc $(CXXFLAGS) -I include -c $< -o $@

main.o : bmp.h stego.h
bmp.o : bmp.h
stego.o : bmp.h stego.h

hw-01_bmp : obj $(OBJECTS)
	gcc $(CXXFLAGS) -o $@ $(OBJECTS)

clean :
	$(RM) -f -r obj hw-01_bmp


