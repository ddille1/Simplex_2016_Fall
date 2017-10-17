#include "AppClass.h"


void Application::InitVariables(void)
{
	////Change this to your name and email
	//m_sProgrammer = "David Dille - dd5564@g.rit.edu";

	////Alberto needed this at this position for software recording.
	//m_pWindow->setPosition(sf::Vector2i(710, 0));
	
	//Set the position and target of the camera
	//(I'm at [0,0,10], looking at [0,0,0] and up is the positive Y axis)
	m_pCameraMngr->SetPositionTargetAndUp(AXIS_Z * 20.0f, ZERO_V3, AXIS_Y);

	//if the light position is zero move it
	if (m_pLightMngr->GetPosition(1) == ZERO_V3)
		m_pLightMngr->SetPosition(vector3(0.0f, 0.0f, 3.0f));

	//if the background is cornflowerblue change it to black (its easier to see)
	if (vector3(m_v4ClearColor) == C_BLUE_CORNFLOWER)
	{
		m_v4ClearColor = vector4(ZERO_V3, 1.0f);
	}
	
	//if there are no segments create 7
	if(m_uOrbits < 1)
		m_uOrbits = 7;

	float fSize = 1.0f; //initial size of orbits
	float fRadius = 0.95f; //initial orbit radius

	//creating a color using the spectrum 
	uint uColor = 650; //650 is Red
	//prevent division by 0
	float decrements = 250.0f / (m_uOrbits > 1? static_cast<float>(m_uOrbits - 1) : 1.0f); //decrement until you get to 400 (which is violet)
	/*
		This part will create the orbits, it start at 3 because that is the minimum subdivisions a torus can have
	*/
	//list of list of stops
	uint uSides = 3; //start with the minimal 3 sides
	for (uint i = uSides; i < m_uOrbits + uSides; i++)
	{
		vector3 v3Color = WaveLengthToRGB(uColor); //calculate color based on wavelength

		m_shapeList.push_back(m_pMeshMngr->GenerateTorus(fSize, fSize - 0.1f, 3, i, v3Color)); //generate a custom torus and add it to the meshmanager

		//Calculate the stops

		//calculate the diff theta by using the nuber of subdivisions
		float theta = 0;
		float diff = (2 * PI) / i;

		//create a vector to store the points
		std::vector<vector3> stopList;

		//calculate the points based on the number of sub divisions radius and diffTheta
		for (int j = 0; j < i; j++)
		{
			stopList.push_back(vector3{ (fRadius * cos(theta)),(fRadius * sin(theta)), 0 });
			theta += diff;
		}
		//add the list of points to the list of the lists
		ListOfstopList.push_back(stopList);
		fSize += 0.5f; //increment the size for the next orbit
		fRadius += 0.5f; //increment the radius for the next orbit
		uColor -= static_cast<uint>(decrements); //decrease the wavelength
	}
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	matrix4 m4View = m_pCameraMngr->GetViewMatrix(); //view Matrix
	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix(); //Projection Matrix
	matrix4 m4Offset = IDENTITY_M4; //offset of the orbits, starts as the global coordinate system
	/*
		The following offset will orient the orbits as in the demo, start without it to make your life easier.
	*/
	//m4Offset = glm::rotate(IDENTITY_M4, 90.0f, AXIS_Z);

	// draw a shapes
	//create a timer (this timer is from the 14B Lerp Video by Alberto)
	static uint uClock = m_pSystem->GenClock();
	static float fTime = 0.0f;
	float fDelta = m_pSystem->GetDeltaTime(uClock);
	fTime += fDelta;

	//create a vector of routes
	static std::vector<uint> routes;

	//fill the vector of routes
	if (routes.size() == 0)
	{
		for (uint i = 0; i < m_uOrbits; ++i)
		{
			routes.push_back(0);
		}
	}
	//create a percentage (also from the 14B Lerp Video)
	float fPercentage = static_cast<float>(MapValue(fTime, 0.0f, 1.0f, 0.0f, 1.0f));

	//loop through the orbits
	for (uint i = 0; i < m_uOrbits; ++i)
	{
		
		//initialize the start and end variable
		vector3 fStartPosition;
		vector3 fEndPosition;

		//get the start position and end position
		if (routes[i] < ListOfstopList[i].size() - 1) 
		{
			fStartPosition = ListOfstopList[i][routes[i]];
			fEndPosition = ListOfstopList[i][routes[i] + 1];
		}
		else 
		{
			fStartPosition = ListOfstopList[i][ListOfstopList[i].size() - 1];
			fEndPosition = ListOfstopList[i][0];
		}


		m_pMeshMngr->AddMeshToRenderList(m_shapeList[i], glm::rotate(m4Offset, 90.0f, AXIS_X));

		//calculate the current position
		vector3 v3CurrentPos = glm::lerp(fStartPosition, fEndPosition, fPercentage);
		matrix4 m4Model = glm::translate(m4Offset, v3CurrentPos);

		//check to see if the percentage is greater than one and if so change the value of route
		if (fPercentage >= 1.0f) 
		{
			routes[i] += 1;
			fTime = 0.0f;
			routes[i] %= ListOfstopList[i].size();
		}

		//draw spheres
		m_pMeshMngr->AddSphereToRenderList(m4Model * glm::scale(vector3(0.1)), C_WHITE);
	}

	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();
	
	//draw gui
	DrawGUI();
	
	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	//release GUI
	ShutdownGUI();
}