#include "Octant.h"

Simplex::Octant::Octant(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	m_uMaxLevel = a_nMaxLevel;
	m_uIdealEntityCount = a_nIdealEntityCount;
	m_pEntityMngr = MyEntityManager::GetInstance();
	m_pMeshMngr = MeshManager::GetInstance();
	m_v3Center = GetCenterGlobal();
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(GetSize(), GetSize(), GetSize()), C_YELLOW);
}

Simplex::Octant::Octant(vector3 a_v3Center, float a_fSize)
{
	m_fSize = a_fSize;
	m_v3Center = a_v3Center;
}

Simplex::Octant::Octant(Octant const & other)
{
}

Simplex::Octant & Simplex::Octant::operator=(Octant const & other)
{
	// TODO: insert return statement here
	return Octant();
}

Simplex::Octant::~Octant(void)
{
}

void Simplex::Octant::Swap(Octant & other)
{
}

float Simplex::Octant::GetSize(void)
{
	return 100.0f;
}

glm::vec3 Simplex::Octant::GetCenterGlobal(void)
{
	vector3 center = vector3((GetMaxGlobal() + GetMinGlobal()) / 2.0f);
	return center;
}

glm::vec3 Simplex::Octant::GetMinGlobal(void)
{
	vector3 min = m_pEntityMngr->GetMin();
	return min;
}

glm::vec3 Simplex::Octant::GetMaxGlobal(void)
{
	vector3 max = m_pEntityMngr->GetMax();
	return max;
}

bool Simplex::Octant::IsColliding(uint a_uRBIndex)
{
	
	return false;
}

void Simplex::Octant::Display(uint a_nIndex, vector3 a_v3Color)
{
	m_pMeshMngr->AddWireCubeToRenderList(IDENTITY_M4, a_v3Color, 1);
}

void Simplex::Octant::Display(vector3 a_v3Color)
{
}

void Simplex::Octant::DisplayLeafs(vector3 a_v3Color)
{
}

void Simplex::Octant::ClearEntityList(void)
{
}

void Simplex::Octant::Subdivide(void)
{
}

Simplex::Octant * Simplex::Octant::GetChild(uint a_nChild)
{
	return nullptr;
}

Simplex::Octant * Simplex::Octant::GetParent(void)
{
	return nullptr;
}

bool Simplex::Octant::IsLeaf(void)
{
	return false;
}

bool Simplex::Octant::ContainsMoreThan(uint a_nEntities)
{
	return false;
}

void Simplex::Octant::KillBranches(void)
{
}

void Simplex::Octant::ConstructTree(uint a_nMaxLevel)
{
}

void Simplex::Octant::AssignIDtoEntity(void)
{
}

unsigned int Simplex::Octant::GetOctantCount(void)
{
	return uint();
}

void Simplex::Octant::Release(void)
{
}

void Simplex::Octant::Init(void)
{
}

void Simplex::Octant::ConstructList(void)
{
}
