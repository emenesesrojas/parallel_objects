#include <vector>
#include <map> 
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "charm++.h"
#include "main.decl.h"

using namespace std;
/*readonly*/ CProxy_Main mainProxy;
/*readonly*/ CProxy_KeyValueStore kvstoreProxy;
/*readonly*/ CProxy_KeyValueClient kvclientProxy;
/*readonly*/ int N; //number of elements in KeyValueStore array
/*readonly*/ int M; //keys stored per array element
/*readonly*/ int K; //number of key requests from an array element

struct KeyValue{
  int key;
  int value;
};

/* Main class */
class Main : public CBase_Main
{
public:
	Main(CkArgMsg *m){
		mainProxy = thisProxy;
		N = CkNumPes();
		if (m->argc < 3) {
			ckout << "ERROR, usage " << m->argv[0] << " <keys stored per array element> <keys requested per client>" << endl;
			CkExit();
		}
		M = atoi(m->argv[1]);
		K = atoi(m->argv[2]);

		ckout << "Num Array Elements (N): " << N << endl;
		ckout << "Num Keys Stored Per Array Element of KeyValueStore (M): " << M << endl;
		ckout << "Num Keys Requested By Each Array Element of KeyValueClient (K): " << K << endl;

		//Initialize the KeyValueStoreArray
		ckout << "initializing KeyValueStore array ...." << endl;
		kvstoreProxy = CProxy_KeyValueStore::ckNew(N);

		//Initialize the KeyValueClientArray
		ckout << "initializing KeyValueClient array ...." << endl;
		kvclientProxy = CProxy_KeyValueClient::ckNew(N);

		//call method run on all the elements of KeyValueClient Array
		ckout << "calling method run in KeyValueClient array ...." << endl;
		/* TO DO: call method run on all elements of array type KeyValueClient (broadcast) */
	};

	void finish() {
		ckout << "all responses received ... exiting ..." << endl;
		CkExit();	
	}
};

class KeyValueStore : public CBase_KeyValueStore{
private:
	std::map<int, int> kvmap;

public:

	KeyValueStore(){
		for(int i=0; i<M; i++)
			kvmap[thisIndex*M + i] = rand();
	}

	KeyValueStore(CkMigrateMessage* m) {};

	void request(int refnum, int key, int reqIdx){
		/* TO DO: respond to KeyValueClient with index reqIdx the value in the map associated to key */
	}
};

class KeyValueClient : public CBase_KeyValueClient {
private:
	KeyValue* kvpairs;
	int i, count;

public:

	KeyValueClient(){
		kvpairs = new KeyValue[K];
		count = K;
	};

	KeyValueClient(CkMigrateMessage* m) {};

	void run() {
		/* TO DO: generate K random keys (use rand function), store each key in kvpairs and contact its KeyValueStore object */
		ckout << thisIndex << " sent all requests" << endl;
	}

	void response(int refnum, int value) {
		/* TO DO: store the value in kvpairs */
		if(!--count){
			ckout << thisIndex << " rcvd all responses ... contributing to reduction" << endl;
			/* TO DO: contribute to function finish at the main chare */
		}
	}

};

#include "main.def.h"
