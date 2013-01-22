#ifndef __MATROX__REGS_H__
#define __MATROX__REGS_H__

#define U8_TO_F0915(x)          (((u32) ((x+1) << 15)) & 0x00FFFFFF)

#define RS16(val)               ( (u16)((s16)(val)))
#define RS18(val)               (((u32)((s32)(val)))&0x003ffff)
#define RS24(val)               (((u32)((s32)(val)))&0x0ffffff)
#define RS27(val)               (((u32)((s32)(val)))&0x7ffffff)

#define DWGSYNC          0x2C4C
#define SYNC_DMA_BUSY    0x8325340              /* just a random number */

#define RST              0x1E40
#define OPMODE           0x1E54

#define CACHEFLUSH       0x1FFF

/* CRTC2 registers */
#define C2CTL            0x3C10
#    define C2EN                0x00000001
#    define C2PIXCLKSEL_PCICLK  0x00000000
#    define C2PIXCLKSEL_VDOCLK  0x00000002
#    define C2PIXCLKSEL_PIXPLL  0x00000004
#    define C2PIXCLKSEL_VIDPLL  0x00000006 /* SYSPLL on G400 */
#    define C2PIXCLKSEL_VDCLK   0x00004000 /* G450/G550 only */
#    define C2PIXCLKSEL_CRISTAL 0x00004002 /* G450/G550 only */
#    define C2PIXCLKSEL_SYSPLL  0x00004004 /* G450/G550 only */
#    define C2PIXCLKDIS         0x00000008
#    define CRTCDACSEL          0x00100000
#    define C2DEPTH_15BPP       0x00200000
#    define C2DEPTH_16BPP       0x00400000
#    define C2DEPTH_32BPP       0x00800000
#    define C2DEPTH_YCBCR422    0x00A00000
#    define C2DEPTH_YCBCR420    0x00E00000
#    define C2VCBCRSINGLE       0x01000000
#    define C2INTERLACE         0x02000000
#    define C2FIELDLENGTH       0x04000000
#    define C2FIELDPOL          0x08000000
#    define C2VIDRSTMOD_FALLING 0x00000000
#    define C2VIDRSTMOD_RISING  0x10000000
#    define C2VIDRSTMOD_BOTH    0x20000000
#    define C2HPLOADEN          0x40000000
#    define C2VPLOADEN          0x80000000
#define C2HPARAM         0x3C14
#define C2HSYNC          0x3C18
#define C2VPARAM         0x3C1C
#define C2VSYNC          0x3C20
#define C2PRELOAD        0x3C24
#define C2STARTADD0      0x3C28
#define C2STARTADD1      0x3C2C
#define C2PL2STARTADD0   0x3C30
#define C2PL2STARTADD1   0x3C34
#define C2PL3STARTADD0   0x3C38
#define C2PL3STARTADD1   0x3C3C
#define C2OFFSET         0x3C40
#define C2MISC           0x3C44
#    define C2HSYNCPOL          0x00000100
#    define C2VSYNCPOL          0x00000200
#define C2VCOUNT         0x3C48
#    define C2FIELD             0x01000000
#define C2DATACTL        0x3C4C
#    define C2DITHEN            0x00000001
#    define C2YFILTEN           0x00000002
#    define C2CBCRFILTEN        0x00000004
#    define C2SUBPICEN          0x00000008
#    define C2NTSCEN            0x00000010
#    define C2STATICKEYEN       0x00000020
#    define C2OFFSETDIVEN       0x00000040
#    define C2UYVYFMT           0x00000080
#    define C2STATICKEY         0x1F000000
#define C2SUBPICLUT      0x3C50
#define C2SPICSTARTADD0  0x3C54
#define C2SPICSTARTADD1  0x3C58

/* Backend scaler registers */
#define BESA1ORG         0x3D00
#define BESA2ORG         0x3D04
#define BESB1ORG         0x3D08
#define BESB2ORG         0x3D0C
#define BESA1CORG        0x3D10
#define BESA2CORG        0x3D14
#define BESB1CORG        0x3D18
#define BESB2CORG        0x3D1C
#define BESA1C3ORG       0x3D60
#define BESA2C3ORG       0x3D64
#define BESB1C3ORG       0x3D68
#define BESB2C3ORG       0x3D6C

