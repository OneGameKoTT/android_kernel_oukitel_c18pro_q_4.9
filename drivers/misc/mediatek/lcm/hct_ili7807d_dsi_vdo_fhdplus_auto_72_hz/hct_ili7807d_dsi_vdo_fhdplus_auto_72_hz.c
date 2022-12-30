
/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
	#include <string.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
	//#include <mach/mt_gpio.h>
#endif
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(1080)
#define FRAME_HEIGHT 										(2244)

#define LCM_ID_ILI7807D                						(0x780704)

#define REGFLAG_DELAY             							0xAB
#define REGFLAG_END_OF_TABLE      							0xAA   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE									0

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))


//static unsigned int lcm_esd_test = FALSE;      ///only for ESD test

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)										lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)
#define set_gpio_lcd_enp(cmd)                               lcm_util.set_gpio_lcd_enp_bias(cmd)
#define set_gpio_lcd_enn(cmd)                               lcm_util.set_gpio_lcd_enn_bias(cmd)
#define hct_lcm_power_settings(mode, value, mdelay1, mdelay2) 					lcm_util.hct_lcm_power_settings(mode, value, mdelay1, mdelay2)
#define set_gpio_tp_incell_rst(cmd)                         lcm_util.set_gpio_tp_incell_rst(cmd)

 struct LCM_setting_table {
	unsigned int cmd;
    unsigned char count;
    unsigned char para_list[64];
};

