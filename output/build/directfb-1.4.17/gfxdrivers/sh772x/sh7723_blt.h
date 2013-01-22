#ifndef __SH7723_BLT_H__
#define __SH7723_BLT_H__

#include <sys/ioctl.h>

#include "sh7722_types.h"



#define SH7723_SUPPORTED_DRAWINGFLAGS      (DSDRAW_NOFX | DSDRAW_BLEND)

#define SH7723_SUPPORTED_DRAWINGFUNCTIONS  (DFXL_FILLRECTANGLE | \
                                            DFXL_FILLTRIANGLE  | \
                                            DFXL_DRAWLINE      | \
                                            DFXL_DRAWRECTANGLE)

#define SH7723_SUPPORTED_BLITTINGFLAGS     (DSBLIT_BLEND_COLORALPHA | \
                                            DSBLIT_SRC_COLORKEY)

#define SH7723_SUPPORTED_BLITTINGFUNCTIONS (DFXL_BLIT)


DFBResult sh7723EngineSync  ( void *drv, void *dev );

void sh7723EngineReset      ( void *drv, void *dev );
void sh7723FlushTextureCache( void *drv, void *dev );

void sh7723EmitCommands     ( void *drv, void *dev );

void sh7723CheckState       ( void *drv, void *dev,
                              CardState *state, DFBAccelerationMask accel );

void sh7723SetState         ( void *drv, void *dev,
                              GraphicsDeviceFuncs *funcs,
                              CardState *state, DFBAccelerationMask accel );

bool sh7723FillRectangle    ( void *drv, void *dev, DFBRectangle *rect );
bool sh7723FillTriangle     ( void *drv, void *dev, DFBTriangle *triangle );
bool sh7723DrawRectangle    ( void *drv, void *dev, DFBRectangle *rect );
bool sh7723DrawLine         ( void *drv, void *dev, DFBRegion *line );
bool sh7723Blit             ( void *drv, void *dev, DFBRectangle *rect, int dx, int dy );

#define SH7723_S16S16(h,l)         ((u32)((((u16)(h)) << 16) | ((u16)(l))))

#define SH7723_XY(x,y)             SH7723_S16S16(x,y)

#define	SH7723_TDG_BASE            0xA4680000

//#define	BEM_HC_DMA_START		   (SH7723_TDG_BASE + 0x00044)
#define	M2DG_SCLR		   (SH7723_TDG_BASE + 0x000)
//#define	BEM_HC_DMA_ADR			   (SH7723_TDG_BASE + 0x00040)
#define	M2DG_DLSAR		   (SH7723_TDG_BASE + 0x048)




#define M2DG_OPCODE_TRAP        0x00000000
#define M2DG_OPCODE_WPR         0x18000000
#define M2DG_OPCODE_SYNC        0x12000000
#define M2DG_OPCODE_LCOFS       0x40000000
#define M2DG_OPCODE_MOVE        0x48000000
#define M2DG_OPCODE_NOP         0x08000000
#define M2DG_OPCODE_INTERRUPT   0x08008000
#define M2DG_SYNC_TCLR          0x00000010

#define M2DG_OPCODE_POLYGON_4C  0x80000000    
#define M2DG_OPCODE_LINE_C      0xB0000000
#define M2DG_OPCODE_BITBLTA     0xA2000100
#define M2DG_OPCODE_BITBLTC     0xA0000000

#define M2DG_DRAWMODE_STRANS    0x00000800
#define M2DG_DRAWMODE_CLIP      0x00002000
#define M2DG_DRAWMODE_ANTIALIAS 0x00000002
#define M2DG_DRAWMODE_ALPHA     0x00000002

#define M2DG_DRAWMODE_SRCDIR_X  0x00000040
#define M2DG_DRAWMODE_SRCDIR_Y  0x00000020
#define M2DG_DRAWMODE_DSTDIR_X  0x00000010
#define M2DG_DRAWMODE_DSTDIR_Y  0x00000008



//ignore and replace 
#define	BEM_WR_CTRL                (0x00400)
#define	BEM_WR_V1                  (0x00410)
#define	BEM_WR_V2                  (0x00414)
#define	BEM_WR_FGC                 (0x00420)

#define	BEM_BE_CTRL                (0x00800)
#define	BEM_BE_V1                  (0x00810)
#define	BEM_BE_V2                  (0x00814)
#define	BEM_BE_V3                  (0x00818)
#define	BEM_BE_V4                  (0x0081C)
#define	BEM_BE_COLOR1              (0x00820)
#define	BEM_BE_SRC_LOC             (0x00830)
#define	BEM_BE_SRC_SIZE            (0x00834)
#define	BEM_BE_MATRIX_A            (0x00850)
#define	BEM_BE_MATRIX_B            (0x00854)
#define	BEM_BE_MATRIX_C            (0x00858)
#define	BEM_BE_MATRIX_D            (0x0085C)
#define	BEM_BE_MATRIX_E            (0x00860)
#define	BEM_BE_MATRIX_F            (0x00864)
#define	BEM_BE_ORIGIN              (0x00870)
#define	BEM_BE_SC_MIN              (0x00880)
#define	BEM_BE_SC_MAX              (0x00884)

#define	BEM_TE_SRC                 (0x00C00)
#define	BEM_TE_SRC_BASE            (0x00C04)
#define	BEM_TE_SRC_SIZE            (0x00C08)
#define	BEM_TE_SRC_CNV             (0x00C0C)
#define	BEM_TE_MASK                (0x00C10)
#define	BEM_TE_MASK_BASE           (0x00C14)
#define	BEM_TE_MASK_SIZE           (0x00C18)
#define	BEM_TE_MASK_CNV            (0x00C1C)
#define	BEM_TE_ALPHA               (0x00C28)
#define	BEM_TE_FILTER              (0x00C30)
#define	BEM_TE_INVALID             (0x00C40)

