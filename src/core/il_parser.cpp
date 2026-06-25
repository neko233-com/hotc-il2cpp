#include "il_parser.h"
#include <fstream>
#include <stdexcept>

namespace hotc {

MethodBody ILParser::Parse(const uint8_t* data, size_t size) {
    MethodBody body;
    body.il_bytes.assign(data, data + size);

    size_t offset = 0;
    while (offset < size) {
        body.instructions.push_back(DecodeInstruction(data, offset));
    }

    return body;
}

MethodBody ILParser::ParseFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + path);
    }

    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);

    return Parse(buffer.data(), size);
}

Instruction ILParser::DecodeInstruction(const uint8_t* data, size_t& offset) {
    Instruction instr;
    instr.offset = static_cast<uint32_t>(offset);

    uint8_t opcode = ReadUInt8(data, offset);
    instr.opcode = static_cast<OpCode>(opcode);

    switch (instr.opcode) {
        case OpCode::Ldarg_S:
        case OpCode::Stloc_S:
        case OpCode::Ldloc_S:
        case OpCode::Ldc_I4_S:
        case OpCode::Br_S:
        case OpCode::Brfalse_S:
        case OpCode::Brtrue_S:
        case OpCode::Beq_S:
        case OpCode::Bge_S:
        case OpCode::Bgt_S:
        case OpCode::Ble_S:
        case OpCode::Blt_S:
        case OpCode::Bne_Un_S:
        case OpCode::Bge_Un_S:
        case OpCode::Bgt_Un_S:
        case OpCode::Ble_Un_S:
        case OpCode::Blt_Un_S:
            instr.operand = ReadUInt8(data, offset);
            break;

        case OpCode::Ldarg:
        case OpCode::Stloc:
        case OpCode::Ldloc:
        case OpCode::Ldc_I4:
        case OpCode::Br:
        case OpCode::Brfalse:
        case OpCode::Brtrue:
        case OpCode::Beq:
        case OpCode::Bge:
        case OpCode::Bgt:
        case OpCode::Ble:
        case OpCode::Blt:
        case OpCode::Bne_Un:
        case OpCode::Bge_Un:
        case OpCode::Bgt_Un:
        case OpCode::Ble_Un:
        case OpCode::Blt_Un:
        case OpCode::Call:
        case OpCode::Calli:
        case OpCode::Newarr:
        case OpCode::Box:
        case OpCode::Ldobj:
        case OpCode::Stobj:
        case OpCode::Ldfld:
        case OpCode::Stfld:
        case OpCode::Ldsfld:
        case OpCode::Stsfld:
        case OpCode::Newobj:
        case OpCode::Castclass:
        case OpCode::Isinst:
        case OpCode::Throw:
            instr.operand = ReadUInt32(data, offset);
            break;

        case OpCode::Ldc_R4:
            instr.operand = ReadUInt32(data, offset);
            break;

        case OpCode::Ldc_R8: {
            uint64_t raw = ReadUInt32(data, offset);
            raw = (raw << 32) | ReadUInt32(data, offset);
            instr.operand = static_cast<uint32_t>(raw);
            break;
        }

        default:
            instr.operand = 0;
            break;
    }

    return instr;
}

uint32_t ILParser::ReadUInt32(const uint8_t* data, size_t& offset) {
    uint32_t value = data[offset] |
                     (data[offset + 1] << 8) |
                     (data[offset + 2] << 16) |
                     (data[offset + 3] << 24);
    offset += 4;
    return value;
}

uint16_t ILParser::ReadUInt16(const uint8_t* data, size_t& offset) {
    uint16_t value = data[offset] | (data[offset + 1] << 8);
    offset += 2;
    return value;
}

uint8_t ILParser::ReadUInt8(const uint8_t* data, size_t& offset) {
    return data[offset++];
}

} // namespace hotc
