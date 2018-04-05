CC=gcc
CXX=g++
RM=rm -f

SRC=./src
CSRC=../Common/src
LIB=./lib
LDLIBS=-lpthread -lm -lsamplerate -lpulse-simple -lpulse -lasound
OBJS=\
$(LIB)/AudioFile.o $(LIB)/AudioProcessor.o $(LIB)/MicrophoneInput.o $(LIB)/Playback.o $(LIB)/SoundCardUtils.o \
$(LIB)/BTrack.o $(LIB)/OnsetDetectionFunction.o $(LIB)/kiss_fft.o\
$(LIB)/I2C.o $(LIB)/PCA9685.o $(LIB)/Servo.o $(LIB)/ServoController.o \
$(LIB)/mongoose.o $(LIB)/Webserver.o \
$(LIB)/base64.o $(LIB)/orientation.o $(LIB)/point.o $(LIB)/serializer.o \
$(LIB)/spatial.o $(LIB)/stringhelper.o $(LIB)/util.o $(LIB)/zip.o  \
$(LIB)/BodyKinematics.o $(LIB)/StewartKinematics.o \
$(LIB)/Move.o $(LIB)/Dancer.o $(LIB)/RhythmDetector.o\
$(LIB)/main.o 


INCLUDES=-I$(SRC)  -I$(CSRC)
CXX_FLAGS=-DUSE_KISS_FFT -DMG_ENABLE_THREADS -g0 -O3 -Wall -c -fmessage-length=0 -std=c++11 

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

$(LIB)/%.o:$(SRC)/webserver/%.c $(SRC)/webserver/%.h
	$(CXX) -o $@ -g -c $(INCLUDES) $(CXX_FLAGS) $<

$(LIB)/%.o:$(SRC)/servo/%.cpp $(SRC)/servo/%.h
	$(CXX) -o $@ -g -c $(INCLUDES) $(CXX_FLAGS) $<

$(LIB)/%.o:$(SRC)/%.cpp $(SRC)/%.h
	$(CXX) -o $@ -g -c $(INCLUDES) $(CXX_FLAGS) $<

$(LIB)/%.o:$(CSRC)/basics/%.cpp $(CSRC)/basics/%.h
	$(CXX) -o $@ -g -c $(INCLUDES) $(CXX_FLAGS) $<

$(LIB)/%.o:$(CSRC)/dance/%.cpp $(CSRC)/dance/%.h
	$(CXX) -o $@ -g -c $(INCLUDES) $(CXX_FLAGS) $<

$(LIB)/%.o:$(CSRC)/stewart/%.cpp $(CSRC)/stewart/%.h
	$(CXX) -o $@ -g -c $(INCLUDES) $(CXX_FLAGS) $<

$(LIB)/%.o:$(SRC)/%.cpp $
	$(CXX) -o $@ -g -c $(INCLUDES) $(CXX_FLAGS) $<



clean:
	$(RM) $(OBJS) Donna