#define	BEM_PE_DST                 (0x01000)
#define	BEM_PE_DST_BASE            (0x01004)
#define	BEM_PE_DST_SIZE            (0x01008)
#define	BEM_PE_SC                  (0x0100C)
#define	BEM_PE_SC0_MIN             (0x01010)
#define	BEM_PE_SC0_MAX             (0x01014)
#define	BEM_PE_CKEY                (0x01040)
#define	BEM_PE_CKEY_B              (0x01044)
#define	BEM_PE_CKEY_A              (0x01048)
#define	BEM_PE_COLORCHANGE         (0x01050)
#define	BEM_PE_ALPHA               (0x01058)
#define	BEM_PE_COLORCHANGE_0       (0x01060)
#define	BEM_PE_COLORCHANGE_1       (0x01064)
#define	BEM_PE_OPERATION           (0x01080)
#define	BEM_PE_FIXEDALPHA          (0x01084)
#define	BEM_PE_OFFSET              (0x01088)
#define	BEM_PE_MASK                (0x01094)
#define	BEM_PE_CACHE               (0x010B0)

/*
 * BEM_BE_CTRL
 */
#define BE_FLIP_NONE               0x00000000
#define BE_FLIP_HORIZONTAL         0x01000000
#define BE_FLIP_VERTICAL           0x02000000
#define BE_FLIP_BOTH               0x03000000

#define BE_CTRL_FIXMODE_20_12      0x00000000
#define BE_CTRL_FIXMODE_16_16      0x00100000
#define BE_CTRL_CLIP               0x00080000
#define BE_CTRL_ORIGIN             0x00040000
#define BE_CTRL_ZOOM               0x00020000
#define BE_CTRL_MATRIX             0x00010000

#define BE_CTRL_SCANMODE_LINE      0x00000000
#define BE_CTRL_SCANMODE_4x4       0x00001000
#define BE_CTRL_SCANMODE_8x4       0x00002000

#define BE_CTRL_BLTDIR_FORWARD     0x00000000
#define BE_CTRL_BLTDIR_BACKWARD    0x00000100
#define BE_CTRL_BLTDIR_AUTOMATIC   0x00000200

#define BE_CTRL_TEXTURE            0x00000020
#define BE_CTRL_QUADRANGLE         0x00000002
#define BE_CTRL_RECTANGLE          0x00000001

/*
 * BEM_PE_OPERATION
 */
#define BLE_FUNC_NONE              0x00000000
#define BLE_FUNC_AxB_plus_CxD      0x10000000
#define BLE_FUNC_CxD_minus_AxB     0x20000000
#define BLE_FUNC_AxB_minus_CxD     0x30000000

#define BLE_ROP_XOR                0x01660000

#define BLE_SRCA_FIXED             0x00000000
#define BLE_SRCA_SOURCE_ALPHA      0x00001000
#define BLE_SRCA_ALPHA_CHANNEL     0x00002000

#define BLE_DSTA_FIXED             0x00000000
#define BLE_DSTA_DEST_ALPHA        0x00000100

#define BLE_SRCF_ZERO              0x00000000
#define BLE_SRCF_ONE               0x00000010
#define BLE_SRCF_DST               0x00000020
#define BLE_SRCF_1_DST             0x00000030
#define BLE_SRCF_SRC_A             0x00000040
#define BLE_SRCF_1_SRC_A           0x00000050
#define BLE_SRCF_DST_A             0x00000060
#define BLE_SRCF_1_DST_A           0x00000070

#define BLE_DSTO_DST               0x00000000
#define BLE_DSTO_OFFSET            0x00000008

#define BLE_DSTF_ZERO              0x00000000
#define BLE_DSTF_ONE               0x00000001
#define BLE_DSTF_SRC               0x00000002
#define BLE_DSTF_1_SRC             0x00000003
#define BLE_DSTF_SRC_A             0x00000004
#define BLE_DSTF_1_SRC_A           0x00000005
#define BLE_DSTF_DST_A             0x00000006
#define BLE_DSTF_1_DST_A           0x00000007

/*
 * BEM_PE_CKEY
 */
#define CKEY_EXCLUDE_UNUSED        0x00100000
#define CKEY_EXCLUDE_ALPHA         0x00010000
#define CKEY_A_ENABLE              0x00000100
#define CKEY_B_ENABLE              0x00000001

/*
 * BEM_PE_COLORCHANGE
 */
#define COLORCHANGE_DISABLE        0x00000000
#define COLORCHANGE_COMPARE_FIRST  0x0000000b
#define COLORCHANGE_EXCLUDE_UNUSED 0x00010000

/*
 * BEM_PE_MASK
 */
#define PE_MASK_DISABLE            0x00000000
#define PE_MASK_COLOR              0x00000001
#define PE_MASK_ALPHA              0x00000080

/*
 * BEM_TE_MASK
 */
#define TE_MASK_DISABLE            0x00000000
#define TE_MASK_ENABLE             0x00010000

/*
 * BEM_WR_CTRL
 */
#define WR_CTRL_LINE               0x00000002
#define WR_CTRL_POLYLINE           0x00000003
#define WR_CTRL_ANTIALIAS          0x00020100
#define WR_CTRL_ENDPOINT           0x00001000

#endif
