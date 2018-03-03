CC=gcc
CXX=g++
RM=rm -f

SRC=./src
LIB=./lib
LDLIBS=-lpthread -lao -lm -lsamplerate
OBJS=$(LIB)/AudioFile.o \
$(LIB)/base64.o $(LIB)/orientation.o $(LIB)/point.o $(LIB)/serializer.o $(LIB)/spatial.o $(LIB)/stringhelper.o $(LIB)/util.o $(LIB)/zip.o  \
$(LIB)/BTrack.o $(LIB)/OnsetDetectionFunction.o $(LIB)/kiss_fft.o \
$(LIB)/I2C.o $(LIB)/PCA9685.o $(LIB)/PCA9685Servo.o \
$(LIB)/BodyKinematics.o $(LIB)/StewartKinematics.o $(LIB)/DanceMove.o $(LIB)/main.o $(LIB)/MoveMaker.o $(LIB)/RhythmDetector.o


INCLUDES=-I$(SRC) 
CXX_FLAGS=-DUSE_KISS_FFT -g3 -Wall -c -fmessage-length=0 -std=c++11 

all: Donna

Donna: $(OBJS) 
	$(CXX) $(LDFLAGS) -o Donna $(OBJS) $(LDLIBS) 

$(LIB)/%.o:$(SRC)/AudioFile/%.cpp $(SRC)/AudioFile/%.h
	$(CXX) -o $@ -g -c $(INCLUDES) $(CXX_FLAGS) $<

$(LIB)/%.o:$(SRC)/basics/%.cpp $(SRC)/basics/%.h
	$(CXX) -o $@ -g -c $(INCLUDES) $(CXX_FLAGS) $<

$(LIB)/%.o:$(SRC)/BTrack/%.cpp $(SRC)/BTrack/%.h
	$(CXX) -o $@ -g -c $(INCLUDES) $(CXX_FLAGS) $<

$(LIB)/%.o:$(SRC)/servo/%.cpp $(SRC)/servo/%.h
	$(CXX) -o $@ -g -c $(INCLUDES) $(CXX_FLAGS) $<

$(LIB)/%.o:$(SRC)/Stewart/%.cpp $(SRC)/Stewart/%.h
	$(CXX) -o $@ -g -c $(INCLUDES) $(CXX_FLAGS) $<

$(LIB)/%.o:$(SRC)/%.cpp $(SRC)/%.h
	$(CXX) -o $@ -g -c $(INCLUDES) $(CXX_FLAGS) $<

$(LIB)/%.o:$(SRC)/kiss_fft130/%.c $(SRC)/kiss_fft130/%.h
	$(CXX) -o $@ -g -c $(INCLUDES) $(CXX_FLAGS) $<

$(LIB)/%.o:$(SRC)/%.cpp 
	$(CXX) -o $@ -g -c $(INCLUDES) $(CXX_FLAGS) $<


clean:
	$(RM) $(OBJS) Donna




