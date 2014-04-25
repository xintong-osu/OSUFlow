/*
sample usage: ./VortexDetector ~/nsf_data/backstep_labels/Block_1/backstep_binary/backstep_block1 35 25 25 17 22 10 backstep_block1

./VortexDetector ~/nsf_data/delta_wing/delta_binary/delta 67 0 209 0 49 0 delta_wing-full
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "OSUFlow.h"
#include "CurvilinearGrid.h"
#include "GeometricDerivations.h"
#include "calculus.h"

#define unit 0.01f
#define generateRegularGrid 0

float vec_magnitude(VECTOR3 vec)
{
	float mag = sqrt(vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]);
	return mag;
}

float calc_curvature(GeometricDerivations gmder, VECTOR3 pos, VECTOR3 sub)
{
	float curv;

	VECTOR3 normal;

        // The gradient will give us the rate of change in X, Y, and Z for U, V, and W
        MATRIX3 jacobian = gmder.TangentGradient(pos);

	//calculate tangent
	VECTOR3 tangent;
        gmder.vectorField->at_phys(pos,0,tangent);

	tangent = tangent - sub;

	// Make it a unit vector
        tangent.Normalize();

	// jacobian * tangent yields normal
        normal = mvmult(jacobian, tangent);

	curv = vec_magnitude(normal);
	
	return curv;
}

int main(int argc, char ** argv)
{
/*
	OSUFlow *osuflow = new OSUFlow();
        printf("read file %s\n", argv[1]);

        VECTOR3 minLen, maxLen, minB, maxB;
        minB[0] = 0; minB[1] = 0; minB[2] = 0;
    maxB[0] = 200; maxB[1] = 200; maxB[2] = 200;

        osuflow->LoadDataCurvilinear((const char*)argv[1], true, minB, maxB); //true: a steady flow field
        osuflow->Boundary(minLen, maxLen);
        printf(" volume boundary X: [%f %f] Y: [%f %f] Z: [%f %f]\n",
                                                                minLen[0], maxLen[0], minLen[1], maxLen[1],
                                                                minLen[2], maxLen[2]);


        int dim[3]      ;
        CurvilinearGrid *grid = (CurvilinearGrid*)osuflow->GetFlowField()->GetGrid();
        grid->GetDimension(dim[0], dim[1], dim[2]);
        printf("dim: %d %d %d\n", dim[0], dim[1], dim[2]);
        int i,j,k;
	Cell in_cell;
	VECTOR3 from, to;
	//VECTOR3 cur_position(3.03,0.36,2.15);
	VECTOR3 cur_position(2.95,0.32,2.10);
	grid->locate(cur_position,in_cell);
	printf("cell info is %f %f %f\n",in_cell.ijk[0],in_cell.ijk[1],in_cell.ijk[2]);
	grid->coordinates_at_vertex(VECTOR3(in_cell.ijk[0], in_cell.ijk[1], in_cell.ijk[2]), &from);
	printf("position info is %f %f %f\n",from[0],from[1],from[2]);
        GeometricDerivations gmder;

        gmder.GetData(osuflow);
        VECTOR3 normal,point;

	VECTOR3 tangent;
        gmder.vectorField->at_phys(VECTOR3(from[0], from[1], from[2]),0,tangent);
	printf("velocity is %f %f %f\n",tangent[0],tangent[1],tangent[2]);

	//test
	grid->coordinates_at_vertex(VECTOR3(30, 17, 32), &from);
	MATRIX3 jacobian1 = gmder.TangentGradient(from);

	for(int i=0; i<3; i++)
	{
		for(int j=0; j<3; j++)
		printf("%f ",jacobian1[i][j]);

	printf("\n");
	}
	printf("Ciurvature value is %f\n",calc_curvature(gmder, from, VECTOR3(0.87803,0.012206,-0.027457)));
	//test end

	float curv=0;

	for(int z=-2; z<=2; z++)
	for(int y=-2; y<=2; y++)
	for(int x=-2; x<=2; x++)
	{
		grid->coordinates_at_vertex(VECTOR3(in_cell.ijk[0]+x, in_cell.ijk[1]+y, in_cell.ijk[2]+z), &from);
		curv+= calc_curvature(gmder, from, VECTOR3(0.87803,0.012206,-0.027457));
	}

	printf("curvature is %f\n",curv/125);
/*
	// Write back the jacobians

	FILE *fp2 = fopen("jacobian_matrix.raw","wb");
	FILE *fp3 = fopen("channel_vectors.raw","wb");

	for(int k = 0; k < dim[2]; ++k)
  	{
          for(int j = 0; j < dim[1]; ++j)
          for(int i = 0; i < dim[0]; ++i)
          {
		MATRIX3 jacob;
		VECTOR3 vec, new_loc;
		grid->coordinates_at_vertex(VECTOR3(i,j,k), &from);
		jacob = gmder.TangentGradient(from);
		(osuflow->GetFlowField())->at_phys(from,0,vec);
		
		for(int xx=0; xx<3; xx++)
                for(int yy=0; yy<3; yy++)
                {
                	float j_val = jacob(xx,yy);
                	fwrite(&j_val, 1, sizeof(float), fp2);
                }

		
		for(int xx=0; xx<3; xx++)
               	fwrite(&vec[xx], 1, sizeof(float), fp3);
		

	  }
	}

	fclose(fp2);
	fclose(fp3);
*/

	//write back the vectors



	OSUFlow *osuflow = new OSUFlow();
	printf("read file %s\n", argv[1]);

	VECTOR3 minLen, maxLen, minB, maxB;
	minB[0] = 0; minB[1] = 0; minB[2] = 0;
    maxB[0] = 200; maxB[1] = 200; maxB[2] = 200;

	printf("This is working\n");

	osuflow->LoadDataCurvilinear((const char*)argv[1], true, minB, maxB); //true: a steady flow field
	osuflow->Boundary(minLen, maxLen);
	printf(" volume boundary X: [%f %f] Y: [%f %f] Z: [%f %f]\n",
								minLen[0], maxLen[0], minLen[1], maxLen[1],
								minLen[2], maxLen[2]);


	int dim[3]	;
	CurvilinearGrid *grid = (CurvilinearGrid*)osuflow->GetFlowField()->GetGrid();
	grid->GetDimension(dim[0], dim[1], dim[2]);
	printf("dim: %d %d %d\n", dim[0], dim[1], dim[2]);
	int i,j,k;
#if 0
	for (k=0; k<dim[2]; k++)
		for (j=0; j<dim[1]; j++)
			for (i=0; i<dim[0]; i++) {
				VECTOR3 v;
				grid->coordinates_at_vertex(VECTOR3(i,j,k),&v);	//return 1 velocity value
				printf("%f %f %f\n", v[0], v[1], v[2]);
			}
#endif

	int i1, i2, j1, j2, k1, k2;
	if (argc<8) {
		printf("Please input range: (i2 i1 j2 j1 k2 k1)\n");
		scanf("%d %d %d %d %d %d", &i2, &i1, &j2, &j1, &k2, &k1);
	} else {
		i2 = atoi(argv[2]);
		i1 = atoi(argv[3]);
		j2 = atoi(argv[4]);
		j1 = atoi(argv[5]);
		k2 = atoi(argv[6]);
		k1 = atoi(argv[7]);
	}
	// Does not include upbound


	char out_fname[5][256];
	char prefix[5][10]={"lambda2", "q", "delta", "gamma", "grid"};
	for (i=0; i<5; i++)
	{
		sprintf(out_fname[i], "%s_%s.raw", argv[8], prefix[i]);
		printf("Output file: %s\n", out_fname[i]);
	}

	float x,y,z;
	VECTOR3 from, to;
	grid->coordinates_at_vertex(VECTOR3(i1, j1, k1), &from);
	grid->coordinates_at_vertex(VECTOR3(i2, j2, k2), &to);
	printf("from: %f %f %f, to: %f %f %f\n", from[0], from[1], from[2], to[0], to[1], to[2]);

	// get min offset unit
	float min_off[3] =  {1e+9,1e+9,1e+9};
	{
		for (i=i1+1; i<i2; i++) {
			VECTOR3 v1, v2;
			grid->coordinates_at_vertex(VECTOR3(i-1, j1, k1), &v1);
			grid->coordinates_at_vertex(VECTOR3(i, j1, k1), &v2);
			min_off[0] = min(v2[0]-v1[0], min_off[0]);
		}
		for (j=j1+1; j<j2; j++) {
			VECTOR3 v1, v2;
			grid->coordinates_at_vertex(VECTOR3(i1, j-1, k1), &v1);
			grid->coordinates_at_vertex(VECTOR3(i1, j, k1), &v2);
			min_off[1] = min(v2[1]-v1[1], min_off[1]);
		}
		for (k=k1+1; k<k2; k++) {
			VECTOR3 v1, v2;
			grid->coordinates_at_vertex(VECTOR3(i1, j1, k-1), &v1);
			grid->coordinates_at_vertex(VECTOR3(i1, j1, k), &v2);
			min_off[2] = min(v2[2]-v1[2], min_off[2]);
		}
		printf("Min grid unit: %f %f %f\n", min_off[0], min_off[1], min_off[2]);
	}

	if (!(from[0]<=to[0] && from[1]<=to[1] && from[2]<=to[2]))
		printf("Input invalid.  Program halts\n");
	int count=0;
	FILE *fp[5];
	for (i=0; i<5; i++)
		fp[i] = fopen(out_fname[i], "wb");

	if(generateRegularGrid)
	{

		for (z=from[2]; z< to[2]; z+=unit) {
			for (y=from[1]; y< to[1]; y+=unit)
				for (x=from[0]; x< to[0]; x+=unit)
				{
					#if 0
					//VECTOR3 v;
					//osuflow->GetFlowField()->at_phys(VECTOR3(x,y,z), 0, v);
					//printf("%f %f %f\n", v[0], v[1], v[2]);
					#endif
					float f[4]; //lambda2, q, delta, gamma;
					osuflow->GetFlowField()->GenerateVortexMetrics(VECTOR3(x,y,z), f[0], f[1], f[2], f[3]);
					for (i=0; i<4; i++)
						fwrite((char *)&f[i], 1, 4, fp[i]);
					count++;
				}
			printf("z=%f\n", z);
		}
		


		// get dim for given range
		int bdim[3];
		bdim[0] = bdim[1] = bdim[2] = 0;
		{
			int d;
			for (d=0; d<3; d++)
				for (x=from[d]; x<=to[d]; x+=unit)
					bdim[d] ++;
		}

		for (i=0; i<4; i++)
		{
			// get out_fname filename only (no path)
			char *out_fname_no_path = strrchr(out_fname[i], '/');
			if (out_fname_no_path==NULL) out_fname_no_path = out_fname[i]; else out_fname_no_path++;

			char out_desc_fname[256];
			sprintf(out_desc_fname, "%s_%s.nhdr", argv[8], prefix[i]);
			FILE *fp = fopen(out_desc_fname, "wt");
			fprintf(fp,
					"NRRD0001\n"
					"type: float\n"
					"dimension: 3\n"
					"sizes: %d %d %d\n"
					"encoding: raw\n"
					"data file: %s\n"
					"# sampling distance: %f\n"
					"# grid range: %d %d %d - %d %d %d\n"
					"# physical range: %f %f %f - %f %f %f\n"
					"# min grid unit: %f %f %f\n",
					bdim[0], bdim[1], bdim[2], out_fname_no_path,
					unit, i1, j1, k1, i2, j2, k2, from[0], from[1], from[2], to[0], to[1], to[2], min_off[0], min_off[1], min_off[2]);
			fclose(fp);
		}

		printf("Done (%d elems)\n", count);

	}
	else
	{
		// Only write at the (maybe uneven spaced) grid locations
		cout<<"Start writing data"<<endl;
		for(int k=k1; k<k2; k++)
		{
			for(int j=j1; j<j2; j++)
			{
				for (int i = i1; i < i2; ++i)
				{
					VECTOR3 v1;
					grid->coordinates_at_vertex(VECTOR3(i, j, k), &v1);	//get the location

					float f[4]; //lambda2, q, delta, gamma;
					osuflow->GetFlowField()->GenerateVortexMetrics(v1, f[0], f[1], f[2], f[3]);
					for (int i=0; i<4; i++)
						fwrite((char *)&f[i], 1, 4, fp[i]);

					fwrite((char *)&v1[0], 1, 4, fp[4]);
					fwrite((char *)&v1[1], 1, 4, fp[4]);
					fwrite((char *)&v1[2], 1, 4, fp[4]);

					//cout<<i<<" "<<j<<" "<<k<<endl;
				}
				cout<<" "<<j<<flush;
			}
				cout<<endl<<"k is "<<k<<endl;
		}
				cout<<"Start writing Headers"<<endl;

		for (i=0; i<4; i++)
		{
			// get out_fname filename only (no path)
			char *out_fname_no_path = strrchr(out_fname[i], '/');
			if (out_fname_no_path==NULL) out_fname_no_path = out_fname[i]; else out_fname_no_path++;

			char out_desc_fname[256];
			sprintf(out_desc_fname, "%s_%s.nhdr", argv[8], prefix[i]);
			FILE *fp1 = fopen(out_desc_fname, "wt");
			fprintf(fp1,
					"NRRD0001\n"
					"type: float\n"
					"dimension: 3\n"
					"sizes: %d %d %d\n"
					"encoding: raw\n"
					"data file: %s\n"
					"# sampling distance: As given in the geometry file\n"
					"# grid range: %d %d %d - %d %d %d\n"
					"# physical range: %f %f %f - %f %f %f\n"
					"# min grid unit: %f %f %f\n",
					i2-i1, j2-j1, k2-k1, out_fname_no_path,
					i1, j1, k1, i2, j2, k2, from[0], from[1], from[2], to[0], to[1], to[2], min_off[0], min_off[1], min_off[2]);
			fclose(fp1);
		}
	}

	for (i=0; i<5; i++)	// close output files
	fclose(fp[i]);

	return 0;

}
