#include <stdlib.h>
#include <vector>
#include "pup_stl.h"
#include "Particle.h"
#include "ParticleExercise.decl.h"

#define RANGE (1.0)
#define ITERATIONS (100)
#define NEIGHBORS 4

#define LB_FREQ 10

/*readonly*/ CProxy_Main mainProxy;
/*readonly*/ CProxy_Cell cellProxy;
/*readonly*/ CProxy_ParticleGroup particleGroupProxy;
/*readonly*/ int elementsPerCell;
/*readonly*/ int cellDimension;

using namespace std;

// ParticleGroup class 
class ParticleGroup : public CBase_ParticleGroup {
public:
	vector<int> particles_per_processor;

	// Constructor
	ParticleGroup(){
		particles_per_processor.resize(ITERATIONS, 0);
	};

	// Function to write particles per processor in each iteration
    void collectNumParticles(int iter, int p){
        particles_per_processor[iter] += p;
	};
	
	// Funtion to print out statistics
	void printStatistics(int iter) {
		contribute(sizeof(int), &particles_per_processor[iter], CkReduction::sum_int, CkCallback(CkReductionTarget(Main, printAVGperproc), mainProxy));
		contribute(sizeof(int), &particles_per_processor[iter], CkReduction::max_int, CkCallback(CkReductionTarget(Main, printMAXperproc), mainProxy));
	};

};


/* Main class, defines main chare functionality */
class Main: public CBase_Main {
public:
	int doneSteps, doneCells;

	// Constructor
	Main(CkArgMsg* m) {
		doneSteps = 0; doneCells = 0;

		// checking arguments
		if(m->argc < 3) CkAbort("USAGE: ./charmrun +p<number_of_processors> ./particle <number of particles per cell> <size of array>");
		elementsPerCell = atoi(m->argv[1]);
		cellDimension = atoi(m->argv[2]);
 
		// setting value of constants
		mainProxy = thisProxy;
		delete m;

		//create the group, a group object will be created per processor
		particleGroupProxy = CProxy_ParticleGroup::ckNew();
      
		CkArrayOptions opts(cellDimension, cellDimension);
		//create the grid and start the simulation by calling run()
		cellProxy = CProxy_Cell::ckNew(opts);

		/* TO DO: broadcast run method on cellProxy */

    }
    
	//reduction functions printAVG & printMAX
	void printAVG(int result){
		CkPrintf("AVG per chare: %d  \n", (result)/(cellDimension*cellDimension));
	}
	void printMAX(int result){
		CkPrintf("MAX per chare: %d\n ", result);
	}
    
	//reduction functions printAVGperproc & printMAXperproc
	void printAVGperproc(int result){
		CkPrintf("AVG per proc: %d  \n", (result)/(CkNumPes()));
	}
	void printMAXperproc(int result){
		CkPrintf("MAX per proc: %d\n ", result);
	}
	
	// Function to finish execution
	void done() {
		doneCells++;
		if(doneCells >= cellDimension*cellDimension){
			CkPrintf("EXIT!\n");
			CkExit();
		}
	}

	// Function after finishing one step
    void donestep(int iter){
		particleGroupProxy.printStatistics(iter);
		/* TO DO: broadcast run method on cellProxy */
	}
};

/* This class represent the cells of the simulation
 * Each cell contains a vector of particles
 * On each time step, the cell perturbs the particles and moves them to neighboring cells as necessary */
class Cell: public CBase_Cell {
public:
	double xMin, xMax, yMin, yMax;
	int iteration;
	int remoteNeighbors;
	vector<Particle> particles;

	// Collects the particles that need transferring to neighborring cells
	vector<Particle> N, E, O, W, S;

	// Constructor
	Cell() {
		iteration = 0;
		remoteNeighbors = 0;
		initializeBounds();
		populateCell(elementsPerCell);
		usesAtSync = true;
    }

	// Migration constructor
    Cell(CkMigrateMessage* m) {}

    // PUP method
	void pup(PUP::er &p) {
		/* TO DO II: pup all properties of this chare */
	}

	// Function to start main computation of each iteration
	void run() {
		int x, y;

		// checking for termination
		if(iteration == ITERATIONS) {
			mainProxy.done();
		} else {

			// updating positions of particles
			updateParticles(iteration);

			// sending particle messages to neighbors
			x = thisIndex.x;
			y = thisIndex.y;
			thisProxy(wrap(x), wrap(y + 1)).updateNeighbor(iteration, N);
			thisProxy(wrap(x + 1), wrap(y)).updateNeighbor(iteration, W);
			thisProxy(wrap(x - 1), wrap(y)).updateNeighbor(iteration, E);
			thisProxy(wrap(x), wrap(y - 1)).updateNeighbor(iteration, S);
		}
	}

