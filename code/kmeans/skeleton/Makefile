OPTS	 = -O3
CHARMDIR = 
CHARMC	 = $(CHARMDIR)/bin/charmc $(OPTS)

OBJS = main.o

all: main

main: $(OBJS)
	$(CHARMC) -language charm++ -o main $(OBJS)

projections: $(OBJS)
	$(CHARMC) -language charm++ -tracemode projections -lz -o main.prj $(OBJS)

summary: $(OBJS)
	$(CHARMC) -language charm++ -tracemode summary -lz -o main.sum $(OBJS)

main.decl.h: main.ci
	$(CHARMC)  main.ci

clean:
	rm -f *.decl.h *.def.h conv-host *.o main charmrun *~

main.o: main.cpp main.decl.h
	$(CHARMC) -c main.cpp

test: main
	$(call run, +p2 ./main 32 16 )