extern int RT5081_db_pos_neg_setting(void);
extern int RT5081_db_pos_neg_disable(void);
extern int PMU_db_pos_neg_setting_delay(int ms);
extern int PMU_db_pos_neg_setting_delay_hct(int ms, int vol);
extern int PMU_db_pos_neg_disable_delay(int ms);
static void push_table(struct LCM_setting_table *table, unsigned int count,
		unsigned char force_update)
{
	unsigned int i;

	for (i = 0; i < count; i++) {

		unsigned cmd;
		cmd = table[i].cmd;

		switch (cmd) {

		case REGFLAG_DELAY:
			MDELAY(table[i].count);
			break;

		case REGFLAG_END_OF_TABLE:
			break;

		default:
			dsi_set_cmdq_V2(cmd, table[i].count,
					table[i].para_list, force_update);
		}
	}

}
static struct LCM_setting_table lcm_initialization_setting[] = {
{0xFF, 3,{0x78,0x07,0x01}}, 	 //Page1
{0x42, 1,{0x11}}, 	 //VGH=x4 VGL=x4
{0x43, 1,{0xA3}}, 	 //VGH_CLP = 9.5V
{0x44, 1,{0xA8}}, 	 //VGL_CLP = -9V
{0x45, 1,{0x19}}, 	 //VGHO  = 8.5 V
{0x46, 1,{0x28}}, 	 //VGLO  = -8.0V
{0x4A, 1,{0x02}}, 	 //VSPR short to VSP
{0x4B, 1,{0x02}}, 	 //VSNR short to VSN
{0xB3, 1,{0x70}}, 	 //VGHO short VGH@r11
{0xB4, 1,{0x70}}, 	 //VGLO short VGL@r11
{0x50, 1,{0x64}},     //55   GVDDP 5F  = 5.0V  55=4.7---
{0x51, 1,{0x64}}, 	 //GVDDN  = -5.0V--
//{0xA1, 1,{0x20}},   //烧录了VCOM需要屏蔽此寄存器才有效
{0xA2, 1,{0x01}}, 	 //VCOM1 --
{0xA3, 1,{0x0F}},	 //VCOM1 --
{0xFF, 3,{0x78,0x07,0x01}}, 	 //Page1
{0x22, 1,{0x06}}, 	 //SS&NW
{0x36, 1,{0x00}},          //2 mux 6
{0x63, 1,{0x04}},          //SDT
{0x64, 1,{0x08}}, 
{0x6C, 1,{0x45}}, 	 //PRC & PRCB
{0x6D, 1,{0x00}}, 	 //PCT2
{0x5A, 1,{0x33}}, 	 //LVD setting
{0x35, 1,{0x22}}, 
{0xFF, 3,{0x78,0x07,0x02}}, 	 //Page2
{0x00, 1,{0x00}}, 
{0x01, 1,{0x51}},	
{0x02, 1,{0x00}}, 
{0x03, 1,{0x52}},	
{0x04, 1,{0x00}}, 
{0x05, 1,{0x6B}},	
{0x06, 1,{0x00}}, 
{0x07, 1,{0x84}},	
{0x08, 1,{0x00}}, 
{0x09, 1,{0x9B}},	
{0x0A, 1,{0x00}}, 
{0x0B, 1,{0xAF}},	
{0x0C, 1,{0x00}}, 
{0x0D, 1,{0xC1}},	
{0x0E, 1,{0x00}}, 
{0x0F, 1,{0xD1}},	
{0x10, 1,{0x00}},	
{0x11, 1,{0xE0}},	
{0x12, 1,{0x01}}, 
{0x13, 1,{0x13}},	
{0x14, 1,{0x01}}, 
{0x15, 1,{0x3B}},	
{0x16, 1,{0x01}}, 
{0x17, 1,{0x78}},	
{0x18, 1,{0x01}}, 
{0x19, 1,{0xA7}},	
{0x1A, 1,{0x01}}, 
{0x1B, 1,{0xEF}},	
{0x1C, 1,{0x02}},  // 128
{0x1D, 1,{0x2A}},	
{0x1E, 1,{0x02}}, 
{0x1F, 1,{0x2B}},	
{0x20, 1,{0x02}},	
{0x21, 1,{0x63}},	
{0x22, 1,{0x02}},	
{0x23, 1,{0xA3}},	
{0x24, 1,{0x02}},	
{0x25, 1,{0xCC}},	
{0x26, 1,{0x03}},	
{0x27, 1,{0x00}},	
{0x28, 1,{0x03}},	
{0x29, 1,{0x22}},	
{0x2A, 1,{0x03}},	
{0x2B, 1,{0x4C}},	
{0x2C, 1,{0x03}},	
{0x2D, 1,{0x58}},	
{0x2E, 1,{0x03}},	
{0x2F, 1,{0x66}},	
{0x30, 1,{0x03}},	
{0x31, 1,{0x76}},	
{0x32, 1,{0x03}},	
{0x33, 1,{0x87}},	
{0x34, 1,{0x03}},	
{0x35, 1,{0x9B}},	
{0x36, 1,{0x03}},	
{0x37, 1,{0xB1}},	
{0x38, 1,{0x03}},	
{0x39, 1,{0xC7}},	
{0x3A, 1,{0x03}},	
{0x3B, 1,{0xC8}},	
{0x3C, 1,{0x00}},	
{0x3D, 1,{0x51}},	
{0x3E, 1,{0x00}},	
{0x3F, 1,{0x52}},	
{0x40, 1,{0x00}},	
{0x41, 1,{0x6B}},	
{0x42, 1,{0x00}},	
{0x43, 1,{0x84}},	
{0x44, 1,{0x00}},	
{0x45, 1,{0x9B}},	
{0x46, 1,{0x00}},	
{0x47, 1,{0xAF}},	
{0x48, 1,{0x00}},	
{0x49, 1,{0xC1}},	
{0x4A, 1,{0x00}},	
{0x4B, 1,{0xD1}},	
{0x4C, 1,{0x00}},	
{0x4D, 1,{0xE0}},	
{0x4E, 1,{0x01}},	
{0x4F, 1,{0x13}},	
{0x50, 1,{0x01}},	
{0x51, 1,{0x3B}},	
{0x52, 1,{0x01}},	
{0x53, 1,{0x78}},	
{0x54, 1,{0x01}},	
{0x55, 1,{0xA7}},	
{0x56, 1,{0x01}},	
{0x57, 1,{0xEF}},	
{0x58, 1,{0x02}},	
{0x59, 1,{0x2A}},	
{0x5A, 1,{0x02}},	
{0x5B, 1,{0x2B}},	
{0x5C, 1,{0x02}},	
{0x5D, 1,{0x63}},	
{0x5E, 1,{0x02}},	
{0x5F, 1,{0xA3}},	
{0x60, 1,{0x02}},	
{0x61, 1,{0xCC}},	
{0x62, 1,{0x03}},	
{0x63, 1,{0x00}},	
{0x64, 1,{0x03}},	
{0x65, 1,{0x22}},	
{0x66, 1,{0x03}},	
{0x67, 1,{0x4C}},	
{0x68, 1,{0x03}},	
{0x69, 1,{0x58}},	
{0x6A, 1,{0x03}},	
{0x6B, 1,{0x66}},	
{0x6C, 1,{0x03}},	
{0x6D, 1,{0x76}},	
{0x6E, 1,{0x03}},	
{0x6F, 1,{0x87}},	
{0x70, 1,{0x03}},	
{0x71, 1,{0x9B}},	
{0x72, 1,{0x03}},	
{0x73, 1,{0xB1}},	
{0x74, 1,{0x03}},	
{0x75, 1,{0xC7}},	
{0x76, 1,{0x03}},	
{0x77, 1,{0xC8}},	
{0x78, 1,{0x01}},
{0x79, 1,{0x01}},	

{0xFF, 3,{0x78,0x07,0x03}},	 //Page3
{0x00, 1,{0x00}},
{0x01, 1,{0xA8}},
{0x02, 1,{0x00}},
{0x03, 1,{0xB2}},
{0x04, 1,{0x00}},
{0x05, 1,{0xC5}},
{0x06, 1,{0x00}},
{0x07, 1,{0xD5}},
{0x08, 1,{0x00}},
{0x09, 1,{0xE4}},
{0x0A, 1,{0x00}},
{0x0B, 1,{0xF3}},
{0x0C, 1,{0x01}},
{0x0D, 1,{0x00}},
{0x0E, 1,{0x01}},
{0x0F, 1,{0x0C}},
{0x10, 1,{0x01}},
{0x11, 1,{0x17}},
{0x12, 1,{0x01}},
{0x13, 1,{0x40}},
{0x14, 1,{0x01}},
{0x15, 1,{0x60}},
{0x16, 1,{0x01}},
{0x17, 1,{0x94}},
{0x18, 1,{0x01}},
{0x19, 1,{0xBE}},
{0x1A, 1,{0x02}},
{0x1B, 1,{0x00}},
{0x1C, 1,{0x02}},
{0x1D, 1,{0x38}},
{0x1E, 1,{0x02}},
{0x1F, 1,{0x39}},
{0x20, 1,{0x02}},
{0x21, 1,{0x70}},
{0x22, 1,{0x02}},
{0x23, 1,{0xAE}},
{0x24, 1,{0x02}},
{0x25, 1,{0xD6}},
{0x26, 1,{0x03}},
{0x27, 1,{0x09}},
{0x28, 1,{0x03}},
{0x29, 1,{0x2A}},
{0x2A, 1,{0x03}},
{0x2B, 1,{0x53}},
{0x2C, 1,{0x03}},
{0x2D, 1,{0x5F}},
{0x2E, 1,{0x03}},
{0x2F, 1,{0x6D}},
{0x30, 1,{0x03}},
{0x31, 1,{0x7C}},
{0x32, 1,{0x03}},
{0x33, 1,{0x8C}},
{0x34, 1,{0x03}},
{0x35, 1,{0x9F}},
{0x36, 1,{0x03}},
{0x37, 1,{0xB4}},
{0x38, 1,{0x03}},
{0x39, 1,{0xC6}},
{0x3A, 1,{0x03}},
{0x3B, 1,{0xC8}},
{0x3C, 1,{0x00}},
{0x3D, 1,{0xA8}},
{0x3E, 1,{0x00}},
{0x3F, 1,{0xB2}},
{0x40, 1,{0x00}},
{0x41, 1,{0xC5}},
{0x42, 1,{0x00}},
{0x43, 1,{0xD5}},
{0x44, 1,{0x00}},
{0x45, 1,{0xE4}},
{0x46, 1,{0x00}},
{0x47, 1,{0xF3}},
{0x48, 1,{0x01}},
{0x49, 1,{0x00}},
{0x4A, 1,{0x01}},
{0x4B, 1,{0x0C}},
{0x4C, 1,{0x01}},
{0x4D, 1,{0x17}},
{0x4E, 1,{0x01}},
{0x4F, 1,{0x40}},
{0x50, 1,{0x01}},
{0x51, 1,{0x60}},
{0x52, 1,{0x01}},
{0x53, 1,{0x94}},
{0x54, 1,{0x01}},
{0x55, 1,{0xBE}},
{0x56, 1,{0x02}},
{0x57, 1,{0x00}},
{0x58, 1,{0x02}},
{0x59, 1,{0x38}},
{0x5A, 1,{0x02}},
{0x5B, 1,{0x39}},
{0x5C, 1,{0x02}},
{0x5D, 1,{0x70}},
{0x5E, 1,{0x02}},
{0x5F, 1,{0xAE}},
{0x60, 1,{0x02}},
{0x61, 1,{0xD6}},
{0x62, 1,{0x03}},
{0x63, 1,{0x09}},
{0x64, 1,{0x03}},
{0x65, 1,{0x2A}},
{0x66, 1,{0x03}},
{0x67, 1,{0x53}},
{0x68, 1,{0x03}},
{0x69, 1,{0x5F}},
{0x6A, 1,{0x03}},
{0x6B, 1,{0x6D}},
{0x6C, 1,{0x03}},
{0x6D, 1,{0x7C}},
{0x6E, 1,{0x03}},
{0x6F, 1,{0x8C}},
{0x70, 1,{0x03}},
{0x71, 1,{0x9F}},
{0x72, 1,{0x03}},
{0x73, 1,{0xB4}},
{0x74, 1,{0x03}},
{0x75, 1,{0xC6}},
{0x76, 1,{0x03}},
{0x77, 1,{0xC8}},
{0x78, 1,{0x01}},
{0x79, 1,{0x01}},	

{0xFF, 3,{0x78,0x07,0x04}},	 //Page4
{0x00, 1,{0x00}},
{0x01, 1,{0xCA}},
{0x02, 1,{0x00}},
{0x03, 1,{0xD2}},
{0x04, 1,{0x00}},
{0x05, 1,{0xE3}},
{0x06, 1,{0x00}},
{0x07, 1,{0xF1}},
{0x08, 1,{0x00}},
{0x09, 1,{0xFF}},
{0x0A, 1,{0x01}},
{0x0B, 1,{0x0B}},
{0x0C, 1,{0x01}},
{0x0D, 1,{0x17}},
{0x0E, 1,{0x01}},
{0x0F, 1,{0x22}},
{0x10, 1,{0x01}},
{0x11, 1,{0x2D}},
{0x12, 1,{0x01}},
{0x13, 1,{0x51}},
{0x14, 1,{0x01}},
{0x15, 1,{0x6F}},
{0x16, 1,{0x01}},
{0x17, 1,{0xA0}},
{0x18, 1,{0x01}},
{0x19, 1,{0xC7}},
{0x1A, 1,{0x02}},
{0x1B, 1,{0x06}},
{0x1C, 1,{0x02}},
{0x1D, 1,{0x3B}},
{0x1E, 1,{0x02}},
{0x1F, 1,{0x3D}},
{0x20, 1,{0x02}},
{0x21, 1,{0x72}},
{0x22, 1,{0x02}},
{0x23, 1,{0xB0}},
{0x24, 1,{0x02}},
{0x25, 1,{0xD7}},
{0x26, 1,{0x03}},
{0x27, 1,{0x0A}},
{0x28, 1,{0x03}},
{0x29, 1,{0x2A}},
{0x2A, 1,{0x03}},
{0x2B, 1,{0x53}},
{0x2C, 1,{0x03}},
{0x2D, 1,{0x60}},
{0x2E, 1,{0x03}},
{0x2F, 1,{0x6D}},
{0x30, 1,{0x03}},
{0x31, 1,{0x7C}},
{0x32, 1,{0x03}},
{0x33, 1,{0x8C}},
{0x34, 1,{0x03}},
{0x35, 1,{0x9F}},
{0x36, 1,{0x03}},
{0x37, 1,{0xB4}},
{0x38, 1,{0x03}},
{0x39, 1,{0xC6}},
{0x3A, 1,{0x03}},
{0x3B, 1,{0xC8}},
{0x3C, 1,{0x00}},
{0x3D, 1,{0xCA}},
{0x3E, 1,{0x00}},
{0x3F, 1,{0xD2}},
{0x40, 1,{0x00}},
{0x41, 1,{0xE3}},
{0x42, 1,{0x00}},
{0x43, 1,{0xF1}},
{0x44, 1,{0x00}},
{0x45, 1,{0xFF}},
{0x46, 1,{0x01}},
{0x47, 1,{0x0B}},
{0x48, 1,{0x01}},
{0x49, 1,{0x17}},
{0x4A, 1,{0x01}},
{0x4B, 1,{0x22}},
{0x4C, 1,{0x01}},
{0x4D, 1,{0x2D}},
{0x4E, 1,{0x01}},
{0x4F, 1,{0x51}},
{0x50, 1,{0x01}},
{0x51, 1,{0x6F}},
{0x52, 1,{0x01}},
{0x53, 1,{0xA0}},
{0x54, 1,{0x01}},
{0x55, 1,{0xC7}},
{0x56, 1,{0x02}},
{0x57, 1,{0x06}},
{0x58, 1,{0x02}},
{0x59, 1,{0x3B}},
{0x5A, 1,{0x02}},
{0x5B, 1,{0x3D}},
{0x5C, 1,{0x02}},
{0x5D, 1,{0x72}},
{0x5E, 1,{0x02}},
{0x5F, 1,{0xB0}},
{0x60, 1,{0x02}},
{0x61, 1,{0xD7}},
{0x62, 1,{0x03}},
{0x63, 1,{0x0A}},
{0x64, 1,{0x03}},
{0x65, 1,{0x2A}},
{0x66, 1,{0x03}},
{0x67, 1,{0x53}},
{0x68, 1,{0x03}},
{0x69, 1,{0x60}},
{0x6A, 1,{0x03}},
{0x6B, 1,{0x6D}},
{0x6C, 1,{0x03}},
{0x6D, 1,{0x7C}},
{0x6E, 1,{0x03}},
{0x6F, 1,{0x8C}},
{0x70, 1,{0x03}},
{0x71, 1,{0x9F}},
{0x72, 1,{0x03}},
{0x73, 1,{0xB4}},
{0x74, 1,{0x03}},
{0x75, 1,{0xC6}},
{0x76, 1,{0x03}},
{0x77, 1,{0xC8}},
{0x78, 1,{0x01}},
{0x79, 1,{0x01}}, 
{0xFF, 3,{0x78,0x07,0x06}}, 
{0x00, 1,{0x43}}, 
{0x01, 1,{0x22}}, 
{0x02, 1,{0x3d}}, 
{0x03, 1,{0x3d}}, 
{0x04, 1,{0x03}}, 
{0x05, 1,{0x12}}, 
{0x06, 1,{0x0b}}, 
{0x07, 1,{0x0b}}, 
{0x08, 1,{0x83}}, 
{0x09, 1,{0x00}}, 
{0x0a, 1,{0x30}}, 
{0x0b, 1,{0x10}}, 
{0x0c, 1,{0x0b}}, 
{0x0d, 1,{0x0b}}, 
{0x0e, 1,{0x00}}, 
{0x0f, 1,{0x00}}, 
{0x10, 1,{0x00}}, 
{0x11, 1,{0x00}}, 
{0x12, 1,{0x00}}, 
{0x13, 1,{0x00}}, 
{0x14, 1,{0x84}}, 
{0x15, 1,{0x84}}, 
{0x31, 1,{0x08}}, 
{0x32, 1,{0x01}}, 
{0x33, 1,{0x00}}, 
{0x34, 1,{0x11}}, 
{0x35, 1,{0x13}}, 
{0x36, 1,{0x26}}, 
{0x37, 1,{0x22}}, 
{0x38, 1,{0x22}}, 
{0x39, 1,{0x0c}}, 
{0x3a, 1,{0x02}}, 
{0x3b, 1,{0x02}}, 
{0x3c, 1,{0x02}}, 
{0x3d, 1,{0x02}}, 
{0x3e, 1,{0x28}}, 
{0x3f, 1,{0x29}}, 
{0x40, 1,{0x2a}}, 
{0x41, 1,{0x08}}, 
{0x42, 1,{0x01}}, 
{0x43, 1,{0x00}}, 
{0x44, 1,{0x10}}, 
{0x45, 1,{0x12}}, 
{0x46, 1,{0x26}}, 
{0x47, 1,{0x22}}, 
{0x48, 1,{0x22}}, 
{0x49, 1,{0x0c}}, 
{0x4a, 1,{0x02}}, 
{0x4b, 1,{0x02}}, 
{0x4c, 1,{0x02}}, 
{0x4d, 1,{0x02}}, 
{0x4e, 1,{0x28}}, 
{0x4f, 1,{0x29}}, 
{0x50, 1,{0x2a}}, 
{0x61, 1,{0x0c}}, 
{0x62, 1,{0x01}}, 
{0x63, 1,{0x00}}, 
{0x64, 1,{0x12}}, 
{0x65, 1,{0x10}}, 
{0x66, 1,{0x26}}, 
{0x67, 1,{0x22}}, 
{0x68, 1,{0x22}}, 
{0x69, 1,{0x08}}, 
{0x6a, 1,{0x02}}, 
{0x6b, 1,{0x02}}, 
{0x6c, 1,{0x02}}, 
{0x6d, 1,{0x02}}, 
{0x6e, 1,{0x28}}, 
{0x6f, 1,{0x29}}, 
{0x70, 1,{0x2a}}, 
{0x71, 1,{0x0c}}, 
{0x72, 1,{0x01}}, 
{0x73, 1,{0x00}}, 
{0x74, 1,{0x13}}, 
{0x75, 1,{0x11}}, 
{0x76, 1,{0x26}}, 
{0x77, 1,{0x22}}, 
{0x78, 1,{0x22}}, 
{0x79, 1,{0x08}}, 
{0x7a, 1,{0x02}}, 
{0x7b, 1,{0x02}}, 
{0x7c, 1,{0x02}}, 
{0x7d, 1,{0x02}}, 
{0x7e, 1,{0x28}}, 
{0x7f, 1,{0x29}}, 
{0x80, 1,{0x2a}}, 
{0x96, 1,{0x80}}, 
{0x97, 1,{0x22}}, 
{0xd0, 1,{0x01}}, 
{0xd1, 1,{0x11}}, 
{0xd2, 1,{0x10}}, 
{0xd3, 1,{0x15}}, 
{0xd4, 1,{0x10}}, 
{0xd5, 1,{0x01}}, 
{0xd6, 1,{0x00}}, 
{0xd7, 1,{0x00}}, 
{0xd8, 1,{0x00}}, 
{0xd9, 1,{0x00}}, 
{0xda, 1,{0x00}}, 
{0xdb, 1,{0x47}}, 
{0xdc, 1,{0x02}}, 
{0xdd, 1,{0x55}}, 
{0xe5, 1,{0x98}}, 
{0xe6, 1,{0x70}}, 
{0xa0, 1,{0x0F}}, 
{0xa2, 1,{0x09}}, 
{0xa3, 1,{0x19}}, 
{0xa6, 1,{0x32}}, 
{0xa7, 1,{0x00}}, 
{0xb2, 1,{0x22}}, 
{0xb3, 1,{0x22}}, 
{0xFF, 3,{0x78,0x07,0x01}}, 
{0x63, 1,{0x04}}, 
{0x2E, 1,{0x61}},    //2244 Lines
{0x2F, 1,{0x04}},    //2244 Lines
{0xFF, 3,{0x78,0x07,0x07}}, 
{0x32, 1,{0x39}}, 
{0x37, 1,{0x00}}, 
{0x12, 1,{0x22}}, 
{0xFF, 3,{0x78,0x07,0x00}}, 
{0x35, 1,{0x00}},    
{0x11, 1,{0x00}},
{REGFLAG_DELAY, 120, {0}},
{0x29, 1,{0x00}},
{REGFLAG_DELAY, 20, {0}},
{REGFLAG_END_OF_TABLE, 0x00, {}}
};


