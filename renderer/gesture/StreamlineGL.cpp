#include "StreamlineGL.h"

StreamlineGL::StreamlineGL (OpenGLContext& openGLContext)
{
	//if (shapeFile.load (BinaryData::teapot_obj).wasOk())
	//	for (int i = 0; i < shapeFile.shapes.size(); ++i)
	//		vertexBuffers.add (new VertexBuffer (openGLContext, *shapeFile.shapes.getUnchecked(i)));

	_lineBuffer = new VertexBuffer(openGLContext);
}

StreamlineGL::StreamlineGL (OpenGLContext& openGLContext, vector<vector<float3>>* sls)
{
	//if (shapeFile.load (BinaryData::teapot_obj).wasOk())
	//for (int i = 0; i < sls->size(); ++i)
	//_openGLContext = openGLContext;
	//vertexBuffers.add (new VertexBuffer (openGLContext, sls));
	_lineBuffer = new VertexBuffer(openGLContext);
}

void StreamlineGL::UpdateData(OpenGLContext& openGLContext, vector<vector<float3>>* sls)
{
	if(sls->size() <= 0)
		return;
	//if(NULL != _lineBuffer)
	//	delete _lineBuffer;
	_lineBuffer->UpdateData(openGLContext, sls, _lengths, _indices);
}

void StreamlineGL::draw (OpenGLContext& openGLContext)//, Attributes& attributes)
{
	//for (int i = 0; i < vertexBuffers.size(); ++i)
	//{
	//	VertexBuffer& vertexBuffer = *vertexBuffers.getUnchecked (i);
		//openGLContext.extensions.glVertexAttribPointer (position->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), 0);
		//openGLContext.extensions.glEnableVertexAttribArray (position->attributeID);
		_lineBuffer->bind();

		//attributes.enable (openGLContext);
		//glDrawElements (GL_TRIANGLES, vertexBuffer.numIndices, GL_UNSIGNED_INT, 0);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, 0);
		for(int j = 0; j < _lengths.size(); j++)	{
			glDrawArrays(GL_LINE_STRIP, _indices.at(j), _lengths.at(j));
		}
		glDisableClientState(GL_VERTEX_ARRAY);
	//	attributes.disable (openGLContext);
	//}
	//_lineBuffer->bind();
	//openGLContext.extensions.gl
}

VertexBuffer::VertexBuffer (OpenGLContext& context) : openGLContext (context)
{
	//numIndices = shape.mesh.indices.size();
	openGLContext.extensions.glGenBuffers (1, &vertexBuffer);
	//openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);

	//createVertexListFromMesh (shape.mesh, vertices, Colours::green);
	//vector<float3> vertices;




	//openGLContext.extensions.glGenBuffers (1, &indexBuffer);
	//openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	//openGLContext.extensions.glBufferData (GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof (juce::uint32),
	//	shape.mesh.indices.getRawDataPointer(), GL_STATIC_DRAW);
}