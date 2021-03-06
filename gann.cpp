//##########################################################################
//   GANN project
//   Copyright (C) 2015  BENOIT-PILVEN Clément / MARTY Damien
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License along
//   with this program; if not, write to the Free Software Foundation, Inc.,
//   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//##########################################################################


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

//#include <SDL/SDL.h>

//#define BIASWHEEL_STATS

double f[5];
double bestEver;

//====================================================

class RandomList {
protected:
	int size;
	int* array;
public:
	RandomList(double max);
	virtual ~RandomList();
	static double RandAB(double a, double b);
	virtual int* GetMixedArray(void);
};

RandomList::RandomList(double max) {
	size = max;
	array = (int*) malloc(sizeof(int)*size);
	for(int i = 0; i< size; i++)
		array[i]=i;
}

RandomList::~RandomList() {
	free(array);
}

double RandomList::RandAB(double a, double b) {
	return ( rand()/(double)RAND_MAX ) * (b-a) + a;
}

int* RandomList::GetMixedArray(void) {
	int pickedNumber = 0;
	int temp = 0;
	for(int i = 0; i< size; i++){
		pickedNumber = (int) RandAB(0,(double)size);
		// On échange les contenus des cases i et nombre_tire
		temp = array[i];
		array[i] = array[pickedNumber];
		array[pickedNumber] = temp;
	}
	return array;
}

//====================================================

class objectIO {
public:
	virtual double getOutput(double in_date) = 0;
};

//====================================================

class bit: public objectIO {
protected:
	double v;
public:
	bit(double n);
	virtual void setData(double n);
	virtual double getOutput(double in_date);
};

bit::bit(double n) {
	setData(n);
}

void bit::setData(double n) {
	v = n;
}

double bit::getOutput(double in_date) {
	return v;
}

//====================================================

class numInput {
protected:
	bit **b;
	int iSize;
public:
	numInput(int n, int sz);
	void setData(int n);
	bit *getBit(int index);
	bit **getBits(void);
};

numInput::numInput(int n, int sz) {
	iSize = sz;
	b = (bit**) malloc (sizeof(bit*)*sz);
	for(int i = 0; i < iSize; i++) {
		double v = 0.0;
		if((n>>i)&0x01)
			v = 1.0;
		b[i] = new bit(v);
	}
}

void numInput::setData(int n) {
	for(int i = 0; i < iSize; i++) {
		double v = -1.0;
		if((n>>i)&0x01)
			v = 1.0;
		b[i]->setData(v);
	}
}

bit *numInput::getBit(int index) {
	if(index<iSize) {
		return b[index];
	} else {
		printf("Fault asked index %d max is %d\n",index,iSize);
	}
	return NULL;
}

bit **numInput::getBits(void) {
	return b;
}
 
//====================================================

class neuron: public objectIO {
protected:
	int sz;
	int allSz;
	double sum;
	double thr;
	double *w;
	objectIO **x;
	double date;
	double output;
public:
	neuron(objectIO **ios, int sz, double t = 0);
	// Network creation stuffs
	virtual void checkAllocatedAxones(int s);
	virtual  void connectObject(objectIO *n, double weight = -99.99);
	// Reset a neuron
	virtual void randomiseWeight(void);
	// Output stuffs
	virtual double evaluate(double in_date);
	virtual double getOutput(double in_date); // need to be as fast as possible
	// Genetics stuffs
	virtual int getGeneLen(void);
	virtual double *getGenes(void);
	virtual void setGenes(double *genes);
	// Debug stuffs
	virtual void printState(void);
};

neuron::neuron(objectIO **ios, int s, double t) {

	// Set all Clean
	x = NULL;
	w = NULL;
	sz = 0;
	date = 0.0;
	output = 0.0;
	
	// Random Threshold or value from constructor 
	if(t == 0)
		thr = (double)rand()/(double)(RAND_MAX/1.0);
	else
		thr = t;
	
	// Links to Obj
	if((s!=0) && (ios!=NULL)){
		// Set Object Links
		for(int i = 0; i<s; i++) {
			connectObject(ios[i]);
		}
	}
}

void neuron::checkAllocatedAxones(int s) {
	// Always alloc 2 times more memory that we actually need
	if(s>allSz) {
		allSz = s * 2;
		// man tells us if x is NULL realloc(x,sz) = malloc(sz)
		x = (objectIO**) realloc(x,sizeof(objectIO*)*allSz);
		w = (double*) realloc (w,sizeof(double)*allSz);
	}
}