#define BESCTL           0x3D20
#    define BESEN             0x00000001
#    define BESV1SRCSTP       0x00000040
#    define BESV2SRCSTP       0x00000080
#    define BESHFEN           0x00000400
#    define BESVFEN           0x00000800
#    define BESCUPS           0x00010000
#    define BES420PL          0x00020000

#define BESGLOBCTL       0x3DC0
#    define BESCORDER         0x00000008
#    define BES3PLANE         0x00000020
#    define BESUYVYFMT        0x00000040
#    define BESPROCAMP        0x00000080
#    define BESRGB15          0x00000100
#    define BESRGB16          0x00000200
#    define BESRGB32          0x00000300

#define BESHCOORD        0x3D28
#define BESHISCAL        0x3D30
#define BESHSRCEND       0x3D3C
#define BESHSRCLST       0x3D50
#define BESHSRCST        0x3D38
#define BESLUMACTL       0x3D40
#define BESPITCH         0x3D24
#define BESSTATUS        0x3DC4
#define BESV1SRCLST      0x3D54
#define BESV2SRCLST      0x3D58
#define BESV1WGHT        0x3D48
#define BESV2WGHT        0x3D4C
#define BESVCOORD        0x3D2C
#define BESVISCAL        0x3D34

/* DAC Registers */
#define DAC_INDEX        0x3C00
#define DAC_DATA         0x3C0A

#define MGAREG_VCOUNT    0x1e20

/* Alpha registers */

#define ALPHASTART       0x2C70
#define ALPHAXINC        0x2C74
#define ALPHAYINC        0x2C78

#define ALPHACTRL        0x2C7C
#define      SRC_ZERO                    0x00000000
#define      SRC_ONE                     0x00000001
#define      SRC_DST_COLOR               0x00000002
#define      SRC_ONE_MINUS_DST_COLOR     0x00000003
#define      SRC_ALPHA                   0x00000004
#define      SRC_ONE_MINUS_SRC_ALPHA     0x00000005
#define      SRC_DST_ALPHA               0x00000006
#define      SRC_ONE_MINUS_DST_ALPHA     0x00000007
#define      SRC_SRC_ALPHA_SATURATE      0x00000008

#define      DST_ZERO                    0x00000000
#define      DST_ONE                     0x00000010
#define      DST_SRC_COLOR               0x00000020
#define      DST_ONE_MINUS_SRC_COLOR     0x00000030
#define      DST_SRC_ALPHA               0x00000040
#define      DST_ONE_MINUS_SRC_ALPHA     0x00000050
#define      DST_DST_ALPHA               0x00000060
#define      DST_ONE_MINUS_DST_ALPHA     0x00000070

#define      ALPHACHANNEL                0x00000100
#define      VIDEOALPHA                  0x00000200

#define      DIFFUSEDALPHA               0x01000000
#define      MODULATEDALPHA              0x02000000

/* Texture registers */

#define TEXCTL        0x2C30
#define TEXCTL2       0x2C3C
#define TEXFILTER     0x2C58
#define TEXWIDTH      0x2C28
#define TEXHEIGHT     0x2C2C
#define TEXORG        0x2C24
#define TEXORG1       0x2CA4
#define TEXORG2       0x2CA8
#define TEXORG3       0x2CAC
#define TEXORG4       0x2CB0
#define TEXTRANS      0x2C34
#define TEXTRANSHIGH  0x2C38
#define TDUALSTAGE0   0x2CF8
#define TDUALSTAGE1   0x2CFC

#define TMR0          0x2C00
#define TMR1          0x2C04
#define TMR2          0x2C08
#define TMR3          0x2C0C
#define TMR4          0x2C10
#define TMR5          0x2C14
#define TMR6          0x2C18
#define TMR7          0x2C1C
#define TMR8          0x2C20

#define CUR_XWINDOWS    0x03

/* TEXCTL */
#define TW4           0x00000000
#define TW8           0x00000001
#define TW15          0x00000002
#define TW16          0x00000003
#define TW12          0x00000004

#define TW32          0x00000006
#define TW8A          0x00000007
#define TW8AL         0x00000008
#define TW422         0x0000000A
#define TW422UYVY     0x0000000B

#define TFORMAT       0x0000000F
#define TPITCHLIN     0x00000100
#define TPITCHEXT     0x000FFE00

#define NOPERSPECTIVE 0x00200000
#define TAKEY         0x02000000
#define TAMASK        0x04000000
#define CLAMPUV       0x18000000

