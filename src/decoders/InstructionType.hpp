//
// Created by aleksandr on 12.10.25.
//
#pragma once

enum InstructionType : char {
    /** Meaningful lower bits */
    BINOP = 0x00,
    CONST = 0x10,
    STRING = 0x11,
    SEXP = 0x12,
    STI = 0x13,
    STA = 0x14,
    JMP = 0x15,
    END = 0x16,
    RET = 0x17,
    DROP = 0x18,
    DUP = 0x19,
    SWAP = 0x1A,
    ELEM = 0x1B,
    /** Meaningful lower bits */
    LD = 0x20,
    /** Meaningful lower bits */
    LDA = 0x30,
    /** Meaningful lower bits */
    ST = 0x40,
    CJMPz = 0x50,
    CJMPnz = 0x51,
    CALL_READ = 0x70,
    CALL_WRITE = 0x71,
    CALL_LENGTH = 0x72,
    CALL_STRING = 0x73,
    CALL_ARRAY = 0x74,
    BEGIN = 0x52,
    CBEGIN = 0x53,
    CLOSURE = 0x54,
    CALLC = 0x55,
    CALL = 0x56,
    TAG = 0x57,
    ARRAY = 0x58,
    FAIL = 0x59,
    LINE = 0x5A,
    /** Meaningful lower bits */
    PATT = 0x60,
};
