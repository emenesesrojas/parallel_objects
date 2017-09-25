#include "pi.decl.h"

CProxy_Master mainProxy; // readonly

/* Master (main chare) class */
class Master : public CBase_Master {
public:
	int count, totalInside, totalNumTrials;

	Master(CkArgMsg* m) {
		if (m->argc < 3) {
			ckout << "ERROR, usage " << m->argv[0] << " <number of trials> <number of chares>" << endl;
			CkExit();
		}
		int numTrials = atoi(m->argv[1]);
		int numChares = atoi(m->argv[2]);
		if (numTrials % numChares) { 
			ckout << "ERROR, number of trials need to be divisible by number of chares" << endl;
			CkExit();
		}	  
		for (int i = 0; i < numChares; i++)
			CProxy_Worker::ckNew(numTrials/numChares);
		count = numChares; // wait for count responses.
		mainProxy= thisProxy;
		totalInside = 0;
		totalNumTrials = 0;
	};

	void addContribution(int numInside, int numTrials) {
		totalInside += numInside;
		totalNumTrials += numTrials;
		count--;
		if (count == 0) {
			double myPi = 4.0* ((double) (totalInside))
				/ ((double) (totalNumTrials));
			ckout << "Approximated value of pi is:" << myPi << endl;
			CkExit();
		}
	}

};

/* Worker class */
class Worker : public CBase_Worker {
public:
	float y;

	Worker(int numTrials) {
		int inside = 0;
		double x, y;
		ckout << "Hello from a simple chare running on " << CkMyPe() << endl;

		for (int i=0; i< numTrials; i++) { 
			x = drand48();
			y = drand48();
			if ((x*x + y*y) < 1.0) 
				inside++;
		}
		mainProxy.addContribution(inside, numTrials);
	}

};

#include "pi.def.h"
