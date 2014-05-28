#ifndef STREAMLINE_GL_H
#define STREAMLINE_GL_H

#include "JuceLibraryCode/JuceHeader.h"
#include "vector_types.h"
#include "vector_functions.h"
#include <vector>
using namespace std;

struct Vertex
{
	float position[3];
//	float normal[3];
	//float colour[4];
	//float texCoord[2];
};

struct Attributes
{
	Attributes (OpenGLContext& openGLContext, OpenGLShaderProgram& shader)
	{
		position      = createAttribute (openGLContext, shader, "position");
		normal        = createAttribute (openGLContext, shader, "normal");
		/*sourceColour  = createAttribute (openGLContext, shader, "sourceColour");
		texureCoordIn = createAttribute (openGLContext, shader, "texureCoordIn");*/
	}

	void enable (OpenGLContext& openGLContext)
	{
		if (position != nullptr)
		{
			openGLContext.extensions.glVertexAttribPointer (position->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), 0);
			openGLContext.extensions.glEnableVertexAttribArray (position->attributeID);
		}

		if (normal != nullptr)
		{
			openGLContext.extensions.glVertexAttribPointer (normal->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 3));
			openGLContext.extensions.glEnableVertexAttribArray (normal->attributeID);
		}

		//if (sourceColour != nullptr)
		//{
		//	openGLContext.extensions.glVertexAttribPointer (sourceColour->attributeID, 4, GL_FLOAT, GL_FALSE, sizeof (float3), (GLvoid*) (sizeof (float) * 6));
		//	openGLContext.extensions.glEnableVertexAttribArray (sourceColour->attributeID);
		//}

		//if (texureCoordIn != nullptr)
		//{
		//	openGLContext.extensions.glVertexAttribPointer (texureCoordIn->attributeID, 2, GL_FLOAT, GL_FALSE, sizeof (float3), (GLvoid*) (sizeof (float) * 10));
		//	openGLContext.extensions.glEnableVertexAttribArray (texureCoordIn->attributeID);
		//}
	}

	void disable (OpenGLContext& openGLContext)
	{
		if (position != nullptr)       openGLContext.extensions.glDisableVertexAttribArray (position->attributeID);
		if (normal != nullptr)         openGLContext.extensions.glDisableVertexAttribArray (normal->attributeID);
		//if (sourceColour != nullptr)   openGLContext.extensions.glDisableVertexAttribArray (sourceColour->attributeID);
		//if (texureCoordIn != nullptr)  openGLContext.extensions.glDisableVertexAttribArray (texureCoordIn->attributeID);
	}

	ScopedPointer<OpenGLShaderProgram::Attribute> position, normal, sourceColour, texureCoordIn;

private:
	static OpenGLShaderProgram::Attribute* createAttribute (OpenGLContext& openGLContext,
		OpenGLShaderProgram& shader,
		const char* attributeName)
	{
		if (openGLContext.extensions.glGetAttribLocation (shader.getProgramID(), attributeName) < 0)
			return nullptr;

		return new OpenGLShaderProgram::Attribute (shader, attributeName);
	}
};

static void createPrimitivesFromVectors(vector<vector<float3>>* sls, Array<Vertex>& list, vector<int> &lengths, vector<int> &indices)
{
	lengths.clear();
	indices.clear();
	int firstIdx = 0;
	for(int i = 0; i < sls->size(); i++)	{
		lengths.push_back((int)sls->at(i).size());
		indices.push_back(firstIdx);
		firstIdx += lengths.back();
		vector<float3>* sl = &sls->at(i);
		for(int j = 0; j < sl->size(); j++)	{
			//vertices.push_back(sl->at(j));
			Vertex vert = 
			{
				{sl->at(j).x, sl->at(j).y, sl->at(j).z}//,
				//{1, 0, 0}
			};
			list.add(vert);
		}
	}
}

class VertexBuffer
{
public:
	VertexBuffer (OpenGLContext& context);

