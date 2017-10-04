#include "MyMesh.h"
void MyMesh::Init(void)
{
	m_bBinded = false;
	m_uVertexCount = 0;

	m_VAO = 0;
	m_VBO = 0;

	m_pShaderMngr = ShaderManager::GetInstance();
}
void MyMesh::Release(void)
{
	m_pShaderMngr = nullptr;

	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);

	if (m_VAO > 0)
		glDeleteVertexArrays(1, &m_VAO);

	m_lVertex.clear();
	m_lVertexPos.clear();
	m_lVertexCol.clear();
}
MyMesh::MyMesh()
{
	Init();
}
MyMesh::~MyMesh() { Release(); }
MyMesh::MyMesh(MyMesh& other)
{
	m_bBinded = other.m_bBinded;

	m_pShaderMngr = other.m_pShaderMngr;

	m_uVertexCount = other.m_uVertexCount;

	m_VAO = other.m_VAO;
	m_VBO = other.m_VBO;
}
MyMesh& MyMesh::operator=(MyMesh& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyMesh temp(other);
		Swap(temp);
	}
	return *this;
}
void MyMesh::Swap(MyMesh& other)
{
	std::swap(m_bBinded, other.m_bBinded);
	std::swap(m_uVertexCount, other.m_uVertexCount);

	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);

	std::swap(m_lVertex, other.m_lVertex);
	std::swap(m_lVertexPos, other.m_lVertexPos);
	std::swap(m_lVertexCol, other.m_lVertexCol);

	std::swap(m_pShaderMngr, other.m_pShaderMngr);
}
void MyMesh::CompleteMesh(vector3 a_v3Color)
{
	uint uColorCount = m_lVertexCol.size();
	for (uint i = uColorCount; i < m_uVertexCount; ++i)
	{
		m_lVertexCol.push_back(a_v3Color);
	}
}
void MyMesh::AddVertexPosition(vector3 a_v3Input)
{
	m_lVertexPos.push_back(a_v3Input);
	m_uVertexCount = m_lVertexPos.size();
}
void MyMesh::AddVertexColor(vector3 a_v3Input)
{
	m_lVertexCol.push_back(a_v3Input);
}
void MyMesh::CompileOpenGL3X(void)
{
	if (m_bBinded)
		return;

	if (m_uVertexCount == 0)
		return;

	CompleteMesh();

	for (uint i = 0; i < m_uVertexCount; i++)
	{
		//Position
		m_lVertex.push_back(m_lVertexPos[i]);
		//Color
		m_lVertex.push_back(m_lVertexCol[i]);
	}
	glGenVertexArrays(1, &m_VAO);//Generate vertex array object
	glGenBuffers(1, &m_VBO);//Generate Vertex Buffered Object

	glBindVertexArray(m_VAO);//Bind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);//Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * 2 * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);//Generate space for the VBO

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)0);

	// Color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	m_bBinded = true;

	glBindVertexArray(0); // Unbind VAO
}
void MyMesh::Render(matrix4 a_mProjection, matrix4 a_mView, matrix4 a_mModel)
{
	// Use the buffer and shader
	GLuint nShader = m_pShaderMngr->GetShaderID("Basic");
	glUseProgram(nShader); 

	//Bind the VAO of this object
	glBindVertexArray(m_VAO);

	// Get the GPU variables by their name and hook them to CPU variables
	GLuint MVP = glGetUniformLocation(nShader, "MVP");
	GLuint wire = glGetUniformLocation(nShader, "wire");

	//Final Projection of the Camera
	matrix4 m4MVP = a_mProjection * a_mView * a_mModel;
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(m4MVP));
	
	//Solid
	glUniform3f(wire, -1.0f, -1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);  

	//Wire
	glUniform3f(wire, 1.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glBindVertexArray(0);// Unbind VAO so it does not get in the way of other objects
}
void MyMesh::AddTri(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft)
{
	//C
	//| \
	//A--B
	//This will make the triangle A->B->C 
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);
}
void MyMesh::AddQuad(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft, vector3 a_vTopRight)
{
	//C--D
	//|  |
	//A--B
	//This will make the triangle A->B->C and then the triangle C->B->D
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);

	AddVertexPosition(a_vTopLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopRight);
}
void MyMesh::GenerateCube(float a_fSize, vector3 a_v3Color)
{
	if (a_fSize < 0.01f)
		a_fSize = 0.01f;

	Release();
	Init();

	float fValue = a_fSize * 0.5f;
	//3--2
	//|  |
	//0--1

	vector3 point0(-fValue,-fValue, fValue); //0
	vector3 point1( fValue,-fValue, fValue); //1
	vector3 point2( fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue,-fValue,-fValue); //4
	vector3 point5( fValue,-fValue,-fValue); //5
	vector3 point6( fValue, fValue,-fValue); //6
	vector3 point7(-fValue, fValue,-fValue); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCuboid(vector3 a_v3Dimensions, vector3 a_v3Color)
{
	Release();
	Init();

	vector3 v3Value = a_v3Dimensions * 0.5f;
	//3--2
	//|  |
	//0--1
	vector3 point0(-v3Value.x, -v3Value.y, v3Value.z); //0
	vector3 point1(v3Value.x, -v3Value.y, v3Value.z); //1
	vector3 point2(v3Value.x, v3Value.y, v3Value.z); //2
	vector3 point3(-v3Value.x, v3Value.y, v3Value.z); //3

	vector3 point4(-v3Value.x, -v3Value.y, -v3Value.z); //4
	vector3 point5(v3Value.x, -v3Value.y, -v3Value.z); //5
	vector3 point6(v3Value.x, v3Value.y, -v3Value.z); //6
	vector3 point7(-v3Value.x, v3Value.y, -v3Value.z); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	//calculate the diff theta by using the nuber of subdivisions
	float theta = 0; 
	float diff = (2 * PI) / a_nSubdivisions;
	vector3 pointFirst(0.0f, 0.0f, -(a_fHeight * 0.5f));
	//create a vector to store the points
	std::vector<vector3> points;
	points.push_back(pointFirst);
	//calculate the points based on the number of sub divisions radius and diffTheta
	for (int i = 0; i < a_nSubdivisions; i++) 
	{
		points.push_back(vector3{ (a_fRadius * cos(theta)),(a_fRadius * sin(theta)), -(a_fHeight * 0.5f) });
		theta += diff;
	}
	points.push_back(vector3{ 0.0f, 0.0f, (a_fHeight * 0.5f) });

	for (int i = 1; i < a_nSubdivisions; i++)
	{
		AddTri(points[0], points[i + 1], points[i]);
		AddTri(points[a_nSubdivisions + 1], points[i], points[i + 1]);
	}

	//add in the last chunk of the shape manually
	AddTri(points[0], points[1], points[a_nSubdivisions]);
	AddTri(points[a_nSubdivisions + 1], points[a_nSubdivisions], points[1]);
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCylinder(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	//calculate the diff theta by using the nuber of subdivisions
	float theta = 0;
	float diff = (2 * PI) / a_nSubdivisions;
	vector3 pointFirst(0.0f, 0.0f, -(a_fHeight * 0.5f));
	//create a vector to store the points
	std::vector<vector3> pointsBottom;
	std::vector<vector3> pointsTop;
	pointsBottom.push_back(pointFirst);
	pointsTop.push_back(vector3{ 0.0f, 0.0f, (a_fHeight * 0.5f) });
	//calculate the points based on the number of sub divisions radius and diffTheta
	//odds in the vector are on the bottom of the cylander while evens are on the top
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		pointsBottom.push_back(vector3{ (a_fRadius * cos(theta)),(a_fRadius * sin(theta)), -(a_fHeight * 0.5f) });
		pointsTop.push_back(vector3{ (a_fRadius * cos(theta)),(a_fRadius * sin(theta)), (a_fHeight * 0.5f) });
		theta += diff;
	}

	for (int i = 1; i < a_nSubdivisions; i++)
	{
		AddTri(pointsBottom[0], pointsBottom[i + 1], pointsBottom[i]);
		AddTri(pointsTop[0], pointsTop[i], pointsTop[i + 1]);
		AddQuad(pointsBottom[i + 1], pointsTop[i + 1], pointsBottom[i], pointsTop[i]);

	}
	//add in the last chunk of the shape manually
	AddTri(pointsBottom[0], pointsBottom[1], pointsBottom[a_nSubdivisions]);
	AddTri(pointsTop[0], pointsTop[a_nSubdivisions], pointsTop[1]);
	AddQuad(pointsBottom[1], pointsTop[1], pointsBottom[a_nSubdivisions], pointsTop[a_nSubdivisions]);
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTube(float a_fOuterRadius, float a_fInnerRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	//calculate the diff theta by using the nuber of subdivisions
	float theta = 0;
	float diff = (2 * PI) / a_nSubdivisions;
	//create a vector to store the points
	std::vector<vector3> pointsBottomInner;
	std::vector<vector3> pointsTopInner;
	std::vector<vector3> pointsBottomOuter;
	std::vector<vector3> pointsTopOuter;
	//calculate the points based on the number of sub divisions radius and diffTheta
	//odds in the vector are on the bottom of the cylander while evens are on the top
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		pointsBottomInner.push_back(vector3{ (a_fInnerRadius * cos(theta)),(a_fInnerRadius * sin(theta)), -(a_fHeight * 0.5f) });
		pointsTopInner.push_back(vector3{ (a_fInnerRadius * cos(theta)),(a_fInnerRadius * sin(theta)), (a_fHeight * 0.5f) });
		pointsBottomOuter.push_back(vector3{ (a_fOuterRadius * cos(theta)),(a_fOuterRadius * sin(theta)), -(a_fHeight * 0.5f) });
		pointsTopOuter.push_back(vector3{ (a_fOuterRadius * cos(theta)),(a_fOuterRadius * sin(theta)), (a_fHeight * 0.5f) });
		theta += diff;
	}

	for (int i = 0; i < a_nSubdivisions - 1; i++)
	{
		AddQuad(pointsBottomInner[i], pointsTopInner[i], pointsBottomInner[i + 1], pointsTopInner[i + 1]);
		AddQuad(pointsBottomOuter[i + 1], pointsTopOuter[i + 1], pointsBottomOuter[i], pointsTopOuter[i]);
		AddQuad(pointsBottomOuter[i], pointsBottomInner[i], pointsBottomOuter[i + 1], pointsBottomInner[i + 1]);
		AddQuad(pointsTopOuter[i + 1], pointsTopInner[i + 1], pointsTopOuter[i], pointsTopInner[i]);

	}
	//add in the last chunk of the shape manually
	AddQuad(pointsBottomInner[(a_nSubdivisions - 1)], pointsTopInner[(a_nSubdivisions - 1)], pointsBottomInner[0], pointsTopInner[0]);
	AddQuad(pointsBottomOuter[0], pointsTopOuter[0], pointsBottomOuter[a_nSubdivisions - 1], pointsTopOuter[(a_nSubdivisions - 1)]);
	AddQuad(pointsBottomOuter[(a_nSubdivisions - 1)], pointsBottomInner[(a_nSubdivisions - 1)], pointsBottomOuter[0], pointsBottomInner[0]);
	AddQuad(pointsTopOuter[0], pointsTopInner[0], pointsTopOuter[(a_nSubdivisions - 1)], pointsTopInner[(a_nSubdivisions - 1)]);
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTorus(float a_fOuterRadius, float a_fInnerRadius, int a_nSubdivisionsA, int a_nSubdivisionsB, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_nSubdivisionsA < 3)
		a_nSubdivisionsA = 3;
	if (a_nSubdivisionsA > 360)
		a_nSubdivisionsA = 360;

	if (a_nSubdivisionsB < 3)
		a_nSubdivisionsB = 3;
	if (a_nSubdivisionsB > 360)
		a_nSubdivisionsB = 360;

	Release();
	Init();

	// Replace this with your code
	GenerateCube(a_fOuterRadius * 2.0f, a_v3Color);
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateSphere(float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	//Sets minimum and maximum of subdivisions
	if (a_nSubdivisions < 1)
	{
		GenerateCube(a_fRadius * 2.0f, a_v3Color);
		return;
	}
	//if (a_nSubdivisions > 6)
		//a_nSubdivisions = 6;

	Release();
	Init();

	// Replace this with your code
	float thetaRight = 0;
	float thetaLeft = 0;
	float oThetaRight = 0; 
	float oThetaLeft = 0;
	std::vector<vector3> points;

	for (int i = 0; i < a_nSubdivisions; i++)
	{
		thetaLeft = ((PI*i) / a_nSubdivisions);
		thetaRight = ((PI*(i + 1)) / a_nSubdivisions);
		for (int j = 0; j < a_nSubdivisions; j++) 
		{
			oThetaLeft = ((2 * PI*j) / a_nSubdivisions);
			oThetaRight = ((2 * PI*(j + 1)) / a_nSubdivisions);
			//make the points theta is side to side while otheta is up and down this will create the points for the quads in the sphere and create its quad then iterate through to the next quad in the sphere
			points.push_back(vector3{a_fRadius * cos(oThetaLeft) * sin(thetaLeft), a_fRadius * sin(oThetaLeft) * sin(thetaLeft), a_fRadius * cos(thetaLeft)});
			points.push_back(vector3{ a_fRadius * cos(oThetaLeft) * sin(thetaRight), a_fRadius * sin(oThetaLeft) * sin(thetaRight), a_fRadius * cos(thetaRight) });
			points.push_back(vector3{ a_fRadius * cos(oThetaRight) * sin(thetaLeft), a_fRadius * sin(oThetaRight) * sin(thetaLeft), a_fRadius * cos(thetaLeft) });
			points.push_back(vector3{ a_fRadius * cos(oThetaRight) * sin(thetaRight), a_fRadius * sin(oThetaRight) * sin(thetaRight), a_fRadius * cos(thetaRight) });

			//make the quad with the four points 
			AddQuad(points[0], points[1], points[2], points[3]);

			points.pop_back();
			points.pop_back();
			points.pop_back();
			points.pop_back();
		}
	}


	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}