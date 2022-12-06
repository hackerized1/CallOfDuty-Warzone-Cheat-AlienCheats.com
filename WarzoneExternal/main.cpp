#include "overlay.h"
#include "memory.h"
#include "utility.h"
#include "struct.h"
#include <string>
#include <iostream>
#include <windows.h>
#define COLOUR(x) x/255

float distance = 500;
bool enable = true;
bool boxesp = true;
bool CornerBox = true;
bool SkeletonESP = true;
bool healthbar = true;
bool snapline = false;
bool distanceesp = true;
bool NAMEESP = false;
bool fillbox = true;
bool Aimbot = true;
bool crosshairr = true;
float AimbotFov = 60;
int aSmoothAmount = 1;
uintptr_t bone_base;

float distanceToLocal(auto pos, auto local_pos) {
	return pos.distancee(local_pos);
}

ImColor cRainbow;
	

D3DCOLOR FLOAT4TOD3DCOLOR(float Col[])
{
	ImU32 col32_no_alpha = ImGui::ColorConvertFloat4ToU32(ImVec4(Col[0], Col[1], Col[2], Col[3]));
	float a = (col32_no_alpha >> 24) & 255;
	float r = (col32_no_alpha >> 16) & 255;
	float g = (col32_no_alpha >> 8) & 255;
	float b = col32_no_alpha & 255;
	return D3DCOLOR_ARGB((int)a, (int)r, (int)g, (int)b);
}

namespace Screen
{
	static int Width = GetSystemMetrics(SM_CXSCREEN);
	static int Height = GetSystemMetrics(SM_CYSCREEN);
}

namespace Colors
{
	float CrosshairColors[] = { 255.f, 55.f, 55.f, 255.f };
}


namespace crosshair
{
	namespace sizes
	{

		namespace cross
		{
			float length = 4.0f;
			float gap = 1.0f;
		}


	}
}
void Crosshair()
{
	auto RGB = ImGui::GetColorU32({ 255, 255, 255, 255 });
	ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(GetWindowSize().x / 2 + crosshair::sizes::cross::gap + 1, GetWindowSize().y / 2 - 1), ImVec2(GetWindowSize().x / 2 + crosshair::sizes::cross::length + crosshair::sizes::cross::gap + 1, GetWindowSize().y / 2 + 1), cRainbow);
	ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(GetWindowSize().x / 2 - crosshair::sizes::cross::gap - 1, GetWindowSize().y / 2 + 1), ImVec2(GetWindowSize().x / 2 - crosshair::sizes::cross::length - crosshair::sizes::cross::gap - 1, GetWindowSize().y / 2 - 1), cRainbow);
	ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(GetWindowSize().x / 2 - 1, GetWindowSize().y / 2 + crosshair::sizes::cross::gap + 1), ImVec2(GetWindowSize().x / 2 + 1, GetWindowSize().y / 2 + crosshair::sizes::cross::length + crosshair::sizes::cross::gap + 1), cRainbow);
	ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(GetWindowSize().x / 2 + 1, GetWindowSize().y / 2 - crosshair::sizes::cross::gap - 1), ImVec2(GetWindowSize().x / 2 - 1, GetWindowSize().y / 2 - crosshair::sizes::cross::length - crosshair::sizes::cross::gap - 1), cRainbow);
}

