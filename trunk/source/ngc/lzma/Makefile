#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC)
endif

include $(DEVKITPPC)/gamecube_rules

PROG = ../../../build_cube/liblzma.a
BDIR = ../../../build_cube
CFLAGS = -c -O2 -D_LZMA_IN_CB=1 -D_SZ_ONE_DIRECTORY  $(MACHDEP) -I./include -I$(DEVKITPRO)/libogc/include -DGEKKO -mcpu=750 -meabi -mhard-float \
	 -D_SZ_NO_INT_64=0
CXX = @powerpc-gekko-gcc
LIB = -lm -static
RM = @rm -f


AR = @powerpc-gekko-ar

OBJS = $(BDIR)/7zAlloc.o $(BDIR)/7zBuffer.o $(BDIR)/7zCrc.o $(BDIR)/7zDecode.o \
       $(BDIR)/7zExtract.o $(BDIR)/7zHeader.o $(BDIR)/7zIn.o $(BDIR)/7zItem.o \
       $(BDIR)/7zMethodID.o $(BDIR)/LzmaDecode.o 

all: $(PROG)

$(PROG): $(OBJS)
	$(AR) rc $(PROG) $(OBJS)

$(BDIR)/7zAlloc.o: 7zAlloc.c
	$(CXX) $(CFLAGS) 7zAlloc.c -o $@

$(BDIR)/7zBuffer.o: 7zBuffer.c
	$(CXX) $(CFLAGS) 7zBuffer.c -o $@

$(BDIR)/7zCrc.o: 7zCrc.c
	$(CXX) $(CFLAGS) 7zCrc.c -o $@

$(BDIR)/7zDecode.o: 7zDecode.c
	$(CXX) $(CFLAGS) 7zDecode.c -o $@

$(BDIR)/7zExtract.o: 7zExtract.c
	$(CXX) $(CFLAGS) 7zExtract.c -o $@

$(BDIR)/7zHeader.o: 7zHeader.c
	$(CXX) $(CFLAGS) 7zHeader.c -o $@

$(BDIR)/7zIn.o: 7zIn.c
	$(CXX) $(CFLAGS) 7zIn.c -o $@

$(BDIR)/7zItem.o: 7zItem.c
	$(CXX) $(CFLAGS) 7zItem.c -o $@

$(BDIR)/7zMethodID.o: 7zMethodID.c
	$(CXX) $(CFLAGS) 7zMethodID.c -o $@

$(BDIR)/LzmaDecode.o: LzmaDecode.c
	$(CXX) $(CFLAGS) LzmaDecode.c -o $@


clean:
	-$(RM) $(PROG) $(OBJS)

