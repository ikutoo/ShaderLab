#include "passRenderer.h"
#include <boost/format.hpp>
#include "AudioEngineInterface.h"
#include "constants.h"
#include "common.h"
#include "textureUtil.h"
#include "gameShadingTechnique.h"
#include "sceneRenderer.h"
#include "Application.h"
#include "meshRenderer.h"

CPassRenderer::CPassRenderer()
{

}

CPassRenderer::~CPassRenderer()
{
	__destroy();
}

//*********************************************************************************
//FUNCTION:
void CPassRenderer::init(const SPassConfig& vPassConfig)
{
	m_Config = vPassConfig;
	m_pShadingTechnique = CGameShadingTechnique::getInstance();
	m_pSceneRenderer = CSceneRenderer::getInstance();
	m_pQuadRenderer = new CMeshRenderer(constant::QUAD_VERTICES, sizeof(constant::QUAD_VERTICES));

	__initTextures();
	__initBuffers();
	__initShaders();
	__initAudio();
}

//*********************************************************************************
//FUNCTION:
void CPassRenderer::renderPass()
{
	auto& PassID = m_Config.passID;
	auto& Type = m_Config.type;

	if (EPassType::BUFFER == Type) {
		glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, m_CaptureRBO);
	}
	else if (EPassType::IMAGE == Type) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	auto PassName = boost::str(boost::format("pass%1%") % PassID);

	__updateKeyboardTexture();
	__updateAudioTexture();

	m_pShadingTechnique->enableProgram(PassName);
	__updateShaderUniforms4ImagePass();
	m_pQuadRenderer->draw();
	m_pShadingTechnique->disableProgram();

	glGenerateMipmap(GL_TEXTURE_2D); //TODO: ������

	for (unsigned int i = 0; i < m_TextureSet.size(); ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	if (EPassType::BUFFER == Type) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
}

//*********************************************************************************
//FUNCTION:
void CPassRenderer::__initTextures()
{
	for (auto& Config : m_Config.channelConfigSet)
	{
		if (EChannelType::BUFFER == Config.type) {
			auto Index = atoi(Config.value.c_str());
			auto TextureID = m_pSceneRenderer->getRenderTextureByPassID(Index);
			_ASSERTE(TextureID > 0);
			m_TextureSet.push_back(TextureID);
		}
		else if (EChannelType::TEXTURE == Config.type) {
			auto TextureID = util::loadTexture(Config.value.c_str(), Config.filterMode, Config.wrapMode, Config.vflip, Config.isMipmap);
			_ASSERTE(TextureID > 0);
			m_TextureSet.push_back(TextureID);
		}
		else if (EChannelType::KEYBOARD == Config.type) {
			m_KeyboardTex = util::setupTexture2D(constant::KEYBORAD_TEX_WIDTH, constant::KEYBORAD_TEX_HEIGHT, GL_R8, GL_RED, GL_UNSIGNED_BYTE, GL_NEAREST);
			_ASSERTE(m_KeyboardTex > 0);
			m_TextureSet.push_back(m_KeyboardTex);
		}
		else if (EChannelType::SOUND == Config.type) {
			m_AudioFilePath = Config.value;
			m_AudioTex = util::setupTexture2D(constant::AUDIO_TEX_WIDTH, constant::AUDIO_TEX_HEIGHT, GL_R32F, GL_RED, GL_FLOAT, GL_NEAREST);
			_ASSERTE(m_AudioTex > 0);
			m_TextureSet.push_back(m_AudioTex);
		}
	}
}

//*********************************************************************************
//FUNCTION:
void CPassRenderer::__initShaders()
{
	auto pShadingPass = new CProgram;
	const SSceneConfig& SceneConfig = CSceneRenderer::getInstance()->getSceneConfig();

	pShadingPass->addShader(constant::DRAW_QUAD_SHADER_PATH, VERTEX_SHADER);
	std::vector<std::string> ShaderFilesPaths = { constant::MAIN_IMAGE_SHADER_PATH };
	if (!SceneConfig.commonShaderPath.empty()) { ShaderFilesPaths.push_back(SceneConfig.commonShaderPath); }
	ShaderFilesPaths.push_back(m_Config.shaderPath);

	pShadingPass->addShader(ShaderFilesPaths, FRAGMENT_SHADER);
	auto PassName = boost::str(boost::format("pass%1%") % m_Config.passID);
	m_pShadingTechnique->addProgram(PassName, pShadingPass);
}