#define DECALCKEY     0x01000000
#define TMODULATE     0x20000000
#define STRANS        0x40000000


/* TEXTCTL2 */
#define IDECAL        0x00000002
#define DECALDIS      0x00000004
#define CKSTRANSDIS   0x00000010


/* TEXFILTER */
#define MIN_NRST      0x00000000
#define MIN_BILIN     0x00000002
#define MIN_ANISO     0x0000000D
#define MAG_NRST      0x00000000
#define MAG_BILIN     0x00000020
#define FILTER_ALPHA  0x00100000

/* SGN */
#define SGN_BRKLEFT   0x00000100

#define DSTORG        0x2cb8
#define SRCORG        0x2cb4

#define MACCESS        0x1C04
#     define PW8       0x00000000
#     define PW16      0x00000001
#     define PW32      0x00000002
#     define PW24      0x00000003
#     define ZW16      0x00000000
#     define ZW32      0x00000008
#     define ZW15      0x00000010
#     define ZW24      0x00000018
#     define BYPASS332 0x10000000
#     define TLUTLOAD  0x20000000
#     define NODITHER  0x40000000
#     define DIT555    0x80000000


#define EXECUTE       0x100      /* or with register to execute a programmed
                                    accel command */

#define DWGCTL        0x1C00     /* Drawing control */
     /* opcod - Operation code */
#     define OP_LINE_OPEN      0x00
#     define OP_AUTOLINE_OPEN  0x01
#     define OP_LINE_CLOSE     0x02
#     define OP_AUTOLINE_CLOSE 0x03
#     define OP_TRAP           0x04
#     define OP_TRAP_ILOAD     0x05
#     define OP_TEXTURE_TRAP   0x06
#     define OP_ILOAD_HIQH     0x07
#     define OP_BITBLT         0x08
#     define OP_ILOAD          0x09
#     define OP_IDUMP          0x0A
#     define OP_FBITBLT        0x0C
#     define OP_ILOAD_SCALE    0x0D
#     define OP_ILOAD_HIQHV    0x0E
#     define OP_ILOAD_FILTER   0x0F

     /* atype - Access type */
#     define ATYPE_RPL         0x00
#     define ATYPE_RSTR        0x10
#     define ATYPE_ZI          0x30
#     define ATYPE_BLK         0x40
#     define ATYPE_I           0x70

     /* Flag */
#     define LINEAR            0x80
#     define NOCLIP          (1<<31)
#     define TRANSC          (1<<30)

     /* zmode - Z drawing mode */
#     define ZMODE_NOZCMP      0x000
#     define ZMODE_ZE          0x200
#     define ZMODE_ZNE         0x300
#     define ZMODE_ZLT         0x400
#     define ZMODE_ZLTE        0x500
#     define ZMODE_ZGT         0x600
#     define ZMODE_ZGTE        0x700

     /* Flags */
#     define SOLID             0x0800
#     define ARZERO            0x1000
#     define SGNZERO           0x2000
#     define SHFTZERO          0x4000

     /* bop - Boolean operation */
#     define BOP_CLEAR         0x00000
#     define BOP_NOR           0x10000
#     define BOP_COPYINV       0x30000
#     define BOP_INVERT        0x50000
#     define BOP_XOR           0x60000
#     define BOP_NAND          0x70000
#     define BOP_AND           0x80000
#     define BOP_EQUIV         0x90000
#     define BOP_NOOP          0xA0000
#     define BOP_IMP           0xB0000
#     define BOP_COPY          0xC0000
#     define BOP_OR            0xE0000
#     define BOP_SET           0xF0000

     /* bltmod - Blit mode selection */
#     define BLTMOD_BMONOLEF   0x00000000
#     define BLTMOD_BMONOWF    0x08000000
#     define BLTMOD_BPLAN      0x02000000
#     define BLTMOD_BFCOL      0x04000000
#     define BLTMOD_BUYUV      0x1C000000
#     define BLTMOD_BU32BGR    0x06000000
#     define BLTMOD_BU32RGB    0x0E000000
#     define BLTMOD_BU24BGR    0x16000000
#     define BLTMOD_BU24RGB    0x1E000000

