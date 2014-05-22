#include "DataMgr.h"

void DataMgr::initOSUFlow()
{
			///////////////////////////////////////////////////////////////
	// initialize OSU flow
	osuflow = new OSUFlow(); 

	// load the scalar field
	//LOG(printf("read file %s\n", argv[1])); 

	//osuflow->LoadData((const char*)argv[1], true); //true: a steady flow field 
	//osuflow->LoadData("D:/Dropbox/gesture/OSUFlow/sample_data/regular/circle.vec", true); //true: a steady flow field 
	osuflow->LoadData("D:/data/isabel/UVWf01.vec", true); //true: a steady flow field 
	//osuflow->LoadData("D:/data/plume/15plume3d421.vec", true); //true: a steady flow field 
	
	//szVecFilePath = argv[1];	// ADD-BY-LEETEN 09/29/2012

	// comptue the bounding box of the streamlines 
	VECTOR3 minB, maxB; 
	osuflow->Boundary(minLen, maxLen); // get the boundary 
	minB[0] = minLen[0]; minB[1] = minLen[1];  minB[2] = minLen[2];
	maxB[0] = maxLen[0]; maxB[1] = maxLen[1];  maxB[2] = maxLen[2];
	len[0] = maxLen[0] - minLen[0];
	len[1] = maxLen[1] - minLen[1]; 
	len[2] = maxLen[2] - minLen[2];
	//  osuflow->SetBoundary(minB, maxB);  // set the boundary. just to test
											// the subsetting feature of OSUFlow
	printf(" volume boundary X: [%f %f] Y: [%f %f] Z: [%f %f]\n", 
								minLen[0], maxLen[0], minLen[1], maxLen[1], 
								minLen[2], maxLen[2]); 

	center[0] = (minLen[0]+maxLen[0])/2.0; 
	center[1] = (minLen[1]+maxLen[1])/2.0; 
	center[2] = (minLen[2]+maxLen[2])/2.0; 

}

//inline float3 VECTOR3ToFloat3

////////////////////////////////////////////////////////////////////////////
void DataMgr::compute_streamlines() 
{

  float from[3], to[3]; 

  from[0] = minLen[0];   from[1] = minLen[1];   from[2] = minLen[2]; 
  to[0] = maxLen[0];   to[1] = maxLen[1];   to[2] = maxLen[2]; 

  printf("generating seeds...\n"); 
  //osuflow->SetRandomSeedPoints(from, to, 5); 
  if(_seeds.size() <= 0)
	  return; 
  osuflow->SetSeedPoints(&_seeds[0], _seeds.size());
  int nSeeds; 
  VECTOR3* seeds = osuflow->GetSeeds(nSeeds); 
  for (int i=0; i<nSeeds; i++) 
    printf(" seed no. %d : [%f %f %f]\n", i, seeds[i][0], 
	   seeds[i][1], seeds[i][2]); 

  sl_list.clear(); 

  printf("compute streamlines..\n"); 
  osuflow->SetIntegrationParams(0.5, 0.2, 1.0);  
  osuflow->GenStreamLines(sl_list , BACKWARD_AND_FORWARD, 200, 0); 
  printf(" done integrations\n"); 
  printf("list size = %d\n", (int)sl_list.size());  

  _streamlines.clear();

  for(list<vtListSeedTrace*>::const_iterator
				pIter = sl_list.begin(); 
		pIter!=sl_list.end(); 
		pIter++) 
		{
			vector<float3> sl;
			const vtListSeedTrace *trace = *pIter; 
			for(list<VECTOR3*>::const_iterator
					pnIter = trace->begin(); 
				pnIter!= trace->end(); 
				pnIter++) 
			{
				float3 p = make_float3((**pnIter)[0], (**pnIter)[1], (**pnIter)[2]);
				sl.push_back(p);
			}
				_streamlines.push_back(sl);
		}
}

DataMgr::DataMgr()
{
}

DataMgr::~DataMgr()
{
}

float3 DataMgr::ConvertCoordinates(VECTOR3 v)	{
	return make_float3((v[0] - len[0] * 0.5)/ len[0], (v[1]  - len[1] * 0.5)/ len[1], (v[2]  - len[2] * 0.5)/ len[2]);
}

float3 DataMgr::ConvertCoordinates(float3 v)	{
	return make_float3((v.x - len[0] * 0.5)/ len[0], (v.y  - len[1] * 0.5)/ len[1], (v.z  - len[2] * 0.5)/ len[2]);
}

//float3 DataMgr::ConvertCoordinates(VECTOR3 v)	{
//	return VECTOR3((v[0] - len[0] * 0.5)/ len[0], (v[1]  - len[1] * 0.5)/ len[1], (v[2]  - len[2] * 0.5)/ len[2]);
//}


void DataMgr::ClearSeeds()
{
	_seeds.clear();
}

VECTOR3 DataMgr::ConvCoordsLeap2Data(float x, float y, float z)
{
	//return VECTOR3(
	//				int((x + 100) * 0.25) * 4 * 0.005 * (maxLen[0] - minLen[0]) + minLen[0], 
	//				int((y - 50	) * 0.25) * 4 * 0.005 * (maxLen[1] - minLen[1]) + minLen[1], 
	//				int((z + 100) * 0.25) * 4 * 0.005 * (maxLen[2] - minLen[2]) + minLen[2]);

	return VECTOR3(
				(x + 100) * 0.005 * (maxLen[0] - minLen[0]) + minLen[0], 
				(y - 50	)* 0.005 * (maxLen[1] - minLen[1]) + minLen[1], 
				(z + 100)* 0.005 * (maxLen[2] - minLen[2]) + minLen[2]);
}

void DataMgr::InsertSeed(VECTOR3 v)
{
	_seeds.push_back(ConvCoordsLeap2Data(v[0], v[1], v[2]));
}

vector<vector<float3>>* DataMgr::GetStreamLines()
{
	return &_streamlines;
}