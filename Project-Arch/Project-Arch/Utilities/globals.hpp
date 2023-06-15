#pragma once 

int SetupWindow();

struct TextSection
{
    uint8_t* sectionData;
    uint32_t sectionSize;
    uint64_t startAddress;
    uint64_t endAddress;
    uint64_t virtualSize;
    uint64_t entryPoint;
    uint32_t flags;
    bool readable;
    bool writable;
    bool executable;
};

struct PdataSection
{
    uint8_t* sectionData;
    uint32_t sectionSize;
    uint64_t startAddress;
    uint64_t endAddress;
    uint64_t virtualSize;
    uint64_t entryPoint;
    uint32_t flags;
    bool readable;
    bool writable;
    bool executable;
};

struct DataSection
{
    uint8_t* sectionData;
    uint32_t sectionSize;
    uint64_t startAddress;
    uint64_t endAddress;
    uint64_t virtualSize;
    uint64_t entryPoint;
    uint32_t flags;
    bool readable;
    bool writable;
    bool executable;
};

struct DisassembledInstruction
{
    uint64_t address;
    std::string mnemonic;
    std::string operands;
    bool is_alignment = false;
    bool is_possible_end = false;
};

struct AllSectionInfo
{
    std::vector<TextSection> text;
    std::vector<PdataSection> pdata;
    std::vector<DataSection> data;
};

struct ExtractedStrings
{
    std::vector<std::string> cStrings;
};

struct EntryPoints
{
    uint64_t entry_points;
};

enum class SectionType
{
    Text,
    Pdata,
    Data
    // Add more section types as needed
};


class GlobalData 
{
public:
    std::vector<uint8_t> raw_image = { 0 };

    /* todo: add these too GUI */
    uint32_t alignment = 16;
    uint64_t base = 0x140000000;
    ImVec2 size = { 850, 650 };
};

inline GlobalData data;