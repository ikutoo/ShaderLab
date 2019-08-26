#include "Application.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "gameShadingTechnique.h"
#include "meshRenderer.h"
#include "textureUtil.h"
#include "constants.h"
#include "common.h"
#include "gameConfig.h"
#include "passRenderer.h"
#include "sceneRenderer.h"

using namespace constant;

namespace
{
	bool g_Buttons[3];
	bool g_Keys[KEYBORAD_TEX_WIDTH];
	glm::vec2 g_CursorPos;
}

CApplication* CApplication::m_pInstance = nullptr;

CApplication::CApplication()
{
}

CApplication::~CApplication()
{
	__destory();
}

//*********************************************************************************
//FUNCTION:
bool CApplication::_initV()
{
	m_pShadingTechnique = CGameShadingTechnique::getInstance();
	m_pShadingTechnique->initV();

	m_pSceneRenderer = CSceneRenderer::getInstance();
	m_pSceneRenderer->init();
	m_pSceneRenderer->loadScene(CGameConfig::getInstance()->getConfig().entrySceneID);

	return true;
}

//***********************************************************************************************
//FUNCTION:
void CApplication::_updateV()
{

}

//*********************************************************************************
//FUNCTION:
void CApplication::_renderV()
{
	m_pSceneRenderer->renderScene();
}

//*********************************************************************************
//FUNCTION:
void CApplication::__destory()
{
	//sengine::audioEngine::destroy();
}