#pragma once

using namespace ImGui;

static bool read_file = false;
static bool section_info_retrieved = false;

AllSectionInfo section_info;

void RenderInterface()
{
    static int test = 0;

    Begin("##Arch", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove);

    SetWindowPos(ImVec2(0, 0));
    SetWindowSize(ImVec2(data.size.x, data.size.y));

    if (BeginMenuBar()) 
    {
        if (BeginMenu("File")) 
        {
            if (MenuItem("Open"))
            {
                std::wstring file_path = Operations::OpenFile();
                if (!file_path.empty())
                {
                    if (!Operations::ReadFileToMemory(file_path.c_str(), &data.raw_image))
                    {
                        MessageBox(NULL, L"Failed to read the file", L"Error", MB_OK | MB_ICONERROR);
                        read_file = false;
                        exit(0);
                    }
                    else if (!Operations::Is64BitExecutable(data.raw_image))
                    {
                        MessageBox(NULL, L"The selected file is not a valid x64 executable", L"Error", MB_OK | MB_ICONERROR);
                        read_file = false;
                        exit(0);
                    }
                    else
                    {
                        read_file = true;
                    }
                }

            }
            ImGui::EndMenu();
        }
        EndMenuBar();
    }

    BeginChild("##child", ImVec2(GetWindowWidth() / 4, GetWindowHeight() - GetStyle().WindowPadding.y * 5), 1, ImGuiWindowFlags_NoScrollbar);

    BeginChild("##child_1", ImVec2(GetWindowWidth() - GetStyle().WindowPadding.x * 2, GetWindowHeight() - GetStyle().WindowPadding.y * 2), 1);

    /* todo: some how calculate start and end address of assembly view and turn into seprate "sub_functions" */
    for (int i = 0; i < 50; ++i) {
        if (Selectable(("sub_" + std::to_string(i)).c_str(), test == i)) {
            test = i;
        }
    }

    EndChild();

    EndChild();

    SameLine();

    BeginChild("##main", ImVec2(GetWindowWidth() - (GetWindowWidth() / 4) - GetStyle().WindowPadding.x * 2 - GetStyle().ItemSpacing.x, GetWindowHeight() - GetStyle().WindowPadding.y * 5), 1);

    if (BeginTabBar("##tabs", ImGuiTabBarFlags_None))
    {
        if (BeginTabItem(".text"))
        {
            BeginChild("##output_section_text", ImVec2(GetWindowWidth() - GetStyle().WindowPadding.x * 2, GetWindowHeight() / 4), 1);

            if (read_file && data.raw_image.size() != 0)
            {
                Text("file size: %d bytes", data.raw_image.size());

                section_info = Operations::GetAllSectionInfo(data.raw_image, data.base, data.alignment);

                std::vector<TextSection> text = section_info.text;

                for (const auto& textsection : text)
                {
                    TextWrapped("section start address: 0x%llx\n", textsection.startAddress);
                    TextWrapped("section end address: 0x%llx\n", textsection.endAddress);
                    TextWrapped("section virtual size: 0x%llx (%llu bytes)\n", textsection.virtualSize, textsection.virtualSize);
                    TextWrapped("section flags: 0x%x\n", textsection.flags);
                    TextWrapped("section permissions: %c%c%c\n", textsection.readable ? 'R' : ' ', textsection.writable ? 'W' : ' ', textsection.executable ? 'X' : ' ');
                }
            }
            else
            {
                Text("file size: %d bytes", data.raw_image.size() - 1);
            }

            EndChild();

            BeginChild("##output_instructions", ImVec2(GetWindowWidth() - GetStyle().WindowPadding.x * 2, GetWindowHeight() / 3 * 2), 1);

            std::vector<DisassembledInstruction> instructions;

            instructions.clear();

            for (const auto& textsection : section_info.text)
            {
                std::vector<DisassembledInstruction> sectionInstructions;
                Disassemble::DisassembleCode(textsection.sectionData, textsection.sectionSize, textsection.startAddress, sectionInstructions);
                instructions.insert(instructions.end(), sectionInstructions.begin(), sectionInstructions.end());
            }

            for (const auto& instruction : instructions)
            {
                Text("0x%llx %s %s\n", instruction.address, instruction.mnemonic.c_str(), instruction.operands.c_str());

                if (instruction.is_possible_end)
                {
                    ImGuiStyle& style = ImGui::GetStyle();
                    ImVec4 originalTextColor = style.Colors[ImGuiCol_Text];

                    ImVec4 lighterRed = ImVec4(1.0f, 0.5f, 0.5f, 1.0f);
                    style.Colors[ImGuiCol_Text] = lighterRed;

                    Text("0x%llx possible end", instruction.address);

                    style.Colors[ImGuiCol_Text] = originalTextColor;
                }
            }

            EndChild();

            EndTabItem();
        }

        if (BeginTabItem(".pdata"))
        {
            BeginChild("##output_section_pdata", ImVec2(GetWindowWidth() - GetStyle().WindowPadding.x * 2, GetWindowHeight() / 4), 1);

            if (read_file && data.raw_image.size() != 0)
            {
                Text("file size: %d bytes", data.raw_image.size());

                section_info = Operations::GetAllSectionInfo(data.raw_image, data.base, data.alignment);

                std::vector<PdataSection> pdata = section_info.pdata;

                for (const auto& pdatasection : pdata)
                {
                    TextWrapped("section start address: 0x%llx\n", pdatasection.startAddress);
                    TextWrapped("section end address: 0x%llx\n", pdatasection.endAddress);
                    TextWrapped("section virtual size: 0x%llx (%llu bytes)\n", pdatasection.virtualSize, pdatasection.virtualSize);
                    TextWrapped("section flags: 0x%x\n", pdatasection.flags);
                    TextWrapped("section permissions: %c%c%c\n", pdatasection.readable ? 'R' : ' ', pdatasection.writable ? 'W' : ' ', pdatasection.executable ? 'X' : ' ');
                }
            }
            else
            {
                Text("file size: %d bytes", data.raw_image.size() - 1);
            }

            EndChild();

            BeginChild("##output_instructions", ImVec2(GetWindowWidth() - GetStyle().WindowPadding.x * 2, GetWindowHeight() / 3 * 2), 1);

            std::vector<DisassembledInstruction> instructions;

            instructions.clear();

            for (const auto& pdatasection : section_info.pdata)
            {
                std::vector<DisassembledInstruction> sectionInstructions;
                Disassemble::DisassembleCode(pdatasection.sectionData, pdatasection.sectionSize, pdatasection.startAddress, sectionInstructions);
                instructions.insert(instructions.end(), sectionInstructions.begin(), sectionInstructions.end());
            }

            for (const auto& instruction : instructions)
            {
                Text("0x%llx %s %s\n", instruction.address, instruction.mnemonic.c_str(), instruction.operands.c_str());

                if (instruction.is_possible_end)
                {
                    ImGuiStyle& style = GetStyle();
                    ImVec4 originalTextColor = style.Colors[ImGuiCol_Text];

                    ImVec4 lighterRed = ImVec4(1.0f, 0.5f, 0.5f, 1.0f);
                    style.Colors[ImGuiCol_Text] = lighterRed;

                    Text("0x%llx possible end", instruction.address);

                    style.Colors[ImGuiCol_Text] = originalTextColor;
                }
            }

            EndChild();

            EndTabItem();
        }

        if (BeginTabItem(".data"))
        {
            BeginChild("##output_section_data", ImVec2(GetWindowWidth() - GetStyle().WindowPadding.x * 2, GetWindowHeight() / 4), 1);

            if (read_file && data.raw_image.size() != 0)
            {
                Text("file size: %d bytes", data.raw_image.size());

                section_info = Operations::GetAllSectionInfo(data.raw_image, data.base, data.alignment);

                std::vector<DataSection> data = section_info.data;

                for (const auto& datasection : data)
                {
                    TextWrapped("section start address: 0x%llx\n", datasection.startAddress);
                    TextWrapped("section end address: 0x%llx\n", datasection.endAddress);
                    TextWrapped("section virtual size: 0x%llx (%llu bytes)\n", datasection.virtualSize, datasection.virtualSize);
                    TextWrapped("section flags: 0x%x\n", datasection.flags);
                    TextWrapped("section permissions: %c%c%c\n", datasection.readable ? 'R' : ' ', datasection.writable ? 'W' : ' ', datasection.executable ? 'X' : ' ');
                }
            }
            else
            {
                Text("file size: %d bytes", data.raw_image.size() - 1);
            }

            EndChild();

            EndTabItem();
        }

        if (BeginTabItem("strings"))
        {
            BeginChild("##output_section_strings", ImVec2(GetWindowWidth() - GetStyle().WindowPadding.x * 2, GetWindowHeight() / 1.1), 1);

            const ExtractedStrings& extracted_strings = Operations::ExtractStrings(data.raw_image);

            for (const std::string& cStr : extracted_strings.cStrings)
            {
                TextWrapped("%s", cStr.c_str());
            }

            EndChild();

            EndTabItem();
        }

        if (BeginTabItem("entry point"))
        {
            BeginChild("##output_section_entry", ImVec2(GetWindowWidth() - GetStyle().WindowPadding.x * 2, GetWindowHeight() / 1.1), 1);

            uint64_t entry_points = Operations::AllEntryPoints(data.raw_image);

            TextWrapped("[main] entry point: 0x%p\n", entry_points);

            EndChild();

            EndTabItem();
        }

        EndTabBar();
    }

    EndChild();

    End();
}