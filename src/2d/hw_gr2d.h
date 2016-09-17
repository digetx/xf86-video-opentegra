/*
 * Copyright (C) 2012-2013 NVIDIA Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

 /*
  * Function naming determines intended use:
  *
  *     <x>_r(void) : Returns the offset for register <x>.
  *
  *     <x>_w(void) : Returns the word offset for word (4 byte) element <x>.
  *
  *     <x>_<y>_s(void) : Returns size of field <y> of register <x> in bits.
  *
  *     <x>_<y>_f(uint32_t v) : Returns a value based on 'v' which has been shifted
  *         and masked to place it at field <y> of register <x>.  This value
  *         can be |'d with others to produce a full register value for
  *         register <x>.
  *
  *     <x>_<y>_m(void) : Returns a mask for field <y> of register <x>.  This
  *         value can be ~'d and then &'d to clear the value of field <y> for
  *         register <x>.
  *
  *     <x>_<y>_<z>_f(void) : Returns the constant value <z> after being shifted
  *         to place it at field <y> of register <x>.  This value can be |'d
  *         with others to produce a full register value for <x>.
  *
  *     <x>_<y>_v(uint32_t r) : Returns the value of field <y> from a full register
  *         <x> value 'r' after being shifted to place its LSB at bit 0.
  *         This value is suitable for direct comparison with other unshifted
  *         values appropriate for use in field <y> of register <x>.
  *
  *     <x>_<y>_<z>_v(void) : Returns the constant value for <z> defined for
  *         field <y> of register <x>.  This value is suitable for direct
  *         comparison with unshifted values appropriate for use in field <y>
  *         of register <x>.
  */

#ifndef HW_GR2D_H__
#define HW_GR2D_H__

static inline uint32_t gr2d_trigger_r(void)
{
    return 0x9;
}

static inline uint32_t gr2d_trigger_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_trigger_trigger_s(void)
{
    return 16;
}

static inline uint32_t gr2d_trigger_trigger_f(uint32_t v)
{
    return (v & 0xffff) << 0;
}

static inline uint32_t gr2d_trigger_trigger_m(void)
{
    return 0xffff << 0;
}

static inline uint32_t gr2d_trigger_trigger_v(uint32_t r)
{
    return (r >> 0) & 0xffff;
}

static inline uint32_t gr2d_cmdsel_r(void)
{
    return 0xc;
}

static inline uint32_t gr2d_cmdsel_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_cmdsel_sbor2d_s(void)
{
    return 1;
}

static inline uint32_t gr2d_cmdsel_sbor2d_f(uint32_t v)
{
    return (v & 0x1) << 0;
}

static inline uint32_t gr2d_cmdsel_sbor2d_m(void)
{
    return 0x1 << 0;
}

static inline uint32_t gr2d_cmdsel_sbor2d_v(uint32_t r)
{
    return (r >> 0) & 0x1;
}

static inline uint32_t gr2d_cmdsel_sbor2d_g2_v(void)
{
    return 0;
}

static inline uint32_t gr2d_cmdsel_sbor2d_sb_v(void)
{
    return 1;
}

static inline uint32_t gr2d_cmdsel_cbenable_s(void)
{
    return 1;
}

static inline uint32_t gr2d_cmdsel_cbenable_f(uint32_t v)
{
    return (v & 0x1) << 4;
}

static inline uint32_t gr2d_cmdsel_cbenable_m(void)
{
    return 0x1 << 4;
}

static inline uint32_t gr2d_cmdsel_cbenable_v(uint32_t r)
{
    return (r >> 4) & 0x1;
}

static inline uint32_t gr2d_cmdsel_cbenable_disable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_cmdsel_cbenable_enable_v(void)
{
    return 1;
}

static inline uint32_t gr2d_cmdsel_vitrigger_s(void)
{
    return 1;
}

static inline uint32_t gr2d_cmdsel_vitrigger_f(uint32_t v)
{
    return (v & 0x1) << 5;
}

static inline uint32_t gr2d_cmdsel_vitrigger_m(void)
{
    return 0x1 << 5;
}

static inline uint32_t gr2d_cmdsel_vitrigger_v(uint32_t r)
{
    return (r >> 5) & 0x1;
}

static inline uint32_t gr2d_cmdsel_hosttrigger_s(void)
{
    return 1;
}

static inline uint32_t gr2d_cmdsel_hosttrigger_f(uint32_t v)
{
    return (v & 0x1) << 6;
}

static inline uint32_t gr2d_cmdsel_hosttrigger_m(void)
{
    return 0x1 << 6;
}

static inline uint32_t gr2d_cmdsel_hosttrigger_v(uint32_t r)
{
    return (r >> 6) & 0x1;
}

static inline uint32_t gr2d_cmdsel_cbsbdisable_s(void)
{
    return 1;
}

static inline uint32_t gr2d_cmdsel_cbsbdisable_f(uint32_t v)
{
    return (v & 0x1) << 7;
}

static inline uint32_t gr2d_cmdsel_cbsbdisable_m(void)
{
    return 0x1 << 7;
}

static inline uint32_t gr2d_cmdsel_cbsbdisable_v(uint32_t r)
{
    return (r >> 7) & 0x1;
}

static inline uint32_t gr2d_cmdsel_g2output_s(void)
{
    return 2;
}

static inline uint32_t gr2d_cmdsel_g2output_f(uint32_t v)
{
    return (v & 0x3) << 8;
}

static inline uint32_t gr2d_cmdsel_g2output_m(void)
{
    return 0x3 << 8;
}

static inline uint32_t gr2d_cmdsel_g2output_v(uint32_t r)
{
    return (r >> 8) & 0x3;
}

static inline uint32_t gr2d_cmdsel_g2output_memory_v(void)
{
    return 0;
}

static inline uint32_t gr2d_cmdsel_g2output_epp_v(void)
{
    return 1;
}

static inline uint32_t gr2d_cmdsel_g2output_reserved2_v(void)
{
    return 2;
}

static inline uint32_t gr2d_cmdsel_g2output_reserved3_v(void)
{
    return 3;
}

static inline uint32_t gr2d_cmdsel_clip_source_top_bottom_s(void)
{
    return 1;
}

static inline uint32_t gr2d_cmdsel_clip_source_top_bottom_f(uint32_t v)
{
    return (v & 0x1) << 10;
}

static inline uint32_t gr2d_cmdsel_clip_source_top_bottom_m(void)
{
    return 0x1 << 10;
}

static inline uint32_t gr2d_cmdsel_clip_source_top_bottom_v(uint32_t r)
{
    return (r >> 10) & 0x1;
}

static inline uint32_t gr2d_cmdsel_clip_source_top_bottom_disable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_cmdsel_clip_source_top_bottom_enable_v(void)
{
    return 1;
}

static inline uint32_t gr2d_cmdsel_frame_start_s(void)
{
    return 1;
}

static inline uint32_t gr2d_cmdsel_frame_start_f(uint32_t v)
{
    return (v & 0x1) << 14;
}

static inline uint32_t gr2d_cmdsel_frame_start_m(void)
{
    return 0x1 << 14;
}

static inline uint32_t gr2d_cmdsel_frame_start_v(uint32_t r)
{
    return (r >> 14) & 0x1;
}

static inline uint32_t gr2d_cmdsel_frame_end_s(void)
{
    return 1;
}

static inline uint32_t gr2d_cmdsel_frame_end_f(uint32_t v)
{
    return (v & 0x1) << 15;
}

static inline uint32_t gr2d_cmdsel_frame_end_m(void)
{
    return 0x1 << 15;
}

static inline uint32_t gr2d_cmdsel_frame_end_v(uint32_t r)
{
    return (r >> 15) & 0x1;
}

static inline uint32_t gr2d_cmdsel_buffer_index_s(void)
{
    return 8;
}

static inline uint32_t gr2d_cmdsel_buffer_index_f(uint32_t v)
{
    return (v & 0xff) << 16;
}

static inline uint32_t gr2d_cmdsel_buffer_index_m(void)
{
    return 0xff << 16;
}

