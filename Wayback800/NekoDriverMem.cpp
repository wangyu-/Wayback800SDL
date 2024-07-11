#include "NekoDriver.h"
#include "wintypes.h"
extern "C" {
#ifdef HANDYPSP
#include "ANSI/w65c02.h"
#else
#include "ANSI/65C02.h"
#endif
}
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "CC800IOName.h"
#include "NekoDriverIO.h"
#include "comm.h"


void super_switch();
iofunction1 ioread[0x40]  = {
    Read00BankSwitch,       // $00
    Read01IntStatus,       // $01
    NullRead,       // $02
    NullRead,       // $03
    Read04StopTimer0,     // $04
    Read05StartTimer0,    // $05
    Read06StopTimer1,     // $06
    Read07StartTimer1,    // $07
    ReadPort0,      // $08
    ReadPort1,      // $09
    NullRead,       // $0A
    NullRead,       // $0B
    NullRead,       // $0C
    NullRead,       // $0D
    NullRead,       // $0E
    NullRead,       // $0F
    NullRead,       // $10
    NullRead,       // $11
    NullRead,       // $12
    NullRead,       // $13
    NullRead,       // $14
    NullRead,       // $15
    NullRead,       // $16
    NullRead,       // $17
    Read18Port4,       // $18
    NullRead,       // $19
    NullRead,       // $1A
    NullRead,       // $1B
    NullRead,       // $1C
    NullRead,       // $1D
    NullRead,       // $1E
    NullRead,       // $1F
    NullRead,       // $20
    NullRead,       // $21
    NullRead,       // $22
    NullRead,       // $23
    NullRead,       // $24
    NullRead,       // $25
    NullRead,       // $26
    NullRead,       // $27
    NullRead,       // $28
    NullRead,       // $29
    NullRead,       // $2A
    NullRead,       // $2B
    NullRead,       // $2C
    NullRead,       // $2D
    NullRead,       // $2E
    NullRead,       // $2F
    NullRead,       // $30
    NullRead,       // $31
    NullRead,       // $32
    NullRead,       // $33
    NullRead,       // $34
    NullRead,       // $35
    NullRead,       // $36
    NullRead,       // $37
    NullRead,       // $38
    NullRead,       // $39
    NullRead,       // $3A
    NullRead,       // $3B
    NullRead,       // $3C
    NullRead,       // $3D
    NullRead,       // $3E
    NullRead,       // $3F
};

iofunction2 iowrite[0x40] = {
    Write00BankSwitch,          // $00
    Write01IntEnable,      // $01
    NullWrite,          // $02 Timer0不需特殊处理
    NullWrite,          // $03 Timer1也不须?
    Write04GeneralCtrl,      // $04
    Write05ClockCtrl,           // $05
    Write06LCDStartAddr,        // $06
    Write07PortConfig,      // $07
    Write08Port0,               // $08
    Write09Port1,               // $09
    Write0AROABBS,                // $0A
    Write0BPort3LCDStartAddr,      // $0B
    Write0CTimer01Control,        // $0C
    Write0DVolumeIDLCDSegCtrl,  // $0D
    NullWrite,      // $0E
    WriteZeroPageBankswitch,    // $0F
    NullWrite,      // $10
    NullWrite,      // $11
    NullWrite,      // $12
    NullWrite,      // $13
    NullWrite,      // $14
    Write15Dir1,               // $15
    NullWrite,      // $16
    NullWrite,      // $17
    Write18Port4,      // $18
    Write19CkvSelect,      // $19
    NullWrite,      // $1A
    NullWrite,      // $1B
    NullWrite,      // $1C
    NullWrite,      // $1D
    NullWrite,      // $1E
    NullWrite,      // $1F
    Write20JG,      // $20
    NullWrite,      // $21
    NullWrite,      // $22
    NullWrite,      // $23
    NullWrite,      // $24
    NullWrite,      // $25
    NullWrite,      // $26
    NullWrite,      // $27
    NullWrite,      // $28
    NullWrite,      // $29
    NullWrite,      // $2A
    NullWrite,      // $2B
    NullWrite,      // $2C
    NullWrite,      // $2D
    NullWrite,      // $2E
    NullWrite,      // $2F
    NullWrite,      // $30
    NullWrite,      // $31
    NullWrite,      // $32
    NullWrite,      // $33
    NullWrite,      // $34
    NullWrite,      // $35
    NullWrite,      // $36
    NullWrite,      // $37
    NullWrite,      // $38
    NullWrite,      // $39
    NullWrite,      // $3A
    NullWrite,      // $3B
    NullWrite,      // $3C
    NullWrite,      // $3D
    NullWrite,      // $3E
    NullWrite,      // $3F
};

