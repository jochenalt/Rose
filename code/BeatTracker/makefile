CC=gcc
CXX=g++
RM=rm -f

SRC=./src
LIB=./lib
LDLIBS=-lpthread -lao -lm -lsamplerate
OBJS=$(LIB)/AudioFile.o $(LIB)/audio/AudioProcessor.o $(LIB)/MicrophoneInput.o $(LIB)/Playback.o \
$(LIB)/beat/BTrack.o $(LIB)/beat/OnsetDetectionFunction.o $(LIB)/fft/kiss_fft.o\
$(LIB)/servo/I2C.o $(LIB)/servo/PCA9685.o $(LIB)/servo/Servo.o $(LIB)/servo/ServoController.o \
$(LIB)/webserver/mongoose.o $(LIB)/Webserver.o \
$(LIB)/Common/basics/base64.o $(LIB)/Common/basics/orientation.o $(LIB)/Common/basics/point.o $(LIB)/Common/basics/serializer.o \
$(LIB)/Common/basics/spatial.o $(LIB)/Common/basics/stringhelper.o $(LIB)/Common/basics/util.o $(LIB)/Common/basics/zip.o  \
$(LIB)/Common/Stewart/BodyKinematics.o $(LIB)/Common/Stewart/StewartKinematics.o \
$(LIB)/Common/dance/Move.o $(LIB)/Common/dance/Dancer.o $(LIB)/Common/dance\RhythmDetector.o
$(LIB)/main.o 


INCLUDES=-I$(SRC) 
CXX_FLAGS=-DUSE_KISS_FFT -g3 -Wall -c -fmessage-length=0 -std=c++11 

all: Donna

Donna: $(OBJS) 
	$(CXX) $(LDFLAGS) -o Donna $(OBJS) $(LDLIBS) 

$(LIB)/%.o:$(SRC)/audio/%.cpp $(SRC)/audio/%.h
	$(CXX) -o $@ -g -c $(INCLUDES) $(CXX_FLAGS) $<

$(LIB)/%.o:$(SRC)/beat/%.cpp $(SRC)/beat/%.h
	$(CXX) -o $@ -g -c $(INCLUDES) $(CXX_FLAGS) $<

$(LIB)/%.o:$(SRC)/fft/%.cpp $(SRC)/fft/%.h
	$(CXX) -o $@ -g -c $(INCLUDES) $(CXX_FLAGS) $<

$(LIB)/%.o:$(SRC)/webserver/%.cpp $(SRC)/webserver/%.h
	$(CXX) -o $@ -g -c $(INCLUDES) $(CXX_FLAGS) $<

$(LIB)/%.o:$(SRC)/%.cpp $(SRC)/%.h
	$(CXX) -o $@ -g -c $(INCLUDES) $(CXX_FLAGS) $<

$(LIB)/%.o:$(SRC)/../Common/basics/%.cpp $(SRC)/Common/basics/%.h
	$(CXX) -o $@ -g -c $(INCLUDES) $(CXX_FLAGS) $<

$(LIB)/%.o:$(SRC)/../Common/dance/%.cpp $(SRC)/Common/dance/%.h
	$(CXX) -o $@ -g -c $(INCLUDES) $(CXX_FLAGS) $<

$(LIB)/%.o:$(SRC)/../Common/Stewart/%.cpp $(SRC)/Stewart/dance/%.h
	$(CXX) -o $@ -g -c $(INCLUDES) $(CXX_FLAGS) $<


$(LIB)/%.o:$(SRC)/BTrack/%.cpp $(SRC)/BTrack/%.h
	$(CXX) -o $@ -g -c $(INCLUDES) $(CXX_FLAGS) $<



clean:
	$(RM) $(OBJS) Donna