void neuron::connectObject(objectIO *n, double weight) {
	// Check if the pointer can handle sz + 1;
	checkAllocatedAxones(sz+1);
	// Apply the connexion
	x[sz] = n;
	w[sz] = (weight == -99.99)?(double)rand()/(double)(RAND_MAX/2.0)-1.0:weight;
	sz++;
}

void neuron::randomiseWeight(void) {
	for(int i = 0; i < sz; i++) {
		w[i] = (double)rand()/(double)(RAND_MAX/2.0)-1.0;
	}
}

double neuron::evaluate(double in_date) {
	sum = 0;
	for(int i = 0; i < sz; i++)
			sum += w[i]*(x[i]->getOutput(in_date));
	date = in_date;
	output = 1.0/(1.0+exp(-5*sum));
	//output = 1.0/(1.0+exp(-sum));
	return output;
}

double neuron::getOutput(double in_date) {
	if (in_date <= date)
		return output;
	else 
		return evaluate(in_date);
}

int neuron::getGeneLen(void) {
	return sz;
}

double *neuron::getGenes(void) {
	return w;
}

void neuron::setGenes(double *genes) {
	memcpy(w,genes,sz*sizeof(double));
}

void neuron:: printState(void) {
	printf("[%1.5f] [%1.5f] [%p] [",thr,sum,w);
	for(int i = 0; i < sz; i++) 
		printf(" %1.5f(%p) ",w[i],x[i]);
	printf("]\n");
}

//====================================================

class network {
protected:
	int nLayers;
	int *nPerLayer;
	int genomeSz;
	neuron ***neurons;
	double networkDate;
public:

	// Construtor
	network(int nL, int *nPL);
	
	// Input	
	virtual void *getInputData(void) = 0;
	virtual void setInputData(void*) = 0;
		
	// Do
	void step();
	
	// Visualize
	void print();
	
	// output and error estimation
	int  output(double *f, int max);
	virtual double error(double res) = 0;
	
	// Genome stuffs
	int genomeSize(void);
	double *extractGenome(bool print);
	void setGenome(double* g);
	
	//NEAT stuffs
	//void insertNewNeuron(void);
};

network::network(int nL, int *nPL) {
	networkDate = 0;
	genomeSz = 0;
	nLayers = nL;
	nPerLayer = (int*) malloc(sizeof(int)*nLayers);
	// Apply the number per layers
	for(int i = 0; i<nLayers; i++) {
		nPerLayer[i] = nPL[i];
	}
	// Allocate space for layers 
	neurons = (neuron***) malloc(sizeof(neuron**)*nLayers);
	int nPrevLayer = 0;
	for(int i = 0; i<nLayers; i++) {
		neurons[i] = (neuron**) malloc(sizeof(neuron*)*nPerLayer[i]);
		objectIO **list;
		if(i == 0) {
			list = NULL; // This is the first layer
		} else {
			list = (objectIO**)neurons[i-1];
		}
		
		for(int j = 0; j<nPerLayer[i]; j++) {
			neurons[i][j] = new neuron(list,nPrevLayer);
			//printf("Created Neuron[%d][%d] %p with nPrevLayer = %d, list %p\n", i, j, neurons[i][j], nPrevLayer, list);
		}
		nPrevLayer = nPerLayer[i];
	}
}

/*
void network::insertNewNeuron(void) {
	int targetLayer = (int) RandomList::RandAB(1,nLayers-1);
	neurons[targetLayer] = (neuron**) realloc(neurons[targetLayer],sizeof(neuron*)*(nPerLayer[targetLayer]+1));
	
	// neuron birth, with no connexion
	neurons[targetLayer][nPerLayer[targetLayer]] = new neuron(0,NULL);
	// connexion with upstream layers
	int nMaxConnex = 0;
	for (int i = 0; i < targetLayer; i++) {
		nMaxConnex += nPerLayer[i];
	}
	int nbConnexions = (int) RandomList::RandAB(1,nMaxConnex);
	for (int i = 0; i<nbConnexions ; i++) {
		int layer = (int) RandomList::RandAB(0,targetLayer-1);
		int neuron = (int) RandomList::RandAB(0,nPerLayer[layer]);
		neurons[targetLayer][nPerLayer[targetLayer]]->addConnexion((objectIO*) neurons[layer][neuron]);
	} 
	
	// connexion with downstream layers
	nMaxConnex = 0;
	for (int i = targetLayer+1; i < nLayers; i++) {
		nMaxConnex += nPerLayer[i];
	}
	nbConnexions = (int) RandomList::RandAB(1,nMaxConnex);
	for (int i = 0; i<nbConnexions ; i++) {
		int layer = (int) RandomList::RandAB(targetLayer+1,nLayers);
		int neuron = (int) RandomList::RandAB(0,nPerLayer[layer]);
		neurons[layer][neuron]->addConnexion((objectIO*) neurons[targetLayer][nPerLayer[targetLayer]]);
	}
	
	nPerLayer[targetLayer]++;
}
*/


