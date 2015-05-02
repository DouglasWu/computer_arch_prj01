TARGET=single_cycle

$(TARGET): main.o instruction.o error.o
	gcc -o $(TARGET) main.o instruction.o error.o

main.o: main.cpp parameter.h instruction.h
	gcc -c main.cpp

instruction.o: instruction.h instruction.cpp parameter.h error.h
	gcc -c instruction.cpp
	
error.o: error.h error.cpp parameter.h
	gcc -c error.cpp

clean:
	rm -f $(TARGET) *.o *.bin *.rpt *.exe *.out