static inline uint32_t gr2d_cmdsel_buffer_index_v(uint32_t r)
{
    return (r >> 16) & 0xff;
}

static inline uint32_t gr2d_cmdsel_linken_s(void)
{
    return 1;
}

static inline uint32_t gr2d_cmdsel_linken_f(uint32_t v)
{
    return (v & 0x1) << 24;
}

static inline uint32_t gr2d_cmdsel_linken_m(void)
{
    return 0x1 << 24;
}

static inline uint32_t gr2d_cmdsel_linken_v(uint32_t r)
{
    return (r >> 24) & 0x1;
}

static inline uint32_t gr2d_cmdsel_linkval_s(void)
{
    return 3;
}

static inline uint32_t gr2d_cmdsel_linkval_f(uint32_t v)
{
    return (v & 0x7) << 25;
}

static inline uint32_t gr2d_cmdsel_linkval_m(void)
{
    return 0x7 << 25;
}

static inline uint32_t gr2d_cmdsel_linkval_v(uint32_t r)
{
    return (r >> 25) & 0x7;
}

static inline uint32_t gr2d_cmdsel_priority_s(void)
{
    return 1;
}

static inline uint32_t gr2d_cmdsel_priority_f(uint32_t v)
{
    return (v & 0x1) << 28;
}

static inline uint32_t gr2d_cmdsel_priority_m(void)
{
    return 0x1 << 28;
}

static inline uint32_t gr2d_cmdsel_priority_v(uint32_t r)
{
    return (r >> 28) & 0x1;
}

static inline uint32_t gr2d_cmdsel_priority_low_v(void)
{
    return 0;
}

static inline uint32_t gr2d_cmdsel_priority_high_v(void)
{
    return 1;
}

static inline uint32_t gr2d_vdda_r(void)
{
    return 0x11;
}

static inline uint32_t gr2d_vdda_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_vdda_vdstep_s(void)
{
    return 32;
}

static inline uint32_t gr2d_vdda_vdstep_f(uint32_t v)
{
    return (v & 0xffffffff) << 0;
}

static inline uint32_t gr2d_vdda_vdstep_m(void)
{
    return 0xffffffff << 0;
}

static inline uint32_t gr2d_vdda_vdstep_v(uint32_t r)
{
    return (r >> 0) & 0xffffffff;
}

static inline uint32_t gr2d_vddaini_r(void)
{
    return 0x12;
}

static inline uint32_t gr2d_vddaini_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_vddaini_vdtini_s(void)
{
    return 8;
}

static inline uint32_t gr2d_vddaini_vdtini_f(uint32_t v)
{
    return (v & 0xff) << 0;
}

static inline uint32_t gr2d_vddaini_vdtini_m(void)
{
    return 0xff << 0;
}

static inline uint32_t gr2d_vddaini_vdtini_v(uint32_t r)
{
    return (r >> 0) & 0xff;
}

static inline uint32_t gr2d_vddaini_vdbini_s(void)
{
    return 8;
}

static inline uint32_t gr2d_vddaini_vdbini_f(uint32_t v)
{
    return (v & 0xff) << 8;
}

static inline uint32_t gr2d_vddaini_vdbini_m(void)
{
    return 0xff << 8;
}

static inline uint32_t gr2d_vddaini_vdbini_v(uint32_t r)
{
    return (r >> 8) & 0xff;
}

static inline uint32_t gr2d_hdda_r(void)
{
    return 0x13;
}

static inline uint32_t gr2d_hdda_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_hdda_hdstep_s(void)
{
    return 32;
}

static inline uint32_t gr2d_hdda_hdstep_f(uint32_t v)
{
    return (v & 0xffffffff) << 0;
}

static inline uint32_t gr2d_hdda_hdstep_m(void)
{
    return 0xffffffff << 0;
}

static inline uint32_t gr2d_hdda_hdstep_v(uint32_t r)
{
    return (r >> 0) & 0xffffffff;
}

static inline uint32_t gr2d_hddainils_r(void)
{
    return 0x14;
}

static inline uint32_t gr2d_hddainils_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_hddainils_hdini_s(void)
{
    return 8;
}

static inline uint32_t gr2d_hddainils_hdini_f(uint32_t v)
{
    return (v & 0xff) << 0;
}

static inline uint32_t gr2d_hddainils_hdini_m(void)
{
    return 0xff << 0;
}

static inline uint32_t gr2d_hddainils_hdini_v(uint32_t r)
{
    return (r >> 0) & 0xff;
}

static inline uint32_t gr2d_cscfirst_r(void)
{
    return 0x15;
}

static inline uint32_t gr2d_cscfirst_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_cscfirst_cub_s(void)
{
    return 10;
}

static inline uint32_t gr2d_cscfirst_cub_f(uint32_t v)
{
    return (v & 0x3ff) << 0;
}

static inline uint32_t gr2d_cscfirst_cub_m(void)
{
    return 0x3ff << 0;
}

static inline uint32_t gr2d_cscfirst_cub_v(uint32_t r)
{
    return (r >> 0) & 0x3ff;
}

static inline uint32_t gr2d_cscfirst_cvr_s(void)
{
    return 10;
}

static inline uint32_t gr2d_cscfirst_cvr_f(uint32_t v)
{
    return (v & 0x3ff) << 12;
}

static inline uint32_t gr2d_cscfirst_cvr_m(void)
{
    return 0x3ff << 12;
}

static inline uint32_t gr2d_cscfirst_cvr_v(uint32_t r)
{
    return (r >> 12) & 0x3ff;
}

static inline uint32_t gr2d_cscfirst_yos_s(void)
{
    return 8;
}

static inline uint32_t gr2d_cscfirst_yos_f(uint32_t v)
{
    return (v & 0xff) << 24;
}

static inline uint32_t gr2d_cscfirst_yos_m(void)
{
    return 0xff << 24;
}

static inline uint32_t gr2d_cscfirst_yos_v(uint32_t r)
{
    return (r >> 24) & 0xff;
}

static inline uint32_t gr2d_cscsecond_r(void)
{
    return 0x16;
}

static inline uint32_t gr2d_cscsecond_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_cscsecond_cug_s(void)
{
    return 9;
}

static inline uint32_t gr2d_cscsecond_cug_f(uint32_t v)
{
    return (v & 0x1ff) << 0;
}

static inline uint32_t gr2d_cscsecond_cug_m(void)
{
    return 0x1ff << 0;
}

static inline uint32_t gr2d_cscsecond_cug_v(uint32_t r)
{
    return (r >> 0) & 0x1ff;
}

static inline uint32_t gr2d_cscsecond_cur_s(void)
{
    return 10;
}

static inline uint32_t gr2d_cscsecond_cur_f(uint32_t v)
{
    return (v & 0x3ff) << 12;
}

static inline uint32_t gr2d_cscsecond_cur_m(void)
{
    return 0x3ff << 12;
}

static inline uint32_t gr2d_cscsecond_cur_v(uint32_t r)
{
    return (r >> 12) & 0x3ff;
}

static inline uint32_t gr2d_cscsecond_cyx_s(void)
{
    return 8;
}

static inline uint32_t gr2d_cscsecond_cyx_f(uint32_t v)
{
    return (v & 0xff) << 24;
}

static inline uint32_t gr2d_cscsecond_cyx_m(void)
{
    return 0xff << 24;
}

static inline uint32_t gr2d_cscsecond_cyx_v(uint32_t r)
{
    return (r >> 24) & 0xff;
}

static inline uint32_t gr2d_cscthird_r(void)
{
    return 0x17;
}

static inline uint32_t gr2d_cscthird_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_cscthird_cvg_s(void)
{
    return 9;
}

static inline uint32_t gr2d_cscthird_cvg_f(uint32_t v)
{
    return (v & 0x1ff) << 0;
}

static inline uint32_t gr2d_cscthird_cvg_m(void)
{
    return 0x1ff << 0;
}

static inline uint32_t gr2d_cscthird_cvg_v(uint32_t r)
{
    return (r >> 0) & 0x1ff;
}

