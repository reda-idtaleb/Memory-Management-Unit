LIBDIR  = ../../hardware

CC     = gcc
CFLAGS = -Wall -Wextra
INCDIR = -I$(LIBDIR)/include
LIBS   = -L$(LIBDIR)/lib -lhardware

###------------------------------
### Main targets
###------------------------------------------------------------
BINARIES= mmu_manager oracle

all: clean $(BINARIES)

###------------------------------
### Main rules
###------------------------------------------------------------

%.o: %.c
	$(CC) $(CFLAGS) -c $< $(INCDIR)

%: %.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

Makefile.depends:
	$(CC) -MM *.c $(INCDIR) > Makefile.depends

include Makefile.depends

mmu_manager: mmu_manager.o user_process.o matrix.o
oracle: oracle.o matrix.o

###------------------------------
### Misc.
###------------------------------------------------------------
.PHONY: clean realclean depend
clean:
	$(RM) *.o $(BINARIES) Makefile.depends
realclean: clean
	$(RM) vdiskA.bin vdiskB.bin