regsrec regs;
// LPBYTE  mem          = NULL;
unsigned char fixedram0000[0x10002]; // just like simulator
unsigned char* pmemmap[8]; // 0000~1FFF ... E000~FFFF
unsigned char* may4000ptr;
unsigned char* norbankheader[0x10];
unsigned char* volume0array[0x100]; // even volume
unsigned char* volume1array[0x100]; // odd volume
#ifdef USE_BUSROM
unsigned char* volume2array[0x100];
unsigned char* volume3array[0x100];
#endif
unsigned char* bbsbankheader[0x10];


// WQXSIM
extern bool timer0waveoutstart;
extern int prevtimer0value;
extern unsigned short gThreadFlags;
extern unsigned char* gGeneralCtrlPtr;
extern unsigned short mayGenralnClockCtrlValue;

void FillC000BIOSBank(unsigned char** array);
void InitRAM0IO();


void MemDestroy () {

}

void MemInitialize ()
{
    memset(&fixedram0000[0], 0, 0x10002);
    zp40ptr = &fixedram0000[0x40];

    qDebug("RESET will jump to 0x0418 in norflash page1.");

    MemReset();
    InitRAM0IO();
    if(nc2000){
        super_switch();
    }
}


void MemReset ()
{
    pmemmap[map0000] = &fixedram0000[0];
    pmemmap[map2000] = &fixedram0000[0x2000];
    pmemmap[map4000] = &fixedram0000[0x4000];
    pmemmap[map6000] = &fixedram0000[0x6000];
    pmemmap[map8000] = &fixedram0000[0x8000];
    pmemmap[mapA000] = &fixedram0000[0xA000];
    pmemmap[mapC000] = &fixedram0000[0xC000];
    pmemmap[mapE000] = &fixedram0000[0xE000];

    // InitInternalAddress
    theNekoDriver->InitInternalAddrs();

    // Initialize the cpu
    CpuInitialize(); // Read pc from reset vector
#ifdef HANDYPSP
    setPS(0x24);
#else
    regs.ps = 0x24; // 00100100 unused P(bit5) = 1, I(bit3) = 1, B(bit4) = 0
#endif
}

void InitRAM0IO() 
{
   memset(zpioregs, 0, sizeof(zpioregs));
    zpioregs[io1B_pwm_data] = 0;
    zpioregs[io01_int_status] = 0; 
    w01_int_enable = 0; // Enable all IRQ int
    zpioregs[io04_general_ctrl] = 0;
    zpioregs[io05_clock_ctrl] = 0;
    gThreadFlags = 0;
    //zpioregs[io08_port0_data] = 0;
    r08_port0_ID = 0;
    w08_port0_OL = 0;
    zpioregs[io00_bank_switch] = 0;
    //zpioregs[io09_port1_data] = 0;
    r09_port1_ID = 0;
    w08_port0_OL = 0;
}

// TODO: iofunction?
/*
unsigned char GetByte( unsigned short address )
{
    //unsigned int row = address / 0x2000; // SHR
    //return *(pmemmap[row] + address % 0x2000);
    unsigned int row = address >> 0xD; // 0000 0000 0000 0111 0~7
    return *(pmemmap[row] + (address & 0x1FFF)); // 0001 1111 1111 1111
}

unsigned short GetWord( unsigned short address )
{
    unsigned char low = GetByte(address);
    unsigned char high = GetByte(address == 0xFFFF?0:address + 1);
    return ((high << 8) | low);
}

void SetByte( unsigned short address, unsigned char value )
{
    unsigned int row = address / 0x2000; // SHR
    *(pmemmap[row] + address % 0x2000) = value;
}*/

BYTE __iocallconv NullRead (BYTE address) {
    //qDebug("ggv wanna read io, [%04x] -> %02x", address, mem[address]);
    return zpioregs[address];
}

void __iocallconv NullWrite(BYTE address, BYTE value) {
    //qDebug("ggv wanna write io, [%04x] (%02x) -> %02x", address, mem[address], value);
    zpioregs[address] = value;
}


void TNekoDriver::InitInternalAddrs()
{
    FillC000BIOSBank(volume0array);
    pmemmap[mapC000] = bbsbankheader[0];
    may4000ptr = volume0array[0];
    // E000~FFFF stores jsr E006 (and almost nmi/irq/reset handler)
    pmemmap[mapE000] = volume0array[0] + 0x2000; // lea     ecx, [eax+2000h]
    Switch4000toBFFF(0);

    //mayGenralnClockCtrlValue = 0;
    //regs.sp = 0x100;

    //b5 :  BOUT   : Battery detect output of level 1
    //b3 :  AUTOBRK: Battery detect output of level 2
    zpioregs[io0C_general_status] = 0x28; // ([0C] & 3) * 1000 || [06] * 10 = LCDAddr
    if(nc2000){
        super_switch();
    }
}