//*********************************************************************************
//FUNCTION:
void CPassRenderer::__initBuffers()
{
	if (EPassType::BUFFER != m_Config.type) return;

	glGenFramebuffers(1, &m_CaptureFBO);
	glGenRenderbuffers(1, &m_CaptureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_CaptureRBO);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_CaptureRBO);

	auto WinSize = m_pSceneRenderer->getWinSize();
	m_RenderTex = m_pSceneRenderer->getRenderTextureByPassID(m_Config.passID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, WinSize.x, WinSize.y);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_RenderTex, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//*********************************************************************************
//FUNCTION:
float* __equalizerVisualization(const std::vector<float>& vSampleData) { //TODO: �����������������ﲻ����
	if (vSampleData.empty()) return nullptr;

	const int WAVE_DATA_SIZE = constant::AUDIO_TEX_WIDTH;
	const int EQUALIZER_COLUMNS = 128; //TODO: too many magic numbers
	const int EQUALIZER_INERTIA = 500;

	static float spectrumData[WAVE_DATA_SIZE];
	static float previousSpectrumData[WAVE_DATA_SIZE];
	static int columnsInertia[EQUALIZER_COLUMNS];

	int picker = vSampleData.size() / WAVE_DATA_SIZE;
	int columnWidth = WAVE_DATA_SIZE / EQUALIZER_COLUMNS;
	for (int columnNumber = 0; columnNumber < EQUALIZER_COLUMNS; columnNumber++) {
		float sum = 0;

		for (int i = 0; i < columnWidth; i++) {
			int currentIndex = columnWidth * columnNumber + i;
			sum += vSampleData[currentIndex * picker];
		}
		sum *= 0.00000015;
		sum = pow(sum, 0.45);
		for (int i = 0; i < columnWidth; i++) {
			int currentIndex = columnWidth * columnNumber + i;

			if (sum > previousSpectrumData[currentIndex]) {
				columnsInertia[columnNumber] = EQUALIZER_INERTIA;
				spectrumData[currentIndex] = sum;
			}
			else {
				spectrumData[currentIndex] = previousSpectrumData[currentIndex] - previousSpectrumData[currentIndex] * ((float)EQUALIZER_INERTIA + 1 - columnsInertia[columnNumber]) / ((float)EQUALIZER_INERTIA);
			}
		}
	}

	for (int i = 0; i < EQUALIZER_COLUMNS; ++i) { if (columnsInertia[i] != 0) columnsInertia[i]--; }
	for (int i = 0; i < WAVE_DATA_SIZE; ++i) { previousSpectrumData[i] = spectrumData[i]; }

	return spectrumData;
}

//*********************************************************************************
//FUNCTION:
void CPassRenderer::__updateAudioTexture()
{
	if (0 == m_AudioTex) return;

	const int Width = constant::AUDIO_TEX_WIDTH, Height = constant::AUDIO_TEX_HEIGHT;
	_ASSERTE(Height >= 2);

	_ASSERTE(m_AudioID >= 0);
	std::vector<float> SampleData;
	sengine::audioEngine::dumpAudioSampleData(m_AudioID, SampleData);
	float* pSpectrumData = __equalizerVisualization(SampleData);

	float ImageData[Width*Height]{ 0 };
	for (int i = 0; i < Width; ++i) {
		ImageData[i] = pSpectrumData[i];
	}

	glBindTexture(GL_TEXTURE_2D, m_AudioTex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Width, Height, GL_RED, GL_FLOAT, ImageData);
	glBindTexture(GL_TEXTURE_2D, 0);
}

