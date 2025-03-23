ALLEGRO_VERSION=5.0.10
PATH_ALLEGRO=C:\allegro-$(ALLEGRO_VERSION)-mingw-4.7.0
LIB_ALLEGRO=\lib\liballegro-$(ALLEGRO_VERSION)-monolith-mt.a
INCLUDE_ALLEGRO=\include

all: kappers.exe meu_kappers.exe

kappers.exe: kappers.o 
	gcc -o kappers.exe kappers.o $(PATH_ALLEGRO)$(LIB_ALLEGRO)	
	
kappers.o: kappers.c 
	gcc -I $(PATH_ALLEGRO)$(INCLUDE_ALLEGRO) -c kappers.c

meu_kappers.exe: meu_kappers.o 
	gcc -o meu_kappers.exe meu_kappers.o $(PATH_ALLEGRO)$(LIB_ALLEGRO)	
	
meu_kappers.o: meu_kappers.c 

	gcc -I $(PATH_ALLEGRO)$(INCLUDE_ALLEGRO) -c meu_kappers.c


clean:
	del kappers.o 
	del kappers.exe
	del meu_kappers.o 
	del meu_kappers.exe