void network::step(void) {
	networkDate++;
	for(int i = 0; i<nLayers; i++) {
		for(int j = 0; j<nPerLayer[i]; j++) {
			neurons[i][j]->evaluate(networkDate);
		}	
	}
}

void network::print(void) {
	for(int i = 0; i<nLayers; i++) {
		printf("=========l%d=========\n",i);
		for(int j = 0; j<nPerLayer[i]; j++) {
			neurons[i][j]->printState();
		}
	}
}

int network::output(double *f, int max) {
	networkDate++;
	int highestLayer = nLayers-1;
	int nNeurons = (nPerLayer[highestLayer]<max)?nPerLayer[highestLayer]:max;
	for(int i = 0; i < nNeurons ; i++) {
		*(f+i) = neurons[highestLayer][i]->getOutput(networkDate);
	}
	return nNeurons;
}

int network::genomeSize(void) {
	int geneTotalLen = 0;
	for(int i = 0; i<nLayers; i++) {
		for(int j = 0; j<nPerLayer[i]; j++) {
			geneTotalLen += neurons[i][j]->getGeneLen();
		}
	}
	return geneTotalLen;
}

double *network::extractGenome(bool print) {
	// iterate over layers
	int geneIndex = 0;
	int geneTotalLen = genomeSize();
	//printf("geneTotalLen %d\n", geneTotalLen);
	double *genome = (double*) malloc (sizeof(double)*geneTotalLen);
	for(int i = 0; i<nLayers; i++) {
		for(int j = 0; j<nPerLayer[i]; j++) {
			int l = neurons[i][j]->getGeneLen();
			double *g = neurons[i][j]->getGenes();
			for(int k = 0; k<l; k++) {
				genome[geneIndex] = *(g+k);
				if(print)
					printf("[%d] %1.4f\n",geneIndex,*(g+k));
				geneIndex++;
			}
		}
	}
	return genome;
}

void network::setGenome(double* g) {
	int index = 0;
	for(int i = 0; i<nLayers; i++) {
		for(int j = 0; j<nPerLayer[i]; j++) {
			int l = neurons[i][j]->getGeneLen();
			neurons[i][j]->setGenes(g+index);
			index += l;
		}
	}
}

//====================================================


class booleanOperation: public network {
protected:
	int a;
	int b;
	int intSz;
	int mask;
	int nInput;
	numInput **li;
	typedef struct _coupleData {
		int a;
		int b;
	} coupleData;
public:
	booleanOperation(int iSz, int nLayer, int *nPerLayer);
	void init(int iSz);
	virtual void *getInputData(void);
	virtual void setInputData(void*);
	void setAB(int la, int lb);
	void setA(int la);
	void setB(int lb);
	int getA(void);
	int getB(void);
	virtual double error(double res);
	virtual bool trueResult() = 0;
};

booleanOperation::booleanOperation(int iSz, int nLayer, int *nPerLayer): network(nLayer, nPerLayer) {
	nInput = 2;
	intSz = iSz;
	mask = 0;
	for(int i = 0; i< iSz; i++) {
		mask <<= 1;
		mask += 1;
	}
	//printf("mask %08x\n", mask);
	a = 0;
	b = 0;
	li = (numInput**) malloc(sizeof(numInput*)*nInput);
	li[0] = new numInput(a,intSz);
	li[1] = new numInput(b,intSz);
	
	// Here the network is created but none 
	// of the input are connected so connect 
	// first layer to our inputs
	for(int i = 0; i<intSz; i++) {
		for(int j = 0; j<nInput; j++) {
			for(int k = 0; k<nPerLayer[0]; k++) {
				neurons[0][k]->connectObject(li[j]->getBit(i));
			}
		}
	}
}

void *booleanOperation::getInputData(void) {
	coupleData *c = (coupleData*) malloc(sizeof(coupleData));
	c->a = rand() & mask;
	c->b = rand() & mask;
	//printf("new input %d %d\n", c->a, c->b);
	return (void*) c;
}

void booleanOperation::setInputData(void* data) {
	coupleData *c = (coupleData*) data;
	setAB(c->a,c->b);
}

void booleanOperation::setAB(int la, int lb) {
	a = la;
	b = lb;
	li[0]->setData(a);
	li[1]->setData(b);
}

