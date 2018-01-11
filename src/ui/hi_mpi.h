/******************************************************************************

  Copyright (C), 2018, Vision Soft. Co., Ltd.

 ******************************************************************************
  File Name     : hi_md.h
  Author        : Hisilicon multimedia software (IVE) group
  Created       : 2018/01/09
  Author		: backup4mark@gmail.com
******************************************************************************/

#ifndef _HI_MPI_H_
#define _HI_MPI_H_

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

enum vitype {
    HI_VGA = 0,
    HI_HDMI,
    HI_DVI,
    HI_CVBS,
    HI_SDI
};

#define VO_INTF_VGA      (0x01L<<2)
#define VO_INTF_BT656    (0x01L<<3)
#define VO_INTF_BT1120   (0x01L<<4)
#define VO_INTF_HDMI     (0x01L<<5)

int hi_sys_init(void);
void hi_sys_exit(void);
int vo_bind_vi(void);
int fb_alloc(void);
int hi_vo_init(void);
int hi_vi_init(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif