#include "StreamlineGL.h"

StreamlineGL::StreamlineGL (OpenGLContext& openGLContext, vector<vector<float3>>* sls)
{
	//if (shapeFile.load (BinaryData::teapot_obj).wasOk())
	//for (int i = 0; i < sls->size(); ++i)
		vertexBuffers.add (new VertexBuffer (openGLContext, sls));
}

void StreamlineGL::draw (OpenGLContext& openGLContext, Attributes& attributes)
{
	for (int i = 0; i < vertexBuffers.size(); ++i)
	{
		VertexBuffer& vertexBuffer = *vertexBuffers.getUnchecked (i);
		vertexBuffer.bind();

		attributes.enable (openGLContext);
		//glDrawElements (GL_TRIANGLES, vertexBuffer.numIndices, GL_UNSIGNED_INT, 0);
		for(int j = 0; j < vertexBuffer._lengths.size(); j++)	{
			glDrawArrays(GL_LINE_STRIP, vertexBuffer._indices.at(0), vertexBuffer._lengths.at(0));
		}
		attributes.disable (openGLContext);
	}
}