void booleanOperation::setA(int la) {
	a =la;
	li[0]->setData(a);
}

void booleanOperation::setB(int lb) {
	b =lb;
	li[1]->setData(b);
}

int booleanOperation::getA(void){
	return a;
}

int booleanOperation::getB(void){
	return b;
}

double booleanOperation::error(double res) {
	bool tRes = trueResult();
	if(tRes) {	// true res is 1
		if(res <= 0.5){ // we fail do a big error
			return 1.0; 	
		} else {			// we do it right make a small error
			return 0.0;
		}
	} else { // true res is 0
		if(res <= 0.5){  // we do it right make a small error
			return 0.0; 
		} else { // we fail do a big error
			return 1.0;
		}
	}
}
//====================================================

class AsupB: public booleanOperation {
public:
	AsupB(int iSz, int nLayer, int *nPerLayer);
	virtual bool trueResult();
};

AsupB::AsupB(int iSz, int nLayer, int *nPerLayer): booleanOperation(iSz,nLayer,nPerLayer) { }

bool AsupB::trueResult() {
	return a>b;
}

//====================================================

class AandB: public booleanOperation {
public:
	AandB(int nLayer, int *nPerLayer);
	virtual bool trueResult();
};

AandB::AandB(int nLayer, int *nPerLayer): booleanOperation(1,nLayer,nPerLayer) { }

bool AandB::trueResult() {
	return a & b;
}

//====================================================

class AorB: public booleanOperation {
public:
	AorB(int nLayer, int *nPerLayer);
	virtual bool trueResult();
};

AorB::AorB(int nLayer, int *nPerLayer): booleanOperation(1,nLayer,nPerLayer) { }

bool AorB::trueResult() {
	return a | b;
}

//====================================================

class AxorB: public booleanOperation {
private:
	bit bias;
public:
	AxorB(int nLayer, int *nPerLayer);
	virtual bool trueResult();
};

AxorB::AxorB(int nLayer, int *nPerLayer): booleanOperation(1,nLayer,nPerLayer), bias(1.0) {
	neurons[0][0]->connectObject(&bias);
	neurons[0][1]->connectObject(&bias);
	neurons[1][0]->connectObject(&bias);
}

bool AxorB::trueResult() {
	return a ^ b;
}

//====================================================

class AequalB: public booleanOperation {
public:
	AequalB(int nLayer, int *nPerLayer);
	virtual double error(double res);
	virtual bool trueResult() {return false;};
};

AequalB::AequalB(int nLayer, int *nPerLayer): booleanOperation(4,nLayer,nPerLayer) { }

double AequalB::error(double res) {
	if(a == b){
		//printf("Equal\n");
		if((res<=0.6) && (res >= 0.4)) {
			//printf("Yeah\n");
			return 0.0;
		} else { 
			return 1.0;
		}
	} else {
		return 1.0;
	}
}

//====================================================

class biasWheel {
protected:
	void **objs;				// The objects
	double *probs;			// The proba
	double *normProbs;	// The normalised proba 
	int *indexes;				// The indexes of object in intial list
	double normProbSum;	// The sum of all normalised proba
	int szMax;					// The max size of bias wheel
	int curSz;					// The current size of bias wheel
public:
	typedef struct _couple {
		void *A;
		int iA;
		double pA;
		void *B;
		int iB;
		double pB;
	} couple;
	biasWheel(int sz);
	virtual ~biasWheel();
	virtual void print(void);
	virtual void normilize(void);
	virtual void addObject(void *obj, double proba, int index = 0);
	virtual void elect(void **obj, double *prob, int *ind);
	virtual void electCouple(couple *c);
};

biasWheel::biasWheel(int sz) {
	curSz = 0;
	szMax = sz;
	normProbSum = 0;
	objs = (void**) malloc (sizeof(void*)*sz);
	indexes = (int*) malloc (sizeof(int)*sz);
	probs = (double*) malloc (sizeof(double)*sz);
	normProbs = (double*) malloc (sizeof(double)*sz);
}

biasWheel::~biasWheel() {
	free(objs);
	free(indexes);
	free(probs);
	free(normProbs);
}

void biasWheel::print() {
	for(int i = 0; i<curSz; i++) {
		printf("%p - %03d - %1.6f - %1.6f\n",objs[i],indexes[i],probs[i],normProbs[i]);
	}
}

void biasWheel::normilize(void){
	for(int i = 0; i<curSz; i++) {
		
	}
};

