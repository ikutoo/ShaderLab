#pragma once

#include <string>
#include <ctime>
#include <vector>
#include <GLM/glm.hpp>
#include "glt/ApplicationBase.h"

class CSceneRenderer;
class CShadingTechnique;
class CMeshRenderer;
class CPassRenderer;
struct SPassConfig;

class CApplication : public glt::CApplicationBase
{
public:
	virtual ~CApplication();

	_SINGLETON(CApplication);

	static bool isKeyPressed(int vKeyCode);
	static bool isButtonPressed(int vButtonCode);
	static const glm::vec2& getCursorPos();

protected:
	CApplication();
	_DISALLOW_COPY_AND_ASSIGN(CApplication);

	virtual bool _initV() override;
	virtual void _updateV() override;
	virtual void _renderV() override;

private:
	static CApplication* m_pInstance;
	CShadingTechnique* m_pShadingTechnique = nullptr;
	CSceneRenderer* m_pSceneRenderer = nullptr;

	void __destory();
};