	void UpdateData(OpenGLContext& context, vector<vector<float3>>* sls, 
		vector<int> &lengths, vector<int> &indices)
	{
		openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
		Array<Vertex> vertices;
		createPrimitivesFromVectors(sls, vertices, lengths, indices);
		openGLContext.extensions.glBufferData (GL_ARRAY_BUFFER, vertices.size() * sizeof (Vertex),
			vertices.getRawDataPointer(), GL_STATIC_DRAW);
		openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
	}

	~VertexBuffer()
	{
		openGLContext.extensions.glDeleteBuffers (1, &vertexBuffer);
		//openGLContext.extensions.glDeleteBuffers (1, &indexBuffer);
	}

	void bind()
	{
		openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
		//openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	}

private:


	GLuint vertexBuffer, indexBuffer;
	//int numIndices;
	OpenGLContext& openGLContext;





	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VertexBuffer)
};

//==============================================================================
// This class just manages the uniform values that the demo shaders use.
struct Uniforms
{
	Uniforms (OpenGLContext& openGLContext, OpenGLShaderProgram& shader)
	{
		projectionMatrix = createUniform (openGLContext, shader, "projectionMatrix");
		viewMatrix       = createUniform (openGLContext, shader, "viewMatrix");
		texture          = createUniform (openGLContext, shader, "demoTexture");
		lightPosition    = createUniform (openGLContext, shader, "lightPosition");
		bouncingNumber   = createUniform (openGLContext, shader, "bouncingNumber");
	}

	ScopedPointer<OpenGLShaderProgram::Uniform> projectionMatrix, viewMatrix, texture, lightPosition, bouncingNumber;

private:
	static OpenGLShaderProgram::Uniform* createUniform (OpenGLContext& openGLContext,
		OpenGLShaderProgram& shader,
		const char* uniformName)
	{
		if (openGLContext.extensions.glGetUniformLocation (shader.getProgramID(), uniformName) < 0)
			return nullptr;

		return new OpenGLShaderProgram::Uniform (shader, uniformName);
	}
};


class StreamlineGL
{
public:
	StreamlineGL::StreamlineGL (OpenGLContext& openGLContext);

	StreamlineGL (OpenGLContext& openGLContext, vector<vector<float3>>* sls);

	void UpdateData(OpenGLContext& openGLContext, vector<vector<float3>>* sls);

	void draw (OpenGLContext& openGLContext);//, Attributes& attributes);


private:


	VertexBuffer *_lineBuffer;

	//	OwnedArray<VertexBuffer> vertexBuffers;
	vector<int> _lengths;
	vector<int> _indices;
	//	OpenGLContext _openGLContext;

	//WavefrontObjFile shapeFile;


	//static void createVertexListFromMesh (const WavefrontObjFile::Mesh& mesh, Array<Vertex>& list, Colour colour)
	//{
	//	const float scale = 0.2f;
	//	WavefrontObjFile::TextureCoord defaultTexCoord = { 0.5f, 0.5f };
	//	WavefrontObjFile::Vertex defaultNormal = { 0.5f, 0.5f, 0.5f };

	//	for (int i = 0; i < mesh.vertices.size(); ++i)
	//	{
	//		const WavefrontObjFile::Vertex& v = mesh.vertices.getReference (i);

	//		const WavefrontObjFile::Vertex& n
	//			= i < mesh.normals.size() ? mesh.normals.getReference (i) : defaultNormal;

	//		const WavefrontObjFile::TextureCoord& tc
	//			= i < mesh.textureCoords.size() ? mesh.textureCoords.getReference (i) : defaultTexCoord;

	//		Vertex vert =
	//		{
	//			{ scale * v.x, scale * v.y, scale * v.z, },
	//			{ scale * n.x, scale * n.y, scale * n.z, },
	//			{ colour.getFloatRed(), colour.getFloatGreen(), colour.getFloatBlue(), colour.getFloatAlpha() },
	//			{ tc.x, tc.y }
	//		};

	//		list.add (vert);
	//	}
	//}
};

#endif