void hile()
{
	sdk::client_info = decryption::get_client_info();
	sdk::client_info_base = decryption::decrypt_client_base(sdk::client_info);
	sdk::player_t local(sdk::client_info_base + (sdk::local_index() * offsets::player::size));
	Vector3 local_pos = local.get_pos();
	auto local_team = local.team_id();
	//printf("%d\n", sdk::local_index());
	for (int i = 0; i < 150; i++) {
		sdk::player_t player(sdk::client_info_base + (i * offsets::player::size));
		uintptr_t ref_def_ptr = decryption::get_ref_def();
		sdk::ref_def = driver.read<sdk::ref_def_t>(ref_def_ptr);

		Vector2 screen;
		Vector3 pos = player.get_pos();
		Vector3 headPos = pos; headPos.z = pos.z + 66.f;
		Vector2 BaseBone, HeadBone;
		sdk::w2s(pos, BaseBone); sdk::w2s(headPos, HeadBone);

	
		//int dist = (int)distanceToLocal(pos, local_pos);
		if (enable)
		{
			auto dist = (int)sdk::units_to_m(local_pos.distance_to(pos));
			std::string DistanceSt = std::to_string(dist);
			if (dist < distance)
			{
				{
					{
						if (NAMEESP)
						{
							sdk::names = sdk::GetNameList();
						}

						sdk::NameEntry nameEntry = player.get_name_entry(i);

						if (dist < 1) { continue; }

						if (!player.is_valid() || player.dead()) {
							continue;
						}
						bool isEnemy = true; //Is the entity your teammate or not

						if (player.team_id() == local_team)
							isEnemy = false;

						if (local_pos == pos) continue;
						wchar_t buf[6];
						float height = abs(HeadBone.y - BaseBone.y);
						float width = height * 0.55;
						float middle = BaseBone.x - (width / 2.f);
						if (AimFov(player, pos) < 960)
						{

							if (SkeletonESP)
							{
								if (dist < 75)
								{
									{
										if (AimFov(player, pos) < 480)
										{
											const auto bone_base = decryption::decrypt_bone_base(sdk::client_info);
											Vector3 bone_base_pos = player.get_bone_base_pos();
											const auto bone_index = decryption::get_bone_index(i); if (bone_index == decryption::get_bone_index(sdk::local_index())) continue;
											uintptr_t bone_ptr = player.get_bone_ptr(bone_base, bone_index);
											if (!bone_ptr) continue;
											for (std::pair<int, int> p2p : player.skeleton)
											{
												Vector3 bone1_3d = player.get_bone_position(bone_ptr, bone_base_pos, p2p.first);
												Vector3 bone2_3d = player.get_bone_position(bone_ptr, bone_base_pos, p2p.second);

												Vector2 bone1;
												Vector2 bone2;

												sdk::w2s(bone1_3d, bone1);
												sdk::w2s(bone2_3d, bone2);
												//If the bones are less than 3 meters apart
												if (sdk::units_to_m(bone1_3d.distance_to(bone2_3d)) <= 3.f)
													//If bones are not within 2 meters of the player
													if (!sdk::units_to_m(local_pos.distance_to(bone1_3d)) <= 2.f && !sdk::units_to_m(local_pos.distance_to(bone2_3d)) <= 2.f)
														DrawLine(ImVec2(bone1.x, bone1.y), ImVec2(bone2.x, bone2.y), cRainbow, 1);
											}
										}
									}
								}
							}

						    if(CornerBox)
						    { 
								if (isEnemy) {
									DrawCorneredBox(BaseBone.x - (width / 2) - 1, HeadBone.y - 1, width + 2, height + 2, ImVec4(0.f, 0.f, 0.f, 255.f), 1.0f);
									DrawCorneredBox(BaseBone.x - (width / 2), HeadBone.y, width, height, cRainbow, 1.0f);
									DrawCorneredBox(BaseBone.x - (width / 2) + 1, HeadBone.y + 1, width - 2, height - 2, ImVec4(0.f, 0.f, 0.f, 255.f), 1.0f);
								}
								else
								{
									DrawCorneredBox(BaseBone.x - (width / 2) - 1, HeadBone.y - 1, width + 2, height + 2, ImVec4(0.f, 0.f, 0.f, 255.f), 1.0f);
									DrawCorneredBox(BaseBone.x - (width / 2), HeadBone.y, width, height, ImVec4(0, 255.f, 0, 255.f), 1.0f);
									DrawCorneredBox(BaseBone.x - (width / 2) + 1, HeadBone.y + 1, width - 2, height - 2, ImVec4(0.f, 0.f, 0.f, 255.f), 1.0f);
								}
							}
							if (boxesp)
							{
								if (isEnemy) {
									DrawRectImGui(BaseBone.x - (width / 2) - 1, HeadBone.y - 1, width + 2, height + 2, ImVec4(0.f, 0.f, 0.f, 255.f), 1.0f);
									DrawRectImGui(BaseBone.x - (width / 2), HeadBone.y, width, height, cRainbow, 1.f);
									DrawRectImGui(BaseBone.x - (width / 2) + 1, HeadBone.y + 1, width - 2, height - 2, ImVec4(0.f, 0.f, 0.f, 255.f), 1.0f);
								}
								else
								{
									DrawRectImGui(BaseBone.x - (width / 2) - 1, HeadBone.y - 1, width + 2, height + 2, ImVec4(0.f, 0.f, 0.f, 255.f), 1.0f);
									DrawRectImGui(BaseBone.x - (width / 2), HeadBone.y, width, height, ImVec4(0, 255.f, 0, 255.f), 1.f);
									DrawRectImGui(BaseBone.x - (width / 2) + 1, HeadBone.y + 1, width - 2, height - 2, ImVec4(0.f, 0.f, 0.f, 255.f), 1.0f);
								}
							}
							if (healthbar)
							{
								if (isEnemy) {

									std::string HEALTHSTRING = std::to_string(nameEntry.health);

									float entHp = std::stof(HEALTHSTRING);
									if (entHp > 101) entHp = 100;
									float HealthHeightCalc = ((float)entHp / 100) * (float)height;
									DrawFilledRectImGui(BaseBone.x - (width / 2), HeadBone.y, 2, height, ImVec4(COLOUR(30.0f), COLOUR(30.0f), COLOUR(30.0f), COLOUR(220.0f)));
									DrawFilledRectImGui(BaseBone.x - (width / 2), HeadBone.y, 2, HealthHeightCalc, ImVec4(COLOUR(80.0f), COLOUR(220.0f), COLOUR(100.0f), COLOUR(255.0f)));
								}
							}
							if (snapline)
							{
								if (isEnemy) {
									DrawLine(ImVec2(1920 / 2, 1080), ImVec2(BaseBone.x, BaseBone.y), IM_COL32(0, 0, 0, 255), 1);
								}
								else
								{
									DrawLine(ImVec2(1920 / 2, 1080), ImVec2(BaseBone.x, BaseBone.y), IM_COL32(0, 255, 0, 255), 1);
								}
							}
							if (distanceesp)
								DrawString(12, BaseBone.x, BaseBone.y, &Col.white, true, true, DistanceSt.c_str());
							if(NAMEESP)
							    DrawString(12, HeadBone.x, HeadBone.y, &Col.white, true, true, nameEntry.name);
							if(fillbox)
								DrawFilledRectImGui(BaseBone.x - (width / 2), HeadBone.y, width, height, ImVec4(COLOUR(0.0f), COLOUR(0.0f), COLOUR(0.0f), COLOUR(100.0f)));
							
							if (isEnemy) {
								Vector2 TargetAngles;
								sdk::w2s(headPos, TargetAngles);
								if (AimFov(player, headPos) < AimbotFov)
								{
									if (GetAsyncKeyState(aimkey))
									{
										if (Aimbot)
										{
											CURSORINFO ci = { sizeof(CURSORINFO) };
											if (GetCursorInfo(&ci))
											{
												if (ci.flags == 0)
													mouse_event(MOUSEEVENTF_MOVE, (TargetAngles.x - 1920 / 2) / aSmoothAmount, (TargetAngles.y - 1080 / 2) / aSmoothAmount, 0, 0);
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}


	}
}
int tab = 1;
void draw_menu()
{
	auto s = ImVec2{}, p = ImVec2{}, gs = ImVec2{ 620, 485 };
	ImGui::SetNextWindowSize(ImVec2(gs));
	ImGui::SetNextWindowBgAlpha(1.0f);
	ImGui::Begin(skCrypt("ALIEN CHEATS - (ver. 0.0.2)"), NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_::ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
	{

		ImGui::SetCursorPosX(121);
		s = ImVec2(ImGui::GetWindowSize().x - ImGui::GetStyle().WindowPadding.x * 2, ImGui::GetWindowSize().y - ImGui::GetStyle().WindowPadding.y * 2); p = ImVec2(ImGui::GetWindowPos().x + ImGui::GetStyle().WindowPadding.x, ImGui::GetWindowPos().y + ImGui::GetStyle().WindowPadding.y); auto draw = ImGui::GetWindowDrawList();
		draw->AddRectFilled(p, ImVec2(p.x + s.x, p.y + s.y), ImColor(15, 17, 19), 5);
		draw->AddRectFilled(ImVec2(p.x, p.y + 40), ImVec2(p.x + s.x, p.y + s.y), ImColor(18, 20, 21), 5, ImDrawCornerFlags_Bot);

		ImGui::PushFont(icons);
		auto logo_size = ImGui::CalcTextSize(skCrypt("A"));
		draw->AddText(ImVec2(p.x + 28 - logo_size.x / 2, p.y + 29 - (logo_size.y / 2) + 4), cRainbow, skCrypt("A"));
		ImGui::PopFont();

		ImGui::PushFont(main_font2);
		auto logo_size2 = ImGui::CalcTextSize(skCrypt("A"));
		draw->AddText(ImVec2(p.x + 42 - logo_size2.x / 2, p.y + 29 - logo_size2.y), cRainbow, skCrypt("AlienCheats"));
		ImGui::PopFont();
		ImGui::PushFont(main_font);
		draw->AddText(ImVec2(p.x + 42 - logo_size2.x / 2, p.y + 40 - logo_size2.y), cRainbow, "(WARZONE LEGIT)");
		ImGui::PopFont();

		ImGui::PushFont(main_font);
		ImGui::BeginGroup();
		ImGui::SameLine(140);
		if (ImGui::tab(skCrypt("MAIN"), tab == 1))tab = 1; ImGui::SameLine();
		ImGui::EndGroup();
		ImGui::PopFont();

		if (tab == 1)
		{
			ImGui::PushFont(main_font);
			{//left upper
				ImGui::SetCursorPosY(50);
				ImGui::BeginGroup();
				ImGui::SetCursorPosX(15);
				ImGui::MenuChild(skCrypt("Aimbot"), ImVec2(285, 415), false);
				{
					ImGui::Checkbox(skCrypt("Aim Bot"), &Aimbot);
					ImGui::Text(skCrypt("")); ImGui::SameLine(8); HotkeyButton(aimkey, ChangeKey, keystatus); ImGui::SameLine(); ImGui::Text(skCrypt("Aimbot Key"));
					ImGui::SliderFloat(skCrypt("Fov"), &AimbotFov, 1, 360, "% .2f");
					ImGui::SliderInt(skCrypt("Smooth"), &aSmoothAmount, 1, 10, "% .2f");
					ImGui::Text(skCrypt(""));
					ImGui::Checkbox(skCrypt("Crosshair"), &crosshairr);
				}
				ImGui::EndChild();
				ImGui::EndGroup();
			}
			{//right
				ImGui::SetCursorPosY(50);
				ImGui::BeginGroup();
				ImGui::SetCursorPosX(320);
				ImGui::MenuChild(skCrypt("ESP"), ImVec2(285, 415), false);
				{
					ImGui::Checkbox(skCrypt("Enable ESP"), &enable);
					ImGui::SliderFloat(skCrypt("Distance"), &distance, 1.f, 750.f);
					ImGui::Checkbox(skCrypt("Box ESP"), &boxesp);
					if (boxesp == true)
					{
						CornerBox = false;
					}
					ImGui::Checkbox(skCrypt("Cornoer Box ESP"), &CornerBox);
					if (CornerBox == true)
					{
						boxesp = false;
					}
					ImGui::Checkbox(skCrypt("Fill Box"), &fillbox);
					ImGui::Checkbox(skCrypt("Skeleton ESP"), &SkeletonESP);
					ImGui::Checkbox(skCrypt("Health Bar"), &healthbar);
					ImGui::Checkbox(skCrypt("Snapline ESP"), &snapline);
					ImGui::Checkbox(skCrypt("Distance ESP"), &distanceesp);
					ImGui::Checkbox(skCrypt("Name ESP"), &NAMEESP);
				}
				ImGui::EndChild();
				ImGui::EndGroup();
				ImGui::PopFont();
			}
		}
	}
	ImGui::End();
}

void render()
{
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (Aimbot)
	{
		ImGui::GetOverlayDrawList()->AddCircle(ImVec2(ScreenCenterX, ScreenCenterY), AimbotFov, IM_COL32_WHITE, 10000, 1);
	}
	if (crosshairr)
	{
		Crosshair();
	}

	ImVec2 curPos = ImGui::GetCursorPos();
	ImVec2 curWindowPos = ImGui::GetWindowPos();
	curPos.x += curWindowPos.x;
	curPos.y += curWindowPos.y;
	static float flRainbow;
	float flSpeed = 0.0015f;
	flRainbow += flSpeed;
	if (flRainbow > 1.f) flRainbow = 0.f;
	for (int i = 0; i < 485; i++)
	{
		float hue = (1.f / (float)485) * i;
		hue -= flRainbow;
		if (hue < 0.f) hue += 1.f;

		cRainbow = ImColor::HSV(hue, 1.f, 1.f);
		//ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(curPos.x + i, 10), ImVec2(curPos.x + 485, curPos.y - 3), cRainbow);
	}

	if (GetAsyncKeyState(VK_INSERT) & 1) { showmenu = !showmenu; }
	hile();

	if (showmenu) { draw_menu(); }
	ImGui::EndFrame();
	p_Device->SetRenderState(D3DRS_ZENABLE, false);
	p_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	p_Device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
	p_Device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	if (p_Device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		p_Device->EndScene();
	}

	HRESULT result = p_Device->Present(NULL, NULL, NULL, NULL);

	if (result == D3DERR_DEVICELOST && p_Device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
	{
		ImGui_ImplDX9_InvalidateDeviceObjects();
		p_Device->Reset(&d3dpp);
		ImGui_ImplDX9_CreateDeviceObjects();
	}
}
#include "protect/protectmain.h"
#include "xorstr.hpp"
#include "auth.hpp"

using namespace KeyAuth;

std::string namee = _xor_("Warzone Cheat"); // application name. right above the blurred text aka the secret on the licenses tab among other tabs
std::string ownerid = _xor_("jHHXUBClti"); // ownerid, found in account settings. click your profile picture on top right of dashboard and then account settings.
std::string secret = _xor_("1daa154c0d6a089948625ee1bc662df50c7eb0e953320bab506ca5dcd18acd24"); // app secret, the blurred text on licenses tab and other tabs
std::string version = _xor_("1.0"); // leave alone unless you've changed version on website
std::string url = _xor_("https://auth.aliencheats.com/api/1.1/"); // change if you're self-hosting
std::string sslPin = _xor_("ssl pin key (optional)"); // don't change unless you intend to pin public certificate key. you can get here in the "Pin SHA256" field https://www.ssllabs.com/ssltest/analyze.html?d=keyauth.win&latest. If you do this you need to be aware of when SSL key expires so you can update it
api KeyAuthApp(namee, ownerid, secret, version, url, sslPin);



int main()
{
	SetConsoleTitle(skCrypt("Warzone External Cheat"));
	int horizontal = 0, vertical = 0;
	int x = 350, y = 250; //// alta doðru
	HWND consoleWindow = GetConsoleWindow();
	SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO     cursorInfo;
	GetConsoleCursorInfo(out, &cursorInfo);
	SetConsoleCursorInfo(out, &cursorInfo);
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(cfi);
	cfi.nFont = 0;
	cfi.dwFontSize.X = 0;
	cfi.dwFontSize.Y = 15;
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;
	system(skCrypt("color 4"));
	wcscpy_s(cfi.FaceName, skCrypt(L"Consolas"));
	SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
	HWND hwnd = GetConsoleWindow();
	MoveWindow(hwnd, 0, 0, x, y, TRUE);
	LONG lStyle = GetWindowLong(hwnd, GWL_STYLE);
	lStyle &= ~(WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
	SetWindowLong(hwnd, GWL_STYLE, lStyle);
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(console, &csbi);
	COORD scrollbar = {
		csbi.srWindow.Right - csbi.srWindow.Left + 1,
		csbi.srWindow.Bottom - csbi.srWindow.Top + 1
	};
	if (console == 0)
		throw 0;
	else
		SetConsoleScreenBufferSize(console, scrollbar);
	SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hwnd, 0, 225, LWA_ALPHA);
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);

	SetConsoleTitleA(skCrypt("Warzone External Cheat"));
	mainprotect();

	std::string DosyaKonumu = (_xor_("C:\\Windows\\System\\WarzonePrivateLicense.txt"));

	KeyAuthApp.init();
EnBasaDon:
	std::string License;
	FILE* Dosya;
	if (Dosya = fopen((DosyaKonumu.c_str()), skCrypt("r"))) {
		std::ifstream DosyaOku(DosyaKonumu.c_str());
		std::string Anahtar;
		std::getline(DosyaOku, Anahtar);
		DosyaOku.close();
		fclose(Dosya);
		License = Anahtar;
		goto LisansaGit;
	}
	else
	{
		SetConsoleTitleA(skCrypt("  "));

		system(skCrypt("cls"));
		std::cout << skCrypt("\n\n  [+] Connecting..");
		std::cout << skCrypt("\n\n  [+] Enter License: ");
		std::cin >> License;
	LisansaGit:
		std::ofstream key; key.open(DosyaKonumu.c_str());
		key << License;
		key.close();
		KeyAuthApp.license(License);
		if (!KeyAuthApp.data.success)
		{
			std::cout << _xor_("\n Status: ").c_str() + KeyAuthApp.data.message;
			Sleep(1500);
			remove(DosyaKonumu.c_str());
			goto EnBasaDon;
			//exit(0);
		}
		system(skCrypt("cls"));
		Sleep(300);
		std::cout << skCrypt("\n\n  [+] License Activated.") << std::endl;;
	}

r8:
	if (!driver.init())
	{
	r7:
		if (FindWindowA(skCrypt("ModernWarfare.exe"), NULL))
		{
			printf(skCrypt("[>] close game please...\n"));
			Sleep(1000);
			goto r7;
		}
		if (LoadDriver())
		{
			printf(skCrypt("[>] driver loaded!\n"));
			Sleep(1000);
			system("cls");
			goto r8;
		}
	}
	HWND Entryhwnd = NULL;
	while (Entryhwnd == NULL)
	{
		printf(skCrypt("[>] waiting for game\n"));
		Sleep(1);
		g_pid = get_pid();
		printf(skCrypt("[>] pid: %d\n"), g_pid);
		Entryhwnd = get_process_wnd(g_pid);
		Sleep(1);
	}
	driver.attach(g_pid);
	setup_window();
	init_wndparams(MyWnd);
	g_base = driver.get_process_base(g_pid);
	sdk::module_base = g_base;
	sdk::peb = driver.get_peb(g_pid);
	sdk::client_info = decryption::get_client_info();
	sdk::client_info_base = decryption::decrypt_client_base(sdk::client_info);
	uintptr_t ref_def_ptr = decryption::get_ref_def();
	sdk::ref_def = driver.read<sdk::ref_def_t>(ref_def_ptr);

	printf("g_base %d\n", sdk::module_base);
	printf("peb %d\n", sdk::peb);
	printf("sdk::client_info %d\n", sdk::client_info);
	printf("client_info_base %d\n", sdk::client_info_base);

	Style();
	static RECT old_rc;
	ZeroMemory(&Message, sizeof(MSG));
	while (Message.message != WM_QUIT)
	{
		if (PeekMessage(&Message, MyWnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}

		HWND hwnd_active = GetForegroundWindow();


		if (hwnd_active == GameWnd) {
			HWND hwndtest = GetWindow(hwnd_active, GW_HWNDPREV);
			SetWindowPos(MyWnd, hwndtest, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		RECT rc;
		POINT xy;

		ZeroMemory(&rc, sizeof(RECT));
		ZeroMemory(&xy, sizeof(POINT));
		GetClientRect(GameWnd, &rc);
		ClientToScreen(GameWnd, &xy);
		rc.left = xy.x;
		rc.top = xy.y;

		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = GameWnd;
		io.DeltaTime = 1.0f / 60.0f;

		POINT p;
		GetCursorPos(&p);
		io.MousePos.x = p.x - xy.x;
		io.MousePos.y = p.y - xy.y;
		if (GetAsyncKeyState(0x1)) {
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else io.MouseDown[0] = false;

		if (rc.left != old_rc.left || rc.right != old_rc.right || rc.top != old_rc.top || rc.bottom != old_rc.bottom) {

			old_rc = rc;

			Width = rc.right;
			Height = rc.bottom;

			p_Params.BackBufferWidth = Width;
			p_Params.BackBufferHeight = Height;
			SetWindowPos(MyWnd, (HWND)0, xy.x, xy.y, Width, Height, SWP_NOREDRAW);
			p_Device->Reset(&p_Params);
		}
		render();
		Sleep(10);
	}

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	cleanup_d3d();
	DestroyWindow(MyWnd);
	return Message.wParam;
}