static inline uint32_t gr2d_cscthird_cvb_s(void)
{
    return 10;
}

static inline uint32_t gr2d_cscthird_cvb_f(uint32_t v)
{
    return (v & 0x3ff) << 16;
}

static inline uint32_t gr2d_cscthird_cvb_m(void)
{
    return 0x3ff << 16;
}

static inline uint32_t gr2d_cscthird_cvb_v(uint32_t r)
{
    return (r >> 16) & 0x3ff;
}

static inline uint32_t gr2d_uba_r(void)
{
    return 0x1a;
}

static inline uint32_t gr2d_uba_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_uba_su1sa_s(void)
{
    return 32;
}

static inline uint32_t gr2d_uba_su1sa_f(uint32_t v)
{
    return (v & 0xffffffff) << 0;
}

static inline uint32_t gr2d_uba_su1sa_m(void)
{
    return 0xffffffff << 0;
}

static inline uint32_t gr2d_uba_su1sa_v(uint32_t r)
{
    return (r >> 0) & 0xffffffff;
}

static inline uint32_t gr2d_vba_r(void)
{
    return 0x1b;
}

static inline uint32_t gr2d_vba_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_vba_sv1sa_s(void)
{
    return 32;
}

static inline uint32_t gr2d_vba_sv1sa_f(uint32_t v)
{
    return (v & 0xffffffff) << 0;
}

static inline uint32_t gr2d_vba_sv1sa_m(void)
{
    return 0xffffffff << 0;
}

static inline uint32_t gr2d_vba_sv1sa_v(uint32_t r)
{
    return (r >> 0) & 0xffffffff;
}

static inline uint32_t gr2d_sbformat_r(void)
{
    return 0x1c;
}

static inline uint32_t gr2d_sbformat_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_sbformat_sifmt_s(void)
{
    return 5;
}

static inline uint32_t gr2d_sbformat_sifmt_f(uint32_t v)
{
    return (v & 0x1f) << 0;
}

static inline uint32_t gr2d_sbformat_sifmt_m(void)
{
    return 0x1f << 0;
}

static inline uint32_t gr2d_sbformat_sifmt_v(uint32_t r)
{
    return (r >> 0) & 0x1f;
}

static inline uint32_t gr2d_sbformat_sifmt_u8y8v8y8_ob_v(void)
{
    return 0;
}

static inline uint32_t gr2d_sbformat_sifmt_y8u8y8v8_ob_v(void)
{
    return 1;
}

static inline uint32_t gr2d_sbformat_sifmt_y8v8y8u8_ob_v(void)
{
    return 2;
}

static inline uint32_t gr2d_sbformat_sifmt_v8y8u8y8_ob_v(void)
{
    return 3;
}

static inline uint32_t gr2d_sbformat_sifmt_u8y8v8y8_tc_v(void)
{
    return 4;
}

static inline uint32_t gr2d_sbformat_sifmt_y8u8y8v8_tc_v(void)
{
    return 5;
}

static inline uint32_t gr2d_sbformat_sifmt_y8v8y8u8_tc_v(void)
{
    return 6;
}

static inline uint32_t gr2d_sbformat_sifmt_v8y8u8y8_tc_v(void)
{
    return 7;
}

static inline uint32_t gr2d_sbformat_sifmt_b5g6r5_v(void)
{
    return 8;
}

static inline uint32_t gr2d_sbformat_sifmt_b5g6r5bs_v(void)
{
    return 12;
}

static inline uint32_t gr2d_sbformat_sifmt_r8g8b8a8_v(void)
{
    return 14;
}

static inline uint32_t gr2d_sbformat_sifmt_b8g8r8a8_v(void)
{
    return 15;
}

static inline uint32_t gr2d_sbformat_difmt_s(void)
{
    return 5;
}

static inline uint32_t gr2d_sbformat_difmt_f(uint32_t v)
{
    return (v & 0x1f) << 8;
}

static inline uint32_t gr2d_sbformat_difmt_m(void)
{
    return 0x1f << 8;
}

static inline uint32_t gr2d_sbformat_difmt_v(uint32_t r)
{
    return (r >> 8) & 0x1f;
}

static inline uint32_t gr2d_sbformat_difmt_u8y8v8y8_ob_v(void)
{
    return 0;
}

static inline uint32_t gr2d_sbformat_difmt_y8u8y8v8_ob_v(void)
{
    return 1;
}

static inline uint32_t gr2d_sbformat_difmt_y8v8y8u8_ob_v(void)
{
    return 2;
}

static inline uint32_t gr2d_sbformat_difmt_v8y8u8y8_ob_v(void)
{
    return 3;
}

static inline uint32_t gr2d_sbformat_difmt_u8y8v8y8_tc_v(void)
{
    return 4;
}

static inline uint32_t gr2d_sbformat_difmt_y8u8y8v8_tc_v(void)
{
    return 5;
}

static inline uint32_t gr2d_sbformat_difmt_y8v8y8u8_tc_v(void)
{
    return 6;
}

static inline uint32_t gr2d_sbformat_difmt_v8y8u8y8_tc_v(void)
{
    return 7;
}

static inline uint32_t gr2d_sbformat_difmt_b5g6r5_v(void)
{
    return 8;
}

static inline uint32_t gr2d_sbformat_difmt_b5g6r5bs_v(void)
{
    return 12;
}

static inline uint32_t gr2d_sbformat_difmt_r8g8b8a8_v(void)
{
    return 14;
}

static inline uint32_t gr2d_sbformat_difmt_b8g8r8a8_v(void)
{
    return 15;
}

static inline uint32_t gr2d_controlsb_r(void)
{
    return 0x1d;
}

static inline uint32_t gr2d_controlsb_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_controlsb_imode_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsb_imode_f(uint32_t v)
{
    return (v & 0x1) << 5;
}

static inline uint32_t gr2d_controlsb_imode_m(void)
{
    return 0x1 << 5;
}

static inline uint32_t gr2d_controlsb_imode_v(uint32_t r)
{
    return (r >> 5) & 0x1;
}

static inline uint32_t gr2d_controlsb_imode_multiplex_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlsb_imode_planar_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsb_enavf_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsb_enavf_f(uint32_t v)
{
    return (v & 0x1) << 6;
}

static inline uint32_t gr2d_controlsb_enavf_m(void)
{
    return 0x1 << 6;
}

static inline uint32_t gr2d_controlsb_enavf_v(uint32_t r)
{
    return (r >> 6) & 0x1;
}

static inline uint32_t gr2d_controlsb_enavf_disable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlsb_enavf_enable_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsb_enahf_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsb_enahf_f(uint32_t v)
{
    return (v & 0x1) << 7;
}

static inline uint32_t gr2d_controlsb_enahf_m(void)
{
    return 0x1 << 7;
}

static inline uint32_t gr2d_controlsb_enahf_v(uint32_t r)
{
    return (r >> 7) & 0x1;
}

static inline uint32_t gr2d_controlsb_enahf_disable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlsb_enahf_enable_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsb_uvst_s(void)
{
    return 2;
}

static inline uint32_t gr2d_controlsb_uvst_f(uint32_t v)
{
    return (v & 0x3) << 8;
}

static inline uint32_t gr2d_controlsb_uvst_m(void)
{
    return 0x3 << 8;
}

static inline uint32_t gr2d_controlsb_uvst_v(uint32_t r)
{
    return (r >> 8) & 0x3;
}

static inline uint32_t gr2d_controlsb_uvst_uvs2x_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlsb_uvst_uvs1x_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsb_uvst_uvs4x_v(void)
{
    return 2;
}

static inline uint32_t gr2d_controlsb_uvst_uvs_g2uvstride_v(void)
{
    return 3;
}

static inline uint32_t gr2d_controlsb_vftype_s(void)
{
    return 2;
}

static inline uint32_t gr2d_controlsb_vftype_f(uint32_t v)
{
    return (v & 0x3) << 16;
}

static inline uint32_t gr2d_controlsb_vftype_m(void)
{
    return 0x3 << 16;
}