void __iocallconv Write00BankSwitch( BYTE write, BYTE bank )
{
    //qDebug("ggv wanna switch to bank 0x%02x", bank);
    //theNekoDriver->SwitchNorBank(value);
    if (zpioregs[io0A_roa] & 0x80) {
        // ROA == 1
        // RAM (norflash?!)
        char norbank = bank & 0xF; // nor only have 0~F page
        may4000ptr = norbankheader[norbank];
        theNekoDriver->Switch4000toBFFF(norbank);
    } else {
        // ROA == 0
        // BROM
        if (zpioregs[io0D_volumeid] & 1) {
            // VolumeID == 1, 3
            may4000ptr = volume1array[bank];
            theNekoDriver->Switch4000toBFFF(bank);
        } else {
            // VolumeID == 0, 2
            may4000ptr = volume0array[bank];
            theNekoDriver->Switch4000toBFFF(bank);
        }
    }
    // update at last
    zpioregs[io00_bank_switch] = bank;
    (void) write;

    if(nc2000){
        super_switch();
    }
}

BYTE __iocallconv Read00BankSwitch( BYTE )
{
    BYTE r = zpioregs[io00_bank_switch];
    //qDebug("ggv wanna read bank. current bank 0x%02x", r);
    return r;
}

void FillC000BIOSBank(unsigned char** array)
{
    bbsbankheader[0] = array[0];
    if (zpioregs[io0D_volumeid] & 1) {
        // Volume1,3
        bbsbankheader[1] = norbankheader[0] + 0x2000;
    } else {
        // Volume0,2
        bbsbankheader[1] = &fixedram0000[0x4000];
    }
    bbsbankheader[2] = array[0] + 0x4000;
    bbsbankheader[3] = array[0] + 0x6000;
    for (int i = 0; i < 3; i++)
    {
        // 4567, 89AB, CDEF take first 4page 0000~7FFF in BROM
        bbsbankheader[i * 4 + 4] = array[i + 1];
        bbsbankheader[i * 4 + 5] = array[i + 1] + 0x2000;
        bbsbankheader[i * 4 + 6] = array[i + 1] + 0x4000;
        bbsbankheader[i * 4 + 7] = array[i + 1] + 0x6000;
    }
}

void __iocallconv Write0AROABBS( BYTE write, BYTE value )
{
    if (value != zpioregs[io0A_roa]) {
        // Update memory pointers only on value changed
        unsigned char bank;
        if (value & 0x80u) {
            // ROA == 1
            // RAM (norflash)
            bank = (zpioregs[io00_bank_switch] & 0xF); // bank = 0~F
            may4000ptr = norbankheader[bank];
        } else {
            // ROA == 0
            // ROM (MASKROM/BUSROM)
            bank = zpioregs[io00_bank_switch];
            if (zpioregs[io0D_volumeid] & 1) {
                // Volume1,3
                may4000ptr = volume1array[bank];
            } else {
                // Volume0,2
                may4000ptr = volume0array[bank];
            }
        }
        zpioregs[io0A_roa] = value;
        theNekoDriver->Switch4000toBFFF(bank);
        pmemmap[mapC000] = bbsbankheader[value & 0xF];
    }
    // in simulator destination memory is updated before call WriteIO0A_ROA_BBS
    //fixedzpiocache[io0A_roa] = value;
    (void)write;
    if(nc2000){
        super_switch();
    }
}

// TODO: bank0~127 and bank128~255 to different array
void __iocallconv Write0DVolumeIDLCDSegCtrl(BYTE write, BYTE value)
{
    // b76543 = lcd
    unsigned short lcdwidth = value >> 3;
    if (lcdwidth == 0) {
        lcdwidth |= 0x20;
    }
    lcdwidth <<= 4;
    if (value ^ zpioregs[io0D_volumeid] & 1) {
        // bit0 changed.
        // volume1,3 != volume0,2
        unsigned char bank = zpioregs[io00_bank_switch];
        if (value & 1) {
            // Volume1,3
            FillC000BIOSBank(volume1array);
            may4000ptr = volume1array[bank];
            pmemmap[mapE000] = volume1array[0] + 0x2000;
        } else {
            // Volume0.2
            FillC000BIOSBank(volume0array);
            may4000ptr = volume0array[bank];
            pmemmap[mapE000] = volume0array[0] + 0x2000;
        }
        unsigned char roabbs = zpioregs[io0A_roa];
        if (roabbs & 0x80) {
            // ROA == 1
            // RAM(nor)
            bank = bank & 0x0F;
            may4000ptr = norbankheader[bank];
        }
        pmemmap[mapC000] = bbsbankheader[roabbs & 0x0F];
        theNekoDriver->Switch4000toBFFF(bank);
    }
    zpioregs[io0D_volumeid] = value;
    (void)write;

    if(nc2000){
        super_switch();
    }
}

// unsigned char zp40cache[0x40]; // the real storage for built-in sram 0x00~0x3F

