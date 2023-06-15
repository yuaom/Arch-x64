#pragma once 

class Disassemble
{
public:

    static void DisassembleCode(uint8_t* code, size_t codeSize, uint64_t startAddress, std::vector<DisassembledInstruction>& instructions)
    {
        if (!instructions.empty())
        {
            return;
        }

        csh handle;
        cs_insn* insn;
        size_t count;

        /* change these to support 32bit */
        if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK)
        {
            return;
        }

        cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
        count = cs_disasm(handle, code, codeSize, startAddress, 0, &insn);

        if (count > 0)
        {
            for (size_t i = 0; i < count; ++i)
            {
                DisassembledInstruction instruction;
                instruction.address = insn[i].address;
                instruction.mnemonic = insn[i].mnemonic;
                instruction.operands = insn[i].op_str;
                
                /* not a good way to determine the end of instrction flow but kinda works */
                if (strcmp(insn[i].mnemonic, "ret") == 0 || strcmp(insn[i].mnemonic, "retn") == 0 || strcmp(insn[i].mnemonic, "jmp") == 0)
                {
                    instruction.is_possible_end = true;
                }
                if (strcmp(insn[i].mnemonic, "int3") == 0)
                {
                    instruction.mnemonic = "alignment";
                    instruction.is_alignment = true;
                }

                instructions.push_back(instruction);
            }

            cs_free(insn, count);
        }
        else
        {
            printf("[!] Failed to disassemble code\n");
        }

        cs_close(&handle);
    }



};