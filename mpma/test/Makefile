#compiler setup
CFLAGS = -Wall -pthread -rdynamic -I ../code -D linux -fno-strict-aliasing -std=c++0x
DEBUG_FLAGS = -ggdb -D _DEBUG
RELEASE_FLAGS = -O2

BASE_TESTS = -D TESTS_BASE
GEO_TESTS = -D TESTS_GEO
NET_TESTS = -D TESTS_NET
GFXSETUP_TESTS = -D TESTS_GFXSETUP
GFX_TESTS = -D TESTS_GFX
INPUT_TESTS = -D TESTS_INPUT
AUDIO_TESTS = -D TESTS_AUDIO
VR_TESTS = -D TESTS_VR

BASE_FLAGS = 
GEO_FLAGS = 
NET_FLAGS = 
GFXSETUP_FLAGS = 
GFX_FLAGS = -I /usr/include/freetype2
INPUT_FLAGS = 
AUDIO_FLAGS = -I /usr/include/opus
VR_FLAGS = 

BASE_LIBS = -lrt
GEO_LIBS = 
NET_LIBS = 
GFXSETUP_LIBS = -lX11 -lGL -lGLU
GFX_LIBS = -lfreeimage -lfreetype
INPUT_LIBS = 
AUDIO_LIBS = -lopenal -logg -lvorbis -lvorbisfile -lvorbisenc -lFLAC++
VR_LIBS = 

#files and path setup
ROOT_PATH = ../code
BASE_PATH = ../code/base
GEO_PATH = ../code/geo
NET_PATH = ../code/net
GFXSETUP_PATH = ../code/gfxsetup
GFX_PATH = ../code/gfx
INPUT_PATH = ../code/input
AUDIO_PATH = ../code/audio
VR_PATH = ../code/vr

TEST_CPP = main_console.cpp
BASE_CPP = $(BASE_PATH)/Locks.cpp $(BASE_PATH)/Memory.cpp $(BASE_PATH)/MiscStuff.cpp $(BASE_PATH)/Profiler.cpp $(BASE_PATH)/DebugRouter.cpp $(BASE_PATH)/Vary.cpp $(BASE_PATH)/linux/LocksLin32.cpp $(BASE_PATH)/linux/Thread.cpp $(BASE_PATH)/linux/Timer.cpp $(ROOT_PATH)/Setup.cpp $(BASE_PATH)/Info.cpp $(BASE_PATH)/linux/Info.cpp $(BASE_PATH)/linux/File.cpp $(BASE_PATH)/File.cpp $(BASE_PATH)/linux/Debug.cpp $(BASE_PATH)/ThreadedTask.cpp $(BASE_PATH)/Thread.cpp $(BASE_PATH)/ReferenceCount.cpp
GEO_CPP = $(GEO_PATH)/Geo.cpp $(GEO_PATH)/GeoIntersect.cpp
NET_CPP = $(NET_PATH)/Common.cpp $(NET_PATH)/linux/PlatformSockets.cpp $(NET_PATH)/UDP.cpp $(NET_PATH)/TCP.cpp $(NET_PATH)/UPNP.cpp
GFXSETUP_CPP = $(GFXSETUP_PATH)/linux/GFXSetupLin32.cpp $(GFXSETUP_PATH)/Extensions.cpp $(GFXSETUP_PATH)/linux/ExtensionsLin32.cpp $(GFXSETUP_PATH)/GL.cpp
GFX_CPP = $(GFX_PATH)/Texture.cpp $(GFX_PATH)/Vertex.cpp $(GFX_PATH)/Shader.cpp $(GFX_PATH)/TextWriter.cpp
INPUT_CPP = $(INPUT_PATH)/Unified.cpp $(INPUT_PATH)/Keyboard.cpp $(INPUT_PATH)/Mouse.cpp $(INPUT_PATH)/linux/KeyboardLin32.cpp $(INPUT_PATH)/linux/MouseLin32.cpp $(INPUT_PATH)/GameDevice.cpp $(INPUT_PATH)/linux/GameDeviceLin.cpp $(INPUT_PATH)/linux/GameDeviceMappings.cpp
AUDIO_CPP = $(AUDIO_PATH)/AudioSetup.cpp $(AUDIO_PATH)/Listener.cpp $(AUDIO_PATH)/Source.cpp $(AUDIO_PATH)/Player.cpp $(AUDIO_PATH)/SaveToFile.cpp
VR_CPP = $(VR_PATH)/Hmd.cpp


#build all the different builds
debug32: debug_all32
release32: release_all32

#build all parts into one
debug_all32:
	g++ -m32 -o lin32all $(CFLAGS) $(BASE_FLAGS) $(GEO_FLAGS) $(NET_FLAGS) $(GFXSETUP_FLAGS) $(GFX_FLAGS) $(INPUT_FLAGS) $(AUDIO_FLAGS) $(VR_FLAGS) $(DEBUG_FLAGS) $(TEST_CPP) $(BASE_CPP) $(GEO_CPP) $(NET_CPP) $(GFXSETUP_CPP) $(GFX_CPP) $(INPUT_CPP) $(AUDIO_CPP) $(VR_CPP) $(BASE_TESTS) $(GEO_TESTS) $(NET_TESTS) $(GFXSETUP_TESTS) $(GFX_TESTS) $(INPUT_TESTS) $(AUDIO_TESTS) $(VR_TESTS) $(BASE_LIBS) $(GFXSETUP_LIBS) $(GFX_LIBS) $(INPUT_LIBS) $(AUDIO_LIBS) $(VR_LIBS)