static inline uint32_t gr2d_controlsb_vftype_v(uint32_t r)
{
    return (r >> 16) & 0x3;
}

static inline uint32_t gr2d_controlsb_vftype_interp_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlsb_vftype_avg25_interp75_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsb_vftype_avg50_interp50_v(void)
{
    return 2;
}

static inline uint32_t gr2d_controlsb_vftype_avg_v(void)
{
    return 3;
}

static inline uint32_t gr2d_controlsb_vfen_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsb_vfen_f(uint32_t v)
{
    return (v & 0x1) << 18;
}

static inline uint32_t gr2d_controlsb_vfen_m(void)
{
    return 0x1 << 18;
}

static inline uint32_t gr2d_controlsb_vfen_v(uint32_t r)
{
    return (r >> 18) & 0x1;
}

static inline uint32_t gr2d_controlsb_vfen_disable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlsb_vfen_enable_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsb_discsc_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsb_discsc_f(uint32_t v)
{
    return (v & 0x1) << 19;
}

static inline uint32_t gr2d_controlsb_discsc_m(void)
{
    return 0x1 << 19;
}

static inline uint32_t gr2d_controlsb_discsc_v(uint32_t r)
{
    return (r >> 19) & 0x1;
}

static inline uint32_t gr2d_controlsb_discsc_enable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlsb_discsc_disable_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsb_hftype_s(void)
{
    return 3;
}

static inline uint32_t gr2d_controlsb_hftype_f(uint32_t v)
{
    return (v & 0x7) << 20;
}

static inline uint32_t gr2d_controlsb_hftype_m(void)
{
    return 0x7 << 20;
}

static inline uint32_t gr2d_controlsb_hftype_v(uint32_t r)
{
    return (r >> 20) & 0x7;
}

static inline uint32_t gr2d_controlsb_hftype_interp_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlsb_hftype_lpf1_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsb_hftype_lpf2_v(void)
{
    return 2;
}

static inline uint32_t gr2d_controlsb_hftype_lpf3_v(void)
{
    return 3;
}

static inline uint32_t gr2d_controlsb_hftype_lpf4_v(void)
{
    return 4;
}

static inline uint32_t gr2d_controlsb_hftype_lpf5_v(void)
{
    return 5;
}

static inline uint32_t gr2d_controlsb_hftype_lpf6_v(void)
{
    return 6;
}

static inline uint32_t gr2d_controlsb_hftype_disable_v(void)
{
    return 7;
}

static inline uint32_t gr2d_controlsb_rangeredfrm_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsb_rangeredfrm_f(uint32_t v)
{
    return (v & 0x1) << 23;
}

static inline uint32_t gr2d_controlsb_rangeredfrm_m(void)
{
    return 0x1 << 23;
}

static inline uint32_t gr2d_controlsb_rangeredfrm_v(uint32_t r)
{
    return (r >> 23) & 0x1;
}

static inline uint32_t gr2d_controlsb_rangeredfrm_disable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlsb_rangeredfrm_enable_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsb_sitype_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsb_sitype_f(uint32_t v)
{
    return (v & 0x1) << 24;
}

static inline uint32_t gr2d_controlsb_sitype_m(void)
{
    return 0x1 << 24;
}

static inline uint32_t gr2d_controlsb_sitype_v(uint32_t r)
{
    return (r >> 24) & 0x1;
}

static inline uint32_t gr2d_controlsb_sbsel_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsb_sbsel_f(uint32_t v)
{
    return (v & 0x1) << 25;
}

static inline uint32_t gr2d_controlsb_sbsel_m(void)
{
    return 0x1 << 25;
}

static inline uint32_t gr2d_controlsb_sbsel_v(uint32_t r)
{
    return (r >> 25) & 0x1;
}

static inline uint32_t gr2d_controlsb_sbsel_src_a_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlsb_sbsel_src_b_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsb_dbsel_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsb_dbsel_f(uint32_t v)
{
    return (v & 0x1) << 26;
}

static inline uint32_t gr2d_controlsb_dbsel_m(void)
{
    return 0x1 << 26;
}

static inline uint32_t gr2d_controlsb_dbsel_v(uint32_t r)
{
    return (r >> 26) & 0x1;
}

static inline uint32_t gr2d_controlsb_dbsel_dst_a_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlsb_dbsel_dst_b_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsb_keyen_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsb_keyen_f(uint32_t v)
{
    return (v & 0x1) << 27;
}

static inline uint32_t gr2d_controlsb_keyen_m(void)
{
    return 0x1 << 27;
}

static inline uint32_t gr2d_controlsb_keyen_v(uint32_t r)
{
    return (r >> 27) & 0x1;
}

static inline uint32_t gr2d_controlsb_keyen_disable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlsb_keyen_enable_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsb_kpol_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsb_kpol_f(uint32_t v)
{
    return (v & 0x1) << 28;
}

static inline uint32_t gr2d_controlsb_kpol_m(void)
{
    return 0x1 << 28;
}

static inline uint32_t gr2d_controlsb_kpol_v(uint32_t r)
{
    return (r >> 28) & 0x1;
}

static inline uint32_t gr2d_controlsb_kpol_within_bounds_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlsb_kpol_outside_bounds_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsb_endith_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsb_endith_f(uint32_t v)
{
    return (v & 0x1) << 30;
}

static inline uint32_t gr2d_controlsb_endith_m(void)
{
    return 0x1 << 30;
}

static inline uint32_t gr2d_controlsb_endith_v(uint32_t r)
{
    return (r >> 30) & 0x1;
}

static inline uint32_t gr2d_controlsb_endith_disable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlsb_endith_enable_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsecond_r(void)
{
    return 0x1e;
}

static inline uint32_t gr2d_controlsecond_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_controlsecond_bitswap_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsecond_bitswap_f(uint32_t v)
{
    return (v & 0x1) << 1;
}

static inline uint32_t gr2d_controlsecond_bitswap_m(void)
{
    return 0x1 << 1;
}

static inline uint32_t gr2d_controlsecond_bitswap_v(uint32_t r)
{
    return (r >> 1) & 0x1;
}

static inline uint32_t gr2d_controlsecond_bitswap_disable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlsecond_bitswap_enable_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsecond_bebswap_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsecond_bebswap_f(uint32_t v)
{
    return (v & 0x1) << 2;
}

static inline uint32_t gr2d_controlsecond_bebswap_m(void)
{
    return 0x1 << 2;
}

static inline uint32_t gr2d_controlsecond_bebswap_v(uint32_t r)
{
    return (r >> 2) & 0x1;
}

static inline uint32_t gr2d_controlsecond_bebswap_disable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlsecond_bebswap_enable_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsecond_bewswap_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsecond_bewswap_f(uint32_t v)
{
    return (v & 0x1) << 3;
}

static inline uint32_t gr2d_controlsecond_bewswap_m(void)
{
    return 0x1 << 3;
}

static inline uint32_t gr2d_controlsecond_bewswap_v(uint32_t r)
{
    return (r >> 3) & 0x1;
}

static inline uint32_t gr2d_controlsecond_bewswap_disable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlsecond_bewswap_enable_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsecond_alptype_s(void)
{
    return 5;
}

static inline uint32_t gr2d_controlsecond_alptype_f(uint32_t v)
{
    return (v & 0x1f) << 4;
}

static inline uint32_t gr2d_controlsecond_alptype_m(void)
{
    return 0x1f << 4;
}

static inline uint32_t gr2d_controlsecond_alptype_v(uint32_t r)
{
    return (r >> 4) & 0x1f;
}

static inline uint32_t gr2d_controlsecond_alptype_fix_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlsecond_alptype_pl1bpp_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsecond_alptype_pl2bpp_v(void)
{
    return 2;
}

static inline uint32_t gr2d_controlsecond_alptype_pl4bpp_v(void)
{
    return 3;
}

static inline uint32_t gr2d_controlsecond_alptype_pl8bpp_v(void)
{
    return 4;
}

static inline uint32_t gr2d_controlsecond_alptype_pl44bpp_v(void)
{
    return 5;
}