	// Function to update particles inside the cell	 
	void updateNeighbor(int iter, std::vector<Particle> incoming){
		particles.insert(particles.end(), incoming.begin(), incoming.end());
		
		// checking if all messages have been received
		remoteNeighbors++;
		if(remoteNeighbors == NEIGHBORS)
			finish();
	}

	// Internal function to finish an iteration
	void finish() {
		int numberParticles = particles.size();

		// updating variables
		iteration++;
		remoteNeighbors = 0;

		// inform the group member on this PE about the number of
		// particles in this iteration for this chare.
		particleGroupProxy.ckLocalBranch()->collectNumParticles(iteration, numberParticles);

		// contibute calls for the MAX and AVG reductions on the number of particles
		contribute(sizeof(int), &numberParticles, CkReduction::max_int, CkCallback(CkReductionTarget(Main, printMAX), mainProxy));
		contribute(sizeof(int), &numberParticles, CkReduction::sum_int, CkCallback(CkReductionTarget(Main, printAVG), mainProxy));
       
		/* TO DO II: check for load balancing period, calling AtSync(), or contributing to donestep */
	
		/* TO DO: contribute to donestep using max_int of iteration as operator */
	}

	// Function to resume after load balancing synchronization
	void ResumeFromSync() {
		/* TO DO II: start new iteration */
	}

	// Update particle positions  
	void updateParticles(int iter) {
		clearPreviouslyCollectedParticles();
		int size = particles.size();
		for(int index = 0; index < size; index++) {
			perturb(&particles[index]);
			moveParticleAtIndex(&particles[index]);
		}
 
		//update particles remaining current cell
		particles = O;
	}

private:

	void initializeBounds() {
		xMin = RANGE * thisIndex.x / cellDimension;
		xMax = RANGE * (thisIndex.x + 1) / cellDimension;
		yMin = RANGE * thisIndex.y / cellDimension;
		yMax = RANGE * (thisIndex.y + 1 ) / cellDimension;
	}

	void populateCell(int initialElements) {
		for(int element = 0; element < initialElements; element++) {
			double x = randomWithin(xMin, xMax);	
			double y = randomWithin(yMin, yMax);
  
			Particle p = Particle(x, y);
			particles.push_back(p);
		}
		
		if ((xMax <= .25 && yMax <= .25) ||
			(xMin >= .75 && yMax <= .25) ||
			(xMax <= .25 && yMin >= .75) ||
			(xMin >= .75 && yMin >= .75)){
			for(int element = 0; element < initialElements*10; element++) {
				double x = randomWithin(xMin, xMax);
				double y = randomWithin(yMin, yMax);
				Particle p = Particle(x, y);
				particles.push_back(p);
			}
		}
	}

	double randomWithin(double min, double max) {
		double random = drand48();
		return min + random * (max - min);
	}

	void clearPreviouslyCollectedParticles() {
		N.clear(); E.clear(); O.clear(); W.clear(); S.clear();
	}
	
	// Function to perturb particle positions
	void perturb(Particle* particle) {
		float maxDelta = 0.01; //a const that determines the speed of the particle movement
		double deltax = drand48()*maxDelta - maxDelta/2.0;
		double deltay = drand48()*maxDelta - maxDelta/2.0;

		//particle moves into either x or into y direction randomly.
		double direction = drand48();
		if(direction >= 0.5 )
			particle->x += deltax;
		else particle->y += deltay;
	}

	// This is kind of tricky. You need to decide where to send the particle
	// based on its +deltax/+deltay values but after that you need to remember
	// to reset the values so that it does not exceed the bounds
	void moveParticleAtIndex(Particle* particle) {
		double x = particle->x;
		double y = particle->y;
		double newX = x;
		double newY = y;

		if(newX > RANGE) newX -= 1.0;
		else if(newX < 0.0) newX += 1.0;

		if(newY > RANGE) newY -= 1.0;
		else if(newY < 0.0) newY += 1.0;
		Particle temp(newX, newY);

		// put the particle into the correct vector to be sended
		if(y > yMax) {
			N.push_back(temp);
		} else if (x < xMin) {
			E.push_back(temp);
		} else if (x > xMax) {
			W.push_back(temp);
		} else if (y < yMin) {
			S.push_back(temp);
		} else {
			O.push_back(temp);
		}
    }

	// Function to wrap around dimensions
	int wrap(int w) {  
	if (w >= cellDimension) return 0;
		else if (w < 0) return cellDimension - 1;
		else return w;
	}

};

#include "ParticleExercise.def.h"
