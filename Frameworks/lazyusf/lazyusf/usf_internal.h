#ifndef _USF_INTERNAL_H_
#define _USF_INTERNAL_H_

#include "audio.h"
#include "cpu.h"
#include "rsp_hle/hle.h"
#include "cpu_hle.h"

/* Supported rom image types. */
enum
{
    Z64IMAGE,
    V64IMAGE,
    N64IMAGE
};

/* Supported CIC chips. */
enum
{
    CIC_NUS_6101,
    CIC_NUS_6102,
    CIC_NUS_6103,
    CIC_NUS_6105,
    CIC_NUS_6106
};

/* Supported save types. */
enum
{
    EEPROM_4KB,
    EEPROM_16KB,
    SRAM,
    FLASH_RAM,
    CONTROLLER_PACK,
    NONE
};

typedef enum
{
    SYSTEM_NTSC = 0,
    SYSTEM_PAL,
    SYSTEM_MPAL
} _system_type;

typedef struct
{
    unsigned char init_PI_BSB_DOM1_LAT_REG;  /* 0x00 */
    unsigned char init_PI_BSB_DOM1_PGS_REG;  /* 0x01 */
    unsigned char init_PI_BSB_DOM1_PWD_REG;  /* 0x02 */
    unsigned char init_PI_BSB_DOM1_PGS_REG2; /* 0x03 */
    unsigned int ClockRate;                  /* 0x04 */
    unsigned int PC;                         /* 0x08 */
    unsigned int Release;                    /* 0x0C */
    unsigned int CRC1;                       /* 0x10 */
    unsigned int CRC2;                       /* 0x14 */
    unsigned int Unknown[2];                 /* 0x18 */
    unsigned char Name[20];                  /* 0x20 */
    unsigned int unknown;                    /* 0x34 */
    unsigned int Manufacturer_ID;            /* 0x38 */
    unsigned short Cartridge_ID;             /* 0x3C - Game serial number  */
    unsigned short Country_code;             /* 0x3E */
} _rom_header;

typedef struct _rom_params
{
    _system_type systemtype;
    int vilimit;
    int aidacrate;
    char headername[21];  /* ROM Name as in the header, removing trailing whitespace */
    unsigned char countperop;
} rom_params;

struct usf_state_helper
{
    size_t offset_to_structure;
};

#ifndef RCPREG_DEFINED
#define RCPREG_DEFINED
typedef uint32_t RCPREG;
#endif

#ifdef DEBUG_INFO
#include <stdio.h>
#endif

struct usf_state
{
    // RSP vector registers, need to be aligned to 16 bytes
    // when SSE2 or SSSE3 is enabled, or for any hope of
    // auto vectorization

    // usf_clear takes care of aligning the structure within
    // the memory block passed into it, treating the pointer
    // as usf_state_helper, and storing an offset from the
    // pointer to the actual usf_state structure. The size
    // which is indicated for allocation accounts for this
    // with two pages of padding.

    int16_t VR[32][8];
    int16_t VACC[3][8];
    
    // RSP virtual registers, also needs alignment
    int32_t SR[32];
    
    // rsp/rsp.c, not necessarily in need of alignment
    RCPREG* CR[16];
    
    // rsp/vu/cf.h, all need alignment
    int16_t ne[8]; /* $vco:  high byte "NOTEQUAL" */
    int16_t co[8]; /* $vco:  low byte "carry/borrow in/out" */
    int16_t clip[8]; /* $vcc:  high byte (clip tests:  VCL, VCH, VCR) */
    int16_t comp[8]; /* $vcc:  low byte (VEQ, VNE, VLT, VGE, VCL, VCH, VCR) */
    int16_t vce[8]; /* $vce:  vector compare extension register */
    
    // All further members of the structure need not be aligned

    // rsp/vu/divrom.h
    int32_t DivIn; /* buffered numerator of division read from vector file */
    int32_t DivOut; /* global division result set by VRCP/VRCPL/VRSQ/VRSQH */
#if (0)
    int32_t MovIn; /* We do not emulate this register (obsolete, for VMOV). */
#endif
    
