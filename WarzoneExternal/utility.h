#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <emmintrin.h>
#include "struct.h"
#include "memory.h"
#include <Vector>
#include <iostream>
#include <string>
bool showmenu;
bool esp_enable;


std::uint32_t get_pid(const std::string& name = "ModernWarfare.exe")
{
	const auto snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snap == INVALID_HANDLE_VALUE) {
		return 0;
	}

	PROCESSENTRY32 proc_entry{};
	proc_entry.dwSize = sizeof proc_entry;

	auto found_process = false;
	if (!!Process32First(snap, &proc_entry)) {
		do {
			if (name == proc_entry.szExeFile) {
				found_process = true;
				break;
			}
		} while (!!Process32Next(snap, &proc_entry));
	}

	CloseHandle(snap);
	return found_process
		? proc_entry.th32ProcessID
		: 0;
}

namespace offsets
{
    constexpr auto ref_def_ptr = 0x15D53058;
    constexpr auto name_array = 0x15D5F830;
    constexpr auto name_array_list = 0x4C70;
    constexpr auto name_array_size = 0xD0;
    constexpr auto camera_base = 0x1324EEB0;
    constexpr auto camera_pos = 0x1E8;
    constexpr auto local_index = 0x3DFC0;
    constexpr auto local_index_pos = 0x204;
    constexpr auto recoil = 0x1C960;
    constexpr auto game_mode = 0x15851DAC;
    constexpr auto weapon_definitions = 0xFFFF8009D72BEAC7;
    constexpr auto distribute = 0x1848FD28;
    constexpr auto visible_offset = 0xA83;
    constexpr auto visible = 0x6A24E30;
    namespace player {
        constexpr auto size = 0x60C8;
        constexpr auto valid = 0xDA4;
        constexpr auto pos_info = 0xC0;
        constexpr auto team_id = 0x11E0;
        constexpr auto stance = 0x410;
        constexpr auto weapon_index = 0x57C;
        constexpr auto dead_1 = 0x74;
        constexpr auto dead_2 = 0x5C4;
    }
    namespace bones {
        constexpr auto bone_base = 0x7FC3C;
        constexpr auto size = 0x150;
    }
}

namespace sdk {


	struct ref_def_view {
		Vector2 tan_half_fov;
		char pad[0xC];
		Vector3 axis[3];
	};
    struct NameEntry {
        uint32_t idx;
        char name[0x24];
        uint8_t unk1[0x64];
        uint32_t health;
    };
	struct ref_def_t {
		int x;
		int y;
		int width;
		int height;
		ref_def_view view;
	};

	extern struct ref_def_t ref_def;

	void set_game_hwnd();

	bool in_game();
    int local_index();
	int player_count();

	class player_t {
	public:
		player_t(uintptr_t address) {
			this->address = address;
		}

		uintptr_t address{};

		bool is_valid();

		bool dead();

		int team_id();

		Vector3 get_pos();

        Vector3 get_bone_base_pos();

        uintptr_t get_bone_ptr();

        uintptr_t get_bone_ptr(const uint64_t bone_base, const uint64_t bone_index);

        Vector3 get_bone_position(const uintptr_t bone_ptr, Vector3 base_pos, const int bone);

        NameEntry get_name_entry(uint32_t index);

        std::vector<std::pair<int, int>> skeleton = {
    {8, 7}, {7, 6}, //Head and neck
    {6, 14}, {14, 15}, {15, 16}, //Right Arm and Hand
    {6, 10}, {10, 11}, {11, 12}, //Left Arm and Hand
    {6, 5}, {5, 4}, {4, 3}, {3, 2}, //Spine
    {2, 17}, {17, 18}, {18, 19}, {19, 20}, //Right Leg and Foot
    {2, 21}, {21, 22}, {22, 23}, {23, 24}, //Left Leg and Foot
        };
	};

	bool w2s(Vector3 world_position, Vector2& screen_position);

	float units_to_m(float units);
}

ImVec2 GetWindowSize() {
    return { (float)GetSystemMetrics(SM_CXSCREEN), (float)GetSystemMetrics(SM_CYSCREEN) };
}

namespace sdk {
	uintptr_t	  process_id = NULL;
	uintptr_t module_base = NULL;
	HWND      hwnd = NULL;
	uintptr_t peb = NULL;
	uintptr_t client_info = NULL;
	uintptr_t client_info_base = NULL;
    uint64_t  names;
	BOOL CALLBACK enum_windows(HWND hwnd, LPARAM param) {
		DWORD process_id;
		GetWindowThreadProcessId(hwnd, &process_id);
		if (process_id == param)
		{
			sdk::hwnd = hwnd;
			return false;
		}
		return true;
	}

    float Calc2D_Distt(const Vector2& Src, const Vector2& Dst) {
        return sqrt(powf(Src.x - Dst.x, 2) + powf(Src.y - Dst.y, 2));
    }

    float AimFov(player_t Entity, Vector3 headPos)
    {
        Vector2 ScreenPos;
        if (!w2s(headPos, ScreenPos)) return 1000.f;
        return Calc2D_Distt(Vector2(GetWindowSize().x / 2, GetWindowSize().y / 2), ScreenPos);
    }

	void set_game_hwnd() {
		EnumWindows(enum_windows, (LPARAM)sdk::process_id);
	}

	bool in_game() {
		return driver.read<int>(sdk::module_base + offsets::game_mode) > 1;
	}

	int player_count() {
		return driver.read<uintptr_t>(sdk::module_base + offsets::game_mode);
	}

	int local_index() {
		auto local_index = driver.read<uintptr_t>(sdk::client_info + offsets::local_index);
		return driver.read<int>(local_index + offsets::local_index_pos);
	}
    uint64_t GetNameList() {
        auto ptr = driver.read<uint64_t>(sdk::module_base + offsets::name_array);
        return ptr + offsets::name_array_list;
    }

    NameEntry player_t::get_name_entry(uint32_t index) {
        return driver.read<NameEntry>(sdk::names + (index * offsets::name_array_size));
    }
	bool player_t::is_valid() {
		return driver.read<bool>(address + offsets::player::valid);
	}

	bool player_t::dead() {
		auto dead1 = driver.read<bool>(address + offsets::player::dead_1);
		auto dead2 = driver.read<bool>(address + offsets::player::dead_2);
		return dead1 || dead2;
	}

	int player_t::team_id() {
		return driver.read<int>(address + offsets::player::team_id);
	}

	Vector3 player_t::get_pos() {
		auto local_pos = driver.read<uintptr_t>(address + offsets::player::pos_info);
		return driver.read<Vector3>(local_pos + 0x40);
	}
    Vector3 player_t::get_bone_base_pos()
    {
        return driver.read<Vector3>(sdk::client_info + offsets::bones::bone_base);
    }

    uintptr_t player_t::get_bone_ptr(const uint64_t bone_base, const uint64_t bone_index)
    {
        return driver.read<uintptr_t>(bone_base + (bone_index * offsets::bones::size) + 0xC0);
    }

    Vector3 player_t::get_bone_position(const uintptr_t bone_ptr, Vector3 base_pos, const int bone)
    {
        return base_pos + driver.read<Vector3>(bone_ptr + ((uint64_t)bone * 0x20) + 0x10);
    }
	Vector3 get_camera_position() {
		auto camera = driver.read<uintptr_t>(sdk::module_base + offsets::camera_base);
		if (!camera)
			return {};

		return driver.read<Vector3>(camera + offsets::camera_pos);
	}

	bool world_to_screen(Vector3 world_location, Vector2& out, Vector3 camera_pos, int screen_width, int screen_height, Vector2 fov, Vector3 matricies[3]) {
		auto local = world_location - camera_pos;
		auto trans = Vector3{
			local.dot(matricies[1]),
			local.dot(matricies[2]),
			local.dot(matricies[0])
		};

		if (trans.z < 0.01f) {
			return false;
		}

		out.x = ((float)screen_width / 2.0) * (1.0 - (trans.x / fov.x / trans.z));
		out.y = ((float)screen_height / 2.0) * (1.0 - (trans.y / fov.y / trans.z));

		if (out.x < 1 || out.y < 1 || (out.x > sdk::ref_def.width) || (out.y > sdk::ref_def.height)) {
			return false;
		}

		return true;
	}

	bool w2s(Vector3 world_position, Vector2& screen_position) {
		return world_to_screen(world_position, screen_position, get_camera_position(), ref_def.width, ref_def.height, ref_def.view.tan_half_fov, ref_def.view.axis);
	}

	float units_to_m(float units) {
		return units * 0.0254;
	}

	ref_def_t ref_def;
}