release_all32:
	g++ -m32 -o lin32all $(CFLAGS) $(BASE_FLAGS) $(GEO_FLAGS) $(NET_FLAGS) $(GFXSETUP_FLAGS) $(GFX_FLAGS) $(INPUT_FLAGS) $(AUDIO_FLAGS) $(VR_FLAGS) $(RELEASE_FLAGS) $(TEST_CPP) $(BASE_CPP) $(GEO_CPP) $(NET_CPP) $(GFXSETUP_CPP) $(GFX_CPP) $(INPUT_CPP) $(AUDIO_CPP) $(VR_CPP) $(BASE_TESTS) $(GEO_TESTS) $(NET_TESTS) $(GFXSETUP_TESTS) $(GFX_TESTS) $(INPUT_TESTS) $(AUDIO_TESTS) $(VR_TESTS) $(BASE_LIBS) $(GFXSETUP_LIBS) $(GFX_LIBS) $(INPUT_LIBS) $(AUDIO_LIBS) $(VR_LIBS)
release_all_library32:
	g++ -m32 -c $(CFLAGS) $(BASE_FLAGS) $(GEO_FLAGS) $(NET_FLAGS) $(GFXSETUP_FLAGS) $(GFX_FLAGS) $(AUDIO_FLAGS) $(VR_FLAGS) $(RELEASE_FLAGS) $(BASE_CPP) $(GEO_CPP) $(NET_CPP) $(GFXSETUP_CPP) $(GFX_CPP) $(AUDIO_CPP) $(VR_CPP) $(BASE_LIBS) $(GFXSETUP_LIBS) $(GFX_LIBS) $(INPUT_LIBS) $(AUDIO_LIBS) $(VR_LIBS)
	rm -f libmpma.a
	ar cq libmpma.a *.o

#build all the different builds
debug64: debug_all64
release64: release_all64

#build all parts into one
debug_all64:
	g++ -m64 -o lin64all $(CFLAGS) $(BASE_FLAGS) $(GEO_FLAGS) $(NET_FLAGS) $(GFXSETUP_FLAGS) $(GFX_FLAGS) $(INPUT_FLAGS) $(AUDIO_FLAGS) $(VR_FLAGS) $(DEBUG_FLAGS) $(TEST_CPP) $(BASE_CPP) $(GEO_CPP) $(NET_CPP) $(GFXSETUP_CPP) $(GFX_CPP) $(INPUT_CPP) $(AUDIO_CPP) $(VR_CPP) $(BASE_TESTS) $(GEO_TESTS) $(NET_TESTS) $(GFXSETUP_TESTS) $(GFX_TESTS) $(INPUT_TESTS) $(AUDIO_TESTS) $(VR_TESTS) $(BASE_LIBS) $(GFXSETUP_LIBS) $(GFX_LIBS) $(INPUT_LIBS) $(AUDIO_LIBS) $(VR_LIBS)
release_all64:
	g++ -m64 -o lin64all $(CFLAGS) $(BASE_FLAGS) $(GEO_FLAGS) $(NET_FLAGS) $(GFXSETUP_FLAGS) $(GFX_FLAGS) $(INPUT_FLAGS) $(AUDIO_FLAGS) $(VR_FLAGS) $(RELEASE_FLAGS) $(TEST_CPP) $(BASE_CPP) $(GEO_CPP) $(NET_CPP) $(GFXSETUP_CPP) $(GFX_CPP) $(INPUT_CPP) $(AUDIO_CPP) $(VR_CPP) $(BASE_TESTS) $(GEO_TESTS) $(NET_TESTS) $(GFXSETUP_TESTS) $(GFX_TESTS) $(INPUT_TESTS) $(AUDIO_TESTS) $(VR_TESTS) $(BASE_LIBS) $(GFXSETUP_LIBS) $(GFX_LIBS) $(INPUT_LIBS) $(AUDIO_LIBS) $(VR_LIBS)
release_all_library64:
	g++ -m64 -c $(CFLAGS) $(BASE_FLAGS) $(GEO_FLAGS) $(NET_FLAGS) $(GFXSETUP_FLAGS) $(GFX_FLAGS) $(AUDIO_FLAGS) $(VR_FLAGS) $(RELEASE_FLAGS) $(BASE_CPP) $(GEO_CPP) $(NET_CPP) $(GFXSETUP_CPP) $(GFX_CPP) $(AUDIO_CPP) $(VR_CPP) $(BASE_LIBS) $(GFXSETUP_LIBS) $(GFX_LIBS) $(INPUT_LIBS) $(AUDIO_LIBS) $(VR_LIBS)
	rm -f libmpma.a
	ar cq libmpma.a *.o

#clean junk up
clean:
	rm -f *.o
	rm -f lin32*
	rm -f lin64*
	rm -f libmpma.a