static inline uint32_t gr2d_controlsecond_alptype_pls1bpp_v(void)
{
    return 6;
}

static inline uint32_t gr2d_controlsecond_alptype_pls4bppal_v(void)
{
    return 7;
}

static inline uint32_t gr2d_controlsecond_alptype_pls4bpp_v(void)
{
    return 8;
}

static inline uint32_t gr2d_controlsecond_alptype_pls8bpp_v(void)
{
    return 9;
}

static inline uint32_t gr2d_controlsecond_alptype_pls8bx_v(void)
{
    return 10;
}

static inline uint32_t gr2d_controlsecond_alptype_pls1bppal_v(void)
{
    return 11;
}

static inline uint32_t gr2d_controlsecond_alpsrcordst_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsecond_alpsrcordst_f(uint32_t v)
{
    return (v & 0x1) << 9;
}

static inline uint32_t gr2d_controlsecond_alpsrcordst_m(void)
{
    return 0x1 << 9;
}

static inline uint32_t gr2d_controlsecond_alpsrcordst_v(uint32_t r)
{
    return (r >> 9) & 0x1;
}

static inline uint32_t gr2d_controlsecond_alpsrcordst_disable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlsecond_alpsrcordst_enable_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsecond_clipc_s(void)
{
    return 2;
}

static inline uint32_t gr2d_controlsecond_clipc_f(uint32_t v)
{
    return (v & 0x3) << 21;
}

static inline uint32_t gr2d_controlsecond_clipc_m(void)
{
    return 0x3 << 21;
}

static inline uint32_t gr2d_controlsecond_clipc_v(uint32_t r)
{
    return (r >> 21) & 0x3;
}

static inline uint32_t gr2d_controlsecond_fr_mode_s(void)
{
    return 2;
}

static inline uint32_t gr2d_controlsecond_fr_mode_f(uint32_t v)
{
    return (v & 0x3) << 24;
}

static inline uint32_t gr2d_controlsecond_fr_mode_m(void)
{
    return 0x3 << 24;
}

static inline uint32_t gr2d_controlsecond_fr_mode_v(uint32_t r)
{
    return (r >> 24) & 0x3;
}

static inline uint32_t gr2d_controlsecond_fr_mode_disable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlsecond_fr_mode_src_dst_copy_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsecond_fr_mode_square_v(void)
{
    return 2;
}

static inline uint32_t gr2d_controlsecond_fr_mode_blank_v(void)
{
    return 3;
}

static inline uint32_t gr2d_controlsecond_fr_type_s(void)
{
    return 3;
}

static inline uint32_t gr2d_controlsecond_fr_type_f(uint32_t v)
{
    return (v & 0x7) << 26;
}

static inline uint32_t gr2d_controlsecond_fr_type_m(void)
{
    return 0x7 << 26;
}

static inline uint32_t gr2d_controlsecond_fr_type_v(uint32_t r)
{
    return (r >> 26) & 0x7;
}

static inline uint32_t gr2d_controlsecond_fr_type_flip_x_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlsecond_fr_type_flip_y_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsecond_fr_type_trans_lr_v(void)
{
    return 2;
}

static inline uint32_t gr2d_controlsecond_fr_type_trans_rl_v(void)
{
    return 3;
}

static inline uint32_t gr2d_controlsecond_fr_type_rot_90_v(void)
{
    return 4;
}

static inline uint32_t gr2d_controlsecond_fr_type_rot_180_v(void)
{
    return 5;
}

static inline uint32_t gr2d_controlsecond_fr_type_rot_270_v(void)
{
    return 6;
}

static inline uint32_t gr2d_controlsecond_fr_type_identity_v(void)
{
    return 7;
}

static inline uint32_t gr2d_controlsecond_fr_readwait_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlsecond_fr_readwait_f(uint32_t v)
{
    return (v & 0x1) << 29;
}

static inline uint32_t gr2d_controlsecond_fr_readwait_m(void)
{
    return 0x1 << 29;
}

static inline uint32_t gr2d_controlsecond_fr_readwait_v(uint32_t r)
{
    return (r >> 29) & 0x1;
}

static inline uint32_t gr2d_controlsecond_fr_readwait_disable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlsecond_fr_readwait_enable_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_r(void)
{
    return 0x1f;
}

static inline uint32_t gr2d_controlmain_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_controlmain_cmdt_s(void)
{
    return 2;
}

static inline uint32_t gr2d_controlmain_cmdt_f(uint32_t v)
{
    return (v & 0x3) << 0;
}

static inline uint32_t gr2d_controlmain_cmdt_m(void)
{
    return 0x3 << 0;
}

static inline uint32_t gr2d_controlmain_cmdt_v(uint32_t r)
{
    return (r >> 0) & 0x3;
}

static inline uint32_t gr2d_controlmain_cmdt_bitblt_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlmain_cmdt_linedraw_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_cmdt_vcaa_v(void)
{
    return 2;
}

static inline uint32_t gr2d_controlmain_cmdt_reserved1_v(void)
{
    return 3;
}

static inline uint32_t gr2d_controlmain_turbofill_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_turbofill_f(uint32_t v)
{
    return (v & 0x1) << 2;
}

static inline uint32_t gr2d_controlmain_turbofill_m(void)
{
    return 0x1 << 2;
}

static inline uint32_t gr2d_controlmain_turbofill_v(uint32_t r)
{
    return (r >> 2) & 0x1;
}

static inline uint32_t gr2d_controlmain_test0bit_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_test0bit_f(uint32_t v)
{
    return (v & 0x1) << 3;
}

static inline uint32_t gr2d_controlmain_test0bit_m(void)
{
    return 0x1 << 3;
}

static inline uint32_t gr2d_controlmain_test0bit_v(uint32_t r)
{
    return (r >> 3) & 0x1;
}

static inline uint32_t gr2d_controlmain_test0bit_disable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlmain_test0bit_enable_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_faden_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_faden_f(uint32_t v)
{
    return (v & 0x1) << 4;
}

static inline uint32_t gr2d_controlmain_faden_m(void)
{
    return 0x1 << 4;
}

static inline uint32_t gr2d_controlmain_faden_v(uint32_t r)
{
    return (r >> 4) & 0x1;
}

static inline uint32_t gr2d_controlmain_faden_disable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlmain_faden_enable_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_alpen_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_alpen_f(uint32_t v)
{
    return (v & 0x1) << 5;
}

static inline uint32_t gr2d_controlmain_alpen_m(void)
{
    return 0x1 << 5;
}

static inline uint32_t gr2d_controlmain_alpen_v(uint32_t r)
{
    return (r >> 5) & 0x1;
}

static inline uint32_t gr2d_controlmain_srcsld_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_srcsld_f(uint32_t v)
{
    return (v & 0x1) << 6;
}

static inline uint32_t gr2d_controlmain_srcsld_m(void)
{
    return 0x1 << 6;
}

static inline uint32_t gr2d_controlmain_srcsld_v(uint32_t r)
{
    return (r >> 6) & 0x1;
}

static inline uint32_t gr2d_controlmain_srcsld_disable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlmain_srcsld_enable_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_patsld_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_patsld_f(uint32_t v)
{
    return (v & 0x1) << 7;
}

static inline uint32_t gr2d_controlmain_patsld_m(void)
{
    return 0x1 << 7;
}

static inline uint32_t gr2d_controlmain_patsld_v(uint32_t r)
{
    return (r >> 7) & 0x1;
}

static inline uint32_t gr2d_controlmain_patsld_disable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlmain_patsld_enable_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_patfl_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_patfl_f(uint32_t v)
{
    return (v & 0x1) << 8;
}

static inline uint32_t gr2d_controlmain_patfl_m(void)
{
    return 0x1 << 8;
}

static inline uint32_t gr2d_controlmain_patfl_v(uint32_t r)
{
    return (r >> 8) & 0x1;
}

static inline uint32_t gr2d_controlmain_patfl_disable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlmain_patfl_enable_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_xdir_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_xdir_f(uint32_t v)
{
    return (v & 0x1) << 9;
}

