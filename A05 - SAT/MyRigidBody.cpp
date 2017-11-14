#include "MyRigidBody.h"
using namespace Simplex;
//Allocation
void MyRigidBody::Init(void)
{
	m_pMeshMngr = MeshManager::GetInstance();
	m_bVisibleBS = false;
	m_bVisibleOBB = true;
	m_bVisibleARBB = false;

	m_fRadius = 0.0f;

	m_v3ColorColliding = C_RED;
	m_v3ColorNotColliding = C_WHITE;

	m_v3Center = ZERO_V3;
	m_v3MinL = ZERO_V3;
	m_v3MaxL = ZERO_V3;

	m_v3MinG = ZERO_V3;
	m_v3MaxG = ZERO_V3;

	m_v3HalfWidth = ZERO_V3;
	m_v3ARBBSize = ZERO_V3;

	m_m4ToWorld = IDENTITY_M4;
}
void MyRigidBody::Swap(MyRigidBody& a_pOther)
{
	std::swap(m_pMeshMngr, a_pOther.m_pMeshMngr);
	std::swap(m_bVisibleBS, a_pOther.m_bVisibleBS);
	std::swap(m_bVisibleOBB, a_pOther.m_bVisibleOBB);
	std::swap(m_bVisibleARBB, a_pOther.m_bVisibleARBB);

	std::swap(m_fRadius, a_pOther.m_fRadius);

	std::swap(m_v3ColorColliding, a_pOther.m_v3ColorColliding);
	std::swap(m_v3ColorNotColliding, a_pOther.m_v3ColorNotColliding);

	std::swap(m_v3Center, a_pOther.m_v3Center);
	std::swap(m_v3MinL, a_pOther.m_v3MinL);
	std::swap(m_v3MaxL, a_pOther.m_v3MaxL);

	std::swap(m_v3MinG, a_pOther.m_v3MinG);
	std::swap(m_v3MaxG, a_pOther.m_v3MaxG);

	std::swap(m_v3HalfWidth, a_pOther.m_v3HalfWidth);
	std::swap(m_v3ARBBSize, a_pOther.m_v3ARBBSize);

	std::swap(m_m4ToWorld, a_pOther.m_m4ToWorld);

	std::swap(m_CollidingRBSet, a_pOther.m_CollidingRBSet);
}
void MyRigidBody::Release(void)
{
	m_pMeshMngr = nullptr;
	ClearCollidingList();
}
//Accessors
bool MyRigidBody::GetVisibleBS(void) { return m_bVisibleBS; }
void MyRigidBody::SetVisibleBS(bool a_bVisible) { m_bVisibleBS = a_bVisible; }
bool MyRigidBody::GetVisibleOBB(void) { return m_bVisibleOBB; }
void MyRigidBody::SetVisibleOBB(bool a_bVisible) { m_bVisibleOBB = a_bVisible; }
bool MyRigidBody::GetVisibleARBB(void) { return m_bVisibleARBB; }
void MyRigidBody::SetVisibleARBB(bool a_bVisible) { m_bVisibleARBB = a_bVisible; }
float MyRigidBody::GetRadius(void) { return m_fRadius; }
vector3 MyRigidBody::GetColorColliding(void) { return m_v3ColorColliding; }
vector3 MyRigidBody::GetColorNotColliding(void) { return m_v3ColorNotColliding; }
void MyRigidBody::SetColorColliding(vector3 a_v3Color) { m_v3ColorColliding = a_v3Color; }
void MyRigidBody::SetColorNotColliding(vector3 a_v3Color) { m_v3ColorNotColliding = a_v3Color; }
vector3 MyRigidBody::GetCenterLocal(void) { return m_v3Center; }
vector3 MyRigidBody::GetMinLocal(void) { return m_v3MinL; }
vector3 MyRigidBody::GetMaxLocal(void) { return m_v3MaxL; }
vector3 MyRigidBody::GetCenterGlobal(void){	return vector3(m_m4ToWorld * vector4(m_v3Center, 1.0f)); }
vector3 MyRigidBody::GetMinGlobal(void) { return m_v3MinG; }
vector3 MyRigidBody::GetMaxGlobal(void) { return m_v3MaxG; }
vector3 MyRigidBody::GetHalfWidth(void) { return m_v3HalfWidth; }
matrix4 MyRigidBody::GetModelMatrix(void) { return m_m4ToWorld; }
void MyRigidBody::SetModelMatrix(matrix4 a_m4ModelMatrix)
{
	//to save some calculations if the model matrix is the same there is nothing to do here
	if (a_m4ModelMatrix == m_m4ToWorld)
		return;

	//Assign the model matrix
	m_m4ToWorld = a_m4ModelMatrix;

	//Calculate the 8 corners of the cube
	vector3 v3Corner[8];
	//Back square
	v3Corner[0] = m_v3MinL;
	v3Corner[1] = vector3(m_v3MaxL.x, m_v3MinL.y, m_v3MinL.z);
	v3Corner[2] = vector3(m_v3MinL.x, m_v3MaxL.y, m_v3MinL.z);
	v3Corner[3] = vector3(m_v3MaxL.x, m_v3MaxL.y, m_v3MinL.z);

	//Front square
	v3Corner[4] = vector3(m_v3MinL.x, m_v3MinL.y, m_v3MaxL.z);
	v3Corner[5] = vector3(m_v3MaxL.x, m_v3MinL.y, m_v3MaxL.z);
	v3Corner[6] = vector3(m_v3MinL.x, m_v3MaxL.y, m_v3MaxL.z);
	v3Corner[7] = m_v3MaxL;

	//Place them in world space
	for (uint uIndex = 0; uIndex < 8; ++uIndex)
	{
		v3Corner[uIndex] = vector3(m_m4ToWorld * vector4(v3Corner[uIndex], 1.0f));
	}

	//Identify the max and min as the first corner
	m_v3MaxG = m_v3MinG = v3Corner[0];

	//get the new max and min for the global box
	for (uint i = 1; i < 8; ++i)
	{
		if (m_v3MaxG.x < v3Corner[i].x) m_v3MaxG.x = v3Corner[i].x;
		else if (m_v3MinG.x > v3Corner[i].x) m_v3MinG.x = v3Corner[i].x;

		if (m_v3MaxG.y < v3Corner[i].y) m_v3MaxG.y = v3Corner[i].y;
		else if (m_v3MinG.y > v3Corner[i].y) m_v3MinG.y = v3Corner[i].y;

		if (m_v3MaxG.z < v3Corner[i].z) m_v3MaxG.z = v3Corner[i].z;
		else if (m_v3MinG.z > v3Corner[i].z) m_v3MinG.z = v3Corner[i].z;
	}

	//we calculate the distance between min and max vectors
	m_v3ARBBSize = m_v3MaxG - m_v3MinG;
}
//The big 3
MyRigidBody::MyRigidBody(std::vector<vector3> a_pointList)
{
	Init();
	//Count the points of the incoming list
	uint uVertexCount = a_pointList.size();

	//If there are none just return, we have no information to create the BS from
	if (uVertexCount == 0)
		return;

	//Max and min as the first vector of the list
	m_v3MaxL = m_v3MinL = a_pointList[0];

	//Get the max and min out of the list
	for (uint i = 1; i < uVertexCount; ++i)
	{
		if (m_v3MaxL.x < a_pointList[i].x) m_v3MaxL.x = a_pointList[i].x;
		else if (m_v3MinL.x > a_pointList[i].x) m_v3MinL.x = a_pointList[i].x;

		if (m_v3MaxL.y < a_pointList[i].y) m_v3MaxL.y = a_pointList[i].y;
		else if (m_v3MinL.y > a_pointList[i].y) m_v3MinL.y = a_pointList[i].y;

		if (m_v3MaxL.z < a_pointList[i].z) m_v3MaxL.z = a_pointList[i].z;
		else if (m_v3MinL.z > a_pointList[i].z) m_v3MinL.z = a_pointList[i].z;
	}

	//with model matrix being the identity, local and global are the same
	m_v3MinG = m_v3MinL;
	m_v3MaxG = m_v3MaxL;

	//with the max and the min we calculate the center
	m_v3Center = (m_v3MaxL + m_v3MinL) / 2.0f;

	//we calculate the distance between min and max vectors
	m_v3HalfWidth = (m_v3MaxL - m_v3MinL) / 2.0f;

	//Get the distance between the center and either the min or the max
	m_fRadius = glm::distance(m_v3Center, m_v3MinL);
}
MyRigidBody::MyRigidBody(MyRigidBody const& a_pOther)
{
	m_pMeshMngr = a_pOther.m_pMeshMngr;

	m_bVisibleBS = a_pOther.m_bVisibleBS;
	m_bVisibleOBB = a_pOther.m_bVisibleOBB;
	m_bVisibleARBB = a_pOther.m_bVisibleARBB;

	m_fRadius = a_pOther.m_fRadius;

	m_v3ColorColliding = a_pOther.m_v3ColorColliding;
	m_v3ColorNotColliding = a_pOther.m_v3ColorNotColliding;

	m_v3Center = a_pOther.m_v3Center;
	m_v3MinL = a_pOther.m_v3MinL;
	m_v3MaxL = a_pOther.m_v3MaxL;

	m_v3MinG = a_pOther.m_v3MinG;
	m_v3MaxG = a_pOther.m_v3MaxG;

	m_v3HalfWidth = a_pOther.m_v3HalfWidth;
	m_v3ARBBSize = a_pOther.m_v3ARBBSize;

	m_m4ToWorld = a_pOther.m_m4ToWorld;

	m_CollidingRBSet = a_pOther.m_CollidingRBSet;
}
MyRigidBody& MyRigidBody::operator=(MyRigidBody const& a_pOther)
{
	if (this != &a_pOther)
	{
		Release();
		Init();
		MyRigidBody temp(a_pOther);
		Swap(temp);
	}
	return *this;
}
MyRigidBody::~MyRigidBody() { Release(); };
//--- a_pOther Methods
void MyRigidBody::AddCollisionWith(MyRigidBody* a_pOther)
{
	/*
		check if the object is already in the colliding set, if
		the object is already there return with no changes
	*/
	auto element = m_CollidingRBSet.find(a_pOther);
	if (element != m_CollidingRBSet.end())
		return;
	// we couldn't find the object so add it
	m_CollidingRBSet.insert(a_pOther);
}
void MyRigidBody::RemoveCollisionWith(MyRigidBody* a_pOther)
{
	m_CollidingRBSet.erase(a_pOther);
}
void MyRigidBody::ClearCollidingList(void)
{
	m_CollidingRBSet.clear();
}
bool MyRigidBody::IsColliding(MyRigidBody* const a_pOther)
{
	//check if spheres are colliding as pre-test
	bool bColliding = (glm::distance(GetCenterGlobal(), a_pOther->GetCenterGlobal()) < m_fRadius + a_pOther->m_fRadius);
	
	//if they are colliding check the SAT
	if (bColliding)
	{
		if(SAT(a_pOther) != eSATResults::SAT_NONE)
			bColliding = false;// reset to false
	}

	if (bColliding) //they are colliding
	{
		this->AddCollisionWith(a_pOther);
		a_pOther->AddCollisionWith(this);
	}
	else //they are not colliding
	{
		this->RemoveCollisionWith(a_pOther);
		a_pOther->RemoveCollisionWith(this);
	}

	return bColliding;
}
void MyRigidBody::AddToRenderList(void)
{
	if (m_bVisibleBS)
	{
		if (m_CollidingRBSet.size() > 0)
			m_pMeshMngr->AddWireSphereToRenderList(glm::translate(m_m4ToWorld, m_v3Center) * glm::scale(vector3(m_fRadius)), C_BLUE_CORNFLOWER);
		else
			m_pMeshMngr->AddWireSphereToRenderList(glm::translate(m_m4ToWorld, m_v3Center) * glm::scale(vector3(m_fRadius)), C_BLUE_CORNFLOWER);
	}
	if (m_bVisibleOBB)
	{
		if (m_CollidingRBSet.size() > 0)
			m_pMeshMngr->AddWireCubeToRenderList(glm::translate(m_m4ToWorld, m_v3Center) * glm::scale(m_v3HalfWidth * 2.0f), m_v3ColorColliding);
		else
			m_pMeshMngr->AddWireCubeToRenderList(glm::translate(m_m4ToWorld, m_v3Center) * glm::scale(m_v3HalfWidth * 2.0f), m_v3ColorNotColliding);
	}
	if (m_bVisibleARBB)
	{
		if (m_CollidingRBSet.size() > 0)
			m_pMeshMngr->AddWireCubeToRenderList(glm::translate(GetCenterGlobal()) * glm::scale(m_v3ARBBSize), C_YELLOW);
		else
			m_pMeshMngr->AddWireCubeToRenderList(glm::translate(GetCenterGlobal()) * glm::scale(m_v3ARBBSize), C_YELLOW);
	}
}

