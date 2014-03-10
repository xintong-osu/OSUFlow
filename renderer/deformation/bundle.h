#ifndef BUNDLE_H
#define BUNDLE_H
#include <set>
#include <vector>
#include "VectorMatrix.h"
#include "cluster.h"
using namespace std;

struct clusterTreeNode
{
    int level;
    int size;
    int ID;
    clusterTreeNode* left;
    clusterTreeNode* right;
};

struct clusterSet
{
    clusterTreeNode* node;
    set<int> ids;
    int clusterId;
};

class bundle
{
	int numStream;
	clusterTreeNode *HierarchicalClusterRoot;
	vector<set<int> > HierarchicalClusters;
	double** distmatrix;
	vector<vector<double>> distmatrixVec;
	int *clusterid;

public:
	bundle();
	void _SetData(vector<VECTOR4*> base, vector<int> length);
//	void HierarchicalClustering();
	void getClusters(int numOfClusters);
	void _SaveBundleFile(char *filename);

	void getDist(char* filename);
	void clustering();
	int* getClusterId();


	Node* root;
};

#endif //BUNDLE_H