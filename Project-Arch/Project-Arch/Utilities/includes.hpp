#pragma once 

/*-----Windows-----*/
#include <Windows.h>
#include <string>
#include <cstdint>
#include <intrin.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <codecvt>
#include <thread>
#include <iostream>
#include <mutex>
#include <map>
#include <unordered_map>
#include <vector>
#include <corecrt_math.h>
#include <valarray>
#include <stdio.h>
#include <inttypes.h>
#include <basetsd.h>
#include <minwindef.h>
#include <fstream>
#include <filesystem>
#include <winternl.h>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <tchar.h>

/*-----Dependencies-----*/
#include <capstone/capstone.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <d3d11.h>

/*-----Utilities-----*/
#include "globals.hpp"
#include "fonts.hpp"
#include "disassemble.hpp"
#include "operations.hpp"

/*-----Interface-----*/
#include "interface.hpp"