void biasWheel::addObject(void *obj, double proba, int index) {
	if(curSz<szMax) {
		objs[curSz] = obj;
		probs[curSz] = (proba);
		normProbs[curSz] = (proba)*100;
		indexes[curSz] = index;
		normProbSum += (proba)*100;
		//printf("proba %f\n", proba);
		//printf("proba = %f\n",probs[curSz]);
		//printf("probaSum = %f\n", normProbSum);
		curSz++;
	}
}

void biasWheel::elect(void **obj, double *prob, int *index) {
	double ind = ((double)rand()/((double)RAND_MAX))*normProbSum;
	double d0 = 0;
	int i;
	//double d1 
	for(i = 0; i<curSz;i++) {
		d0 += normProbs[i];
		//printf("d0 %f\n",d0);
		//d1 = prob[i+1];
		if(ind<=d0) {
			break;
		}
	}
	*obj = objs[i];
	*prob = probs[i];
	*index = indexes[i]; 
}

void biasWheel::electCouple(couple *c) {
	void *a,*b;
	int ia,ib;
	double pa,pb;
	elect(&a,&pa,&ia);
	elect(&b,&pb,&ib);
	while(b==a) {
		elect(&b,&pb,&ib);
	}
	
	c->A = a;
	c->iA = ia;
	c->pA = pa;
	
	c->B = b;
	c->iB = ib; 
	c->pB = pb;
}


//====================================================
// 			framework to run simulation
//====================================================

class genetics {
protected:
	int nNets;					
	network **nets;			// Networks represent the population at a given time
	network **childNets;// Chikd represent the new population create after selection
	double **errors;  	// Save the result of a competition at a given time
	double *meanErr;		// Save the mean error over 1 competition step
	int compSize;				// Size of the competition (how many time we run the fitting test)
	int genIndex;				// Generation index of the population
public:
	genetics(int nNets, network **networks, network **childnetworks);		
	void compete(int its);		
	void select(int chunk, int nbiasWheel, double mutFactor = 0.001, double crossOverFactor = 0.7, int nCpy = 10);
	void lovemaking(network *mom, network *dad, network *daughter, network *son, double mutFactor, double crossOverFactor);
	void sort(void);
	void step(void);
	void print(void);
	void fit(void);
};

genetics::genetics(int nNetworks, network **networks, network **childnetworks) {
	nNets = nNetworks;
	nets = networks;
	childNets = childnetworks;
	errors = (double**) malloc (sizeof(double*)*nNets);
	meanErr = (double*) malloc (sizeof(double)*nNets);
	compSize = 0;
	genIndex = 0;
}

void genetics::compete(int its) {
	// allocs
	if(its > compSize) {
		//printf("Do Allocation for errors\n");
		if(compSize == 0) {
			for(int i = 0; i<nNets; i++){
				errors[i] = (double*) malloc(sizeof(double)*its);
			} 
		} else {
			for(int i = 0; i<nNets; i++){
				errors[i] = (double*) realloc(errors[i],sizeof(double)*its);
			} 
		}
		compSize = its;
	}
	int track = 0;
	int total = (its * nNets);
	// competition
	memset(meanErr,0, sizeof(double)*nNets);
	for(int i = 0; i<its; i++) {
		void *data = nets[0]->getInputData();
		for(int j = 0; j<nNets; j++) {
			nets[j]->setInputData(data);
			nets[j]->step();
			nets[j]->output(f,1);
			double e = nets[j]->error(f[0]);
			errors[j][i] = e;
			meanErr[j] += e;
		}
		free(data);
	}
	// Compute the mean error => short
	for(int i = 0; i<nNets; i++) {
		meanErr[i] /= its;
		//printf("bef- meanErr[%d] %f %d\n",i,meanErr[i],its);
		/*
		if(meanErr[i] == 0.0) {
			printf("Normal???\n");
			nets[i]->extractGenome(true);
			while(1) {
				void *data = nets[i]->getInputData();
				nets[i]->setInputData(data);
				nets[i]->step();
				nets[i]->output(f,1);
				double e = nets[i]->error(f[0]);
				printf("%d %d %f %f\n",((booleanOperation*)nets[i])->getA(),((booleanOperation*)nets[i])->getB(),f[0],e);
				free(data);
			}	
		}
		*/
	}
}