/*
static struct LCM_setting_table lcm_sleep_out_setting[] = {
	// Sleep Out
	{0x11, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},

	// Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 10, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
*/
/*
static struct LCM_setting_table lcm_sleep_in_setting[] = {
	// Display off sequence
	{0x01, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},
	
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},

	// Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
*/


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS * util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS * params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type = LCM_TYPE_DSI;

	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

	// enable tearing-free
	params->dbi.te_mode = LCM_DBI_TE_MODE_DISABLED;
	params->dbi.te_edge_polarity = LCM_POLARITY_RISING;

#if (LCM_DSI_CMD_MODE)
	params->dsi.mode = CMD_MODE;
#else
	params->dsi.mode   = SYNC_PULSE_VDO_MODE;//SYNC_EVENT_VDO_MODE;//BURST_VDO_MODE;////
#endif

	// DSI
	/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
	
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;
	
	
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	
#if (LCM_DSI_CMD_MODE)
	params->dsi.intermediat_buffer_num = 0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage
	params->dsi.word_count=FRAME_WIDTH*3;	//DSI CMD mode need set these two bellow params, different to 6577
#else
	params->dsi.intermediat_buffer_num = 0;	//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage
#endif

  // Video mode setting

    params->dsi.packet_size=256;

    params->physical_width = 78;
    params->physical_height = 163;
 
    params->dsi.vertical_sync_active                =  8;//2

    params->dsi.vertical_backporch                    =16;//16 25 30 35 12 8 8

    params->dsi.vertical_frontporch                    = 24;

    params->dsi.vertical_active_line                = FRAME_HEIGHT; 

 

    params->dsi.horizontal_sync_active                =40 ;//56 30

    params->dsi.horizontal_backporch                =120; //104 85

    params->dsi.horizontal_frontporch                =120 ;//20 20

    params->dsi.horizontal_active_pixel                = FRAME_WIDTH;

 

 

    params->dsi.PLL_CLOCK=599;//230 450  550 484 590

	//params->dsi.cont_clock = 1;
	//params->dsi.clk_lp_per_line_enable = 1;
	params->dsi.ssc_disable = 1;	
	
	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
    params->dsi.lcm_esd_check_table[0].cmd          = 0x0a;
    params->dsi.lcm_esd_check_table[0].count        = 1;
    params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9c;


}

