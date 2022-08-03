LIBDIR  = ../../hardware

CC     = gcc
CFLAGS = -Wall -Wextra
INCDIR = -I$(LIBDIR)/include
LIBS   = -L$(LIBDIR)/lib -lhardware

###------------------------------
### Main targets
###------------------------------------------------------------
BINARIES= mi_user.o mi_kernel 

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

mi_kernel: mi_kernel.o mi_user.o

###------------------------------
### Misc.
###------------------------------------------------------------
.PHONY: clean realclean depend
clean:
	$(RM) *.o $(BINARIES) Makefile.depends
realclean: clean
	$(RM) vdiskA.bin vdiskB.bin
