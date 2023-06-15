#pragma once 

class Operations
{
public:

    static std::basic_string<TCHAR> OpenFile()
    {
        OPENFILENAME ofn;
        TCHAR szFile[MAX_PATH] = { 0 };
        ZeroMemory(&ofn, sizeof(ofn));

        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFilter = _T("Executable Files (*.exe)\0*.exe\0All Files (*.*)\0*.*\0");
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

        if (GetOpenFileName(&ofn) == TRUE)
        {
            return std::basic_string<TCHAR>(szFile);
        }

        return _T("");
    }

    
    static bool Is64BitExecutable(std::vector<uint8_t>& raw_image)
    {
        IMAGE_DOS_HEADER* dos_header = reinterpret_cast<IMAGE_DOS_HEADER*>(raw_image.data());
        IMAGE_NT_HEADERS64* nt_header = reinterpret_cast<IMAGE_NT_HEADERS64*>(raw_image.data() + dos_header->e_lfanew);

        if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
        {
            return false;
        }

        if (nt_header->Signature != IMAGE_NT_SIGNATURE || nt_header->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64)
        {
            return false;
        }

        return true;
    }

    static bool ReadFileToMemory(const std::wstring& file_path, std::vector<uint8_t>* out_buffer)
    {
        std::ifstream file_ifstream(file_path, std::ios::binary);
        if (!file_ifstream)
        {
            return false;
        }
        out_buffer->assign((std::istreambuf_iterator<char>(file_ifstream)), std::istreambuf_iterator<char>());
        file_ifstream.close();
        return true;
    }

    static bool IsStringValid(const std::string& cString)
    {
        if (cString.size() <= 2)
        {
            return false;
        }

        for (char c : cString)
        {
            if (c < 32 || c > 126)
            {
                return false;
            }
        }
        return true;
    }