//*********************************************************************************
//FUNCTION:
void CPassRenderer::__updateKeyboardTexture()
{
	if (0 == m_KeyboardTex) return;

	const int Width = constant::KEYBORAD_TEX_WIDTH, Height = constant::KEYBORAD_TEX_HEIGHT;
	GLubyte ImageData[Width*Height] = { 0 };
	for (int i = 0; i < Width; ++i) {
		if (CApplication::isKeyPressed(i)) {
			ImageData[i] = 0xff;
		}
	}

	glBindTexture(GL_TEXTURE_2D, m_KeyboardTex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Width, Height, GL_RED, GL_UNSIGNED_BYTE, ImageData);
	glBindTexture(GL_TEXTURE_2D, 0);
}

//*********************************************************************************
//FUNCTION:
void CPassRenderer::__updateShaderUniforms4ImagePass()
{
	auto Resolution = glm::vec3(glm::vec2(m_pSceneRenderer->getWinSize()), 1.0);
	m_pShadingTechnique->updateStandShaderUniform("iResolution", Resolution);
	m_pShadingTechnique->updateStandShaderUniform("iTime", m_pSceneRenderer->getTime());
	m_pShadingTechnique->updateStandShaderUniform("iTimeDelta", CApplication::getInstance()->getFrameInterval());
	m_pShadingTechnique->updateStandShaderUniform("iFrame", m_pSceneRenderer->getFrameCount());

	for (unsigned int i = 0; i < m_TextureSet.size(); ++i) {
		auto TextureID = m_TextureSet[i];
		_ASSERTE(TextureID != 0);

		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, TextureID);
		std::string UniformName = boost::str(boost::format("iChannel[%1%]") % i);
		m_pShadingTechnique->updateStandShaderUniform(UniformName, i);

		GLint Width, Height;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &Width);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &Height);
		UniformName = boost::str(boost::format("iChannelResolution[%1%]") % i);
		m_pShadingTechnique->updateStandShaderUniform(UniformName, glm::vec3(Width, Height, 0));
	}

	glm::vec2 MousePos = glm::vec2(0);
	bool IsButton1Pressed = CApplication::isButtonPressed(GLFW_MOUSE_BUTTON_1);
	bool IsButton2Pressed = CApplication::isButtonPressed(GLFW_MOUSE_BUTTON_2);
	if (IsButton1Pressed || IsButton2Pressed)
	{
		auto Pos = CApplication::getCursorPos();
		auto WinSize = m_pSceneRenderer->getWinSize();
		MousePos = glm::vec2(Pos.x, WinSize.y - Pos.y);
	}
	glm::vec4 Mouse = glm::vec4(MousePos.x, MousePos.y, IsButton1Pressed, IsButton2Pressed);
	m_pShadingTechnique->updateStandShaderUniform("iMouse", Mouse);
}

//*********************************************************************************
//FUNCTION:
void CPassRenderer::__initAudio()
{
	if (m_AudioFilePath.empty()) return;
	sengine::audioEngine::setGenerateAudioSampleHint();
	m_AudioID = sengine::audioEngine::playAudio2D(m_AudioFilePath);

	m_pShadingTechnique->enableProgram(boost::str(boost::format("pass%1%") % m_Config.passID));
	m_pShadingTechnique->updateStandShaderUniform("iAudioDuration", sengine::audioEngine::getAudioDuration(m_AudioID));
	m_pShadingTechnique->disableProgram();
}

//*********************************************************************************
//FUNCTION:
void CPassRenderer::__destroy()
{
	SAFE_DELETE(m_pQuadRenderer);

	for each (auto Tex in m_TextureSet)
	{
		glDeleteTextures(1, &Tex);
	}
	glDeleteTextures(1, &m_KeyboardTex);
	glDeleteTextures(1, &m_RenderTex);

	glDeleteBuffers(1, &m_CaptureRBO);
	glDeleteBuffers(1, &m_CaptureFBO);

	sengine::audioEngine::stopAllAudios();
}