void genetics::sort(void) {
	FILE *f = fopen("meanErr.csv","a+");
	// Very naive sorting  
	// (create temporary arrays)
	bool *sorted = (bool*) malloc(sizeof(bool)*nNets);
	for(int i = 0; i<nNets; i++) {
		sorted[i] = false;
	}
	// Allocate new table of network
	network **sortedNets = (network**) malloc(sizeof(network*)*nNets);
	double *sortedMeanErr = (double*) malloc (sizeof(double)*nNets);
	for(int i = 0; i<nNets; i++) {
		double min = 1.0;
		int selIndex = 0;
		network *netmin = NULL;
		for(int j = 0; j<nNets; j++) {
			if(!sorted[j]) {
				if(meanErr[j]<=min) {
					selIndex = j;
					netmin = nets[j];
					min = meanErr[j];
				}
			}
		}
		sortedNets[i] = netmin;
		sortedMeanErr[i] = min;
		sorted[selIndex] = true;
	}
	memcpy(meanErr,sortedMeanErr,sizeof(double)*nNets);
	memcpy(nets,sortedNets,sizeof(network *)*nNets);
	// Compute population meanError
	double popMean = 0;
	for(int i = 0; i<nNets; i++) {
		popMean += meanErr[i];
		fprintf(f,"%1.5f;",meanErr[i]);
		//printf("%1.5f ",meanErr[i]);
	}
	/*
	for(int i = 0; i<nNets; i++) {
		printf("aft- meanErr[%d] %f\n",i,meanErr[i]);
	}
	*/
	fprintf(f,"\n");
	popMean /= nNets;
	if(meanErr[0]<bestEver)
		bestEver = meanErr[0];
	printf("PopMean - %f\t MinErr - %1.6f/%1.6f\n",popMean,meanErr[0],bestEver);
	free(sortedNets);
	free(sortedMeanErr);
	free(sorted);
	fclose(f);
}

void genetics::select(int chunk, int nBiasWheel, double mutFactor, double crossOverFactor, int nCpy) {

	for(int it = 0; it<nNets; it+=chunk) {
		RandomList *rl = new RandomList(nNets);
		int* mixedArray = rl->GetMixedArray();
		biasWheel *bw = new biasWheel(nBiasWheel);
		//printf("===== Selection =====\n");
		for(int i = 0; i < nBiasWheel; i++) {
			int ind = mixedArray[i];
			//printf("%1.5f\n",1-meanErr[ind]);
			bw->addObject(nets[ind],1-meanErr[ind],ind);
		}
		//printf("=====================\n");
		//bw->print();
	
#ifdef BIASWHEEL_STATS
		int *selectionIndexes = (int*) malloc (sizeof(int)*nNets);
		double *selectionProb = (double*) malloc (sizeof(double)*nNets);
		for(int p = 0; p<nNets; p++){
			selectionIndexes[p] = 0;
			selectionProb[p] = 0.0;
		}
#endif

		for(int k = 0; k<chunk; k+=2) {
			biasWheel::couple c;
			bw->electCouple(&c);
	
			//printf("Couple elected %p %p\n",c.A,c.B);
	
			network *mom = (network*)c.A;
			network *dad = (network*)c.B;

#ifdef BIASWHEEL_STATS
			printf("%03d %03d %1.6f %1.6f\n",c.iA,c.iB,c.pA,c.pB);
			selectionIndexes[c.iA]++;
			selectionIndexes[c.iB]++;
			
			selectionProb[c.iA] = c.pA;
			selectionProb[c.iB] = c.pB;
#endif
			lovemaking(mom, dad, childNets[it+k], childNets[it+k+1], mutFactor, crossOverFactor);
		}
		
		// TODO Copy the N best elements into child population
		mixedArray = rl->GetMixedArray();
		for(int i = 0; i<nCpy; i++) {
			int ind = mixedArray[i];
			double *top = nets[i]->extractGenome(false);
			childNets[ind]->setGenome(top);
			free(top);
		}
		
		// Alternate between child an parents
		for(int i = 0; i<nNets; i++){
			network *tmp = nets[i];
			if(tmp == NULL){
				printf("%d #YOLO MAIS YOLO\n",i);
				while(1);
			}
			nets[i] = childNets[i];
			childNets[i] = tmp;
		}

#ifdef BIASWHEEL_STATS
		printf("--BW_STATS-\n");
		for(int i = 0; i< nNets; i++) {
			if(selectionProb[i] != 0.0)
			printf("%03d;%1.5f\n",selectionIndexes[i],selectionProb[i]);
		}

		free(selectionIndexes);
		free(selectionProb);
#endif
		delete(bw);
		delete(rl);
	}
}


