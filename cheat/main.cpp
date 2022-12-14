#include "gui.h"
#include "hacks.h"
#include "globals.h"

#include <thread>
#include <memory.h>

int __stdcall wWinMain(
	HINSTANCE instance,
	HINSTANCE previousInstance,
	PWSTR arguments,
	int commandShow)
{

	// read csgo.exe
	Memory mem{ "csgo.exe" };
	
	globals::clientAddr = mem.GetModuleAddress("client.dll");
	globals::engineAddr = mem.GetModuleAddress("engine.dll");

	std::thread(hacks::VisualThread, mem).detach();
	
	// create gui
	gui::CreateHWindow("Cheat Menu");
	gui::CreateDevice();
	gui::CreateImGui();

	while (gui::isRunning)
	{
		gui::BeginRender();
		gui::Render();
		gui::EndRender();

		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

	// destroy gui
	gui::DestroyImGui();
	gui::DestroyDevice();
	gui::DestroyHWindow();

	return EXIT_SUCCESS;
	
}
