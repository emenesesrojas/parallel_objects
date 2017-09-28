OPTS	 = -O3
CHARMDIR = /Users/emeneses/Desktop/charm-6.8.0
CHARMC	 = $(CHARMDIR)/bin/charmc $(OPTS)

OBJS = particle.o

all: particle

particle: $(OBJS)
	$(CHARMC) -language charm++ -module CommonLBs -o particle $(OBJS)

particle.decl.h: particle.ci
	$(CHARMC)  particle.ci

trace: $(OBJS)
	$(CHARMC) -language charm++ -module CommonLBs -tracemode projections -o particle $(OBJS)
clean:
	rm -f *.decl.h *.def.h conv-host *.o particle charmrun *.sts *.gz *.projrc *.topo *.out

particle.o: particle.cpp particle.decl.h
	$(CHARMC) -c particle.cpp

test: all
	./charmrun particle 100 7 $(TESTOPTS) +p4 +balancer RefineLB +LBDebug 1
