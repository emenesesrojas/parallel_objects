#include <vector>
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "charm++.h"
#include "main.decl.h"

using namespace std;
/*readonly*/ CProxy_Main mainProxy;

/* Main class */
class Main : public CBase_Main
{
public:
	Main(CkArgMsg *m) {
		mainProxy = thisProxy;
		if (m->argc < 7) {
			ckout << "ERROR, usage " << m->argv[0] << " <chares> <data points per chare> <data dimensions> <k> <iterations> <balance frequency>" << endl;
			CkExit();
		}

	};

};

class Container : public CBase_Container {
public:

	Container() {
	}

	Container(CkMigrateMessage* m) {};

};

#include "main.def.h"
