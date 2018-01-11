/******************************************************************************

  Copyright (C), 2018, Vision Soft. Co., Ltd.

 ******************************************************************************
  File Name     : hi_md.h
  Author        : Hisilicon multimedia software (IVE) group
  Created       : 2018/01/09
  Author		: backup4mark@gmail.com
******************************************************************************/
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include "log.h"

#include "hi_fb.h"
#include "hi_mpi.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_vb.h"
#include "mpi_sys.h"
#include "hi_type.h"
#include "hi_comm_vi.h"
#include "hi_comm_vo.h"
#include "hi_comm_vb.h"

#define ALIGN_BACK(x, a)            ((a) * (((x) / (a))))

/*
 *  1. HI_MPI_SYS_Exit
 *  2. HI_MPI_VB_Exit
 *
 *  3. HI_MPI_VB_Init
 *  4. HI_MPI_SYS_Init
 */
int hi_sys_init(void)
{
    int ret;
    VB_CONF_S stVbConf;
    MPP_SYS_CONF_S stSysConf = { 0 };

    /* Clear original resource */
    hi_sys_exit();

    /* Init vb & sys */
    memset(&stVbConf, 0, sizeof(VB_CONF_S));

    stVbConf.u32MaxPoolCnt = VB_MAX_POOLS;
    stVbConf.astCommPool[0].u32BlkSize = 3133440; // (1920x1080 YUV 420) 1920 * 1088 * 1.5
    stVbConf.astCommPool[0].u32BlkCnt = 128;

    ret = HI_MPI_VB_SetConf(&stVbConf);
    if (HI_SUCCESS != ret) {
        pr_err();
        return HI_FAILURE;
    }

    ret = HI_MPI_VB_Init();
    if (HI_SUCCESS != ret) {
        pr_err();
        return HI_FAILURE;
    }

    stSysConf.u32AlignWidth = 64;
    ret = HI_MPI_SYS_SetConf(&stSysConf);
    if (HI_SUCCESS != ret) {
        pr_err();
        return HI_FAILURE;
    }

    ret = HI_MPI_SYS_Init();
    if (HI_SUCCESS != ret) {
        pr_err();
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

void hi_sys_exit(void)
{
    int i;

    HI_MPI_SYS_Exit();

    for(i=0;i < VB_MAX_USER;i++) {
        HI_MPI_VB_ExitModCommPool((VB_UID_E)i);
    }

    for(i=0; i < VB_MAX_POOLS; i++) {
        HI_MPI_VB_DestroyPool(i);
    }

    HI_MPI_VB_Exit();
}

int vo_bind_vi(void)
{
    int ret;
    MPP_CHN_S SrcChn, DestChn;

    SrcChn.enModId = HI_ID_VIU;
    //vga : 6/24
    SrcChn.s32DevId = 6;
    SrcChn.s32ChnId = 24;

    DestChn.enModId = HI_ID_VOU;
    //vga : 0/1
    DestChn.s32DevId = 0;
    DestChn.s32ChnId = 1;

    ret = HI_MPI_SYS_Bind(&SrcChn, &DestChn);
    if (ret != HI_SUCCESS) {
        pr_err("HI_MPI_SYS_Bind fail!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

int fb_alloc(void)
{
    HI_BOOL bShow = HI_TRUE;
    int FrameBufferFd = -1;
    struct fb_var_screeninfo DefaultVinfo;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo fix;
    unsigned char * pShowScreen;
    HIFB_ALPHA_S stAlpha;

    /* set resolution for hi3531a */
    int width = 1920;
    int height = 1080;

    static struct fb_bitfield g_a32 = {24, 8, 0};
    static struct fb_bitfield g_r32 = {16, 8, 0};
    static struct fb_bitfield g_g32 = {8, 8, 0};
    static struct fb_bitfield g_b32 = {0, 8, 0};

    char FrameBufferDevFile[16];
    memset(FrameBufferDevFile, 0, sizeof(FrameBufferDevFile));
    sprintf(FrameBufferDevFile, "/dev/fb%d", 0);

    FrameBufferFd = open(FrameBufferDevFile, O_RDWR, 0);
    if(FrameBufferFd < 0) {
        pr_err("open FrameBufferDevFile(%s) failed!", FrameBufferDevFile);
        return HI_FAILURE;
    }

    HIFB_CAPABILITY_S pstCap;
    if(ioctl(FrameBufferFd, FBIOGET_CAPABILITY_HIFB, &pstCap)<0) {
        pr_err("FBIOGET_CAPABILITY_HIFB failed!");
        goto end;
    }

    if (ioctl(FrameBufferFd, FBIOGET_VSCREENINFO, &DefaultVinfo) < 0) {
        pr_err("FBIOGET_VSCREENINFO failed!");
        goto end;
    }

    DefaultVinfo.bits_per_pixel = 4 * 8;
    DefaultVinfo.transp= g_a32;
    DefaultVinfo.red = g_r32;
    DefaultVinfo.green = g_g32;
    DefaultVinfo.blue = g_b32;
    DefaultVinfo.activate = FB_ACTIVATE_FORCE;
    DefaultVinfo.xres = DefaultVinfo.xres_virtual = width;
    DefaultVinfo.yres = height;
    DefaultVinfo.yres_virtual =height * 2;

    if(ioctl(FrameBufferFd, FBIOPUT_VSCREENINFO, &DefaultVinfo) < 0) {
        pr_err("FBIOPUT_VSCREENINFO failed!");
        goto end;
    }

    if (ioctl(FrameBufferFd, FBIOGET_FSCREENINFO, &fix) < 0) {
        pr_err("InitOsd FBIOGET_FSCREENINFO failed!");
        goto end;
    }

    pShowScreen = (unsigned char *)mmap(NULL, fix.smem_len, PROT_READ|PROT_WRITE,MAP_SHARED, FrameBufferFd, 0);
    if(MAP_FAILED == pShowScreen) {
        pr_err("mmap framebuffer failed!");
        goto end;
    }

    if (ioctl(FrameBufferFd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        pr_err("FBIOGET_VSCREENINFO failed!");
        goto end;
    }

    stAlpha.bAlphaEnable = HI_TRUE;
    stAlpha.bAlphaChannel = HI_FALSE;
    stAlpha.u8Alpha0 = 0;  // FILLBOX_TRANSPARENT_COLOR == 0
    stAlpha.u8Alpha1 = 0xff;
    stAlpha.u8GlobalAlpha = 0xff;
    if (ioctl(FrameBufferFd, FBIOPUT_ALPHA_HIFB, &stAlpha) < 0) {
        pr_err("FBIOPUT_ALPHA_HIFB failed\n");
        goto end;
    }

    bShow = HI_TRUE;
    if (ioctl(FrameBufferFd, FBIOPUT_SHOW_HIFB, &bShow) < 0) {
        pr_err("FBIOPUT_SHOW_HIFB failed\n");
        goto end;
    }

    close(FrameBufferFd);
    return HI_SUCCESS;

end:
    close(FrameBufferFd);
    return HI_FAILURE;
}

static HI_VOID AV_COMM_VI_SetMask(VI_DEV vi_dev, VI_DEV_ATTR_S *pstDevAttr)
{
    switch (vi_dev % 2) {
        case 0:
            if (VI_MODE_BT1120_STANDARD == pstDevAttr->enIntfMode)
            {
                pstDevAttr->au32CompMask[0] = 0xFF000000;
                pstDevAttr->au32CompMask[1] = 0xFF0000;
            } else if (VI_MODE_BT656 == pstDevAttr->enIntfMode) {
                pstDevAttr->au32CompMask[0] = 0xFF000000;
                pstDevAttr->au32CompMask[1] = 0x0;
            } else {
                pstDevAttr->au32CompMask[0] = 0x00FF0000;
                pstDevAttr->au32CompMask[1] = 0x0;
            }
            break;
        case 1:
            pstDevAttr->au32CompMask[0] = 0xFF000000;
            pstDevAttr->au32CompMask[1] = 0x0;
            break;
        default:
            break;
    }
}

HI_S32 AV_COMM_VI_StartDev(int vi_dev, int ViSeq, int IntfM)
{
    HI_S32 ret;

    VI_DEV_ATTR_S DEV_ATTR = {
            IntfM,                          //Interface mode
            VI_WORK_MODE_1Multiplex,        //1-, 2-, or 4-channel multiplexed mode
            { 0x00ff0000, 0 },              //Component mask
            VI_CLK_EDGE_SINGLE_UP,          //Clock edge mode
            { -1, -1, -1, -1 },             //Its value range is [–1, +3]. Typically, the default value –1 is recommended.
            ViSeq,                          //Input data sequence (only the YUV format is supported)
            {
                    /* port_vsync   port_vsync_neg     port_hsync        port_hsync_neg */
                    VI_VSYNC_FIELD, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,
                    VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,
                    {
                            0, 0, 0,                //hsync_hfb     hsync_act   hsync_hhb
                            0, 0, 0,                //vsync0_vhb    vsync0_act  vsync0_hhb
                            0, 0, 0                 //vsync1_vhb    vsync1_act  vsync1_hhb
                    }
            },
            VI_PATH_BYPASS,                 //This member is invalid for the Hi3536/Hi3521A/Hi3531A.
            VI_DATA_TYPE_YUV,               //Input data type. Typically, the input data from thesensor is RGB data,
                                            //and the input data from the ADC is YUV data.
            HI_TRUE
    };

    VI_DEV_ATTR_S stDevAttr;
    memset(&stDevAttr,0,sizeof(VI_DEV_ATTR_S));
    memcpy(&stDevAttr, &DEV_ATTR, sizeof(VI_DEV_ATTR_S));
    AV_COMM_VI_SetMask(vi_dev, &stDevAttr);

    ret = HI_MPI_VI_SetDevAttr(vi_dev, &stDevAttr);
    if (HI_SUCCESS != ret ) {
        pr_err();
        return HI_FAILURE;
    }

    ret = HI_MPI_VI_EnableDev(vi_dev);
    if (HI_SUCCESS != ret) {
        pr_err();
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 AV_COMM_VI_StartChn(VI_CHN vi_chn,  SIZE_S *pstDestSize)
{
    HI_S32 ret;
    VI_CHN_ATTR_S stChnAttr;

    memset(&stChnAttr, 0, sizeof(VI_CHN_ATTR_S));

    stChnAttr.stCapRect.s32X = 0;
    stChnAttr.stCapRect.s32Y = 0;
    stChnAttr.stCapRect.u32Width = pstDestSize->u32Width;
    stChnAttr.stCapRect.u32Height = pstDestSize->u32Height;
    stChnAttr.stDestSize.u32Width = pstDestSize->u32Width;
    stChnAttr.stDestSize.u32Height = pstDestSize->u32Height;
    stChnAttr.enCapSel = VI_CAPSEL_BOTH;
    stChnAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stChnAttr.bMirror = HI_FALSE;
    stChnAttr.bFlip = HI_FALSE;
    stChnAttr.s32SrcFrameRate = -1;
    stChnAttr.s32DstFrameRate = -1;
    stChnAttr.enScanMode = VI_SCAN_PROGRESSIVE;

    ret = HI_MPI_VI_SetChnAttr(vi_chn, &stChnAttr);
    if (ret != HI_SUCCESS) {
        pr_err();
        return HI_FAILURE;
    }

    ret = HI_MPI_VI_EnableChn(vi_chn);
    if (ret != HI_SUCCESS) {
        pr_err();
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static int vi_type = HI_VGA;

int hi_vi_init(void)
{
    int ret;
    int ViSeq, IntfM;
    SIZE_S stDestSize;

    if (vi_type == HI_VGA || vi_type == HI_HDMI || vi_type == HI_DVI) {
        IntfM = VI_MODE_BT1120_STANDARD;
        ViSeq = VI_INPUT_DATA_UYVY;
    } else if (vi_type == HI_CVBS) {
        IntfM = VI_MODE_BT656;
        ViSeq = VI_INPUT_DATA_UYVY;
    } else if (vi_type == HI_SDI) {
        IntfM = VI_MODE_BT1120_INTERLEAVED;
        ViSeq = VI_INPUT_DATA_UVUV;
    } else {
        pr_err("input vi_type invaled, support vga hdmi dvi cvbs sdi");
    }

    //vga : 6/24
    ret = HI_MPI_VI_DisableChn(24);
    if (HI_SUCCESS != ret) {
        pr_err("HI_MPI_VI_DisableChn failed\n");
        return HI_FAILURE;
    }

    ret = HI_MPI_VI_DisableDev(6);
    if (HI_SUCCESS != ret) {
        pr_err("HI_MPI_VI_DisableDev failed\n");
        return HI_FAILURE;
    }

    //Important set vi resolution
    stDestSize.u32Width = 1920;
    stDestSize.u32Height = 1080;

    /* Start VI Dev */
    ret = AV_COMM_VI_StartDev(6, ViSeq, IntfM);
    if (HI_SUCCESS != ret) {
        pr_err();
        return HI_FAILURE;
    }

    /* Start VI Chn */
    ret = AV_COMM_VI_StartChn(24, &stDestSize);
    if (HI_SUCCESS != ret) {
        pr_err();
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

int hi_vo_init(void)
{
    HI_S32 ret = HI_SUCCESS;
    VO_PUB_ATTR_S stPubAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;

    /* start VGA output */
    stPubAttr.u32BgColor = 0x00000000;
    stPubAttr.enIntfType = VO_INTF_VGA;
    stPubAttr.enIntfSync = VO_OUTPUT_1080P60;

    //vga : 0:1
    ret = HI_MPI_VO_SetPubAttr(0, &stPubAttr);
    if (ret != HI_SUCCESS) {
        pr_err();
        return HI_FAILURE;
    }

    ret = HI_MPI_VO_Enable(0);
    if (ret != HI_SUCCESS) {
        pr_err();
        return HI_FAILURE;
    }

    memset(&stLayerAttr, 0, sizeof(VO_VIDEO_LAYER_ATTR_S));
    stLayerAttr.enPixFormat             = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stLayerAttr.u32DispFrmRt            = 60;
    stLayerAttr.stDispRect.s32X         = 0;
    stLayerAttr.stDispRect.s32Y         = 0;
    stLayerAttr.stDispRect.u32Width     = 1920;
    stLayerAttr.stDispRect.u32Height    = 1080;
    stLayerAttr.stImageSize.u32Width    = 1920;
    stLayerAttr.stImageSize.u32Height   = 1080;

    ret = HI_MPI_VO_SetVideoLayerAttr(0, &stLayerAttr);
    if (ret != HI_SUCCESS) {
        pr_err();
        return HI_FAILURE;
    }

    ret = HI_MPI_VO_EnableVideoLayer(0);
    if (ret != HI_SUCCESS) {
        pr_err();
        return HI_FAILURE;
    }

    VO_CHN_ATTR_S stChnAttr;
    stChnAttr.stRect.s32X       = ALIGN_BACK(0, 2);
    stChnAttr.stRect.s32Y       = ALIGN_BACK(0, 2);
    stChnAttr.stRect.u32Width   = ALIGN_BACK(1920, 2);
    stChnAttr.stRect.u32Height  = ALIGN_BACK(1080, 2);
    stChnAttr.u32Priority       = 0;
    stChnAttr.bDeflicker        = HI_FALSE;

    //vga : 0:1
    ret = HI_MPI_VO_SetChnAttr(0, 1, &stChnAttr);
    if (ret != HI_SUCCESS) {
        pr_err("%s %d failed\n", __FILE__, __LINE__);
        return HI_FAILURE;
    }

    ret = HI_MPI_VO_EnableChn(0, 1);
    if (ret != HI_SUCCESS) {
        pr_err("%s %d failed\n", __FILE__, __LINE__);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}
