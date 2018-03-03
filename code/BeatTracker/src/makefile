CC=gcc
CXX=g++
RM=rm -f

SRC=../src
LIB=./lib
LDLIBS=
OBJS=$(LIB)/AudioFile.o \ 
     $(LIB)/base64.o $(LIB)/orientation.o $(LIB)/point.o $(LIB)/serializer.o $(LIB)/spatial.o\
     $(LIB)/stringhelper.o $(LIB)/util.o $(LIB)/zip.o \
     $(LIB)/Btrack.o\$(LIB)/OnsetDetectionFunction.o\
     $(LIB)/kiss_fft.o \
     $(LIB)/I2C.o $(LIB) $(LIB)/PCA9685.o $(LIB)/PCA9685Servo.o\
     $(LIB)/BodyKinematics.o $(LIB)/StewartKinematics.o \
     $(LIB)/DanceMove.o $(LIB)/main.o $(LIB)/MoveMaker.o $(LIB)/RhythmDetector

INCLUDES=
CXX_FLAGS=-DINCLUDE_LUCI_SIGHT -O2 -g0 -Wall -c -fmessage-length=0 

all: Luci

Luci: $(OBJS) 
	$(CXX) $(LDFLAGS) -o Luci $(OBJS) $(LDLIBS) 

%.o: $(SRC)/%.cpp $(SRC)/%.h
	$(CXX) -o $@ $(INCLUDES) $(CXX_FLAGS) $<

%.o:$(SRC)/%.cpp 
	$(CXX) -o $@ -g -c $(INCLUDES) $(CXX_FLAGS) $<


clean:
	$(RM) $(OBJS) Luci




