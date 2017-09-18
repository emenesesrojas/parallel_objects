OPTS	 = -O3
CHARMDIR = 
CHARMC	 = $(CHARMDIR)/bin/charmc $(OPTS)

OBJS = pi.o

all: pi

pi: $(OBJS)
	$(CHARMC) -language charm++ -o pi $(OBJS)

projections: $(OBJS)
	$(CHARMC) -language charm++ -tracemode projections -lz -o pi.prj $(OBJS)

summary: $(OBJS)
	$(CHARMC) -language charm++ -tracemode summary -lz -o pi.sum $(OBJS)

pi.decl.h: pi.ci
	$(CHARMC)  pi.ci

clean:
	rm -f *.decl.h *.def.h conv-host *.o pi charmrun *~

pi.o: pi.cpp pi.decl.h
	$(CHARMC) -c pi.cpp

test: pi
	$(call run, +p2 ./pi 32 16 )