namespace decryption {
    uint64_t get_client_info()
    {
        uint64_t rax = sdk::module_base, rbx = sdk::module_base, rcx = sdk::module_base, rdx = sdk::module_base, rdi = sdk::module_base, rsi = sdk::module_base, r8 = sdk::module_base, r9 = sdk::module_base, r10 = sdk::module_base, r11 = sdk::module_base, r12 = sdk::module_base, r13 = sdk::module_base, r14 = sdk::module_base, r15 = sdk::module_base;
        rbx = driver.read<uintptr_t>(sdk::module_base + 0x15D505A8);
        if (!rbx)
            return rbx;
        rcx = ~sdk::peb;              //mov rcx, gs:[rax]
        r8 = sdk::module_base;    rdx = 0x5CBCE291E88DFB;                 //mov rdx, 0x5CBCE291E88DFB
        rax = 0;                //and rax, 0xFFFFFFFFC0000000
        rax = _rotl64(rax, 0x10);               //rol rax, 0x10
        rax ^= driver.read<uintptr_t>(sdk::module_base + 0x7A400FE);             //xor rax, [0x000000000520179F]
        rax = ~rax;             //not rax
        rax = driver.read<uintptr_t>(rax + 0x19);              //mov rax, [rax+0x19]
        rax *= rdx;             //imul rax, rdx
        rbx *= rax;             //imul rbx, rax
        rax = rbx;              //mov rax, rbx
        rax >>= 0x1F;           //shr rax, 0x1F
        rbx ^= rax;             //xor rbx, rax
        rax = rbx;              //mov rax, rbx
        rax >>= 0x3E;           //shr rax, 0x3E
        rbx ^= rax;             //xor rbx, rax
        rax = 0x542BD6CA51DABE6C;               //mov rax, 0x542BD6CA51DABE6C
        rbx -= rcx;             //sub rbx, rcx
        rbx ^= r8;              //xor rbx, r8
        rbx -= rax;             //sub rbx, rax
        return rbx;
    }
    uintptr_t decrypt_client_base(uintptr_t client_info)
    {
        uint64_t rax = sdk::module_base, rbx = sdk::module_base, rcx = sdk::module_base, rdx = sdk::module_base, rdi = sdk::module_base, rsi = sdk::module_base, r8 = sdk::module_base, r9 = sdk::module_base, r10 = sdk::module_base, r11 = sdk::module_base, r12 = sdk::module_base, r13 = sdk::module_base, r14 = sdk::module_base, r15 = sdk::module_base;
        rax = driver.read<uintptr_t>(client_info + 0xaf828);
        if (!rax)
            return rax;
        r11 = ~sdk::peb;              //mov r11, gs:[rcx]
        rcx = r11;              //mov rcx, r11
        rcx >>= 0x16;           //shr rcx, 0x16
        rcx &= 0xF;
        switch (rcx) {
        case 0:
        {
            rsi = sdk::module_base + 0x611A;          //lea rsi, [0xFFFFFFFFFD7C7704]
            r9 = driver.read<uintptr_t>(sdk::module_base + 0x7A4011E);               //mov r9, [0x000000000520169F]
            rax -= r11;             //sub rax, r11
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x3;            //shr rcx, 0x03
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x6;            //shr rcx, 0x06
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0xC;            //shr rcx, 0x0C
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x18;           //shr rcx, 0x18
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x30;           //shr rcx, 0x30
            rax ^= rcx;             //xor rax, rcx
            rcx = 0x3DB445DE68781F5B;               //mov rcx, 0x3DB445DE68781F5B
            rax += rcx;             //add rax, rcx
            rcx = r11;              //mov rcx, r11
            rcx = ~rcx;             //not rcx
            rcx *= rsi;             //imul rcx, rsi
            rax ^= rcx;             //xor rax, rcx
            rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
            rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
            rcx ^= r9;              //xor rcx, r9
            rcx = ~rcx;             //not rcx
            rcx = driver.read<uintptr_t>(rcx + 0x17);              //mov rcx, [rcx+0x17]
            uintptr_t RSP_0xFFFFFFFFFFFFFF90;
            RSP_0xFFFFFFFFFFFFFF90 = 0x8DA8A167C667B423;            //mov rcx, 0x8DA8A167C667B423 : RBP+0xFFFFFFFFFFFFFF90
            rcx *= RSP_0xFFFFFFFFFFFFFF90;          //imul rcx, [rbp-0x70]
            rax *= rcx;             //imul rax, rcx
            rcx = 0x61AE41EBA552A1A3;               //mov rcx, 0x61AE41EBA552A1A3
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0xC;            //shr rcx, 0x0C
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x18;           //shr rcx, 0x18
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x30;           //shr rcx, 0x30
            rax ^= rcx;             //xor rax, rcx
            return rax;
        }
        case 1:
        {
            r10 = driver.read<uintptr_t>(sdk::module_base + 0x7A4011E);              //mov r10, [0x0000000005201219]
            rcx = r11;              //mov rcx, r11
            rcx = ~rcx;             //not rcx
            rcx -= sdk::module_base;          //sub rcx, [rbp-0x60] -- didn't find trace -> use base
            rdx = 0;                //and rdx, 0xFFFFFFFFC0000000
            rcx += 0xFFFFFFFFFFFF5FFA;              //add rcx, 0xFFFFFFFFFFFF5FFA
            rdx = _rotl64(rdx, 0x10);               //rol rdx, 0x10
            rax += rcx;             //add rax, rcx
            rdx ^= r10;             //xor rdx, r10
            rdx = ~rdx;             //not rdx
            rax *= driver.read<uintptr_t>(rdx + 0x17);             //imul rax, [rdx+0x17]
            rcx = r11;              //mov rcx, r11
            uintptr_t RSP_0xFFFFFFFFFFFFFF90;
            RSP_0xFFFFFFFFFFFFFF90 = sdk::module_base + 0x6DEBB1F5;           //lea rcx, [0x000000006B67C298] : RBP+0xFFFFFFFFFFFFFF90
            rcx *= RSP_0xFFFFFFFFFFFFFF90;          //imul rcx, [rbp-0x70]
            rax += rcx;             //add rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x9;            //shr rcx, 0x09
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x12;           //shr rcx, 0x12
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x24;           //shr rcx, 0x24
            rax ^= rcx;             //xor rax, rcx
            rcx = 0x451F6BAA0282EB6C;               //mov rcx, 0x451F6BAA0282EB6C
            rax -= rcx;             //sub rax, rcx
            rax ^= r11;             //xor rax, r11
            rcx = 0xC6F2A2C282C29AA3;               //mov rcx, 0xC6F2A2C282C29AA3
            rax ^= rcx;             //xor rax, rcx
            rcx = 0x679BCEB9F708783;                //mov rcx, 0x679BCEB9F708783
            rax *= rcx;             //imul rax, rcx
            return rax;
        }
        case 2:
        {
            r10 = driver.read<uintptr_t>(sdk::module_base + 0x7A4011E);              //mov r10, [0x0000000005200DBF]
            r15 = sdk::module_base + 0x79348D65;              //lea r15, [0x0000000076B099F3]
            rcx = sdk::module_base;           //lea rcx, [0xFFFFFFFFFD7C0907]
            rax ^= rcx;             //xor rax, rcx
            rcx = 0xF285BB54134E53F;                //mov rcx, 0xF285BB54134E53F
            rax += rcx;             //add rax, rcx
            rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
            rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
            rcx ^= r10;             //xor rcx, r10
            rcx = ~rcx;             //not rcx
            rax *= driver.read<uintptr_t>(rcx + 0x17);             //imul rax, [rcx+0x17]
            rcx = 0xC006E3918707A367;               //mov rcx, 0xC006E3918707A367
            rax *= rcx;             //imul rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x16;           //shr rcx, 0x16
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x2C;           //shr rcx, 0x2C
            rax ^= rcx;             //xor rax, rcx
            rcx = r11;              //mov rcx, r11
            rcx -= sdk::module_base;          //sub rcx, [rbp-0x60] -- didn't find trace -> use base
            rcx += 0xFFFFFFFFB8779B21;              //add rcx, 0xFFFFFFFFB8779B21
            rax += rcx;             //add rax, rcx
            rdx = r11;              //mov rdx, r11
            rdx = ~rdx;             //not rdx
            rcx = r11 + rax * 1;            //lea rcx, [r11+rax*1]
            rdx *= r15;             //imul rdx, r15
            rax = rdx;              //mov rax, rdx
            rax ^= rcx;             //xor rax, rcx
            return rax;
        }
        case 3:
        {
            r10 = driver.read<uintptr_t>(sdk::module_base + 0x7A4011E);              //mov r10, [0x0000000005200897]
            r15 = sdk::module_base + 0x2C3328FF;              //lea r15, [0x0000000029AF3065]
            r14 = sdk::module_base + 0xC041;          //lea r14, [0xFFFFFFFFFD7CC79B]
            rcx = sdk::module_base;           //lea rcx, [0xFFFFFFFFFD7C0370]
            rax -= rcx;             //sub rax, rcx
            rcx = 0x31A47DE4DD3A4EA7;               //mov rcx, 0x31A47DE4DD3A4EA7
            rax *= rcx;             //imul rax, rcx
            rcx = r14;              //mov rcx, r14
            rcx = ~rcx;             //not rcx
            //failed to translate: inc rcx
            rcx += r11;             //add rcx, r11
            rax += rcx;             //add rax, rcx
            rcx = 0xC4012C729D9B50AC;               //mov rcx, 0xC4012C729D9B50AC
            rdx = r11;              //mov rdx, r11
            rax += rcx;             //add rax, rcx
            rdx = ~rdx;             //not rdx
            rdx *= r15;             //imul rdx, r15
            rax += rdx;             //add rax, rdx
            rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
            rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
            rcx ^= r10;             //xor rcx, r10
            rcx = ~rcx;             //not rcx
            rax *= driver.read<uintptr_t>(rcx + 0x17);             //imul rax, [rcx+0x17]
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x1C;           //shr rcx, 0x1C
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x38;           //shr rcx, 0x38
            rax ^= rcx;             //xor rax, rcx
            rcx = sdk::module_base;           //lea rcx, [0xFFFFFFFFFD7C067D]
            rax += rcx;             //add rax, rcx
            return rax;
        }
        case 4:
        {
            r10 = sdk::module_base + 0x7DBF6CE1;              //lea r10, [0x000000007B3B6F24]
            rdx = driver.read<uintptr_t>(sdk::module_base + 0x7A4011E);              //mov rdx, [0x0000000005200322]
            rcx = 0x55488A75B6B44AA2;               //mov rcx, 0x55488A75B6B44AA2
            rax += rcx;             //add rax, rcx
            rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
            rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
            rcx ^= rdx;             //xor rcx, rdx
            rcx = ~rcx;             //not rcx
            rax *= driver.read<uintptr_t>(rcx + 0x17);             //imul rax, [rcx+0x17]
            rax += r11;             //add rax, r11
            rcx = 0xE9CBD9DE1175317D;               //mov rcx, 0xE9CBD9DE1175317D
            rax *= rcx;             //imul rax, rcx
            rcx = 0xB33D78467BCB9201;               //mov rcx, 0xB33D78467BCB9201
            rax *= rcx;             //imul rax, rcx
            rcx = r11;              //mov rcx, r11
            rcx = ~rcx;             //not rcx
            rcx -= sdk::module_base;          //sub rcx, [rbp-0x60] -- didn't find trace -> use base
            rcx += 0xFFFFFFFFFFFF38ED;              //add rcx, 0xFFFFFFFFFFFF38ED
            rax += rcx;             //add rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0xD;            //shr rcx, 0x0D
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x1A;           //shr rcx, 0x1A
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x34;           //shr rcx, 0x34
            rax ^= rcx;             //xor rax, rcx
            rcx = r11 + 0x1;                //lea rcx, [r11+0x01]
            rcx *= r10;             //imul rcx, r10
            rax += rcx;             //add rax, rcx
            return rax;
        }
        case 5:
        {
            r14 = sdk::module_base + 0x4BC3;          //lea r14, [0xFFFFFFFFFD7C4A15]
            r9 = driver.read<uintptr_t>(sdk::module_base + 0x7A4011E);               //mov r9, [0x00000000051FFEF8]
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x23;           //shr rcx, 0x23
            rax ^= rcx;             //xor rax, rcx
            rcx = 0x6A01899C356E653;                //mov rcx, 0x6A01899C356E653
            rax *= rcx;             //imul rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x6;            //shr rcx, 0x06
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0xC;            //shr rcx, 0x0C
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x18;           //shr rcx, 0x18
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x30;           //shr rcx, 0x30
            rax ^= rcx;             //xor rax, rcx
            rcx = r14;              //mov rcx, r14
            rcx = ~rcx;             //not rcx
            //failed to translate: inc rcx
            rcx += r11;             //add rcx, r11
            rax += rcx;             //add rax, rcx
            rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
            rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
            rcx ^= r9;              //xor rcx, r9
            rcx = ~rcx;             //not rcx
            rax *= driver.read<uintptr_t>(rcx + 0x17);             //imul rax, [rcx+0x17]
            rcx = 0x28DB461A52351F2;                //mov rcx, 0x28DB461A52351F2
            rax += rcx;             //add rax, rcx
            rax ^= r11;             //xor rax, r11
            rcx = 0x6203CA32CC3B048C;               //mov rcx, 0x6203CA32CC3B048C
            rax -= rcx;             //sub rax, rcx
            return rax;
        }
        case 6:
        {
            r9 = driver.read<uintptr_t>(sdk::module_base + 0x7A4011E);               //mov r9, [0x00000000051FF9B1]
            rcx = sdk::module_base;           //lea rcx, [0xFFFFFFFFFD7BF7AE]
            rax += rcx;             //add rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x8;            //shr rcx, 0x08
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x10;           //shr rcx, 0x10
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x20;           //shr rcx, 0x20
            rax ^= rcx;             //xor rax, rcx
            rcx = sdk::module_base;           //lea rcx, [0xFFFFFFFFFD7BF761]
            rax -= rcx;             //sub rax, rcx
            rax ^= rcx;             //xor rax, rcx
            rcx = 0x2BEA0D4394AD8F9B;               //mov rcx, 0x2BEA0D4394AD8F9B
            rax -= rcx;             //sub rax, rcx
            rcx = 0xEFAA259EF38CC7F5;               //mov rcx, 0xEFAA259EF38CC7F5
            rax ^= rcx;             //xor rax, rcx
            rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
            rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
            rcx ^= r9;              //xor rcx, r9
            rcx = ~rcx;             //not rcx
            rax *= driver.read<uintptr_t>(rcx + 0x17);             //imul rax, [rcx+0x17]
            rcx = 0x4DFFDCB941B942F1;               //mov rcx, 0x4DFFDCB941B942F1
            rax *= rcx;             //imul rax, rcx
            return rax;
        }
        case 7:
        {
            r9 = driver.read<uintptr_t>(sdk::module_base + 0x7A4011E);               //mov r9, [0x00000000051FF584]
            rcx = sdk::module_base;           //lea rcx, [0xFFFFFFFFFD7BF26D]
            rax ^= rcx;             //xor rax, rcx
            rcx = 0x9914AD361FB5060D;               //mov rcx, 0x9914AD361FB5060D
            rax ^= rcx;             //xor rax, rcx
            rcx = sdk::module_base + 0x7146;          //lea rcx, [0xFFFFFFFFFD7C6124]
            rcx = ~rcx;             //not rcx
            rax += rcx;             //add rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x9;            //shr rcx, 0x09
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x12;           //shr rcx, 0x12
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x24;           //shr rcx, 0x24
            rax ^= rcx;             //xor rax, rcx
            rcx = 0xDA1A220E6835BAE3;               //mov rcx, 0xDA1A220E6835BAE3
            rax *= rcx;             //imul rax, rcx
            rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
            rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
            rcx ^= r9;              //xor rcx, r9
            rcx = ~rcx;             //not rcx
            rax *= driver.read<uintptr_t>(rcx + 0x17);             //imul rax, [rcx+0x17]
            rcx = 0xC77C627EA8F68261;               //mov rcx, 0xC77C627EA8F68261
            rax *= rcx;             //imul rax, rcx
            return rax;
        }
        case 8:
        {
            rsi = sdk::module_base + 0x9888;          //lea rsi, [0xFFFFFFFFFD7C87F4]
            rdx = driver.read<uintptr_t>(sdk::module_base + 0x7A4011E);              //mov rdx, [0x00000000051FF049]
            rcx = sdk::module_base;           //lea rcx, [0xFFFFFFFFFD7BEE4D]
            rcx += 0x6A19F9A9;              //add rcx, 0x6A19F9A9
            rcx += r11;             //add rcx, r11
            rax += rcx;             //add rax, rcx
            rcx = 0xB6B85D0E0C3F78E3;               //mov rcx, 0xB6B85D0E0C3F78E3
            rax *= rcx;             //imul rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0xE;            //shr rcx, 0x0E
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x1C;           //shr rcx, 0x1C
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x38;           //shr rcx, 0x38
            rax ^= rcx;             //xor rax, rcx
            rcx = r11;              //mov rcx, r11
            rcx = ~rcx;             //not rcx
            rcx += rsi;             //add rcx, rsi
            rax ^= rcx;             //xor rax, rcx
            rcx = sdk::module_base;           //lea rcx, [0xFFFFFFFFFD7BEC3C]
            rax += rcx;             //add rax, rcx
            rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
            rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
            rcx ^= rdx;             //xor rcx, rdx
            rcx = ~rcx;             //not rcx
            rax *= driver.read<uintptr_t>(rcx + 0x17);             //imul rax, [rcx+0x17]
            rcx = sdk::module_base;           //lea rcx, [0xFFFFFFFFFD7BEE76]
            rax ^= rcx;             //xor rax, rcx
            rcx = 0x9BDAADB949486900;               //mov rcx, 0x9BDAADB949486900
            rax ^= rcx;             //xor rax, rcx
            return rax;
        }
        case 9:
        {
            r9 = driver.read<uintptr_t>(sdk::module_base + 0x7A4011E);               //mov r9, [0x00000000051FEBDA]
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x2;            //shr rcx, 0x02
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x4;            //shr rcx, 0x04
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x8;            //shr rcx, 0x08
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x10;           //shr rcx, 0x10
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x20;           //shr rcx, 0x20
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x17;           //shr rcx, 0x17
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x2E;           //shr rcx, 0x2E
            rax ^= rcx;             //xor rax, rcx
            rax += r11;             //add rax, r11
            rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
            rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
            rcx ^= r9;              //xor rcx, r9
            rcx = ~rcx;             //not rcx
            rcx = driver.read<uintptr_t>(rcx + 0x17);              //mov rcx, [rcx+0x17]
            uintptr_t RSP_0x58;
            RSP_0x58 = 0xAD42FB2AC27B00E5;          //mov rcx, 0xAD42FB2AC27B00E5 : RSP+0x58
            rcx *= RSP_0x58;                //imul rcx, [rsp+0x58]
            rax *= rcx;             //imul rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x6;            //shr rcx, 0x06
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0xC;            //shr rcx, 0x0C
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x18;           //shr rcx, 0x18
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x30;           //shr rcx, 0x30
            rax ^= rcx;             //xor rax, rcx
            rcx = 0x57700F6BA73BCBB0;               //mov rcx, 0x57700F6BA73BCBB0
            rax -= rcx;             //sub rax, rcx
            rcx = 0x2D84EE3D56AC1E2C;               //mov rcx, 0x2D84EE3D56AC1E2C
            rax -= rcx;             //sub rax, rcx
            return rax;
        }
        case 10:
        {
            rdx = sdk::module_base + 0x3324;          //lea rdx, [0xFFFFFFFFFD7C18B8]
            r15 = sdk::module_base + 0xD5CB;          //lea r15, [0xFFFFFFFFFD7CBB53]
            r10 = driver.read<uintptr_t>(sdk::module_base + 0x7A4011E);              //mov r10, [0x00000000051FE655]
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x10;           //shr rcx, 0x10
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x20;           //shr rcx, 0x20
            rax ^= rcx;             //xor rax, rcx
            rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
            rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
            rcx ^= r10;             //xor rcx, r10
            rcx = ~rcx;             //not rcx
            rax *= driver.read<uintptr_t>(rcx + 0x17);             //imul rax, [rcx+0x17]
            rcx = r15;              //mov rcx, r15
            rcx = ~rcx;             //not rcx
            rcx ^= r11;             //xor rcx, r11
            rax ^= rcx;             //xor rax, rcx
            rcx = 0x6D06EB8C42394D69;               //mov rcx, 0x6D06EB8C42394D69
            rax *= rcx;             //imul rax, rcx
            rax -= r11;             //sub rax, r11
            rcx = rdx;              //mov rcx, rdx
            rcx = ~rcx;             //not rcx
            rcx -= r11;             //sub rcx, r11
            rax ^= rcx;             //xor rax, rcx
            rcx = 0x530CCC512E7A2941;               //mov rcx, 0x530CCC512E7A2941
            rax ^= rcx;             //xor rax, rcx
            return rax;
        }
        case 11:
        {
            r14 = sdk::module_base + 0x7F7442BA;              //lea r14, [0x000000007CF02382]
            r9 = driver.read<uintptr_t>(sdk::module_base + 0x7A4011E);               //mov r9, [0x00000000051FE157]
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x19;           //shr rcx, 0x19
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x32;           //shr rcx, 0x32
            rax ^= rcx;             //xor rax, rcx
            rcx = 0xC7AFAF970F66E74E;               //mov rcx, 0xC7AFAF970F66E74E
            rax ^= rcx;             //xor rax, rcx
            rcx = sdk::module_base;           //lea rcx, [0xFFFFFFFFFD7BDD79]
            rax -= rcx;             //sub rax, rcx
            rcx = r14;              //mov rcx, r14
            rcx = ~rcx;             //not rcx
            rcx ^= r11;             //xor rcx, r11
            rax += rcx;             //add rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x5;            //shr rcx, 0x05
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0xA;            //shr rcx, 0x0A
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x14;           //shr rcx, 0x14
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x28;           //shr rcx, 0x28
            rax ^= rcx;             //xor rax, rcx
            rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
            rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
            rcx ^= r9;              //xor rcx, r9
            rcx = ~rcx;             //not rcx
            rax *= driver.read<uintptr_t>(rcx + 0x17);             //imul rax, [rcx+0x17]
            rcx = 0x473049D438790D02;               //mov rcx, 0x473049D438790D02
            rax -= rcx;             //sub rax, rcx
            rcx = 0x7CF47D39577865A9;               //mov rcx, 0x7CF47D39577865A9
            rax *= rcx;             //imul rax, rcx
            return rax;
        }
        case 12:
        {
            r10 = driver.read<uintptr_t>(sdk::module_base + 0x7A4011E);              //mov r10, [0x00000000051FDBD2]
            r15 = sdk::module_base + 0x25600E5E;              //lea r15, [0x0000000022DBE8FF]
            rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
            rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
            rcx ^= r10;             //xor rcx, r10
            rcx = ~rcx;             //not rcx
            rax *= driver.read<uintptr_t>(rcx + 0x17);             //imul rax, [rcx+0x17]
            rax ^= r11;             //xor rax, r11
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x2;            //shr rcx, 0x02
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x4;            //shr rcx, 0x04
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x8;            //shr rcx, 0x08
            rdx = r11;              //mov rdx, r11
            rax ^= rcx;             //xor rax, rcx
            rdx = ~rdx;             //not rdx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x10;           //shr rcx, 0x10
            rax ^= rcx;             //xor rax, rcx
            rcx = r15;              //mov rcx, r15
            rcx = ~rcx;             //not rcx
            rdx *= rcx;             //imul rdx, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x20;           //shr rcx, 0x20
            rdx ^= rcx;             //xor rdx, rcx
            rax ^= rdx;             //xor rax, rdx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0xB;            //shr rcx, 0x0B
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x16;           //shr rcx, 0x16
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x2C;           //shr rcx, 0x2C
            rax ^= rcx;             //xor rax, rcx
            rcx = 0x7FD40EA6533CC015;               //mov rcx, 0x7FD40EA6533CC015
            rax *= rcx;             //imul rax, rcx
            rcx = sdk::module_base + 0x41FE;          //lea rcx, [0xFFFFFFFFFD7C1796]
            rcx -= r11;             //sub rcx, r11
            rax += rcx;             //add rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x24;           //shr rcx, 0x24
            rax ^= rcx;             //xor rax, rcx
            return rax;
        }
        case 13:
        {
            r9 = driver.read<uintptr_t>(sdk::module_base + 0x7A4011E);               //mov r9, [0x00000000051FD5A7]
            rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
            rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
            rcx ^= r9;              //xor rcx, r9
            rcx = ~rcx;             //not rcx
            rax *= driver.read<uintptr_t>(rcx + 0x17);             //imul rax, [rcx+0x17]
            rcx = sdk::module_base;           //lea rcx, [0xFFFFFFFFFD7BD2CF]
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x18;           //shr rcx, 0x18
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x30;           //shr rcx, 0x30
            rax ^= rcx;             //xor rax, rcx
            rcx = 0xFAA4FCAD55E16792;               //mov rcx, 0xFAA4FCAD55E16792
            rax ^= rcx;             //xor rax, rcx
            rcx = 0xD781FCC75CD8752;                //mov rcx, 0xD781FCC75CD8752
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x23;           //shr rcx, 0x23
            rax ^= rcx;             //xor rax, rcx
            rcx = 0xA1626019899A2C5;                //mov rcx, 0xA1626019899A2C5
            rax *= rcx;             //imul rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x16;           //shr rcx, 0x16
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x2C;           //shr rcx, 0x2C
            rax ^= rcx;             //xor rax, rcx
            return rax;
        }
        case 14:
        {
            r14 = sdk::module_base + 0x1776;          //lea r14, [0xFFFFFFFFFD7BE6DB]
            r9 = driver.read<uintptr_t>(sdk::module_base + 0x7A4011E);               //mov r9, [0x00000000051FD006]
            rcx = r11;              //mov rcx, r11
            rcx ^= r14;             //xor rcx, r14
            rax += rcx;             //add rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x10;           //shr rcx, 0x10
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x20;           //shr rcx, 0x20
            rax ^= rcx;             //xor rax, rcx
            rcx = 0x4C32E3A41DFFCAA6;               //mov rcx, 0x4C32E3A41DFFCAA6
            rax -= rcx;             //sub rax, rcx
            rcx = sdk::module_base;           //lea rcx, [0xFFFFFFFFFD7BCA7F]
            rcx += 0x406955B3;              //add rcx, 0x406955B3
            rcx += r11;             //add rcx, r11
            rax ^= rcx;             //xor rax, rcx
            rcx = 0xE0F94B0A796AB2D7;               //mov rcx, 0xE0F94B0A796AB2D7
            rax *= rcx;             //imul rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x1;            //shr rcx, 0x01
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x2;            //shr rcx, 0x02
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x4;            //shr rcx, 0x04
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x8;            //shr rcx, 0x08
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x10;           //shr rcx, 0x10
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x20;           //shr rcx, 0x20
            rax ^= rcx;             //xor rax, rcx
            rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
            rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
            rcx ^= r9;              //xor rcx, r9
            rcx = ~rcx;             //not rcx
            rax *= driver.read<uintptr_t>(rcx + 0x17);             //imul rax, [rcx+0x17]
            rcx = 0x2F6C44980B1B5EAA;               //mov rcx, 0x2F6C44980B1B5EAA
            rax ^= rcx;             //xor rax, rcx
            return rax;
        }
        case 15:
        {
            r9 = driver.read<uintptr_t>(sdk::module_base + 0x7A4011E);               //mov r9, [0x00000000051FCA12]
            rax -= sdk::module_base;          //sub rax, [rbp-0x60] -- didn't find trace -> use base
            rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
            rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
            rcx ^= r9;              //xor rcx, r9
            rcx = ~rcx;             //not rcx
            rax *= driver.read<uintptr_t>(rcx + 0x17);             //imul rax, [rcx+0x17]
            rcx = r11;              //mov rcx, r11
            rcx = ~rcx;             //not rcx
            rcx -= r11;             //sub rcx, r11
            rax += rcx;             //add rax, rcx
            rcx = sdk::module_base + 0xA01B;          //lea rcx, [0xFFFFFFFFFD7C65FE]
            rax += rcx;             //add rax, rcx
            rcx = 0x81DAC3EC3E7F45D7;               //mov rcx, 0x81DAC3EC3E7F45D7
            rax *= rcx;             //imul rax, rcx
            rcx = 0x91F89AB5B5B95065;               //mov rcx, 0x91F89AB5B5B95065
            rax *= rcx;             //imul rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x1E;           //shr rcx, 0x1E
            rax ^= rcx;             //xor rax, rcx
            rcx = rax;              //mov rcx, rax
            rcx >>= 0x3C;           //shr rcx, 0x3C
            rax ^= rcx;             //xor rax, rcx
            rcx = 0xF93AE21CD738317F;               //mov rcx, 0xF93AE21CD738317F
            rax *= rcx;             //imul rax, rcx
            return rax;
        }
        }
    }
    uintptr_t decrypt_bone_base(uintptr_t clientInfo)
    {
        uint64_t rax = sdk::module_base, rbx = sdk::module_base, rcx = sdk::module_base, rdx = sdk::module_base, rdi = sdk::module_base, rsi = sdk::module_base, r8 = sdk::module_base, r9 = sdk::module_base, r10 = sdk::module_base, r11 = sdk::module_base, r12 = sdk::module_base, r13 = sdk::module_base, r14 = sdk::module_base, r15 = sdk::module_base;
        r8 = driver.read<uintptr_t>(sdk::module_base + 0x137D7728);
        if (!r8)
            return r8;
        rbx = sdk::peb;              //mov rbx, gs:[rax]
        rax = rbx;              //mov rax, rbx
        rax >>= 0x1C;           //shr rax, 0x1C
        rax &= 0xF;
 
        switch (rax) {
        case 0:
        {
            r10 = driver.read<uintptr_t>(sdk::module_base + 0x7A40239);              //mov r10, [0x0000000005082E5B]
            rax = 0x13942CF9FF13C63E;               //mov rax, 0x13942CF9FF13C63E
            r8 += rax;              //add r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x1F;           //shr rax, 0x1F
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x3E;           //shr rax, 0x3E
            r8 ^= rax;              //xor r8, rax
            rax = sdk::module_base;           //lea rax, [0xFFFFFFFFFD642A47]
            r8 -= rax;              //sub r8, rax
            rax = rbx;              //mov rax, rbx
            rax -= sdk::module_base;          //sub rax, [rbp-0x48] -- didn't find trace -> use base
            rax += 0xFFFFFFFFDE9C1EB7;              //add rax, 0xFFFFFFFFDE9C1EB7
            r8 += rax;              //add r8, rax
            r8 += rbx;              //add r8, rbx
            r8 += rbx;              //add r8, rbx
            rax = 0;                //and rax, 0xFFFFFFFFC0000000
            rax = _rotl64(rax, 0x10);               //rol rax, 0x10
            rax ^= r10;             //xor rax, r10
            rax = _byteswap_uint64(rax);            //bswap rax
            r8 *= driver.read<uintptr_t>(rax + 0x17);              //imul r8, [rax+0x17]
            rax = 0x26171BFA3B25E823;               //mov rax, 0x26171BFA3B25E823
            r8 *= rax;              //imul r8, rax
            return r8;
        }
        case 1:
        {
            r11 = driver.read<uintptr_t>(sdk::module_base + 0x7A40239);              //mov r11, [0x0000000005082A1D]
            rcx = rbx;              //mov rcx, rbx
            rcx = ~rcx;             //not rcx
            rax = sdk::module_base + 0x57503038;              //lea rax, [0x0000000054B455EC]
            rax = ~rax;             //not rax
            rcx += rax;             //add rcx, rax
            rax = r8;               //mov rax, r8
            r8 = 0x76A9697059939A5;                 //mov r8, 0x76A9697059939A5
            rax *= r8;              //imul rax, r8
            r8 = rcx;               //mov r8, rcx
            r8 ^= rax;              //xor r8, rax
            r8 += rbx;              //add r8, rbx
            rax = 0xC0927C254ABA8319;               //mov rax, 0xC0927C254ABA8319
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x1C;           //shr rax, 0x1C
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x38;           //shr rax, 0x38
            r8 ^= rax;              //xor r8, rax
            rax = sdk::module_base + 0x559A0EB8;              //lea rax, [0x0000000052FE358C]
            r8 += rbx;              //add r8, rbx
            r8 += rax;              //add r8, rax
            rax = 0;                //and rax, 0xFFFFFFFFC0000000
            rax = _rotl64(rax, 0x10);               //rol rax, 0x10
            rax ^= r11;             //xor rax, r11
            rax = _byteswap_uint64(rax);            //bswap rax
            r8 *= driver.read<uintptr_t>(rax + 0x17);              //imul r8, [rax+0x17]
            r8 ^= rbx;              //xor r8, rbx
            return r8;
        }
        case 2:
        {
            r10 = driver.read<uintptr_t>(sdk::module_base + 0x7A40239);              //mov r10, [0x000000000508261B]
            rax = rbx;              //mov rax, rbx
            uintptr_t RSP_0x70;
            RSP_0x70 = sdk::module_base + 0x4246688A;                 //lea rax, [0x000000003FAA8C65] : RSP+0x70
            rax *= RSP_0x70;                //imul rax, [rsp+0x70]
            r8 += rax;              //add r8, rax
            rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
            rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
            rax = r8;               //mov rax, r8
            rcx ^= r10;             //xor rcx, r10
            rax >>= 0x18;           //shr rax, 0x18
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x30;           //shr rax, 0x30
            rcx = _byteswap_uint64(rcx);            //bswap rcx
            r8 ^= rax;              //xor r8, rax
            r8 *= driver.read<uintptr_t>(rcx + 0x17);              //imul r8, [rcx+0x17]
            r8 ^= rbx;              //xor r8, rbx
            rax = sdk::module_base + 0x7B37;          //lea rax, [0xFFFFFFFFFD649B3F]
            r8 ^= rax;              //xor r8, rax
            rax = 0xDDFAE13CC7E5B8F8;               //mov rax, 0xDDFAE13CC7E5B8F8
            r8 ^= rax;              //xor r8, rax
            rax = rbx;              //mov rax, rbx
            rax -= sdk::module_base;          //sub rax, [rbp-0x48] -- didn't find trace -> use base
            rax += 0xFFFFFFFFDF32CF51;              //add rax, 0xFFFFFFFFDF32CF51
            r8 += rax;              //add r8, rax
            rax = 0x26DDFCB1C2D5F319;               //mov rax, 0x26DDFCB1C2D5F319
            r8 *= rax;              //imul r8, rax
            rax = 0x6CBEB92912B03E62;               //mov rax, 0x6CBEB92912B03E62
            r8 -= rax;              //sub r8, rax
            return r8;
        }
        case 3:
        {
            r10 = driver.read<uintptr_t>(sdk::module_base + 0x7A40239);              //mov r10, [0x0000000005082159]
            r8 += rbx;              //add r8, rbx
            rax = 0x34C691FA8758ED5D;               //mov rax, 0x34C691FA8758ED5D
            r8 *= rax;              //imul r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x1D;           //shr rax, 0x1D
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x3A;           //shr rax, 0x3A
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
            rax >>= 0x21;           //shr rax, 0x21
            rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
            r8 ^= rax;              //xor r8, rax
            rcx ^= r10;             //xor rcx, r10
            rcx = _byteswap_uint64(rcx);            //bswap rcx
            r8 *= driver.read<uintptr_t>(rcx + 0x17);              //imul r8, [rcx+0x17]
            rax = 0x591CFE7015A1AD15;               //mov rax, 0x591CFE7015A1AD15
            r8 *= rax;              //imul r8, rax
            rax = 0x384742C852C21CB3;               //mov rax, 0x384742C852C21CB3
            r8 *= rax;              //imul r8, rax
            rax = sdk::module_base;           //lea rax, [0xFFFFFFFFFD641A85]
            r8 -= rbx;              //sub r8, rbx
            r8 -= rax;              //sub r8, rax
            r8 -= 0x63418A26;               //sub r8, 0x63418A26
            return r8;
        }
        case 4:
        {
            r10 = driver.read<uintptr_t>(sdk::module_base + 0x7A40239);              //mov r10, [0x0000000005081C61]
            rax = 0x66145219223D15E5;               //mov rax, 0x66145219223D15E5
            r8 *= rax;              //imul r8, rax
            rax = 0x7453294A9847937D;               //mov rax, 0x7453294A9847937D
            r8 *= rax;              //imul r8, rax
            rax = sdk::module_base;           //lea rax, [0xFFFFFFFFFD641620]
            r8 += rax;              //add r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x11;           //shr rax, 0x11
            r8 ^= rax;              //xor r8, rax
            rcx = sdk::module_base;           //lea rcx, [0xFFFFFFFFFD6418D7]
            rcx += 0x57FB;          //add rcx, 0x57FB
            rcx += rbx;             //add rcx, rbx
            rax = r8;               //mov rax, r8
            rax >>= 0x22;           //shr rax, 0x22
            rcx ^= rax;             //xor rcx, rax
            rax = 0x1B925309E5F0E029;               //mov rax, 0x1B925309E5F0E029
            r8 ^= rcx;              //xor r8, rcx
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x6;            //shr rax, 0x06
            rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
            r8 ^= rax;              //xor r8, rax
            rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
            rax = r8;               //mov rax, r8
            rcx ^= r10;             //xor rcx, r10
            rax >>= 0xC;            //shr rax, 0x0C
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x18;           //shr rax, 0x18
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x30;           //shr rax, 0x30
            r8 ^= rax;              //xor r8, rax
            rcx = _byteswap_uint64(rcx);            //bswap rcx
            r8 *= driver.read<uintptr_t>(rcx + 0x17);              //imul r8, [rcx+0x17]
            return r8;
        }
        case 5:
        {
            //failed to translate: pop rbx
            r10 = driver.read<uintptr_t>(sdk::module_base + 0x7A40239);              //mov r10, [0x00000000050817C0]
            rcx = 0;                //and rcx, 0xFFFFFFFFC0000000
            rcx = _rotl64(rcx, 0x10);               //rol rcx, 0x10
            rax = r8;               //mov rax, r8
            rcx ^= r10;             //xor rcx, r10
            rax >>= 0x24;           //shr rax, 0x24
            rcx = _byteswap_uint64(rcx);            //bswap rcx
            r8 ^= rax;              //xor r8, rax
            r8 *= driver.read<uintptr_t>(rcx + 0x17);              //imul r8, [rcx+0x17]
            rax = 0xF5260BCF8BEA5897;               //mov rax, 0xF5260BCF8BEA5897
            r8 *= rax;              //imul r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x11;           //shr rax, 0x11
            r8 ^= rax;              //xor r8, rax
            rcx = r8;               //mov rcx, r8
            rcx >>= 0x22;           //shr rcx, 0x22
            rcx ^= r8;              //xor rcx, r8
            r8 = sdk::module_base + 0x241B;           //lea r8, [0xFFFFFFFFFD6438A3]
            rax = rbx;              //mov rax, rbx
            rax = ~rax;             //not rax
            rax ^= r8;              //xor rax, r8
            r8 = rcx;               //mov r8, rcx
            r8 -= rax;              //sub r8, rax
            rax = sdk::module_base;           //lea rax, [0xFFFFFFFFFD64122A]
            r8 -= rax;              //sub r8, rax
            rax = 0xC8AA1D0AD9BE163B;               //mov rax, 0xC8AA1D0AD9BE163B
            r8 ^= rax;              //xor r8, rax
            rax = 0x63030BA128B27323;               //mov rax, 0x63030BA128B27323
            r8 *= rax;              //imul r8, rax
            return r8;
        }
        case 6:
        {
            r9 = driver.read<uintptr_t>(sdk::module_base + 0x7A40239);               //mov r9, [0x000000000508133B]
            rax = 0;                //and rax, 0xFFFFFFFFC0000000
            rax = _rotl64(rax, 0x10);               //rol rax, 0x10
            rax ^= r9;              //xor rax, r9
            rax = _byteswap_uint64(rax);            //bswap rax
            r8 *= driver.read<uintptr_t>(rax + 0x17);              //imul r8, [rax+0x17]
            rax = sdk::module_base;           //lea rax, [0xFFFFFFFFFD640EDC]
            r8 ^= rax;              //xor r8, rax
            rax = 0x7E4824E63610EBB5;               //mov rax, 0x7E4824E63610EBB5
            r8 *= rax;              //imul r8, rax
            rax = 0xAB05821D08D6E62D;               //mov rax, 0xAB05821D08D6E62D
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x1F;           //shr rax, 0x1F
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x3E;           //shr rax, 0x3E
            r8 ^= rax;              //xor r8, rax
            r8 -= rbx;              //sub r8, rbx
            r8 ^= rbx;              //xor r8, rbx
            rax = 0x88501ED14AC82009;               //mov rax, 0x88501ED14AC82009
            r8 *= rax;              //imul r8, rax
            return r8;
        }
        case 7:
        {
            r9 = driver.read<uintptr_t>(sdk::module_base + 0x7A40239);               //mov r9, [0x0000000005080E06]
            rax = 0x94CA680C68DF87C3;               //mov rax, 0x94CA680C68DF87C3
            rax -= sdk::module_base;          //sub rax, [rbp-0x48] -- didn't find trace -> use base
            r8 += rax;              //add r8, rax
            rax = 0;                //and rax, 0xFFFFFFFFC0000000
            rax = _rotl64(rax, 0x10);               //rol rax, 0x10
            rax ^= r9;              //xor rax, r9
            rax = _byteswap_uint64(rax);            //bswap rax
            rax = driver.read<uintptr_t>(rax + 0x17);              //mov rax, [rax+0x17]
            uintptr_t RSP_0x70;
            RSP_0x70 = 0x3A797DE5A5230E9D;          //mov rax, 0x3A797DE5A5230E9D : RSP+0x70
            rax *= RSP_0x70;                //imul rax, [rsp+0x70]
            r8 *= rax;              //imul r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x9;            //shr rax, 0x09
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x12;           //shr rax, 0x12
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x24;           //shr rax, 0x24
            r8 ^= rax;              //xor r8, rax
            rax = 0xEAC3C15F630A2D3B;               //mov rax, 0xEAC3C15F630A2D3B
            r8 *= rax;              //imul r8, rax
            r8 -= rbx;              //sub r8, rbx
            rax = r8;               //mov rax, r8
            rax >>= 0x21;           //shr rax, 0x21
            r8 ^= rax;              //xor r8, rax
            return r8;
        }
        case 8:
        {
            //failed to translate: pop rbx
            r9 = driver.read<uintptr_t>(sdk::module_base + 0x7A40239);               //mov r9, [0x0000000005080941]
            rax = 0;                //and rax, 0xFFFFFFFFC0000000
            rax = _rotl64(rax, 0x10);               //rol rax, 0x10
            rax ^= r9;              //xor rax, r9
            rax = _byteswap_uint64(rax);            //bswap rax
            r8 *= driver.read<uintptr_t>(rax + 0x17);              //imul r8, [rax+0x17]
            rax = 0xD07A23F9CAC0FD09;               //mov rax, 0xD07A23F9CAC0FD09
            r8 *= rax;              //imul r8, rax
            r8 ^= rbx;              //xor r8, rbx
            rax = r8;               //mov rax, r8
            rax >>= 0x26;           //shr rax, 0x26
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0xA;            //shr rax, 0x0A
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x14;           //shr rax, 0x14
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x28;           //shr rax, 0x28
            r8 ^= rax;              //xor r8, rax
            rax = 0x2C3CD17C7CCA5FDD;               //mov rax, 0x2C3CD17C7CCA5FDD
            r8 *= rax;              //imul r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x17;           //shr rax, 0x17
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x2E;           //shr rax, 0x2E
            r8 ^= rax;              //xor r8, rax
            return r8;
        }
        case 9:
        {
            r10 = driver.read<uintptr_t>(sdk::module_base + 0x7A40239);              //mov r10, [0x000000000508058E]
            rcx = sdk::module_base + 0xA173;          //lea rcx, [0xFFFFFFFFFD64A473]
            rax = 0;                //and rax, 0xFFFFFFFFC0000000
            rax = _rotl64(rax, 0x10);               //rol rax, 0x10
            rax ^= r10;             //xor rax, r10
            rax = _byteswap_uint64(rax);            //bswap rax
            r8 *= driver.read<uintptr_t>(rax + 0x17);              //imul r8, [rax+0x17]
            rax = r8;               //mov rax, r8
            rax >>= 0xF;            //shr rax, 0x0F
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x1E;           //shr rax, 0x1E
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x3C;           //shr rax, 0x3C
            r8 ^= rax;              //xor r8, rax
            rax = 0xE7F42961A48234F;                //mov rax, 0xE7F42961A48234F
            r8 *= rax;              //imul r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0xB;            //shr rax, 0x0B
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x16;           //shr rax, 0x16
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x2C;           //shr rax, 0x2C
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x7;            //shr rax, 0x07
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0xE;            //shr rax, 0x0E
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x1C;           //shr rax, 0x1C
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x38;           //shr rax, 0x38
            r8 ^= rax;              //xor r8, rax
            rax = rbx;              //mov rax, rbx
            rax *= rcx;             //imul rax, rcx
            r8 ^= rax;              //xor r8, rax
            return r8;
        }
        case 10:
        {
            r9 = driver.read<uintptr_t>(sdk::module_base + 0x7A40239);               //mov r9, [0x0000000005080019]
            rax = sdk::module_base;           //lea rax, [0xFFFFFFFFFD63FB83]
            r8 ^= rax;              //xor r8, rax
            r8 -= rax;              //sub r8, rax
            r8 += 0xFFFFFFFF86653900;               //add r8, 0xFFFFFFFF86653900
            r8 += rbx;              //add r8, rbx
            rax = 0;                //and rax, 0xFFFFFFFFC0000000
            rax = _rotl64(rax, 0x10);               //rol rax, 0x10
            rax ^= r9;              //xor rax, r9
            rax = _byteswap_uint64(rax);            //bswap rax
            r8 *= driver.read<uintptr_t>(rax + 0x17);              //imul r8, [rax+0x17]
            rax = r8;               //mov rax, r8
            rax >>= 0x4;            //shr rax, 0x04
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x8;            //shr rax, 0x08
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x10;           //shr rax, 0x10
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x20;           //shr rax, 0x20
            r8 ^= rax;              //xor r8, rax
            rax = 0xD359E307067FA27;                //mov rax, 0xD359E307067FA27
            r8 *= rax;              //imul r8, rax
            rax = 0x8D8167E83F95F63F;               //mov rax, 0x8D8167E83F95F63F
            r8 ^= rax;              //xor r8, rax
            rax = 0xB1695509E1E454CC;               //mov rax, 0xB1695509E1E454CC
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x20;           //shr rax, 0x20
            r8 ^= rax;              //xor r8, rax
            return r8;
        }
        case 11:
        {
            r11 = sdk::module_base + 0x47A8;          //lea r11, [0xFFFFFFFFFD6440EC]
            r9 = driver.read<uintptr_t>(sdk::module_base + 0x7A40239);               //mov r9, [0x000000000507FB55]
            rax = r11;              //mov rax, r11
            rax = ~rax;             //not rax
            rax *= rbx;             //imul rax, rbx
            r8 ^= rax;              //xor r8, rax
            r8 += rbx;              //add r8, rbx
            rax = r8;               //mov rax, r8
            rax >>= 0xD;            //shr rax, 0x0D
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x1A;           //shr rax, 0x1A
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x34;           //shr rax, 0x34
            r8 ^= rax;              //xor r8, rax
            rax = 0x448B6DF0DA709DD5;               //mov rax, 0x448B6DF0DA709DD5
            r8 *= rax;              //imul r8, rax
            r8 += rbx;              //add r8, rbx
            rax = 0;                //and rax, 0xFFFFFFFFC0000000
            rax = _rotl64(rax, 0x10);               //rol rax, 0x10
            rax ^= r9;              //xor rax, r9
            rax = _byteswap_uint64(rax);            //bswap rax
            r8 *= driver.read<uintptr_t>(rax + 0x17);              //imul r8, [rax+0x17]
            rax = 0x563DBB7079CA44E6;               //mov rax, 0x563DBB7079CA44E6
            r8 ^= rax;              //xor r8, rax
            rax = 0xB61D186A8BB33FB6;               //mov rax, 0xB61D186A8BB33FB6
            r8 ^= rax;              //xor r8, rax
            return r8;
        }
        case 12:
        {
            r10 = driver.read<uintptr_t>(sdk::module_base + 0x7A40239);              //mov r10, [0x000000000507F6BE]
            rax = sdk::module_base;           //lea rax, [0xFFFFFFFFFD63EFF8]
            r8 ^= rax;              //xor r8, rax
            rax = 0;                //and rax, 0xFFFFFFFFC0000000
            rax = _rotl64(rax, 0x10);               //rol rax, 0x10
            rax ^= r10;             //xor rax, r10
            rax = _byteswap_uint64(rax);            //bswap rax
            r8 *= driver.read<uintptr_t>(rax + 0x17);              //imul r8, [rax+0x17]
            rax = r8;               //mov rax, r8
            rax >>= 0x1D;           //shr rax, 0x1D
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x3A;           //shr rax, 0x3A
            r8 ^= rax;              //xor r8, rax
            rax = 0xA822620D151961AB;               //mov rax, 0xA822620D151961AB
            r8 *= rax;              //imul r8, rax
            rax = 0xB5079ABD4B1EA64A;               //mov rax, 0xB5079ABD4B1EA64A
            r8 ^= rax;              //xor r8, rax
            rax = sdk::module_base;           //lea rax, [0xFFFFFFFFFD63F33F]
            r8 ^= rax;              //xor r8, rax
            rax = rbx;              //mov rax, rbx
            rax = ~rax;             //not rax
            uintptr_t RSP_0x68;
            RSP_0x68 = sdk::module_base + 0x69AD;             //lea rax, [0xFFFFFFFFFD645E02] : RSP+0x68
            rax ^= RSP_0x68;                //xor rax, [rsp+0x68]
            r8 += rax;              //add r8, rax
            rax = 0x2A1BF6E38A6CF7DB;               //mov rax, 0x2A1BF6E38A6CF7DB
            r8 ^= rax;              //xor r8, rax
            return r8;
        }
        case 13:
        {
            r10 = driver.read<uintptr_t>(sdk::module_base + 0x7A40239);              //mov r10, [0x000000000507F1C4]
            uintptr_t RSP_0xFFFFFFFFFFFFFFA0;
            RSP_0xFFFFFFFFFFFFFFA0 = 0xB6480CBCA81CC155;            //mov rax, 0xB6480CBCA81CC155 : RBP+0xFFFFFFFFFFFFFFA0
            r8 ^= RSP_0xFFFFFFFFFFFFFFA0;           //xor r8, [rbp-0x60]
            r8 -= sdk::module_base;           //sub r8, [rbp-0x48] -- didn't find trace -> use base
            rax = 0;                //and rax, 0xFFFFFFFFC0000000
            rax = _rotl64(rax, 0x10);               //rol rax, 0x10
            rax ^= r10;             //xor rax, r10
            rax = _byteswap_uint64(rax);            //bswap rax
            r8 *= driver.read<uintptr_t>(rax + 0x17);              //imul r8, [rax+0x17]
            rax = sdk::module_base;           //lea rax, [0xFFFFFFFFFD63ED8B]
            r8 += rax;              //add r8, rax
            rax = 0x22222B43E7D53B25;               //mov rax, 0x22222B43E7D53B25
            r8 *= rax;              //imul r8, rax
            rcx = rbx;              //mov rcx, rbx
            rcx = ~rcx;             //not rcx
            rax = sdk::module_base + 0x2C9C;          //lea rax, [0xFFFFFFFFFD64180A]
            rcx *= rax;             //imul rcx, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x25;           //shr rax, 0x25
            rcx ^= rax;             //xor rcx, rax
            r8 ^= rcx;              //xor r8, rcx
            rax = rbx;              //mov rax, rbx
            rax -= sdk::module_base;          //sub rax, [rbp-0x48] -- didn't find trace -> use base
            rax += 0xFFFFFFFFFFFF0E22;              //add rax, 0xFFFFFFFFFFFF0E22
            r8 += rax;              //add r8, rax
            return r8;
        }
        case 14:
        {
            r9 = driver.read<uintptr_t>(sdk::module_base + 0x7A40239);               //mov r9, [0x000000000507ECE0]
            rax = 0x792E3ED8FE82277F;               //mov rax, 0x792E3ED8FE82277F
            r8 -= rax;              //sub r8, rax
            rax = sdk::module_base;           //lea rax, [0xFFFFFFFFFD63EA49]
            r8 += rax;              //add r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0xE;            //shr rax, 0x0E
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x1C;           //shr rax, 0x1C
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x38;           //shr rax, 0x38
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x21;           //shr rax, 0x21
            r8 ^= rax;              //xor r8, rax
            rax = 0;                //and rax, 0xFFFFFFFFC0000000
            rax = _rotl64(rax, 0x10);               //rol rax, 0x10
            rax ^= r9;              //xor rax, r9
            rax = _byteswap_uint64(rax);            //bswap rax
            rax = driver.read<uintptr_t>(rax + 0x17);              //mov rax, [rax+0x17]
            uintptr_t RSP_0x70;
            RSP_0x70 = 0x42D1EA392109E5C9;          //mov rax, 0x42D1EA392109E5C9 : RSP+0x70
            rax *= RSP_0x70;                //imul rax, [rsp+0x70]
            r8 *= rax;              //imul r8, rax
            rax = 0x6574A9E80EFCCB39;               //mov rax, 0x6574A9E80EFCCB39
            r8 *= rax;              //imul r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x20;           //shr rax, 0x20
            r8 ^= rax;              //xor r8, rax
            return r8;
        }
        case 15:
        {
            r9 = driver.read<uintptr_t>(sdk::module_base + 0x7A40239);               //mov r9, [0x000000000507E7F4]
            rax = 0xD3D2F0A3C6351A47;               //mov rax, 0xD3D2F0A3C6351A47
            r8 *= rax;              //imul r8, rax
            rax = 0;                //and rax, 0xFFFFFFFFC0000000
            rax = _rotl64(rax, 0x10);               //rol rax, 0x10
            rax ^= r9;              //xor rax, r9
            rax = _byteswap_uint64(rax);            //bswap rax
            r8 *= driver.read<uintptr_t>(rax + 0x17);              //imul r8, [rax+0x17]
            rax = r8;               //mov rax, r8
            rax >>= 0x19;           //shr rax, 0x19
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x32;           //shr rax, 0x32
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0xA;            //shr rax, 0x0A
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x14;           //shr rax, 0x14
            r8 ^= rax;              //xor r8, rax
            rax = r8;               //mov rax, r8
            rax >>= 0x28;           //shr rax, 0x28
            r8 ^= rax;              //xor r8, rax
            rax = sdk::module_base;           //lea rax, [0xFFFFFFFFFD63E454]
            r8 -= rax;              //sub r8, rax
            rax = 0x7F1C215B9C0D6FC2;               //mov rax, 0x7F1C215B9C0D6FC2
            r8 += rax;              //add r8, rax
            return r8;
        }
        }
    }
    uintptr_t get_bone_index(uint32_t bone_index)
    {
        uint64_t rax = sdk::module_base, rbx = sdk::module_base, rcx = sdk::module_base, rdx = sdk::module_base, rdi = sdk::module_base, rsi = sdk::module_base, r8 = sdk::module_base, r9 = sdk::module_base, r10 = sdk::module_base, r11 = sdk::module_base, r12 = sdk::module_base, r13 = sdk::module_base, r14 = sdk::module_base, r15 = sdk::module_base;
        rbx = bone_index;
        rcx = rbx * 0x13C8;
        rax = 0xAE9DFDB2AAC7C5;                 //mov rax, 0xAE9DFDB2AAC7C5
        r11 = sdk::module_base;           //lea r11, [0xFFFFFFFFFD7D36BD]
        rax = _umul128(rax, rcx, (uintptr_t*)&rdx);             //mul rcx
        r10 = 0xF2068F0D806DAAF9;               //mov r10, 0xF2068F0D806DAAF9
        rdx >>= 0x4;            //shr rdx, 0x04
        rax = rdx * 0x1775;             //imul rax, rdx, 0x1775
        rcx -= rax;             //sub rcx, rax
        rax = 0x8AE81D1DDD2593AF;               //mov rax, 0x8AE81D1DDD2593AF
        r8 = rcx * 0x1775;              //imul r8, rcx, 0x1775
        rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
        rax = r8;               //mov rax, r8
        rax -= rdx;             //sub rax, rdx
        rax >>= 0x1;            //shr rax, 0x01
        rax += rdx;             //add rax, rdx
        rax >>= 0xD;            //shr rax, 0x0D
        rax = rax * 0x297D;             //imul rax, rax, 0x297D
        r8 -= rax;              //sub r8, rax
        rax = 0x97B425ED097B425F;               //mov rax, 0x97B425ED097B425F
        rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
        rax = 0x14CAB88725AF6E75;               //mov rax, 0x14CAB88725AF6E75
        rdx >>= 0x5;            //shr rdx, 0x05
        rcx = rdx * 0x36;               //imul rcx, rdx, 0x36
        rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
        rdx >>= 0x4;            //shr rdx, 0x04
        rcx += rdx;             //add rcx, rdx
        rax = rcx * 0x18A;              //imul rax, rcx, 0x18A
        rcx = r8 * 0x18C;               //imul rcx, r8, 0x18C
        rcx -= rax;             //sub rcx, rax
        rax = driver.read<uint16_t>(rcx + r11 * 1 + 0x7A579D0);                //movzx eax, word ptr [rcx+r11*1+0x7A579D0]
        r8 = rax * 0x13C8;              //imul r8, rax, 0x13C8
        rax = r10;              //mov rax, r10
        rax = _umul128(rax, r8, (uintptr_t*)&rdx);              //mul r8
        rax = r10;              //mov rax, r10
        rdx >>= 0xD;            //shr rdx, 0x0D
        rcx = rdx * 0x21D9;             //imul rcx, rdx, 0x21D9
        r8 -= rcx;              //sub r8, rcx
        r9 = r8 * 0x38F0;               //imul r9, r8, 0x38F0
        rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
        rdx >>= 0xD;            //shr rdx, 0x0D
        rax = rdx * 0x21D9;             //imul rax, rdx, 0x21D9
        r9 -= rax;              //sub r9, rax
        rax = 0x3521CFB2B78C1353;               //mov rax, 0x3521CFB2B78C1353
        rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
        rax = r9;               //mov rax, r9
        rax -= rdx;             //sub rax, rdx
        rax >>= 0x1;            //shr rax, 0x01
        rax += rdx;             //add rax, rdx
        rax >>= 0x6;            //shr rax, 0x06
        rcx = rax * 0x6A;               //imul rcx, rax, 0x6A
        rax = 0xF2B9D6480F2B9D65;               //mov rax, 0xF2B9D6480F2B9D65
        rax = _umul128(rax, r9, (uintptr_t*)&rdx);              //mul r9
        rdx >>= 0x7;            //shr rdx, 0x07
        rcx += rdx;             //add rcx, rdx
        rax = rcx * 0x10E;              //imul rax, rcx, 0x10E
        rcx = r9 * 0x110;               //imul rcx, r9, 0x110
        rcx -= rax;             //sub rcx, rax
        rsi = driver.read<uint16_t>(rcx + r11 * 1 + 0x7A5CCF0);                //movsx esi, word ptr [rcx+r11*1+0x7A5CCF0]
        return rsi;
    }
    struct ref_def_key {
        int ref0;
        int ref1;
        int ref2;
    };