static inline uint32_t gr2d_controlmain_xdir_m(void)
{
    return 0x1 << 9;
}

static inline uint32_t gr2d_controlmain_xdir_v(uint32_t r)
{
    return (r >> 9) & 0x1;
}

static inline uint32_t gr2d_controlmain_ydir_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_ydir_f(uint32_t v)
{
    return (v & 0x1) << 10;
}

static inline uint32_t gr2d_controlmain_ydir_m(void)
{
    return 0x1 << 10;
}

static inline uint32_t gr2d_controlmain_ydir_v(uint32_t r)
{
    return (r >> 10) & 0x1;
}

static inline uint32_t gr2d_controlmain_xytdw_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_xytdw_f(uint32_t v)
{
    return (v & 0x1) << 11;
}

static inline uint32_t gr2d_controlmain_xytdw_m(void)
{
    return 0x1 << 11;
}

static inline uint32_t gr2d_controlmain_xytdw_v(uint32_t r)
{
    return (r >> 11) & 0x1;
}

static inline uint32_t gr2d_controlmain_srcpack_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_srcpack_f(uint32_t v)
{
    return (v & 0x1) << 12;
}

static inline uint32_t gr2d_controlmain_srcpack_m(void)
{
    return 0x1 << 12;
}

static inline uint32_t gr2d_controlmain_srcpack_v(uint32_t r)
{
    return (r >> 12) & 0x1;
}

static inline uint32_t gr2d_controlmain_srcpack_disable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlmain_srcpack_enable_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_patpack_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_patpack_f(uint32_t v)
{
    return (v & 0x1) << 13;
}

static inline uint32_t gr2d_controlmain_patpack_m(void)
{
    return 0x1 << 13;
}

static inline uint32_t gr2d_controlmain_patpack_v(uint32_t r)
{
    return (r >> 13) & 0x1;
}

static inline uint32_t gr2d_controlmain_patpack_disable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlmain_patpack_enable_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_yflip_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_yflip_f(uint32_t v)
{
    return (v & 0x1) << 14;
}

static inline uint32_t gr2d_controlmain_yflip_m(void)
{
    return 0x1 << 14;
}

static inline uint32_t gr2d_controlmain_yflip_v(uint32_t r)
{
    return (r >> 14) & 0x1;
}

static inline uint32_t gr2d_controlmain_yflip_dsiable_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlmain_yflip_enable_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_srcsel_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_srcsel_f(uint32_t v)
{
    return (v & 0x1) << 15;
}

static inline uint32_t gr2d_controlmain_srcsel_m(void)
{
    return 0x1 << 15;
}

static inline uint32_t gr2d_controlmain_srcsel_v(uint32_t r)
{
    return (r >> 15) & 0x1;
}

static inline uint32_t gr2d_controlmain_srcsel_screen_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlmain_srcsel_memory_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_dstcd_s(void)
{
    return 2;
}

static inline uint32_t gr2d_controlmain_dstcd_f(uint32_t v)
{
    return (v & 0x3) << 16;
}

static inline uint32_t gr2d_controlmain_dstcd_m(void)
{
    return 0x3 << 16;
}

static inline uint32_t gr2d_controlmain_dstcd_v(uint32_t r)
{
    return (r >> 16) & 0x3;
}