unsigned char* GetZeroPagePointer(unsigned char bank) {
    unsigned char* result;

    if (bank >= 4) {
        // 4,5,6,7
        // 4 -> 200 5-> 240
        // 6 -> 280 7 -> 2C0
        result = &fixedram0000[(bank + 4) << 6];
    } else {
        // 1,2,3
        result = &fixedram0000[0];
    }
    return result;
}

unsigned char* zp40ptr;

void __iocallconv WriteZeroPageBankswitch (BYTE write, BYTE value)
{
    unsigned char oldzpbank = zpioregs[io0F_zp_bsw] & 7;
    unsigned char newzpbank = value & 7;
    if (oldzpbank != newzpbank) {
        if (newzpbank == 0) {
            zp40ptr = &fixedram0000[0x40];
        } else {
            zp40ptr = GetZeroPagePointer(newzpbank);
        }
    }
    zpioregs[io0F_zp_bsw] = value;
    rw0f_b4_DIR00 = (value & 0x10) != 0;
    rw0f_b5_DIR01 = (value & 0x20) != 0;
    rw0f_b6_DIR023 = (value & 0x40) != 0;
    rw0f_b7_DIR047 = (value & 0x80) != 0;
    (void)write;
    if(nc2000){
        super_switch();
    }
}
#if 0
void TNekoDriver::SwitchNorBank( int bank )
{
    // TODO: norbank header
    zpioregs[io0A_roa] = zpioregs[io0A_roa] | 0x80u;
    //memcpy(&fixedram0000[0x4000], &fNorBuffer[bank * 0x8000], 0x8000);
    pmemmap[map4000] = (unsigned char*)&fNorBuffer[bank * 0x8000]; // 4000
    pmemmap[map6000] = (unsigned char*)&fNorBuffer[bank * 0x8000 + 0x2000]; // 6000
    pmemmap[map8000] = (unsigned char*)&fNorBuffer[bank * 0x8000 + 0x4000]; // 8000
    pmemmap[mapA000] = (unsigned char*)&fNorBuffer[bank * 0x8000 + 0x6000]; // A000
}
#endif

void TNekoDriver::Switch4000toBFFF( unsigned char bank )
{
    if (bank != 0 || zpioregs[io0A_roa] & 0x80) {
        // bank != 0 || ROA == RAM
        pmemmap[map4000] = may4000ptr;
        pmemmap[map6000] = may4000ptr + 0x2000;
    } else {
        // bank == 0 && ROA == ROM
        if (zpioregs[io0D_volumeid] & 0x1) {
            // Volume1,3
            // 4000~7FFF is 0 page of Nor.
            // 8000~BFFF is relative to may4000ptr
            pmemmap[map4000] = norbankheader[0];
            pmemmap[map6000] = norbankheader[0] + 0x2000;
        } else {
            // Volume0,2
            // 4000~5FFF is RAM
            // 6000~7FFF is mirror of 4000~5FFF
            pmemmap[map4000] = &fixedram0000[0x4000];
            pmemmap[map6000] = &fixedram0000[0x4000];
        }
    }
    pmemmap[map8000] = may4000ptr + 0x4000;
    pmemmap[mapA000] = may4000ptr + 0x6000;
}

void checkflashprogram(WORD addr16, BYTE data)
{
    unsigned char* lpaddr16ram = &pmemmap[addr16 >> 13][addr16 & 0x1FFF];
    if ((lpaddr16ram >= &fixedram0000[0x4000]) && (lpaddr16ram < &fixedram0000[0x6000])) {
        // Real physics memory inside 4000~5FFF
        *lpaddr16ram = data;
        return;
    }
    // consider addr16 >= 0x4000, and is not fixed ram
    // goto label_checkregister is not necessary now
    if (addr16 < 0xC000u && zpioregs[io0A_roa] && 0x80u) {
        // addr16 inside 4000~BFFF and ROA is RAM(norflash)
        theNekoDriver->CheckFlashProgramming(addr16, data);
    }
}

bool TNekoDriver::LoadDemoNor(const std::string& filename)
{
    FILE* mariofile = fopen(filename.c_str(), "rb");
    int page = 1;
    while (feof(mariofile) == false) {
        fread(fNorBuffer + 0x8000 * page + 0x4000, 0x4000, 1, mariofile);
        fread(fNorBuffer + 0x8000 * page, 0x4000, 1, mariofile);
        page++;
    }

    fclose(mariofile);
    return true;
}