    static uint64_t AllEntryPoints(std::vector<uint8_t>& raw_image)
    {
        IMAGE_DOS_HEADER* dos_header = reinterpret_cast<IMAGE_DOS_HEADER*>(raw_image.data());
        if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
        {
            return 0;
        }

        IMAGE_NT_HEADERS64* nt_header = reinterpret_cast<IMAGE_NT_HEADERS64*>(raw_image.data() + dos_header->e_lfanew);
        if (nt_header->Signature != IMAGE_NT_SIGNATURE)
        {
            return 0;
        }

        if (nt_header->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
        {
            return 0;
        }

        uint64_t entry_point_address = nt_header->OptionalHeader.ImageBase + nt_header->OptionalHeader.AddressOfEntryPoint;

        return entry_point_address;
    }


    static ExtractedStrings ExtractStrings(const std::vector<uint8_t>& raw_image)
    {
        ExtractedStrings extractedStrings;

        const char* cBuffer = reinterpret_cast<const char*>(raw_image.data());
        size_t bufferSize = raw_image.size();

        std::string currentCString;
        std::wstring currentUnicodeString;
        std::vector<uint8_t> currentByteString;

        for (size_t i = 0; i < bufferSize; ++i)
        {
            if (cBuffer[i] != '\0')
            {
                if (currentCString.size() < 256)
                {
                    currentCString += cBuffer[i];
                }
            }
            else if (!currentCString.empty() && IsStringValid(currentCString))
            {
                extractedStrings.cStrings.push_back(currentCString);
                currentCString.clear();
            }
            else
            {
                currentCString.clear();
            }
        }

        if (!currentCString.empty() && IsStringValid(currentCString))
        {
            extractedStrings.cStrings.push_back(currentCString);
        }

        return extractedStrings;
    }


    static AllSectionInfo GetAllSectionInfo(std::vector<uint8_t>& raw_image, uint64_t base, uint32_t alignment)
    {
        AllSectionInfo section_info{};

        IMAGE_DOS_HEADER* dos_header = reinterpret_cast<IMAGE_DOS_HEADER*>(raw_image.data());
        IMAGE_NT_HEADERS64* nt_header = reinterpret_cast<IMAGE_NT_HEADERS64*>(raw_image.data() + dos_header->e_lfanew);
        IMAGE_SECTION_HEADER* section_header = reinterpret_cast<IMAGE_SECTION_HEADER*>(reinterpret_cast<uint8_t*>(nt_header) + sizeof(IMAGE_NT_HEADERS64));

        std::map<std::string, SectionType> section_map = {
            { ".text", SectionType::Text },
            { ".pdata", SectionType::Pdata },
            { ".data", SectionType::Data }
            // Add more sections as needed
        };

        for (int i = 0; i < nt_header->FileHeader.NumberOfSections; ++i)
        {
            uint8_t* sectionData = reinterpret_cast<uint8_t*>(raw_image.data() + section_header[i].PointerToRawData);
            uint32_t sectionSize = section_header[i].SizeOfRawData;

            uint64_t section_start_rva = section_header[i].VirtualAddress;
            uint64_t section_end_rva = section_start_rva + section_header[i].Misc.VirtualSize;

            uint64_t aligned_start_address = (section_start_rva / alignment) * alignment;
            uint64_t aligned_end_address = ((section_end_rva - 1) / alignment + 1) * alignment;
            uint64_t virtual_size = aligned_end_address - aligned_start_address;

            uint64_t adjusted_start_address = aligned_start_address + base;
            uint64_t adjusted_end_address = aligned_end_address + base;

            bool readable = (section_header[i].Characteristics & IMAGE_SCN_MEM_READ) != 0;
            bool writable = (section_header[i].Characteristics & IMAGE_SCN_MEM_WRITE) != 0;
            bool executable = (section_header[i].Characteristics & IMAGE_SCN_MEM_EXECUTE) != 0;

            const char* section_name = reinterpret_cast<const char*>(section_header[i].Name);

            auto section_it = section_map.find(section_name);
            if (section_it != section_map.end())
            {
                SectionType section_type = section_it->second;

                switch (section_type)
                {
                    case SectionType::Text:
                    {
                        TextSection text_section;

                        text_section.startAddress = adjusted_start_address;
                        text_section.endAddress = adjusted_end_address;
                        text_section.virtualSize = virtual_size;
                        text_section.flags = section_header[i].Characteristics;

                        text_section.readable = readable;
                        text_section.writable = writable;
                        text_section.executable = executable;

                        text_section.entryPoint = nt_header->OptionalHeader.AddressOfEntryPoint;
                        text_section.sectionData = sectionData;
                        text_section.sectionSize = sectionSize;

                        section_info.text.push_back(text_section);
                        break;
                    }
                    case SectionType::Pdata:
                    {
                        PdataSection pdata_section;

                        pdata_section.startAddress = adjusted_start_address;
                        pdata_section.endAddress = adjusted_end_address;
                        pdata_section.virtualSize = virtual_size;
                        pdata_section.flags = section_header[i].Characteristics;

                        pdata_section.readable = readable;
                        pdata_section.writable = writable;
                        pdata_section.executable = executable;

                        pdata_section.entryPoint = nt_header->OptionalHeader.AddressOfEntryPoint;
                        pdata_section.sectionData = sectionData;
                        pdata_section.sectionSize = sectionSize;

                        section_info.pdata.push_back(pdata_section);
                        break;
                    }
                    case SectionType::Data:
                    {
                        DataSection data_section;

                        data_section.startAddress = adjusted_start_address;
                        data_section.endAddress = adjusted_end_address;
                        data_section.virtualSize = virtual_size;
                        data_section.flags = section_header[i].Characteristics;

                        data_section.readable = readable;
                        data_section.writable = writable;
                        data_section.executable = executable;

                        data_section.entryPoint = nt_header->OptionalHeader.AddressOfEntryPoint;
                        data_section.sectionData = sectionData;
                        data_section.sectionSize = sectionSize;

                        section_info.data.push_back(data_section);
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
        }

        return section_info;
    }


};