void genetics::lovemaking(network *mom, network *dad, network *daughter, network *son, double mutFactor, double crossOverFactor) {
	double *genMom = mom->extractGenome(false);
	int genMomSz = mom->genomeSize();
	double *genDad = dad->extractGenome(false);
	int genDadSz = dad->genomeSize();


	if(genMomSz != genDadSz) {
		int d = 0;
		printf("This genetics don't gonna work");
		d = 1/d;
	}
	
	double *genChildDaughter = (double*) malloc (sizeof(double)*genMomSz);
	double *genChildSon = (double*) malloc (sizeof(double)*genMomSz);
	
	bool condom = (RandomList::RandAB(0,1)>=crossOverFactor)?false:true;
	if (!condom) {
		int margeIn = 1;
		int cutPlace = (int) RandomList::RandAB(margeIn,genMomSz-margeIn);

		//printf("[%d] Cutting place %d\n",it,cutPlace);
		for(int i = 0; i<genMomSz; i++){
			if(i<cutPlace) {
				genChildDaughter[i] = genMom[i];
				genChildSon[i] = genDad[i];
			} else {
				genChildDaughter[i] = genDad[i];
				genChildSon[i] = genMom[i];
			}
			//printf("Child [%d], %f\n",i,genChild[i]);
		}
	
		// Mutate
		for(int i = 0; i< genMomSz; i++){
			double mutationProb = RandomList::RandAB(0,1);
			if(mutationProb <= mutFactor) {
				//genChildDaughter[i] = 1-genChildDaughter[i];
				genChildDaughter[i] = (double)rand()/(double)(RAND_MAX/2.0)-1.0;
				//printf("%d - prob %f - Applying new weight %1.5f\n",i,mutationProb,genChildDaughter[i]);
			}
			mutationProb = RandomList::RandAB(0,1);
			if(mutationProb <= mutFactor) {
				//genChildDaughter[i] = 1-genChildDaughter[i];
				genChildDaughter[i] = (double)rand()/(double)(RAND_MAX/2.0)-1.0;
				//printf("%d - prob %f - Applying new weight %1.5f\n",i,mutationProb,genChildDaughter[i]);
			}
		}
		
	} else {
		// When no crossover copy mom and dad genome into childs
		for(int i = 0; i<genMomSz; i++){
			genChildSon[i] = genDad[i];
			genChildDaughter[i] = genMom[i];
		}
	}
	son->setGenome(genChildSon);
	daughter->setGenome(genChildDaughter);
	free(genChildSon);
	free(genChildDaughter);
	free(genMom);
	free(genDad);
}

void genetics::step(void) {
	for(int i = 0; i<nNets; i++) {
		nets[i]->step();
	}
}

void genetics::print(void) {
	for(int i = 0; i<nNets; i++) {
		nets[i]->print();
	}
}

void genetics::fit(void) {
	double max = 0.0;
	int index = 0;
	for(int i = 0; i<nNets; i++) {
		int r = nets[i]->output(f,1);
		if(f[0]>max) {
			max = f[0];
			index = i;
		}
		printf("Result(%d) = %1.5f\n",i,f[0]);
	}
	printf("%f - %d\n",max,index);
}


//====================================================

void biasWheelTest() {
	double prob[50];
	int point[50];
	int nElect[50];
	
	for(int i = 0; i<50; i++) {
		point[i] = i;
		prob[i] = 0.49+i*0.01;
		printf("prob[%d] = %f\n",i,0.45+i*0.001);
	}
	
	biasWheel *bw = new biasWheel(50);
	
	for(int i = 0; i <50; i++) {
		bw->addObject(point+i,prob[i],i);
	}
	
	for(int i=0; i<1000; i++) {
		void *p;
		double prob;
		int j;
		bw->elect(&p,&prob,&j);
		//printf("%d, %d\n",*((int*)p),j);
		nElect[*((int*)p)]++;
	}
	
	for(int i = 0; i<50; i++) {
		printf("%02d\n",nElect[i]);
	}
	
}

//====================================================

/*
int main(int argc, char* args[])
{
    //Start SDL
    SDL_Init(SDL_INIT_EVERYTHING);

    //Quit SDL
    SDL_Quit();

    return 0;
}*/