bool TNekoDriver::LoadBROM( const std::string& filename )
{
    FILE* romfile = fopen(filename.c_str(), "rb");
    // TODO: use proper separate mapper for 0~127 and 128~255
    int page = 0;
#ifdef USE_BUSROM
    while (feof(romfile) == false) {
        fread(fBROMBuffer + 0x8000 * page + 0x4000, 0x4000, 1, romfile);
        fread(fBROMBuffer + 0x8000 * page, 0x4000, 1, romfile);
        page++;
    }
#else
    while (feof(romfile) == false) {
        fread(fBROMBuffer + 0x8000 * page + 0x4000, 0x4000, 1, romfile);
        fread(fBROMBuffer + 0x8000 * page, 0x4000, 1, romfile);
        page++;
    }
    if (page < 0x200) {
        // PC1000a
        for (int i = page - 1; i < 256; i++) {
            volume1array[i] = volume1array[0];
        }
    }
#endif
    fclose(romfile);
    return true;
}

bool TNekoDriver::LoadFullNorFlash( const std::string& filename )
{
    fNorFilename = filename;
    FILE* norfile = fopen(filename.c_str(), "rb");

    int page = 0;
    while (feof(norfile) == false) {
        fread(fNorBuffer + 0x8000 * page + 0x4000, 0x4000, 1, norfile);
        fread(fNorBuffer + 0x8000 * page, 0x4000, 1, norfile);
        page++;
    }
    fclose(norfile);
    fFlashUpdated = false;
    return true;
}

bool TNekoDriver::SaveFullNorFlash()
{
    FILE* norfile = fopen(fNorFilename.c_str(), "wb");

    int page = 0;
    while (page < 0x10) {
        fwrite(fNorBuffer + 0x8000 * page + 0x4000, 0x4000, 1, norfile);
        fwrite(fNorBuffer + 0x8000 * page, 0x4000, 1, norfile);
        page++;
    }
    fclose(norfile);
    fFlashUpdated = false;
    return true;
}

bool TNekoDriver::LoadSRAM(const std::string &filename)
{
    fSRAMFilename = filename;
    FILE* ramfile = fopen(filename.c_str(), "rb");
    if (ramfile == 0) {
        return false;
    }
    char* localram = (char*)malloc(65536);
    fread(localram, 65536, 1, ramfile);
    // TODO: check magic 55AA?
    memcpy(fixedram0000, localram, 65536);
    free(localram);
    
    fclose(ramfile);
    return true;
}

bool TNekoDriver::SaveFullSRAM()
{
    FILE* sramfile = fopen(fSRAMFilename.c_str(), "wb");
    if (sramfile == 0) {
        return false;
    }
    fwrite(fixedram0000, 65536, 1, sramfile);
    fclose(sramfile);
    return true;
}

unsigned char gNor5555_AAFlag = 0, gNorAAAA_AAFlag = 0, gNor8555_AAFlag = 0;
unsigned char gNorSingleByteStep = 0, gNorPageEraseStep = 0;
unsigned char gPrevNor8000 = 0, gPrevNor8001 = 0;
int gErasePos = 0, gEraseBlockAddr = 0;