    uintptr_t get_ref_def() {
        ref_def_key crypt = driver.read<ref_def_key>(sdk::module_base + offsets::ref_def_ptr);
        uint64_t baseAddr = sdk::module_base;

        DWORD lower = crypt.ref0 ^ (crypt.ref2 ^ (uint64_t)(baseAddr + offsets::ref_def_ptr)) * ((crypt.ref2 ^ (uint64_t)(baseAddr + offsets::ref_def_ptr)) + 2);
        DWORD upper = crypt.ref1 ^ (crypt.ref2 ^ (uint64_t)(baseAddr + offsets::ref_def_ptr + 0x4)) * ((crypt.ref2 ^ (uint64_t)(baseAddr + offsets::ref_def_ptr + 0x4)) + 2);

        return (uint64_t)upper << 32 | lower;
    }
}

int realkey;
int keystatus;
int aimkey;

bool GetKey(int key)
{
    realkey = key;
    return true;
}
void ChangeKey(void* blank)
{
    keystatus = 1;
    while (true)
    {
        for (int i = 0; i < 0x87; i++)
        {
            if (GetKeyState(i) & 0x8000)
            {
                aimkey = i;
                keystatus = 0;
                return;
            }
        }
    }
}
static const char* keyNames[] = {
    "",
    "Left Mouse",
    "Right Mouse",
    "Cancel",
    "Middle Mouse",
    "Mouse 5",
    "Mouse 4",
    "",
    "Backspace",
    "Tab",
    "",
    "",
    "Clear",
    "Enter",
    "",
    "",
    "Shift",
    "Control",
    "Alt",
    "Pause",
    "Caps",
    "",
    "",
    "",
    "",
    "",
    "",
    "Escape",
    "",
    "",
    "",
    "",
    "Space",
    "Page Up",
    "Page Down",
    "End",
    "Home",
    "Left",
    "Up",
    "Right",
    "Down",
    "",
    "",
    "",
    "Print",
    "Insert",
    "Delete",
    "",
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "",
    "",
    "",
    "",
    "",
    "Numpad 0",
    "Numpad 1",
    "Numpad 2",
    "Numpad 3",
    "Numpad 4",
    "Numpad 5",
    "Numpad 6",
    "Numpad 7",
    "Numpad 8",
    "Numpad 9",
    "Multiply",
    "Add",
    "",
    "Subtract",
    "Decimal",
    "Divide",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "F11",
    "F12",
};
static bool Items_ArrayGetter(void* data, int idx, const char** out_text)
{
    const char* const* items = (const char* const*)data;
    if (out_text)
        *out_text = items[idx];
    return true;
}
void HotkeyButton(int aimkey, void* changekey, int status)
{
    const char* preview_value = NULL;
    if (aimkey >= 0 && aimkey < IM_ARRAYSIZE(keyNames))
        Items_ArrayGetter(keyNames, aimkey, &preview_value);

    std::string aimkeys;
    if (preview_value == NULL)
        aimkeys = skCrypt("Select Key");
    else
        aimkeys = preview_value;

    if (status == 1)
    {
        aimkeys = skCrypt("Press key to bind");
    }
    if (ImGui::Button(aimkeys.c_str(), ImVec2(100, 20)))
    {
        if (status == 0)
        {
            CreateThread(0, 0, (LPTHREAD_START_ROUTINE)changekey, nullptr, 0, nullptr);
        }
    }
}