static void lcm_init(void)
{
	 
  //  SET_RESET_PIN(0);
    MDELAY(10);
    set_gpio_lcd_enp(1);//3.3V

	hct_lcm_power_settings(HCT_LCM_POWER_MODE1_RT5081, 1, 15, 10);	
	set_gpio_lcd_enn(1);  //1.8
    MDELAY(10);
 //   PMU_db_pos_neg_setting_delay(10);

    MDELAY(5);

    SET_RESET_PIN(1);
    MDELAY(5);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(120);

	push_table(lcm_initialization_setting,sizeof(lcm_initialization_setting) /sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{
	unsigned int array[5];
 	//array[0] = 0x00011500;// read id return two byte,version and id
	//dsi_set_cmdq(array, 1, 1);
	//MDELAY(100);
	/*array[0] = 0x01FE1500;
	dsi_set_cmdq(array, 1, 1);
	MDELAY(5);
	array[0] = 0x00461500;
	dsi_set_cmdq(array, 1, 1);
	MDELAY(5);*/
	array[0] = 0x00280500;
	dsi_set_cmdq(array, 1, 1);
	MDELAY(20);
 	array[0] = 0x00100500;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);
	MDELAY(120);

	SET_RESET_PIN(0);
	MDELAY(20);
    set_gpio_lcd_enp(0);//3.3V
    MDELAY(10);
 
    set_gpio_lcd_enn(0);
    MDELAY(10);
RT5081_db_pos_neg_disable();
}

static unsigned int lcm_compare_id(void);
static void lcm_resume(void)
{
	lcm_init();
}


static unsigned int lcm_compare_id(void)
{

	int array[4];
     char buffer[5];
     char id_high=0;
     char id_midd=0;
     char id_low=0;
     int id=0;

 
	SET_RESET_PIN(1);
     MDELAY(20);   
	SET_RESET_PIN(0);
	MDELAY(20);
	SET_RESET_PIN(1);
	MDELAY(120);
 
	//enable CMD2 Page6
     array[0]=0x00043902;
     array[1]=0x010778ff;
     dsi_set_cmdq(array, 2, 1);
     MDELAY(10);
 
 
     array[0]=0x00033700;
     dsi_set_cmdq(array, 1, 1);
 
     read_reg_v2(0x00, buffer,1);
     id_high = buffer[0]; ///////////////////////0x98
 
     read_reg_v2(0x01, buffer,1);
     id_midd = buffer[0]; ///////////////////////0x81
 
     read_reg_v2(0x02, buffer,1);
     id_low = buffer[0]; ////////////////////////0x0d
 
     id =(id_high << 16) | (id_midd << 8) | id_low;
 
     #if defined(BUILD_LK)
     printf("ILI7807D compare-LK:0x%02x,0x%02x,0x%02x,0x%02x\n", id_high, id_midd, id_low, id);
     #else
     printk("ILI7807D compare:0x%02x,0x%02x,0x%02x,0x%02x\n", id_high, id_midd, id_low, id);
     #endif
 
     return (id == LCM_ID_ILI7807D)?1:0;}

LCM_DRIVER hct_ili7807d_dsi_vdo_fhdplus_auto_72_hz = 
{
	.name			= "hct_ili7807d_dsi_vdo_fhdplus_auto_72_hz",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,	
	.compare_id     = lcm_compare_id,	

#if (LCM_DSI_CMD_MODE)
    //.update         = lcm_update,
#endif	//wqtao
};