void TNekoDriver::CheckFlashProgramming( unsigned short addr16, unsigned char data )
{
    //qDebug("ggv wanna erase flash!");
    // TODO: rewrite to only support SST/BSI flash
    // consider addr16 >= 0x4000, and is not fixed ram
    // goto label_checkregister is not necessary now
    // addr16 inside 4000~BFFF and ROA is RAM(norflash)
    unsigned char nor5555_AAflag = gNor5555_AAFlag;
    // first step
    if ( !gNorSingleByteStep && !gNor5555_AAFlag )
    {
        if ( !gNorAAAA_AAFlag && !gNor8555_AAFlag )
        {
            if ( !gNorPageEraseStep )
            {
                switch ( addr16 )
                {
                case 0x8555:
                    if ( data == 0xAA )
                        gNor8555_AAFlag = 1;
                    break;
                case 0x5555:
                    if ( data == 0xAA )
                    {
                        gNor5555_AAFlag = 1;
                        gNorSingleByteStep = 1;
                        gNorPageEraseStep = 1;
                    }
                    break;
                case 0xAAAA:
                    if ( data == 0xAA )
                        gNorAAAA_AAFlag = 1;
                    break;
                }
                return; //goto label_checkregister;
            }
            goto label_checkpageerase;
        }
        nor5555_AAflag = gNor5555_AAFlag;
    }
    // read ID step (AMD)
    if ( gNor8555_AAFlag )
    {
        // dead code
        // assume every nor operation is start with 5555<-AA
        switch ( gNor8555_AAFlag )
        {
        case 1:
            if ( addr16 == 0x82AA && data == 0x55 )
            {
                gNor8555_AAFlag = 2;
                return; //goto label_checkregister;
            }
            break;
        case 2:
            if ( addr16 == 0x8555 && data == 0x90 )
            {
                gNor8555_AAFlag = 3;
                return; //goto label_checkregister;
            }
            break;
        case 3:
            if ( addr16 == 0xAAAA && data == 0xAA )
            {
                gNor8555_AAFlag = 4;
                return; //goto label_checkregister;
            }
            break;
        case 4:
            if ( addr16 == 0x5555 && data == 0x55 )
            {
                gNor8555_AAFlag = 5;
                return; //goto label_checkregister;
            }
            break;
        case 5:
            if ( addr16 == 0xAAAA && data == 0xA0 )
            {
                gNor8555_AAFlag = 6;
                return; //goto label_checkregister;
            }
            break;
        default:
            if ( gNor8555_AAFlag == 6 && addr16 == 0x8000 && data == 0xF0 )
            {
                gNor8555_AAFlag = 0;
                return; //goto label_checkregister;
            }
            break;
        }
        qDebug("error occurs when read AMD id!");
        return; //goto label_printerr_DestToAddr16_checkregister;
    }
    // read ID step (ST)
    if ( gNorAAAA_AAFlag )
    {
        // assume every nor operation is start with 5555<-AA
        if ( gNorAAAA_AAFlag == 1 )
        {
            // we have an AAAA<-AA
            if ( addr16 == 0x5555 && data == 0x55 )
            {
                gNorAAAA_AAFlag = 2;        // 2 means AAAA<-AA, AAAA<-55
                return; //goto label_checkregister;
            }
        }
        else
        {
            if ( gNorAAAA_AAFlag == 2 )
            {
                // we have two AAAA<-AA, AAAA<-55
                if ( addr16 == 0xAAAA && data == 0x90 )
                {
                    gNorAAAA_AAFlag = 3;    // 3 means AAAA<-AA, AAAA<-55, AAAA<-90
                    return; //goto label_checkregister;
                }
            }
            else
            {
                if ( gNorAAAA_AAFlag == 3 && addr16 == 0x8000 && data == 0xF0 )
                {
                    gNorAAAA_AAFlag = 0;    // reset
                    return; //goto label_checkregister;
                }
            }
        }
        qDebug("error occurs when read ST id!");
        return; //goto label_printerr_DestToAddr16_checkregister;
    }
    // normal second step check
    switch ( nor5555_AAflag )
    {
    case 1:
        // we have an nice 5555<-AA step
        if ( addr16 == 0xAAAA && data == 0x55 )
        {
            gNor5555_AAFlag = 2;        // 2 means 5555<-AA, AAAA<-55
            ++gNorPageEraseStep;
            ++gNorSingleByteStep;
            return; //goto label_checkregister;
        }
        break;
    case 2:
        if ( addr16 == 0x5555 && data == 0x90 )
        {
            // Modify 8000 in bank1
            //gPrevNor8000 = *(unsigned char *)(norbankheader[1] + 0x4000);
            //gPrevNor8001 = *(unsigned char *)(norbankheader[1] + 0x4001);
            //*(unsigned char *)(norbankheader[1] + 0x4000) = 0xBFu;
            //*(unsigned char *)(norbankheader[1] + 0x4001) = 0xD7u;
            qDebug("ggv wanna update bank1 8000 flash!");
            gPrevNor8000 = norbankheader[1][0x4000];
            gPrevNor8001 = norbankheader[1][0x4001];
            norbankheader[1][0x4000] = 0xBFu;
            norbankheader[1][0x4001] = 0xD7u;
            gNorPageEraseStep = 0;
            ++gNor5555_AAFlag;          // 3 means 5555<-AA, AAAA<-55, 5555<-90
            gNorSingleByteStep = 0;
            return; //goto label_DestToAddr16_checkregister;
        }
        break;
    case 3:
        //mayDestAddr = 0x8000u;          // Check range, dead condtion
        if ( data == 0xF0 )
        {
            // Restore 8000 in bank1
            //*(unsigned char *)(norbankheader[1] + 0x4000) = gPrevNor8000;
            //*(unsigned char *)(norbankheader[1] + 0x4001) = gPrevNor8001;
            qDebug("ggv wanna restore bank1 8000 flash!");
            norbankheader[1][0x4000] = gPrevNor8000;
            norbankheader[1][0x4001] = gPrevNor8001;
            gNor5555_AAFlag = 0;        // Finish!
            return; //goto label_DestToAddr16_checkregister;
        }
        qDebug("error occurs when read SST id!");
        return; //goto label_printerr_DestToAddr16_checkregister;
    }
    if ( gNorSingleByteStep == 2 )
    {
        if ( addr16 == 0x5555 && data == 0xA0 )
        {
            gNorSingleByteStep = 3;         // Single byte last check
            gNor5555_AAFlag = 0;            // Finish!
            gNorPageEraseStep = 0;
            return; //goto label_checkregister;
        }
    }
    else
    {
        if ( gNorSingleByteStep == 3 )
        {
            // Single byte mode write step
            // use &= because only can write on dest addr is earse to FF
            //*(unsigned char *)((unsigned __int16)addr16 + may4000ptr - 0x4000) &= data;
            // addr16 = 4000~BFFF -> 0000~7FFF
            qDebug("ggv wanna change single byte flash!");
            unsigned char norbank = zpioregs[io00_bank_switch] & 0xF;
            norbankheader[norbank][addr16 - 0x4000] &= data;
            gNorSingleByteStep = 0;
            fFlashUpdated = true;
            return; //goto label_DestToAddr16_checkregister;
        }
    }
label_checkpageerase:
    if ( gNorPageEraseStep == 2 )
    {
        if ( addr16 == 0x5555 && data == 0x80 )
        {
            // Step3 of PAGE ERASE
            gNorPageEraseStep = 3;
            gNor5555_AAFlag = 0;
            gNorSingleByteStep = 0;
            return; //goto label_checkregister;
        }
    }
    else
    {
        // gNorFlag0 != 2
        if ( (unsigned char)gNorPageEraseStep > 2u )
        {
            // Check ERASE mode
            switch ( gNorPageEraseStep )
            {
            case 3:
                if ( addr16 == 0x5555 && data == 0xAA )
                {
                    // Step4 of PAGE ERASE
                    gNorPageEraseStep = 4;
                    return; //goto label_checkregister;
                }
                break;
            case 4:
                if ( addr16 == 0xAAAA && data == 0x55 )
                {
                    // Step5 of PAGE ERASE
                    gNorPageEraseStep = 5;
                    return; //goto label_checkregister;
                }
                break;
            case 5:
                // 5555<-10 erase all
                // dest<-30 erase 4K
                if ( addr16 == 0x5555 && data == 0x10 )
                {
                    // PAGE EARSE - earse every bank of 16 banks
                    // using gnorflag0 as norbank
                    qDebug("ggv wanna erase 0~F bank flash!");
                    gNorPageEraseStep = 0;
                    do
                    {
                        int i = 0;
                        gErasePos = 0;
                        do
                        {
                            //*(unsigned char *)(norbankheader[(unsigned __int8)gNorPageEraseStep] + i) = 0xFFu;
                            norbankheader[(unsigned char)gNorPageEraseStep][i] = 0xFFu;
                            i = gErasePos++ + 1;
                        }
                        while ( (unsigned int)gErasePos < 0x8000 );
                        ++gNorPageEraseStep;
                    }
                    while ( (unsigned char)gNorPageEraseStep < 0x10u );
                    gErasePos = 0;
                    gNorPageEraseStep = 0;
                    fFlashUpdated = true;
                    return; //goto label_DestToAddr16_checkregister;
                }
                if ( data == 0x30 )
                {
                    // PAGE ERASE - 4K mode
                    // still use gnorflag0 as norbank
                    qDebug("ggv wanna erase one block of flash!");
                    gNorPageEraseStep = zpioregs[io00_bank_switch];// cross?! 557
                    // 5018 -> 5018 - 18 - 4000 = 1000
                    gEraseBlockAddr = (unsigned short)addr16 - (unsigned short)addr16 % 0x1000 - 0x4000;
                    int i2 = 0;
                    gErasePos = 0;
                    do
                    {
                        *(unsigned char *)(gEraseBlockAddr + norbankheader[(unsigned char)gNorPageEraseStep] + i2) = 0xFFu;
                        i2 = gErasePos++ + 1;
                    }
                    while ( (unsigned int)gErasePos < 0x1000 );
                    gErasePos = 0;
                    gNor8555_AAFlag = 0;
                    gNorPageEraseStep = 0;
                    fFlashUpdated = true;
                    return; //goto label_DestToAddr16_checkregister;
                }
                break;
            }
            qDebug("error occurs when erase flash!");
            return; //goto label_printerr_DestToAddr16_checkregister;
        }
    }
    qDebug("error occurs when put a byte in flash!");
//label_printerr_DestToAddr16_checkregister:
//    //printf(errmsg);
//    return; //goto label_DestToAddr16_checkregister;
//label_DestToAddr16_checkregister:
//    // addr16 = mayDestAddr
//    // return; //goto label_checkregister;
//label_checkregister:
//    //if ( addr16 < (unsigned __int16)(unsigned __int8)RegisterRange )
//    //    JUMPOUT(loc_40D680);
//    return;
}

