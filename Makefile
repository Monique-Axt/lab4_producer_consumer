CCX 		= g++
CCINCLUDE	=
CCFLAGS		= -pedantic -std=c++11 -pthread
LDFLAGS		=  -pthread
LIBS		= SharedMem.h SharedQueue.h Restart.h
PROGNAME	= lab4
OBJS		= lab4.o SharedMem.o Restart.o
HDRS		=


.cc.o:
	@echo "Compiling " $<
	$(CCX) -c $(CCFLAGS) $(CCINCLUDE) $<
	@echo $@ "done"

all:	$(PROGNAME)

lab4: $(OBJS)
	$(CCX) -o $@ $(OBJS)  $(LDFLAGS) $(LIBS)

clean:
	rm -f a.out core $(PROGNAME) $(OBJS)

