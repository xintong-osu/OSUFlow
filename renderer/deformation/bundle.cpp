#include "Bundle.h"
//#include "DistanceBWsl.h"
#include "float.h"
#include <fstream>
#include <iostream>
#include <sstream>

//in each streamline, there is one less point in sllist than sl_list in "gcbFileRenderer.cpp" because the first point does not have gradiant.
vector<vector<VECTOR3> > sllist;                  //streamlines before segmentation
bool myComp(set<int> i, set<int> j){return i.size()>j.size();}


void BFSsearch(clusterTreeNode* node, set<int>& clusterMembers)
{
    if(node->left!=NULL && node->right!=NULL)
    {
        BFSsearch(node->left,clusterMembers);
        BFSsearch(node->right,clusterMembers);
    }
    else
        clusterMembers.insert(node->ID);
}

void travelTree(clusterTreeNode* node, int totalLevel, int numOfClusters, vector<set<int> >& clusters)
{
    if(node->level>(totalLevel - numOfClusters))
    {
        travelTree(node->left,totalLevel,numOfClusters,clusters);
        travelTree(node->right,totalLevel,numOfClusters,clusters);
    }
    else
    {
        set<int> clusterMembers;
        BFSsearch(node,clusterMembers);
        clusters.push_back(clusterMembers);
    }
}

void bundle::clustering()
{
	root = treecluster (numStream, numStream, NULL, NULL,  NULL,
		0, 'b', 'm', distmatrix);

}


bundle::bundle()
{
	//numStream = 0;
}

void bundle::_SetData(vector<VECTOR4*> base, vector<int> length)
{
	numStream = length.size();
	for(int i = 0; i < base.size(); i++)
	{
		vector<VECTOR3> oneLine;

		for(int j = 0; j < length[i]; j++)
		{
			oneLine.push_back(*(new VECTOR3(base[i][j][0], base[i][j][1], base[i][j][2])));
		}
		sllist.push_back(oneLine);
	}
}



void bundle::getClusters(int numOfClusters)
{
#if 0
 //   myWindow* theBoss = (myWindow*)(this->parent());
    string message1;
    HierarchicalClusters.clear();
    int totalNumOfStreamlines = numStream;
    travelTree(HierarchicalClusterRoot,totalNumOfStreamlines,numOfClusters,HierarchicalClusters);
    cout<<"Number of clusers:"<<HierarchicalClusters.size()<<endl;
    sort(HierarchicalClusters.begin(),HierarchicalClusters.end(),myComp);
    for(int i=0;i<HierarchicalClusters.size();i++)
    {
        char a[100];
        itoa((int)HierarchicalClusters[i].size(),a,10);
        message1 = message1+string(a)+";";
        
    }
    
   // QString Msg = QString(QString::fromLocal8Bit(message1.c_str()));
  //  theBoss->displayMessage(Msg);
    
  //  displayAllStreamlines == true;
   // displayStreamlinesCluster();
  //  glutSetWindow(id1);
  //  glutPostRedisplay();
#else
	clusterid = (int*)malloc(numStream * sizeof(int));
	double *error = new double[numOfClusters];
	int ifound;
	cuttree(numStream, root, numOfClusters, clusterid);
/*	kcluster(numOfClusters, numStream, numStream,
  NULL, NULL, NULL, 0,
  20, 'a', 'e',
  clusterid, error, &ifound);
  */
#endif
}

int* bundle::getClusterId()
{
	return clusterid;
}


void bundle::_SaveBundleFile(char *filename)
{
	ofstream myfile;
	myfile.open(filename);
	if (!myfile.is_open())
	{
		cout<<"cannot open file: "<<filename<<endl;
	}
	for(int i = 0; i < HierarchicalClusters.size(); i++)
	{
		set<int>::iterator it;
		for(it = HierarchicalClusters[i].begin(); it != HierarchicalClusters[i].end(); ++it)
		{
			myfile << *it<<"\t";
		}
		myfile<<endl;
	}
	myfile.close();
}

void bundle::getDist(char* filename)
{
	ifstream ifs;
	ifs.open(filename);
	if(!ifs.is_open())
	{
		cout<<"Open file "<<filename<<" failed..."<<endl;
		exit(1);
	}
	char line[100000];
	double d;
	while(!ifs.eof())
	{
		ifs.getline(line, 100000);
		if(line[0] == '\0')
			break;
		stringstream iss(line);
		vector<double> oneline;
		while(true)
		{
			iss >> d;
			if(iss.eof())
				break;
			oneline.push_back(d);
		}
		distmatrixVec.push_back(oneline);
	}

	numStream = distmatrixVec.size() + 1;

	distmatrix = (double**)malloc(numStream * sizeof(double*));
	for(int i = 0; i < numStream; i++)
		distmatrix[i] = (double*)malloc(numStream * sizeof(double));

	//assign distance to 2D array
	for(int i = 1; i < numStream; i++)
	{
		for(int j = 0; j < i; j++)
		{
			float tmp1;
			tmp1 = distmatrixVec[i - 1][j];
			distmatrix[i][j] = tmp1;
			float tmp = tmp1 ;
			//symetric
			distmatrix[j][i] = tmp ;
		}
	}

	//distance with itself
	for(int i =0; i < numStream; i++)
		distmatrix[i][i] = 0;

}