uint8_t *ram_io=zpioregs;
uint8_t * * nor_banks=norbankheader;
uint8_t ext_ram[0x8000];

uint8_t * (&memmap)[8]=pmemmap;

uint8_t* (&ram_40) = zp40ptr;

uint8_t ram_buff[0x8000];
//uint8_t* stack = ram_buff + 0x100;
//uint8_t* ram_40 = ram_buff + 0x40;
uint8_t* ram00 = fixedram0000;
uint8_t* ram02 = ram_buff + 0x2000;
uint8_t* ram_b = ram_buff + 0x4000;
uint8_t* ram04 = ram_buff + 0x6000;
uint8_t* GetBank(uint8_t bank_idx){
	uint8_t volume_idx = ram_io[0x0D];
    if (bank_idx < num_nor_pages) {
    	return nor_banks[bank_idx];
    } else if (bank_idx >= 0x80) {

		if(nc2000){
			//printf("<%x\n>",bank_idx);
			//assert(bank_idx==0x80);
			return ext_ram;

			/*
			if(bank_idx%2==0)
			return nc1020_states.ext_ram;
			else
			return nc1020_states.ext_ram2; */
		}
    }
    return NULL;
}

void SwitchBank(){
	uint8_t bank_idx = ram_io[0x00];
	uint8_t* bank = GetBank(bank_idx);
	if(nc2000){
		if(bank== NULL) return;
		//assert(bank!=NULL);
	}
    memmap[2] = bank;
    memmap[3] = bank + 0x2000;
    memmap[4] = bank + 0x4000;
    memmap[5] = bank + 0x6000;
}