int main(int argc, char* argv[]) {

	int nNetworks = 1000;
	int competition = 100;
	int chunk = 50;
	int nbiaisWheel = 50;
	double mutFactor = 0.001;
	double crossOverFactor = 0.7;
	int nCpy	= 10;
	bestEver = 100.0;

	if(argc == 8) {
		for(int i = 1; i<argc; i++) {
			printf("%f\n",atof(argv[i]));
			switch(i) {
				case 1:
					nNetworks = atoi(argv[i]);
					break;
				case 2:
					competition = atoi(argv[i]);
					break;
				case 3:
					chunk = atoi(argv[i]);
					break;
				case 4:
					nbiaisWheel = atoi(argv[i]);
					break;
				case 5:
					mutFactor = atof(argv[i]);
						break;
				case 6:
					crossOverFactor = atof(argv[i]);
					break;
				case 7:
					nCpy = atoi(argv[i]);
					break;
				default:
					break;
				}
		}
	}	else {
		printf("Argument Needed : nNets CompSize Chunks nBiasWheel mutFact crossOverFact nCpy\n");
		return 0;
	}
	
	srand(time(NULL));
	printf("neuronal network\n");
	
	// truncate file
	FILE *f = fopen("meanErr.csv","w+");
	fclose(f);
	
#ifdef FALSE	
	biasWheelTest();
#else

// #define AANDB
// #define AORB
// #define ASUPB
// #define AEQUALB

#if defined(AANDB)

	// Create topology
	int topo[3] = {4,2,1};
	
	// Networks alloc
	AandB **n = (AandB**) malloc(sizeof(AandB*)*nNetworks);
	for(int i = 0; i<nNetworks; i++) {
		n[i] = new AandB(3,topo);
	}
	
	AandB **cn = (AandB**) malloc(sizeof(AandB*)*nNetworks);
	for(int i = 0; i<nNetworks; i++) {
		cn[i] = new AandB(3,topo);
	}
#elif defined (ASUPB)

	// Create topology
	int topo[3] = {4,2,1};
	
	// Networks alloc
	AsupB **n = (AsupB**) malloc(sizeof(AsupB*)*nNetworks);
	for(int i = 0; i<nNetworks; i++) {
		n[i] = new AsupB(4,3,topo);
	}
	
	AsupB **cn = (AsupB**) malloc(sizeof(AsupB*)*nNetworks);
	for(int i = 0; i<nNetworks; i++) {
		cn[i] = new AsupB(4,3,topo);
	}
#elif defined (AORB)
	// Create topology
	int topo[2] = {4,1};
	
	// Networks alloc
	AorB **n = (AorB**) malloc(sizeof(AorB*)*nNetworks);
	for(int i = 0; i<nNetworks; i++) {
		n[i] = new AorB(2,topo);
	}
	
	AorB **cn = (AorB**) malloc(sizeof(AorB*)*nNetworks);
	for(int i = 0; i<nNetworks; i++) {
		cn[i] = new AorB(2,topo);
	}
	
#elif defined (AXORB)
	// Create topology
	#define NLAYER		2
	int topo[NLAYER] = {2,1};
	
	// Networks alloc
	AxorB **n = (AxorB**) malloc(sizeof(AxorB*)*nNetworks);
	for(int i = 0; i<nNetworks; i++) {
		n[i] = new AxorB(NLAYER,topo);
	}
	
	AxorB **cn = (AxorB**) malloc(sizeof(AxorB*)*nNetworks);
	for(int i = 0; i<nNetworks; i++) {
		cn[i] = new AxorB(NLAYER,topo);
	}
	
#elif defined (AEQUALB)

	#warning "EQUALB"
	// Create topology
	int topo[3] = {4,2,1};
	
	// Networks alloc
	AequalB **n = (AequalB**) malloc(sizeof(AequalB*)*nNetworks);
	for(int i = 0; i<nNetworks; i++) {
		n[i] = new AequalB(2,topo);
	}
	
	AequalB **cn = (AequalB**) malloc(sizeof(AequalB*)*nNetworks);
	for(int i = 0; i<nNetworks; i++) {
		cn[i] = new AequalB(2,topo);
	}
	
#endif

	n[0]->print();

	genetics gen = genetics(nNetworks,(network**)n,(network**)cn);
	bool running = true;
	time_t date_in = time(NULL);
	double bestEver = 1000.0;
	while(running) {
		gen.compete(competition);		
		gen.sort();
		gen.select(chunk,nbiaisWheel,mutFactor,crossOverFactor);
		double err[5];
		//n[0]->extractGenome(true);
		double e = 0;
		int nIt = 2000;
		// Run competition to check if the best element can handle the problem
		for(int i = 0 ; i<nIt; i++) {
			void *data = n[0]->getInputData();
			n[0]->setInputData(data);
			//n[0]->step();
			n[0]->output(err,1);
			double er = n[0]->error(err[0]);
			e += er;
			if((i%100)==0)
				printf("%03d %03d %1.5f %1.5f\n",((booleanOperation*)n[0])->getA(),((booleanOperation*)n[0])->getB(),err[0],er);
			free(data);

		}
		if(e/nIt == 0.0)
			running = false;
	}
	time_t date_out = time(NULL);
	n[0]->extractGenome(true);
	printf("Temps execution boucle : %f\n",difftime(date_out,date_in));
	
#endif
	return 0;
}
