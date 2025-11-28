#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <assert.h>


#include "hifb.h"
#include "hi_unf_disp.h"
#include "hi_adp_data.h"
#include "hi_adp_mpi.h"

#ifndef CONFIG_SUPPORT_CA_RELEASE
#define Printf  printf
#else
#define Printf(x...)
#endif


// static struct fb_var_screeninfo ghifb_st_def_vinfo =
// {
//     1920, 1080, 
//     1920, 1080,
//     0, 0,
//     32, 0,
//     {16, 8, 0}, {8, 8, 0}, {0, 8, 0}, {24, 8, 0},
//     0, FB_ACTIVATE_FORCE,
//     0, 0, 0, -1, -1, -1, -1, -1, -1, -1
// };

static struct fb_var_screeninfo ghifb_st_def_vinfo =
{
    1920, 1080, 
    1920, 1080,
    0, 0,
    32, 0,
    {16, 8, 0}, {8, 8, 0}, {0, 8, 0}, {24, 8, 0},
    0, FB_ACTIVATE_FORCE,
    0, 0, 1, 6734, 148, 88, 36, 4, 44, 5
};

static void print_vinfo(struct fb_var_screeninfo *vinfo)
{
    Printf( "Printing vinfo:\n");
    Printf("txres: %d\n", vinfo->xres);
    Printf( "tyres: %d\n", vinfo->yres);
    Printf( "txres_virtual: %d\n", vinfo->xres_virtual);
    Printf( "tyres_virtual: %d\n", vinfo->yres_virtual);
    Printf( "txoffset: %d\n", vinfo->xoffset);
    Printf( "tyoffset: %d\n", vinfo->yoffset);
    Printf( "tbits_per_pixel: %d\n", vinfo->bits_per_pixel);
    Printf( "tgrayscale: %d\n", vinfo->grayscale);
    Printf( "tnonstd: %d\n", vinfo->nonstd);
    Printf( "tactivate: %d\n", vinfo->activate);
    Printf( "theight: %d\n", vinfo->height);
    Printf( "twidth: %d\n", vinfo->width);
    Printf( "taccel_flags: %d\n", vinfo->accel_flags);
    Printf( "tpixclock: %d\n", vinfo->pixclock);
    Printf( "tleft_margin: %d\n", vinfo->left_margin);
    Printf( "tright_margin: %d\n", vinfo->right_margin);
    Printf( "tupper_margin: %d\n", vinfo->upper_margin);
    Printf( "tlower_margin: %d\n", vinfo->lower_margin);
    Printf( "thsync_len: %d\n", vinfo->hsync_len);
    Printf( "tvsync_len: %d\n", vinfo->vsync_len);
    Printf( "tsync: %d\n", vinfo->sync);
    Printf( "tvmode: %d\n", vinfo->vmode);
    Printf( "tred: %d/%d\n", vinfo->red.length, vinfo->red.offset);
    Printf( "tgreen: %d/%d\n", vinfo->green.length, vinfo->green.offset);
    Printf( "tblue: %d/%d\n", vinfo->blue.length, vinfo->blue.offset);
    Printf( "talpha: %d/%d\n", vinfo->transp.length, vinfo->transp.offset);
}

static void print_finfo(struct fb_fix_screeninfo *finfo)
{
    Printf( "Printing finfo:\n");
    Printf( "tsmem_start = %p\n", (char *)finfo->smem_start);
    Printf( "tsmem_len = %d\n", finfo->smem_len);
    Printf( "ttype = %d\n", finfo->type);
    Printf( "ttype_aux = %d\n", finfo->type_aux);
    Printf( "tvisual = %d\n", finfo->visual);
    Printf( "txpanstep = %d\n", finfo->xpanstep);
    Printf( "typanstep = %d\n", finfo->ypanstep);
    Printf( "tywrapstep = %d\n", finfo->ywrapstep);
    Printf( "tline_length = %d\n", finfo->line_length);
    Printf( "tmmio_start = %p\n", (char *)finfo->mmio_start);
    Printf( "tmmio_len = %d\n", finfo->mmio_len);
    Printf( "taccel = %d\n", finfo->accel);
}


int main(int argc, char* argv[])
{
    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo vinfo;
    // HIFB_LAYER_INFO_S layerinfo;
    // HIFB_BUFFER_S CanvasBuf;
    // HI_U32 u32BufSize = 0;
    // HI_U32 u32DisPlayBufSize = 0;
    // HI_U32 u32LineLen = 0;
    HIFB_ALPHA_S stAlpha;
    int console_fd;
    // HIFB_RECT stRect;
    int ret = 0;
    // HI_U32 u32Bpp;
#ifdef CFG_HIFB_ANDRIOD
    HI_BOOL bDecmp = HI_FALSE;
#endif
    const char* file = "/dev/fb0";

    /* open fb device */
    if (argc >= 2)
    {
        file = argv[1];
    }


    HI_SYS_Init();

    HIADP_MCE_Exit();

    // ret = HIADP_Disp_Init(HI_UNF_ENC_FMT_720P_50);
    ret = HIADP_Disp_Init(HI_UNF_ENC_FMT_1080P_60);

    if (ret != HI_SUCCESS)
    {
        return 0;
    }


    console_fd = open(file, O_RDWR, 0);
    if (console_fd < 0)
    {
        Printf ( "Unable to open %s\n", file);
        // PrintfCMDInfo();
        return (-1);
    }

    /* set color format ARGB8888, screen size: 1280*720 */
    if (ioctl(console_fd, FBIOPUT_VSCREENINFO, &ghifb_st_def_vinfo) < 0)
    {
        Printf ( "Unable to set variable screeninfo!\n");
        ret = -1;
        close(console_fd);
        return ret;
    }

    

    /* Get the fix screen info of hardware */
    if (ioctl(console_fd, FBIOGET_FSCREENINFO, &finfo) < 0)
    {
        Printf ( "Couldn't get console hardware info\n");
        ret = -1;
        close(console_fd);
        return ret;
    }

    print_finfo(&finfo);

    /*get  Determine the current screen depth */
    if (ioctl(console_fd, FBIOGET_VSCREENINFO, &vinfo) < 0)
    {
        Printf ( "Couldn't get vscreeninfo\n");
        ret = -1;
        // goto UNMAP;
        return ret;
    }


    print_vinfo(&vinfo);

    /* set alpha */
    stAlpha.bAlphaEnable  = HI_TRUE;
    stAlpha.u8Alpha0 = 0xff;
    stAlpha.u8Alpha1 = 0xff;

    /* set global alpha */
    stAlpha.bAlphaChannel = HI_TRUE;
    stAlpha.u8GlobalAlpha = 0xff;//0xf0,0x80..

    if (ioctl(console_fd, FBIOPUT_ALPHA_HIFB, &stAlpha) < 0)
    {
        Printf ( "Couldn't set alpha\n");
        ret = -1;
        // goto UNMAP;
        return ret;
    }


    pause();




    // HIADP_Disp_DeInit();
    // HI_SYS_DeInit();

    return ret;
}

    
