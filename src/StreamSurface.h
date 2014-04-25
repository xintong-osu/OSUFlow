#include <FieldLine.h>
/* file eigenvals.c */

extern int compute_eigenvalues( float m[3][3], float eigenvalues[3] );

/* file eigenvecs.c */
extern void compute_real_eigenvectors (float m[3][3], float vals[3], float vecs[3][3]);
extern void compute_complex_eigenvectors( float m[3][3], float vals[3], float vecs[3][3] );

class vtCStreamSurface:public vtCFieldLine
{
 public:
	vtCStreamSurface(CVectorField* pField); 
	~vtCStreamSurface(void);
	void execute(const void* userData, list<VECTOR3>& listSeedTraces,                list<int64_t> *listSeedIds = NULL);      
	void setForwardTracing(int enabled);
        void setBackwardTracing(int enabled);
        int  getForwardTracing(void);
        int  getBackwardTracing(void);
	void setRange(float rangeVal);

 protected:
	void computeStreamSurface(const void* userdata,
                        list<VECTOR3>& listSeedTraces,
                                list<int64_t> *listSeedIds);
	void computeStreamSurfaceQuad(const void* userdata,
                        list<VECTOR3>& listSeedTraces,
                                list<int64_t> *listSeedIds);
	float errorAnalysisGlobal(VECTOR3 *points, vtListParticle m_lSeeds1, 
                                                     float dt, int min_steps);
	double length(PointInfo,PointInfo);
	TRACE_DIR m_itsTraceDir;
        float m_fPsuedoTime;
        float m_fCurrentTime;
	float data_range;
};