    int32_t DPH;
    
    // rsp/rsp.h
    int32_t stage; // unused since EMULATE_STATIC_PC is defined by default in rsp/config.h
    int32_t temp_PC;
    int16_t MFC0_count[32];
    
    // rsp_hle
    struct hle_t hle;
    
    _rom_header   ROM_HEADER;
    rom_params    ROM_PARAMS;

    uint32_t cpu_running, cpu_stopped;
    
    // options from file tags
    uint32_t enablecompare, enableFIFOfull;
    
    // options for decoding
    uint32_t enable_hle_audio;
    
    // buffering for rendered sample data
    size_t sample_buffer_count;
    int16_t * sample_buffer;

    // audio.c
    // SampleRate is usually guaranteed to stay the same for the duration
    // of a given track, and depends on the game.
    int32_t SampleRate;
    // Audio is rendered in whole Audio Interface DMA transfers, which are
    // then copied directly to the caller's buffer. Any left over samples
    // from the last DMA transfer that fills the caller's buffer will be
    // stored here until the next call to usf_render()
    int16_t samplebuf[16384];
    size_t samples_in_buffer;
    
    struct ai_dma fifo[2];

    // usf.c
    // This takes care of automatically resampling the console audio
    // to the user requested sample rate, using the same cubic interpolation
    // coefficients as the RSP HLE, which in turn mimics the original RSP
    // microcode used in most games.
    void * resampler;
    int16_t samplebuf2[8192];
    size_t samples_in_buffer_2;
    
    // This buffer does not really need to be that large, as it is likely
    // to only accumulate a handlful of error messages, at which point
    // emulation is immediately halted and the messages are returned to
    // the caller.
    const char * last_error;
    char error_message[1024];
    
    // cpu.c
    uint32_t NextInstruction, JumpToLocation, AudioIntrReg;
    CPU_ACTION * CPU_Action;
    SYSTEM_TIMERS * Timers;
    OPCODE Opcode;
    uint32_t CPURunning, SPHack;
    uint32_t * WaitMode;
    
    // interpreter_ops.c
    uint32_t SWL_MASK[4], SWR_MASK[4], LWL_MASK[4], LWR_MASK[4];
    int32_t SWL_SHIFT[4], SWR_SHIFT[4], LWL_SHIFT[4], LWR_SHIFT[4];
    int32_t RoundingModel;

    // memory.c
    uintptr_t *TLB_Map;
    uint8_t * MemChunk;
    uint32_t RdramSize, SystemRdramSize, RomFileSize;
    uint8_t * N64MEM, * RDRAM, * DMEM, * IMEM, * ROMPages[0x400], * savestatespace, * NOMEM;
    
    uint32_t WrittenToRom;
    uint32_t WroteToRom;
    uint32_t TempValue;
    uint32_t MemoryState;
    
    uint8_t EmptySpace;
    
    // pif.c
    uint8_t *PIF_Ram;
    
    // registers.c
    uint32_t PROGRAM_COUNTER, * CP0,*FPCR,*RegRDRAM,*RegSP,*RegDPC,*RegMI,*RegVI,*RegAI,*RegPI,
	*RegRI,*RegSI, HalfLine, RegModValue, ViFieldNumber, LLBit, LLAddr;
    void * FPRDoubleLocation[32], * FPRFloatLocation[32];
    MIPS_DWORD *GPR, *FPR, HI, LO;
    int32_t fpuControl;
    N64_REGISTERS * Registers;
    
    // tlb.c
    FASTTLB FastTlb[64];
    TLB tlb[32];
    
    uint32_t OLD_VI_V_SYNC_REG/* = 0*/, VI_INTR_TIME/* = 500000*/;
    
	uint32_t cpu_hle_entry_count;
    _HLE_Entry * cpu_hle_entries;
    
#ifdef DEBUG_INFO
    FILE * debug_log;
#endif
};

#define USF_STATE_HELPER ((usf_state_helper_t *)(state))

#define USF_STATE ((usf_state_t *)(((uint8_t *)(state))+((usf_state_helper_t *)(state))->offset_to_structure))

#endif
