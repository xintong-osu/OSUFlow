#ifndef DATA_MGR_H
#define DATA_MGR_H

#include "OSUFlow.h"
#include "vector_types.h"
#include "vector_functions.h"

class DataMgr
{
public:
	DataMgr();
	~DataMgr();
	void initOSUFlow();
	void compute_streamlines() ;
	//VECTOR3 ConvertCoordinates(VECTOR3 v);
	float3 ConvertCoordinates(VECTOR3 v);
	void ClearSeeds();
	void InsertSeed(VECTOR3 v);
	VECTOR3 ConvCoordsLeap2Data(float x, float y, float z);
	float3 ConvertCoordinates(float3 v);
	vector<vector<float3>>* GetStreamLines();
	void GetBBox(float3* coords);
	void GetDataDomain(float3 &min, float3 &max);
	float3 GetDataCenter();
	float GetDomainSize();
	vector<vector<float3>> compute_streamlines(vector<float3> sd) ;

private:
	char *szVecFilePath;	// ADD-BY-LEETEN 09/29/2012
	OSUFlow *osuflow; 
	VECTOR3 minLen, maxLen; 
	list<vtListSeedTrace*> sl_list; 
	float center[3], len[3]; 
	// ADD-BY-LEETEN 07/07/2010-BEGIN
	list<VECTOR4> liv4Colors;
	// ADD-BY-LEETEN 07/07/2010-END
	vector<VECTOR3> _seeds;
	vector<vector<float3>> _streamlines;
};


#endif