static inline uint32_t gr2d_controlmain_dstcd_bpp8_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlmain_dstcd_bpp16_v(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_dstcd_bpp32_v(void)
{
    return 2;
}

static inline uint32_t gr2d_controlmain_dstcd_reserved3_v(void)
{
    return 3;
}

static inline uint32_t gr2d_controlmain_dstt_s(void)
{
    return 2;
}

static inline uint32_t gr2d_controlmain_dstt_f(uint32_t v)
{
    return (v & 0x3) << 18;
}

static inline uint32_t gr2d_controlmain_dstt_m(void)
{
    return 0x3 << 18;
}

static inline uint32_t gr2d_controlmain_dstt_v(uint32_t r)
{
    return (r >> 18) & 0x3;
}

static inline uint32_t gr2d_controlmain_srccd_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_srccd_f(uint32_t v)
{
    return (v & 0x1) << 20;
}

static inline uint32_t gr2d_controlmain_srccd_m(void)
{
    return 0x1 << 20;
}

static inline uint32_t gr2d_controlmain_srccd_v(uint32_t r)
{
    return (r >> 20) & 0x1;
}

static inline uint32_t gr2d_controlmain_hlmono_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_hlmono_f(uint32_t v)
{
    return (v & 0x1) << 21;
}

static inline uint32_t gr2d_controlmain_hlmono_m(void)
{
    return 0x1 << 21;
}

static inline uint32_t gr2d_controlmain_hlmono_v(uint32_t r)
{
    return (r >> 21) & 0x1;
}

static inline uint32_t gr2d_controlmain_srct_s(void)
{
    return 2;
}

static inline uint32_t gr2d_controlmain_srct_f(uint32_t v)
{
    return (v & 0x3) << 22;
}

static inline uint32_t gr2d_controlmain_srct_m(void)
{
    return 0x3 << 22;
}

static inline uint32_t gr2d_controlmain_srct_v(uint32_t r)
{
    return (r >> 22) & 0x3;
}

static inline uint32_t gr2d_controlmain_srcbas_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_srcbas_f(uint32_t v)
{
    return (v & 0x1) << 24;
}

static inline uint32_t gr2d_controlmain_srcbas_m(void)
{
    return 0x1 << 24;
}

static inline uint32_t gr2d_controlmain_srcbas_v(uint32_t r)
{
    return (r >> 24) & 0x1;
}

static inline uint32_t gr2d_controlmain_gcsw_s(void)
{
    return 2;
}

static inline uint32_t gr2d_controlmain_gcsw_f(uint32_t v)
{
    return (v & 0x3) << 25;
}

static inline uint32_t gr2d_controlmain_gcsw_m(void)
{
    return 0x3 << 25;
}

static inline uint32_t gr2d_controlmain_gcsw_v(uint32_t r)
{
    return (r >> 25) & 0x3;
}

static inline uint32_t gr2d_controlmain_srcdir_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_srcdir_f(uint32_t v)
{
    return (v & 0x1) << 27;
}

static inline uint32_t gr2d_controlmain_srcdir_m(void)
{
    return 0x1 << 27;
}

static inline uint32_t gr2d_controlmain_srcdir_v(uint32_t r)
{
    return (r >> 27) & 0x1;
}

static inline uint32_t gr2d_controlmain_dstdir_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_dstdir_f(uint32_t v)
{
    return (v & 0x1) << 28;
}

static inline uint32_t gr2d_controlmain_dstdir_m(void)
{
    return 0x1 << 28;
}

static inline uint32_t gr2d_controlmain_dstdir_v(uint32_t r)
{
    return (r >> 28) & 0x1;
}

static inline uint32_t gr2d_controlmain_patsel_s(void)
{
    return 1;
}

static inline uint32_t gr2d_controlmain_patsel_f(uint32_t v)
{
    return (v & 0x1) << 30;
}

static inline uint32_t gr2d_controlmain_patsel_m(void)
{
    return 0x1 << 30;
}

static inline uint32_t gr2d_controlmain_patsel_v(uint32_t r)
{
    return (r >> 30) & 0x1;
}

static inline uint32_t gr2d_controlmain_patsel_screen_v(void)
{
    return 0;
}

static inline uint32_t gr2d_controlmain_patsel_memory_v(void)
{
    return 1;
}

static inline uint32_t gr2d_ropfade_r(void)
{
    return 0x20;
}

static inline uint32_t gr2d_ropfade_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_ropfade_rop_s(void)
{
    return 8;
}

static inline uint32_t gr2d_ropfade_rop_f(uint32_t v)
{
    return (v & 0xff) << 0;
}

static inline uint32_t gr2d_ropfade_rop_m(void)
{
    return 0xff << 0;
}

static inline uint32_t gr2d_ropfade_rop_v(uint32_t r)
{
    return (r >> 0) & 0xff;
}

static inline uint32_t gr2d_ropfade_fadcoe_s(void)
{
    return 8;
}

static inline uint32_t gr2d_ropfade_fadcoe_f(uint32_t v)
{
    return (v & 0xff) << 16;
}

static inline uint32_t gr2d_ropfade_fadcoe_m(void)
{
    return 0xff << 16;
}

static inline uint32_t gr2d_ropfade_fadcoe_v(uint32_t r)
{
    return (r >> 16) & 0xff;
}

static inline uint32_t gr2d_ropfade_fadoff_s(void)
{
    return 8;
}

static inline uint32_t gr2d_ropfade_fadoff_f(uint32_t v)
{
    return (v & 0xff) << 24;
}

static inline uint32_t gr2d_ropfade_fadoff_m(void)
{
    return 0xff << 24;
}

static inline uint32_t gr2d_ropfade_fadoff_v(uint32_t r)
{
    return (r >> 24) & 0xff;
}

static inline uint32_t gr2d_dstba_r(void)
{
    return 0x2b;
}

static inline uint32_t gr2d_dstba_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_dstba_dstba_s(void)
{
    return 32;
}

static inline uint32_t gr2d_dstba_dstba_f(uint32_t v)
{
    return (v & 0xffffffff) << 0;
}

static inline uint32_t gr2d_dstba_dstba_m(void)
{
    return 0xffffffff << 0;
}

static inline uint32_t gr2d_dstba_dstba_v(uint32_t r)
{
    return (r >> 0) & 0xffffffff;
}

static inline uint32_t gr2d_dstst_r(void)
{
    return 0x2e;
}

static inline uint32_t gr2d_dstst_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_dstst_dsts_s(void)
{
    return 16;
}

static inline uint32_t gr2d_dstst_dsts_f(uint32_t v)
{
    return (v & 0xffff) << 0;
}

static inline uint32_t gr2d_dstst_dsts_m(void)
{
    return 0xffff << 0;
}

static inline uint32_t gr2d_dstst_dsts_v(uint32_t r)
{
    return (r >> 0) & 0xffff;
}

static inline uint32_t gr2d_srcba_r(void)
{
    return 0x31;
}

static inline uint32_t gr2d_srcba_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_srcba_srcba_s(void)
{
    return 32;
}

static inline uint32_t gr2d_srcba_srcba_f(uint32_t v)
{
    return (v & 0xffffffff) << 0;
}

static inline uint32_t gr2d_srcba_srcba_m(void)
{
    return 0xffffffff << 0;
}

static inline uint32_t gr2d_srcba_srcba_v(uint32_t r)
{
    return (r >> 0) & 0xffffffff;
}

static inline uint32_t gr2d_srcst_r(void)
{
    return 0x33;
}

static inline uint32_t gr2d_srcst_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_srcst_srcs_s(void)
{
    return 16;
}

static inline uint32_t gr2d_srcst_srcs_f(uint32_t v)
{
    return (v & 0xffff) << 0;
}

static inline uint32_t gr2d_srcst_srcs_m(void)
{
    return 0xffff << 0;
}

static inline uint32_t gr2d_srcst_srcs_v(uint32_t r)
{
    return (r >> 0) & 0xffff;
}

static inline uint32_t gr2d_srcfgc_r(void)
{
    return 0x35;
}

static inline uint32_t gr2d_srcfgc_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_srcfgc_srcfgc_s(void)
{
    return 32;
}

static inline uint32_t gr2d_srcfgc_srcfgc_f(uint32_t v)
{
    return (v & 0xffffffff) << 0;
}

static inline uint32_t gr2d_srcfgc_srcfgc_m(void)
{
    return 0xffffffff << 0;
}

static inline uint32_t gr2d_srcfgc_srcfgc_v(uint32_t r)
{
    return (r >> 0) & 0xffffffff;
}

static inline uint32_t gr2d_srcsize_r(void)
{
    return 0x37;
}

static inline uint32_t gr2d_srcsize_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_srcsize_srcwidth_s(void)
{
    return 15;
}

static inline uint32_t gr2d_srcsize_srcwidth_f(uint32_t v)
{
    return (v & 0x7fff) << 0;
}

static inline uint32_t gr2d_srcsize_srcwidth_m(void)
{
    return 0x7fff << 0;
}

static inline uint32_t gr2d_srcsize_srcwidth_v(uint32_t r)
{
    return (r >> 0) & 0x7fff;
}

static inline uint32_t gr2d_srcsize_srcheight_s(void)
{
    return 15;
}

static inline uint32_t gr2d_srcsize_srcheight_f(uint32_t v)
{
    return (v & 0x7fff) << 16;
}

static inline uint32_t gr2d_srcsize_srcheight_m(void)
{
    return 0x7fff << 16;
}

static inline uint32_t gr2d_srcsize_srcheight_v(uint32_t r)
{
    return (r >> 16) & 0x7fff;
}

static inline uint32_t gr2d_dstsize_r(void)
{
    return 0x38;
}

static inline uint32_t gr2d_dstsize_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_dstsize_dstwidth_s(void)
{
    return 15;
}

static inline uint32_t gr2d_dstsize_dstwidth_f(uint32_t v)
{
    return (v & 0x7fff) << 0;
}

static inline uint32_t gr2d_dstsize_dstwidth_m(void)
{
    return 0x7fff << 0;
}

static inline uint32_t gr2d_dstsize_dstwidth_v(uint32_t r)
{
    return (r >> 0) & 0x7fff;
}

static inline uint32_t gr2d_dstsize_dstheight_s(void)
{
    return 15;
}

static inline uint32_t gr2d_dstsize_dstheight_f(uint32_t v)
{
    return (v & 0x7fff) << 16;
}

static inline uint32_t gr2d_dstsize_dstheight_m(void)
{
    return 0x7fff << 16;
}

static inline uint32_t gr2d_dstsize_dstheight_v(uint32_t r)
{
    return (r >> 16) & 0x7fff;
}

static inline uint32_t gr2d_srcps_r(void)
{
    return 0x39;
}

static inline uint32_t gr2d_srcps_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_srcps_srcx_s(void)
{
    return 16;
}

static inline uint32_t gr2d_srcps_srcx_f(uint32_t v)
{
    return (v & 0xffff) << 0;
}

static inline uint32_t gr2d_srcps_srcx_m(void)
{
    return 0xffff << 0;
}

static inline uint32_t gr2d_srcps_srcx_v(uint32_t r)
{
    return (r >> 0) & 0xffff;
}

static inline uint32_t gr2d_srcps_srcy_s(void)
{
    return 16;
}

static inline uint32_t gr2d_srcps_srcy_f(uint32_t v)
{
    return (v & 0xffff) << 16;
}

static inline uint32_t gr2d_srcps_srcy_m(void)
{
    return 0xffff << 16;
}

static inline uint32_t gr2d_srcps_srcy_v(uint32_t r)
{
    return (r >> 16) & 0xffff;
}

static inline uint32_t gr2d_dstps_r(void)
{
    return 0x3a;
}

static inline uint32_t gr2d_dstps_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_dstps_dstx_s(void)
{
    return 16;
}

static inline uint32_t gr2d_dstps_dstx_f(uint32_t v)
{
    return (v & 0xffff) << 0;
}

static inline uint32_t gr2d_dstps_dstx_m(void)
{
    return 0xffff << 0;
}

static inline uint32_t gr2d_dstps_dstx_v(uint32_t r)
{
    return (r >> 0) & 0xffff;
}

static inline uint32_t gr2d_dstps_dsty_s(void)
{
    return 16;
}

static inline uint32_t gr2d_dstps_dsty_f(uint32_t v)
{
    return (v & 0xffff) << 16;
}

static inline uint32_t gr2d_dstps_dsty_m(void)
{
    return 0xffff << 16;
}

static inline uint32_t gr2d_dstps_dsty_v(uint32_t r)
{
    return (r >> 16) & 0xffff;
}

static inline uint32_t gr2d_uvstride_r(void)
{
    return 0x44;
}

static inline uint32_t gr2d_uvstride_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_uvstride_uvstride_s(void)
{
    return 16;
}

static inline uint32_t gr2d_uvstride_uvstride_f(uint32_t v)
{
    return (v & 0xffff) << 0;
}

static inline uint32_t gr2d_uvstride_uvstride_m(void)
{
    return 0xffff << 0;
}

static inline uint32_t gr2d_uvstride_uvstride_v(uint32_t r)
{
    return (r >> 0) & 0xffff;
}

static inline uint32_t gr2d_tilemode_r(void)
{
    return 0x46;
}

static inline uint32_t gr2d_tilemode_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_tilemode_src_y_tile_mode_s(void)
{
    return 1;
}

static inline uint32_t gr2d_tilemode_src_y_tile_mode_f(uint32_t v)
{
    return (v & 0x1) << 0;
}

static inline uint32_t gr2d_tilemode_src_y_tile_mode_m(void)
{
    return 0x1 << 0;
}

static inline uint32_t gr2d_tilemode_src_y_tile_mode_v(uint32_t r)
{
    return (r >> 0) & 0x1;
}

static inline uint32_t gr2d_tilemode_src_y_tile_mode_linear_v(void)
{
    return 0;
}

static inline uint32_t gr2d_tilemode_src_y_tile_mode_tiled_v(void)
{
    return 1;
}

static inline uint32_t gr2d_tilemode_src_uv_tile_mode_s(void)
{
    return 1;
}

static inline uint32_t gr2d_tilemode_src_uv_tile_mode_f(uint32_t v)
{
    return (v & 0x1) << 4;
}

static inline uint32_t gr2d_tilemode_src_uv_tile_mode_m(void)
{
    return 0x1 << 4;
}

static inline uint32_t gr2d_tilemode_src_uv_tile_mode_v(uint32_t r)
{
    return (r >> 4) & 0x1;
}

static inline uint32_t gr2d_tilemode_src_uv_tile_mode_linear_v(void)
{
    return 0;
}

static inline uint32_t gr2d_tilemode_src_uv_tile_mode_tiled_v(void)
{
    return 1;
}

static inline uint32_t gr2d_tilemode_pat_y_tile_mode_s(void)
{
    return 1;
}

static inline uint32_t gr2d_tilemode_pat_y_tile_mode_f(uint32_t v)
{
    return (v & 0x1) << 8;
}

static inline uint32_t gr2d_tilemode_pat_y_tile_mode_m(void)
{
    return 0x1 << 8;
}

static inline uint32_t gr2d_tilemode_pat_y_tile_mode_v(uint32_t r)
{
    return (r >> 8) & 0x1;
}

static inline uint32_t gr2d_tilemode_pat_y_tile_mode_linear_v(void)
{
    return 0;
}

static inline uint32_t gr2d_tilemode_pat_y_tile_mode_tiled_v(void)
{
    return 1;
}

static inline uint32_t gr2d_tilemode_pat_uv_tile_mode_s(void)
{
    return 1;
}

static inline uint32_t gr2d_tilemode_pat_uv_tile_mode_f(uint32_t v)
{
    return (v & 0x1) << 12;
}

static inline uint32_t gr2d_tilemode_pat_uv_tile_mode_m(void)
{
    return 0x1 << 12;
}

static inline uint32_t gr2d_tilemode_pat_uv_tile_mode_v(uint32_t r)
{
    return (r >> 12) & 0x1;
}

static inline uint32_t gr2d_tilemode_pat_uv_tile_mode_linear_v(void)
{
    return 0;
}

static inline uint32_t gr2d_tilemode_pat_uv_tile_mode_tiled_v(void)
{
    return 1;
}

static inline uint32_t gr2d_tilemode_dst_rd_tile_mode_s(void)
{
    return 1;
}

static inline uint32_t gr2d_tilemode_dst_rd_tile_mode_f(uint32_t v)
{
    return (v & 0x1) << 16;
}

static inline uint32_t gr2d_tilemode_dst_rd_tile_mode_m(void)
{
    return 0x1 << 16;
}

static inline uint32_t gr2d_tilemode_dst_rd_tile_mode_v(uint32_t r)
{
    return (r >> 16) & 0x1;
}

static inline uint32_t gr2d_tilemode_dst_rd_tile_mode_linear_v(void)
{
    return 0;
}

static inline uint32_t gr2d_tilemode_dst_rd_tile_mode_tiled_v(void)
{
    return 1;
}

static inline uint32_t gr2d_tilemode_dst_wr_tile_mode_s(void)
{
    return 1;
}

static inline uint32_t gr2d_tilemode_dst_wr_tile_mode_f(uint32_t v)
{
    return (v & 0x1) << 20;
}

static inline uint32_t gr2d_tilemode_dst_wr_tile_mode_m(void)
{
    return 0x1 << 20;
}

static inline uint32_t gr2d_tilemode_dst_wr_tile_mode_v(uint32_t r)
{
    return (r >> 20) & 0x1;
}

static inline uint32_t gr2d_tilemode_dst_wr_tile_mode_linear_v(void)
{
    return 0;
}

static inline uint32_t gr2d_tilemode_dst_wr_tile_mode_tiled_v(void)
{
    return 1;
}

static inline uint32_t gr2d_srcba_sb_surfbase_r(void)
{
    return 0x48;
}

static inline uint32_t gr2d_srcba_sb_surfbase_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_srcba_sb_surfbase_src_addr_s(void)
{
    return 32;
}

static inline uint32_t gr2d_srcba_sb_surfbase_src_addr_f(uint32_t v)
{
    return (v & 0xffffffff) << 0;
}

static inline uint32_t gr2d_srcba_sb_surfbase_src_addr_m(void)
{
    return 0xffffffff << 0;
}

static inline uint32_t gr2d_srcba_sb_surfbase_src_addr_v(uint32_t r)
{
    return (r >> 0) & 0xffffffff;
}

static inline uint32_t gr2d_dstba_sb_surfbase_r(void)
{
    return 0x49;
}

static inline uint32_t gr2d_dstba_sb_surfbase_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_dstba_sb_surfbase_dst_addr_s(void)
{
    return 32;
}

static inline uint32_t gr2d_dstba_sb_surfbase_dst_addr_f(uint32_t v)
{
    return (v & 0xffffffff) << 0;
}

static inline uint32_t gr2d_dstba_sb_surfbase_dst_addr_m(void)
{
    return 0xffffffff << 0;
}

static inline uint32_t gr2d_dstba_sb_surfbase_dst_addr_v(uint32_t r)
{
    return (r >> 0) & 0xffffffff;
}

static inline uint32_t gr2d_vba_sb_surfbase_r(void)
{
    return 0x4b;
}

static inline uint32_t gr2d_vba_sb_surfbase_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_vba_sb_surfbase_v_addr_s(void)
{
    return 32;
}

static inline uint32_t gr2d_vba_sb_surfbase_v_addr_f(uint32_t v)
{
    return (v & 0xffffffff) << 0;
}

static inline uint32_t gr2d_vba_sb_surfbase_v_addr_m(void)
{
    return 0xffffffff << 0;
}

static inline uint32_t gr2d_vba_sb_surfbase_v_addr_v(uint32_t r)
{
    return (r >> 0) & 0xffffffff;
}

static inline uint32_t gr2d_uba_sb_surfbase_r(void)
{
    return 0x4c;
}

static inline uint32_t gr2d_uba_sb_surfbase_reset_val_v(void)
{
    return 0x0;
}

static inline uint32_t gr2d_uba_sb_surfbase_u_addr_s(void)
{
    return 32;
}

static inline uint32_t gr2d_uba_sb_surfbase_u_addr_f(uint32_t v)
{
    return (v & 0xffffffff) << 0;
}

static inline uint32_t gr2d_uba_sb_surfbase_u_addr_m(void)
{
    return 0xffffffff << 0;
}

static inline uint32_t gr2d_uba_sb_surfbase_u_addr_v(uint32_t r)
{
    return (r >> 0) & 0xffffffff;
}

#endif