//function to account for rotation in the corners
vector3 Math(glm::mat4 transform, glm::vec3 corner, glm::mat4 m_m4ToWorld) 
{
	glm::vec4 temp(corner, 1);
	temp = m_m4ToWorld * temp;
	temp = transform * temp;
	return vector3(temp.x, temp.y, temp.z);
}
uint MyRigidBody::SAT(MyRigidBody* const a_pOther)
{
	//bool either for whether the objects are colliding or not
	bool isSeperated = false;
	//vector for the normals for the first cube
	std::vector<glm::vec3> normalsOne;

	//vector for the normals for the second cube
	std::vector<glm::vec3> normalsTwo;

	//get the transforms
	glm::mat4 transformOne = this->GetModelMatrix();
	glm::mat4 transformTwo = a_pOther->GetModelMatrix();

	//Calculate the 8 corners of the first cube
	vector3 v3Corner[8];
	//Back square
	v3Corner[0] = Math(transformOne, m_v3MinL, m_m4ToWorld);
	v3Corner[1] = Math(transformOne, vector3(m_v3MaxL.x, m_v3MinL.y, m_v3MinL.z), m_m4ToWorld);
	v3Corner[2] = Math(transformOne, vector3(m_v3MinL.x, m_v3MaxL.y, m_v3MinL.z), m_m4ToWorld);
	v3Corner[3] = Math(transformOne, vector3(m_v3MaxL.x, m_v3MaxL.y, m_v3MinL.z), m_m4ToWorld);

	//Front square
	v3Corner[4] = Math(transformOne, vector3(m_v3MinL.x, m_v3MinL.y, m_v3MaxL.z), m_m4ToWorld);
	v3Corner[5] = Math(transformOne, vector3(m_v3MaxL.x, m_v3MinL.y, m_v3MaxL.z), m_m4ToWorld);
	v3Corner[6] = Math(transformOne, vector3(m_v3MinL.x, m_v3MaxL.y, m_v3MaxL.z), m_m4ToWorld);
	v3Corner[7] = Math(transformOne, m_v3MaxL, m_m4ToWorld);

	//Place the corners in world space
	/*for (uint uIndex = 0; uIndex < 8; ++uIndex)
	{
		v3Corner[uIndex] = vector3(m_m4ToWorld * vector4(v3Corner[uIndex], 1.0f));
	}*/

	//Calculate the 8 corners of the second cube
	vector3 v3CornerOther[8];
	//Back square
	v3CornerOther[0] = Math(transformTwo, a_pOther->GetMinLocal(), m_m4ToWorld);
	v3CornerOther[1] = Math(transformTwo, vector3(a_pOther->GetMaxLocal().x, a_pOther->GetMinLocal().y, a_pOther->GetMinLocal().z), m_m4ToWorld);
	v3CornerOther[2] = Math(transformTwo, vector3(a_pOther->GetMinLocal().x, a_pOther->GetMaxLocal().y, a_pOther->GetMinLocal().z), m_m4ToWorld);
	v3CornerOther[3] = Math(transformTwo, vector3(a_pOther->GetMaxLocal().x, a_pOther->GetMaxLocal().y, a_pOther->GetMinLocal().z), m_m4ToWorld);

	//Front square
	v3CornerOther[4] = Math(transformTwo, vector3(a_pOther->GetMinLocal().x, a_pOther->GetMinLocal().y, a_pOther->GetMaxLocal().z), m_m4ToWorld);
	v3CornerOther[5] = Math(transformTwo, vector3(a_pOther->GetMaxLocal().x, a_pOther->GetMinLocal().y, a_pOther->GetMaxLocal().z), m_m4ToWorld);
	v3CornerOther[6] = Math(transformTwo, vector3(a_pOther->GetMinLocal().x, a_pOther->GetMaxLocal().y, a_pOther->GetMaxLocal().z), m_m4ToWorld);
	v3CornerOther[7] = Math(transformTwo, a_pOther->GetMaxLocal(), m_m4ToWorld);

	//Place the corners in world space
	/*for (uint uIndex = 0; uIndex < 8; ++uIndex)
	{
		v3CornerOther[uIndex] = vector3(m_m4ToWorld * vector4(v3CornerOther[uIndex], 1.0f));
	}*/

	//using the corners calculate the normals using the corners of the first cube
	glm::vec3 U(v3Corner[1] - v3Corner[0]);
	glm::vec3 V(v3Corner[2] - v3Corner[0]);
	normalsOne.push_back(glm::normalize(glm::vec3((U.y * V.z) - (U.z * V.y), (U.z * V.x) - (U.x * V.z), (U.x * V.y) - (U.y * V.x))));

	U = (v3Corner[2] - v3Corner[3]);
	V = (v3Corner[4] - v3Corner[3]);
	normalsOne.push_back(glm::normalize(glm::vec3((U.y * V.z) - (U.z * V.y), (U.z * V.x) - (U.x * V.z), (U.x * V.y) - (U.y * V.x))));

	U = (v3Corner[6] - v3Corner[1]);
	V = (v3Corner[4] - v3Corner[1]);
	normalsOne.push_back(glm::normalize(glm::vec3((U.y * V.z) - (U.z * V.y), (U.z * V.x) - (U.x * V.z), (U.x * V.y) - (U.y * V.x))));

	//using the corners calculate the normals using the corners of the second cube
	U = (v3CornerOther[1] - v3CornerOther[0]);
	V = (v3CornerOther[2] - v3CornerOther[0]);
	normalsTwo.push_back(glm::normalize(glm::vec3((U.y * V.z) - (U.z * V.y), (U.z * V.x) - (U.x * V.z), (U.x * V.y) - (U.y * V.x))));

	U = (v3CornerOther[2] - v3CornerOther[3]);
	V = (v3CornerOther[4] - v3CornerOther[3]);
	normalsTwo.push_back(glm::normalize(glm::vec3((U.y * V.z) - (U.z * V.y), (U.z * V.x) - (U.x * V.z), (U.x * V.y) - (U.y * V.x))));

	U = (v3CornerOther[6] - v3CornerOther[1]);
	V = (v3CornerOther[4] - v3CornerOther[1]);
	normalsTwo.push_back(glm::normalize(glm::vec3((U.y * V.z) - (U.z * V.y), (U.z * V.x) - (U.x * V.z), (U.x * V.y) - (U.y * V.x))));

	//calculate the edge normals
	std::vector<glm::vec3> edgeNormals;

	glm::vec3 edgesCubeOne[3];
	glm::vec3 edgesCubeTwo[3];
	//calculate for the first cube
	edgesCubeOne[0] = v3Corner[1] - v3Corner[0];
	edgesCubeOne[1] = v3Corner[2] - v3Corner[1];
	edgesCubeOne[2] = v3Corner[6] - v3Corner[1];

	//calculate for the second cube
	edgesCubeTwo[0] = v3CornerOther[1] - v3CornerOther[0];
	edgesCubeTwo[1] = v3CornerOther[2] - v3CornerOther[1];
	edgesCubeTwo[2] = v3CornerOther[6] - v3CornerOther[1];

	//create the edge normals
	for (int i = 0; i < 3; i++) 
	{
		for (int j = 0; j < 3; j++) 
		{
			auto temp = glm::cross(edgesCubeOne[i], edgesCubeTwo[j]);
			if (temp != glm::vec3(0.0f, 0.0f, 0.0f)) 
			{
				edgeNormals.push_back(glm::normalize(temp));
			}
		}
	}

	//get the normals (not the edge normals) for first object and project the min and max onto those and compare
	for (int i = 0; i < normalsOne.size(); i++) 
	{
		//get the min and max projected along the axis for both cubes along the normals
		float minOne;
		float maxOne;
		float minTwo;
		float maxTwo;
		//check the first cube 
		//check the first points
		minOne = glm::dot(v3Corner[0], normalsOne[i]);
		maxOne = minOne;
		for (int j = 1; j < 8; j++) 
		{
			float currentProjection = glm::dot(v3Corner[j], normalsOne[i]);

			//if the current projection is smaller than the old minimum projection than it becomes the minimum projection
			if (minOne > currentProjection) 
			{
				minOne = currentProjection;
			}

			//if the current projection is bigger than the old maximum projection than it becomes the maximum projection
			if (maxOne < currentProjection)
			{
				maxOne = currentProjection;
			}
		}

		//check the second cube 
		//check the first points
		minTwo = glm::dot(v3CornerOther[0], normalsOne[i]);
		maxTwo = minTwo;
		for (int k = 1; k < 8; k++)
		{
			float currentProjection = glm::dot(v3CornerOther[k], normalsTwo[i]);

			//if the current projection is smaller than the old minimum projection than it becomes the minimum projection
			if (minTwo > currentProjection)
			{
				minTwo = currentProjection;
			}

			//if the current projection is bigger than the old maximum projection than it becomes the maximum projection
			if (maxTwo < currentProjection)
			{
				maxTwo = currentProjection;
			}
		}

		//compare the 2 objects min and max projections and see if they are seperated
		isSeperated = maxOne < minTwo || maxTwo < minOne;
		if (isSeperated) break;

	}

	//only run if they are not colliding check the axis that are from the second object
	if (!isSeperated) 
	{
		//get the normals (not the edge normals) for first object and project the min and max onto those and compare
		for (int i = 0; i < normalsTwo.size(); i++)
		{
			//get the min and max projected along the axis for both cubes along the normals
			float minOne;
			float maxOne;
			float minTwo;
			float maxTwo;
			//check the first cube 
			//check the first points
			minOne = glm::dot(v3Corner[0], normalsTwo[i]);
			maxOne = minOne;
			for (int j = 1; j < 8; j++)
			{
				float currentProjection = glm::dot(v3Corner[j], normalsOne[i]);

				//if the current projection is smaller than the old minimum projection than it becomes the minimum projection
				if (minOne > currentProjection)
				{
					minOne = currentProjection;
				}

				//if the current projection is bigger than the old maximum projection than it becomes the maximum projection
				if (maxOne < currentProjection)
				{
					maxOne = currentProjection;
				}
			}

			//check the second cube 
			//check the first points
			minTwo = glm::dot(v3CornerOther[0], normalsTwo[i]);
			maxTwo = minTwo;
			for (int k = 1; k < 8; k++)
			{
				float currentProjection = glm::dot(v3CornerOther[k], normalsTwo[i]);

				//if the current projection is smaller than the old minimum projection than it becomes the minimum projection
				if (minTwo > currentProjection)
				{
					minTwo = currentProjection;
				}

				//if the current projection is bigger than the old maximum projection than it becomes the maximum projection
				if (maxTwo < currentProjection)
				{
					maxTwo = currentProjection;
				}
			}

			//compare the 2 objects min and max projections and see if they are seperated
			isSeperated = maxOne < minTwo || maxTwo < minOne;
			if (isSeperated) break;

		}
	}

	//only run if they are not colliding check the axis that are from the second object
	if (!isSeperated)
	{
		//get the normals (not the edge normals) for first object and project the min and max onto those and compare
		for (int i = 0; i < edgeNormals.size(); i++)
		{
			//get the min and max projected along the axis for both cubes along the normals
			float minOne;
			float maxOne;
			float minTwo;
			float maxTwo;
			//check the first cube 
			//check the first points
			minOne = glm::dot(v3Corner[0], edgeNormals[i]);
			maxOne = minOne;
			for (int j = 1; j < 8; j++)
			{
				float currentProjection = glm::dot(v3Corner[j], edgeNormals[i]);

				//if the current projection is smaller than the old minimum projection than it becomes the minimum projection
				if (minOne > currentProjection)
				{
					minOne = currentProjection;
				}

				//if the current projection is bigger than the old maximum projection than it becomes the maximum projection
				if (maxOne < currentProjection)
				{
					maxOne = currentProjection;
				}
			}

			//check the second cube 
			//check the first points
			minTwo = glm::dot(v3CornerOther[0], edgeNormals[i]);
			maxTwo = minTwo;
			for (int k = 1; k < 8; k++)
			{
				float currentProjection = glm::dot(v3CornerOther[k], edgeNormals[i]);

				//if the current projection is smaller than the old minimum projection than it becomes the minimum projection
				if (minTwo > currentProjection)
				{
					minTwo = currentProjection;
				}

				//if the current projection is bigger than the old maximum projection than it becomes the maximum projection
				if (maxTwo < currentProjection)
				{
					maxTwo = currentProjection;
				}
			}

			//compare the 2 objects min and max projections and see if they are seperated
			isSeperated = maxOne < minTwo || maxTwo < minOne;
			if (isSeperated) break;

		}
	}

	//if is colliding is true return 1 and if not return 0
	int collidingNum;
	if (isSeperated == true) 
	{
		collidingNum = 0;
	}
	if (isSeperated == false)
	{
		collidingNum = 1;
	}
	/*
	Your code goes here instead of this comment;

	For this method, if there is an axis that separates the two objects
	then the return will be different than 0; 1 for any separating axis
	is ok if you are not going for the extra credit, if you could not
	find a separating axis you need to return 0, there is an enum in
	Simplex that might help you [eSATResults] feel free to use it.
	(eSATResults::SAT_NONE has a value of 0)
	*/
	return collidingNum;
}