void SwitchVolume(){

	if(nc2000){
		memmap[7] = nor_banks[0]+0x6000 -0x4000;
		bool ramb=  (ram_io[0x0d]&0x04);
		if(ramb){
			memmap[1]=ram_b;
		}else{
			memmap[1]=ram02;
		}
		uint8_t bbs = ram_io[0x0A]&0xf;
		if (bbs==1) {
			memmap[6]=ram04;
		}else if (bbs==0){
			memmap[6]=nor_banks[0]+0x4000  -0x4000;
		}else if (bbs==2){
			memmap[6]=nor_banks[0]+0x8000  -0x4000;
		}else if (bbs==3) {
			memmap[6]=nor_banks[0]+0xa000  -0x4000;
		}else {
			memmap[6]=nor_banks[bbs/4]+0x2000* (bbs%4);
		}
	}

}

void super_switch(){
	uint8_t roa_bbs=ram_io[0x0a];
	uint8_t ramb_vol=ram_io[0x0d];
	uint8_t bs=ram_io[0x00];
	//if(enable_debug_switch)printf("tick=%llx pc=%x bs=%x roa_bbs=%x ramb_vol=%x\n",tick, nc1020_states.cpu.reg_pc,bs, roa_bbs , ramb_vol);

	if(nc2000){
		//assert(bs<0x80);
		if(bs<0x80 &&bs>=num_nor_pages) {
			//printf("ill bs %x ; ",bs);
			//printf("tick=%llx pc=%x bs=%x roa_bbs=%x ramb_vol=%x\n",tick, reg_pc,bs, roa_bbs , ramb_vol);
		}
		if(bs>=0x80) {
			//if(bs!=0x80) {printf("<%x>\n",bs);}
			//assert(bs==0x80);
		}
		if(bs>=0x80 && !(roa_bbs&0x80)){
			if(enable_oops)printf("oops1!\n");
			//assert(false);
		}
		if(bs<0x80 && (roa_bbs&0x80)){
			if(enable_oops)printf("oops2!\n");
			//assert(false);
		}
		if((ramb_vol&0x03)!=0){
			if(enable_oops)printf("oops3!\n");
			assert(false);
		}
	}

	SwitchVolume();
	SwitchBank();
	uint8_t value= ram_io[0xf];
	value&=0x7;
	if(value!=0) {
		//assert(false);
	}
	if(value==0) {
		ram_40=pmemmap[0]+0x40;
	}else if(value==1||value==2||value==3){
		ram_40=pmemmap[0];
	}else{
		uint8_t off=value-4;
		ram_40=pmemmap[0]+0x200+0x40*off;
	}
}

uint8_t CPU_PEEK(uint16_t addr){
    if(addr >= 0x80) {
      return *(pmemmap[unsigned(addr) >> 0xD] + (addr & 0x1FFF));
    }else{
      return (addr >= iorange?zp40ptr[addr-0x40]:ioread[addr & 0xFF]((BYTE)(addr & 0xff)));
    }
}

uint16_t CPU_PEEKW(uint16_t addr){
    return  (CPU_PEEK((addr)) + (CPU_PEEK((addr + 1)) << 8));
}

void CPU_POKE(uint16_t addr, uint8_t a)   
{ 
  if ((addr >= 0x80)) { 
    if (addr < 0x4000) {
    *(pmemmap[unsigned(addr) >> 0xD] + (addr & 0x1FFF)) = (BYTE)(a);
    } else {
    checkflashprogram(addr, (BYTE)(a));
    }
  } else if ((addr >= iorange)) {
    zp40ptr[addr-0x40] = (BYTE)(a);
  }  else {
  iowrite[addr & 0xFF]((BYTE)(addr & 0xff),(BYTE)(a)); 
  }
}
    
