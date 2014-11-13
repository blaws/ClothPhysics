GCC_OPTIONS=-Wall -pedantic -Iinclude -g -Wno-deprecated
GL_OPTIONS=-framework OpenGL -framework GLUT 
OPTIONS=$(GCC_OPTIONS) $(GL_OPTIONS)

all: cloth

InitShader.o: InitShader.cpp
	g++ -c InitShader.cpp $(GCC_OPTIONS)

cloth: InitShader.o cloth.o
	g++ $^ $(OPTIONS) -o $@  

cloth.o: cloth.cpp
	g++ -c cloth.cpp $(GCC_OPTIONS)

clean:
	rm -rf *.o *~ *.dSYM cloth