#define ZORG          0x1C0C
#define PAT0          0x1C10
#define PAT1          0x1C14
#define PLNWT         0x1C1C
#define BCOL          0x1C20
#define FCOL          0x1C24
#define SRC0          0x1C30
#define SRC1          0x1C34
#define SRC2          0x1C38
#define SRC3          0x1C3C
#define XYSTRT        0x1C40
#define XYEND         0x1C44
#define SHIFT         0x1C50
#define DMAPAD        0x1C54
#define SGN           0x1C58
#define LEN           0x1C5C
#define AR0           0x1C60
#define AR1           0x1C64
#define AR2           0x1C68
#define AR3           0x1C6C
#define AR4           0x1C70
#define AR5           0x1C74
#define AR6           0x1C78
#define CXBNDRY       0x1C80
#define FXBNDRY       0x1C84
#define YDSTLEN       0x1C88
#define PITCH         0x1C8C
#define YDST          0x1C90
#define YDSTORG       0x1C94
#define YTOP          0x1C98
#define YBOT          0x1C9C
#define CXLEFT        0x1CA0
#define CXRIGHT       0x1CA4
#define FXLEFT        0x1CA8
#define FXRIGHT       0x1CAC
#define XDST          0x1CB0
#define DR0           0x1CC0
#define DR2           0x1CC8
#define DR3           0x1CCC
#define DR4           0x1CD0
#define DR6           0x1CD8
#define DR7           0x1CDC
#define DR8           0x1CE0
#define WO            0x1CE4
#define DR10          0x1CE8
#define DR11          0x1CEC
#define DR12          0x1CF0
#define DR14          0x1CF8
#define DR15          0x1CFC

#define FIFOSTATUS    0x1E10

#define STATUS        0x1E14
#     define DWGENGSTS   0x10000
#     define ENDPRDMASTS 0x20000

#define IEN           0x1E1C

#define BLIT_LEFT     1
#define BLIT_UP       4


#define SDXL          0x0002
#define SDXR          0x0020


/* DAC registers */

#define XMISCCTRL      0x1E
#     define DACPDN             0x01
#     define MFCSEL_MAFC        0x02
#     define MFCSEL_PANELLINK   0x04
#     define MFCSEL_DIS         0x06
#     define MFCSEL_MASK        0x06
#     define VGA8DAC            0x08
#     define RAMCS              0x10
#     define VDOUTSEL_MAFC12    0x00
#     define VDOUTSEL_BYPASS656 0x40
#     define VDOUTSEL_CRTC2RGB  0x80
#     define VDOUTSEL_CRTC2656  0xC0
#     define VDOUTSEL_MASK      0xE0

#define XGENIOCTRL     0x2A
#define XGENIODATA     0x2B

#define XKEYOPMODE     0x51

#define XCOLMSK0RED    0x52
#define XCOLMSK0GREEN  0x53
#define XCOLMSK0BLUE   0x54

#define XCOLKEY0RED    0x55
#define XCOLKEY0GREEN  0x56
#define XCOLKEY0BLUE   0x57

#define XDISPCTRL      0x8A
#     define DAC1OUTSEL_DIS     0x00
#     define DAC1OUTSEL_EN      0x01
#     define DAC1OUTSEL_MASK    0x01
#     define DAC2OUTSEL_DIS     0x00
#     define DAC2OUTSEL_CRTC1   0x04
#     define DAC2OUTSEL_CRTC2   0x08
#     define DAC2OUTSEL_TVE     0x0C
#     define DAC2OUTSEL_MASK    0x0C
#     define PANOUTSEL_DIS      0x00
#     define PANOUTSEL_CRTC1    0x20
#     define PANOUTSEL_CRTC2RGB 0x40
#     define PANOUTSEL_CRTC2656 0x60
#     define PANOUTSEL_MASK     0x60

#define XSYNCCTRL      0x8B
#     define DAC1HSOFF          0x01
#     define DAC1VSOFF          0x02
#     define DAC1HSPOL          0x04
#     define DAC1VSPOL          0x08
#     define DAC2HSOFF          0x10
#     define DAC2VSOFF          0x20
#     define DAC2HSPOL          0x40
#     define DAC2VSPOL          0x80

#define XPWRCTRL       0xA0
#     define DAC2PDN            0x01
#     define VIDPLLPDN          0x02
#     define PANPDN             0x04
#     define RFIFOPDN           0x08
#     define CFIFOPDN           0x10

#endif

