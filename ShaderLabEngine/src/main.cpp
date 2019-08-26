#include <iostream>
#include "Application.h"

using namespace glt;

int main()
{
	SWindowInfo WindowInfo;
	WindowInfo.Width = 1600;
	WindowInfo.Height = 900;
	WindowInfo.PosX = 200;
	WindowInfo.PosY = 100;
	WindowInfo.Title = "Shader Lab";

	if (!CApplication::getInstance()->init(WindowInfo)) return -1;
	CApplication::getInstance()->run();

	return 0;
}