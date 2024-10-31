##########################
# Makefile for legalizer #
# Author: Shiina         #
##########################
CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2

OBJS = main.o Parser.o Legalizer.o Utilities.o Cell.o Row.o Site.o

legalizer: $(OBJS)
	$(CXX) $(CXXFLAGS) -o legalizer $(OBJS)

main.o: main.cpp Parser.h Legalizer.h Utilities.h
	$(CXX) $(CXXFLAGS) -c main.cpp

Parser.o: Parser.cpp Parser.h Cell.h Row.h Site.h
	$(CXX) $(CXXFLAGS) -c Parser.cpp

Legalizer.o: Legalizer.cpp Legalizer.h Cell.h Row.h Site.h
	$(CXX) $(CXXFLAGS) -c Legalizer.cpp

Utilities.o: Utilities.cpp Utilities.h Cell.h
	$(CXX) $(CXXFLAGS) -c Utilities.cpp

Cell.o: Cell.cpp Cell.h
	$(CXX) $(CXXFLAGS) -c Cell.cpp

Row.o: Row.cpp Row.h Site.h
	$(CXX) $(CXXFLAGS) -c Row.cpp

Site.o: Site.cpp Site.h
	$(CXX) $(CXXFLAGS) -c Site.cpp

clean:
	rm -f *.o legalizer