//
//void bundle::HierarchicalClustering()
//{
////    myWindow* theBoss = (myWindow*)(this->parent());
////    theBoss->displayMessage("Calculating similarity matrix...");
//    float maxDist = FLT_MIN;
////    double startTime = GetTickCount();
//    float** similarityMatrix;
//    similarityMatrix = (float**)malloc(numStream*sizeof(float*));
//
//    for(int i=1;i<numStream;i++)
//    {
//        similarityMatrix[i] = (float*)malloc(i*sizeof(float));
//        for(int j=0;j<i;j++)
//        {
//            //float dist = _2DEuclideanDTW(_2DcurvatureCDF[i],_2DcurvatureCDF[j]);
//            //dist = dist + _2DEuclideanDTW(_2DtorsionCDF[i],_2DtorsionCDF[j]);
//            //dist = dist+_2DEuclidean(_2DtorsionCDF[i],_2DtorsionCDF[j]);
//
//            //float dist = _2DEuclideanDTW(_2DcurlCDF[i],_2DcurlCDF[j]);
//            //float dist = _2DEuclidean(_2DcurlCDF[i],_2DcurlCDF[j]);
//            //float dist = _endPoint(sllist[i],sllist[j]);
//            //float dist = _DTWDistance(this->curvature[i],this->curvature[j]);
//            float dist = _Hausdorff(sllist[i],sllist[j]);
//            //float dist = _MeanMin(sllist[i],sllist[j]);
//            similarityMatrix[i][j] = dist;
//            if(maxDist<dist)
//                maxDist = dist;
//        }
//        //theBoss->setProcessBar((int)((float)i/numStream*100.0f));
//    }
// //   double endTime = GetTickCount();
// //   double t = (double)(endTime-startTime);
//    char a1[100];
// //   gcvt(t,7,a1);
// //   string message = "Running time is "+string(a1)+".";
////    QString Msg = QString(QString::fromLocal8Bit(message.c_str()));
// //   theBoss->displayMessage(Msg);
// //   theBoss->displayMessage("End.");
//
//    for(int i=1;i<numStream;i++)
//    {
//        for(int j=0;j<i;j++)
//        {
//            similarityMatrix[i][j] = similarityMatrix[i][j]/maxDist;
//        }
//    }
//
//    set<int> remainClusters;
//    vector<clusterSet> active;
//    for(int i=0;i<numStream;i++)
//    {
//        remainClusters.insert(i);
//        clusterSet cs;
//        cs.ids.insert(i);
//        cs.node = new clusterTreeNode();
//        cs.node->ID = i;
//        cs.node->left = NULL;
//        cs.node->right = NULL;
//        cs.node->level= 0;
//        cs.node->size = 1;
//        cs.clusterId = i;
//        active.push_back(cs);
//    }
//
//    int left;
//    int right;
//    int level = 1;
//     //   theBoss->displayMessage("Begin Hierarchical Clustering...");
//
//    float** clusterDistance;
//    clusterDistance = (float**)malloc(numStream*sizeof(float*));
//    for(int i=1;i<numStream;i++)
//    {
//        clusterDistance[i] = (float*)malloc(i*sizeof(float));
//        for(int j=0;j<i;j++)
//        {
//            clusterDistance[i][j] = similarityMatrix[i][j];
//        }
//    }
//
//
// //   startTime = GetTickCount();
//    while(active.size() > 1)
//    {
//        float bestD = FLT_MAX;
//        float bestD1 = FLT_MAX;
//        for(int i=0;i<active.size();i++)
//            for(int j=0;j<active.size();j++)
//            {
//                if(active[i].clusterId > active[j].clusterId)
//                {
//                    int numClusters = active.size();
//                    int numStreamlines = sllist.size();
//                    float balanceNumber = (float)(numStreamlines)/(float)(numClusters);
//                    //if(active.size() >=3)
//                    {
//                        // && active[i].ids.size() < 0.7*sllist.size() && active[j].ids.size() < 0.7*sllist.size()
//                        float ddistance = clusterDistance[active[i].clusterId][active[j].clusterId]+0.7f*(active[i].ids.size()+active[j].ids.size())/(float)numStreamlines;
//                        //if((clusterDistance[active[i].clusterId][active[j].clusterId] < bestD && active[i].ids.size() < 5.0*balanceNumber && active[j].ids.size() < 5.0f*balanceNumber)|| (fabs(bestD - bestD1)>=0.5))
//                        if(ddistance < bestD)
//                        {
//                            //bestD1 = bestD;
//                            //bestD = clusterDistance[active[i].clusterId][active[j].clusterId];
//                            bestD = ddistance;
//                            left = j;
//                            right = i;
//                        }
//                    }
//                    /*else
//                    {
//                    if(clusterDistance[active[i].clusterId][active[j].clusterId])
//                    {
//                    bestD = clusterDistance[active[i].clusterId][active[j].clusterId];
//                    left = j;
//                    right = i;
//                    }
//                    }*/
//                }
//            }
//        set<int> newSet;
//        set<int>::iterator sit;
//        set<int>::iterator jsit;
//        for(sit=active[left].ids.begin();sit!=active[left].ids.end();sit++)
//            newSet.insert(*sit);
//        for(sit=active[right].ids.begin();sit!=active[right].ids.end();sit++)
//            newSet.insert(*sit);
//
//        if(left > right)
//        {
//            clusterSet cs;
//            cs.clusterId = active[right].clusterId;
//            cs.ids=newSet;
//            cs.node = new clusterTreeNode();
//            cs.node->level = level;
//            cs.node->left = active[left].node;
//            cs.node->right = active[right].node;
//            cs.node->size = cs.ids.size();
//            active.erase(active.begin()+left);
//            active.erase(active.begin()+right);
//            active.push_back(cs);
//
//            for(int i=0;i<active.size();i++)
//            {
//                float dist=FLT_MIN;
//                if(active[i].clusterId<cs.clusterId)
//                {
//                    for(sit=newSet.begin();sit!=newSet.end();sit++)
//                        for(jsit=active[i].ids.begin();jsit!=active[i].ids.end();jsit++)
//                        {
//                            if(*sit > *jsit)
//                            {
//                                if(dist < similarityMatrix[*sit][*jsit])
//                                    dist = similarityMatrix[*sit][*jsit];
//                            }
//                            else
//                            {
//                                if(dist < similarityMatrix[*jsit][*sit])
//                                    dist = similarityMatrix[*jsit][*sit];
//                            }
//                        }
//                    clusterDistance[cs.clusterId][active[i].clusterId] = dist;
//                }
//                else if(active[i].clusterId>cs.clusterId)
//                {
//                    for(sit=newSet.begin();sit!=newSet.end();sit++)
//                        for(jsit=active[i].ids.begin();jsit!=active[i].ids.end();jsit++)
//                        {
//                            if(*sit > *jsit)
//                            {
//                                if(dist < similarityMatrix[*sit][*jsit])
//                                    dist = similarityMatrix[*sit][*jsit];
//                            }
//                            else
//                            {
//                                if(dist < similarityMatrix[*jsit][*sit])
//                                    dist = similarityMatrix[*jsit][*sit];
//                            }
//                        }
//                    clusterDistance[active[i].clusterId][cs.clusterId] = dist;
//                }
//            }
//        }
//        else
//        {
//            clusterSet cs;
//            cs.clusterId = active[left].clusterId;
//            cs.ids=newSet;
//            cs.node = new clusterTreeNode();
//            cs.node->level = level;
//            cs.node->left = active[left].node;
//            cs.node->right = active[right].node;
//            cs.node->size = cs.ids.size();
//            active.erase(active.begin()+right);
//            active.erase(active.begin()+left);
//            active.push_back(cs);
//
//            for(int i=0;i<active.size();i++)
//            {
//                float dist=FLT_MIN;
//                if(active[i].clusterId<cs.clusterId)
//                {
//                    for(sit=newSet.begin();sit!=newSet.end();sit++)
//                        for(jsit=active[i].ids.begin();jsit!=active[i].ids.end();jsit++)
//                        {
//                            if(*sit > *jsit)
//                            {
//                                if(dist < similarityMatrix[*sit][*jsit])
//                                    dist = similarityMatrix[*sit][*jsit];
//                            }
//                            else
//                            {
//                                if(dist < similarityMatrix[*jsit][*sit])
//                                    dist = similarityMatrix[*jsit][*sit];
//                            }
//                        }
//                    clusterDistance[cs.clusterId][active[i].clusterId] = dist;
//                }
//                else if(active[i].clusterId>cs.clusterId)
//                {
//                    for(sit=newSet.begin();sit!=newSet.end();sit++)
//                        for(jsit=active[i].ids.begin();jsit!=active[i].ids.end();jsit++)
//                        {
//                            if(*sit > *jsit)
//                            {
//                                if(dist < similarityMatrix[*sit][*jsit])
//                                    dist = similarityMatrix[*sit][*jsit];
//                            }
//                            else
//                            {
//                                if(dist < similarityMatrix[*jsit][*sit])
//                                    dist = similarityMatrix[*jsit][*sit];
//                            }
//                        }
//                    clusterDistance[active[i].clusterId][cs.clusterId] = dist;
//                }
//            }
//        }
//
//
//
//        level++;
//    //    theBoss->setProcessBar((int)((float)level/numStream*100.0f));
//    }
//  //  endTime = GetTickCount();
//  //  t = (double)(endTime-startTime);
//  //  char a2[100];
// //   gcvt(t,7,a2);
//  //  string message2 = "Running time is "+string(a2)+".";
//  //  QString Msg2 = QString(QString::fromLocal8Bit(message2.c_str()));
// //   theBoss->displayMessage(Msg2);
//  //  cout<<"done"<<endl;
//    HierarchicalClusterRoot = active[0].node;
//
//}