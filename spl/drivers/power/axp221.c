/*
 * (C) Copyright 2018-2020
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 *
 */

#include <common.h>
#include <arch/pmic_bus.h>
#include <arch/axp221_reg.h>


static int pmu_set_vol(char *name, int set_vol, int onoff);

static axp_contrl_info axp_ctrl_tbl[] = {

	{ "dcdc2", 600, 1540, BOOT_POWER221_DC2OUT_VOL, 0x3f, BOOT_POWER221_OUTPUT_CTL1, 2, 0,
	{ {600, 1540, 20},} },

	{ "dcdc3", 600, 1860, BOOT_POWER221_DC3OUT_VOL, 0x3f, BOOT_POWER221_OUTPUT_CTL1, 3, 0,
	{ {600, 1540, 20},} },

	{ "dcdc5", 1000, 2550, BOOT_POWER221_DC5OUT_VOL, 0x1f, BOOT_POWER221_OUTPUT_CTL1, 5, 0,
	{ {1000, 2550, 50},} },

};

int axp221_set_power_reset(void)
{
	u8 reg_value;
	if (pmic_bus_read(AXP221_RUNTIME_ADDR, BOOT_POWER221_VOFF_SET, &reg_value)) {
		return -1;
	}
	reg_value |= (1 << 6);
	if (pmic_bus_write(AXP221_RUNTIME_ADDR, BOOT_POWER221_VOFF_SET, reg_value)) {
		return -1;
	}
	return 0;
}

int axp221_probe_power_key(void)
{
	u8 reg_value;

	if (pmic_bus_read(AXP221_RUNTIME_ADDR, BOOT_POWER221_POWER_KEY_STATUS,
			  &reg_value)) {
		return -1;
	}
	/* POKLIRQ,POKSIRQ */
	reg_value &= (0x03 << BOOT_POWER221_POWER_KEY_OFFSET);
	if (reg_value) {
		if (pmic_bus_write(AXP221_RUNTIME_ADDR, BOOT_POWER221_POWER_KEY_STATUS,
				   reg_value)) {
			return -1;
		}
	}

	return (reg_value >> BOOT_POWER221_POWER_KEY_OFFSET) & 3;
}

int axp221_get_power_source(void)
{
	u8 reg_value;
	if (pmic_bus_read(AXP221_RUNTIME_ADDR, BOOT_POWER221_OTG_STATUS, &reg_value)) {
		return -1;
	}

	if (reg_value & (1 << 0)) {
		return AXP_BOOT_SOURCE_BUTTON;
	} else if (reg_value & (1 << 1)) {
		return AXP_BOOT_SOURCE_CHARGER;
	} else if (reg_value & (1 << 2)) {
		return AXP_BOOT_SOURCE_BATTERY;
	}
	return -1;
}

static axp_contrl_info *get_ctrl_info_from_tbl(char *name)
{
	int i    = 0;
	int size = ARRAY_SIZE(axp_ctrl_tbl);
	for (i = 0; i < size; i++) {
		if (!strncmp(name, axp_ctrl_tbl[i].name,
			     strlen(axp_ctrl_tbl[i].name))) {
			break;
		}
	}
	if (i >= size) {
		return NULL;
	}
	return (axp_ctrl_tbl + i);
}

static int pmu_axp221_set_vol(char *name, int set_vol, int onoff)
{
	u8 reg_value, i;
	axp_contrl_info *p_item = NULL;
	u8 base_step		= 0;

	p_item = get_ctrl_info_from_tbl(name);
	if (!p_item) {
		return -1;
	}

	if ((set_vol > 0) && (p_item->min_vol)) {
		if (set_vol < p_item->min_vol) {
			set_vol = p_item->min_vol;
		} else if (set_vol > p_item->max_vol) {
			set_vol = p_item->max_vol;
		}
		if (pmic_bus_read(AXP221_RUNTIME_ADDR, p_item->cfg_reg_addr,
				  &reg_value)) {
			return -1;
		}

		reg_value &= ~p_item->cfg_reg_mask;

		for (i = 0; p_item->axp_step_tbl[i].step_max_vol != 0; i++) {
			if ((set_vol > p_item->axp_step_tbl[i].step_max_vol) &&
				(set_vol < p_item->axp_step_tbl[i+1].step_min_vol)) {
				set_vol = p_item->axp_step_tbl[i].step_max_vol;
			}
			if (p_item->axp_step_tbl[i].step_max_vol >= set_vol) {
				reg_value |= ((base_step + ((set_vol - p_item->axp_step_tbl[i].step_min_vol)/
					p_item->axp_step_tbl[i].step_val)) << p_item->reg_addr_offest);
				if (p_item->axp_step_tbl[i].regation) {
					u8 reg_value_temp = (~reg_value & p_item->cfg_reg_mask);
					reg_value &= ~p_item->cfg_reg_mask;
					reg_value |= reg_value_temp;
				}
				break;
			} else {
				base_step += ((p_item->axp_step_tbl[i].step_max_vol -
					p_item->axp_step_tbl[i].step_min_vol + p_item->axp_step_tbl[i].step_val) /
					p_item->axp_step_tbl[i].step_val);
			}
		}

		if (pmic_bus_write(AXP221_RUNTIME_ADDR, p_item->cfg_reg_addr,
				   reg_value)) {
			return -1;
		}
	}

	if (onoff < 0) {
		return 0;
	}
	if (pmic_bus_read(AXP221_RUNTIME_ADDR, p_item->ctrl_reg_addr,
			  &reg_value)) {
		return -1;
	}
	if (onoff == 0) {
		reg_value &= ~(1 << p_item->ctrl_bit_ofs);
	} else {
		reg_value |= (1 << p_item->ctrl_bit_ofs);
	}
	if (pmic_bus_write(AXP221_RUNTIME_ADDR, p_item->ctrl_reg_addr,
			   reg_value)) {
		return -1;
	}


	pmic_bus_read(AXP221_RUNTIME_ADDR, BOOT_POWER221_DC5OUT_VOL, &reg_value);
	printf(" name = %s ,BOOT_POWER221_DC5OU33T_VOL = %x\n", name, reg_value);

	return 0;

}

static int pmu_get_vol(char *name)
{

	u8 reg_value, i;
	axp_contrl_info *p_item = NULL;
	u8 base_step1 = 0;
	u8 base_step2 = 0;
	int vol;

	p_item = get_ctrl_info_from_tbl(name);
	if (!p_item) {
		return -1;
	}

	if (pmic_bus_read(AXP221_RUNTIME_ADDR, p_item->ctrl_reg_addr,
		&reg_value)) {
		return -1;
	}
	if (!(reg_value & (0x01 << p_item->ctrl_bit_ofs))) {
		return 0;
	}

	if (pmic_bus_read(AXP221_RUNTIME_ADDR, p_item->cfg_reg_addr,
		&reg_value)) {
		return -1;
	}
	reg_value &= p_item->cfg_reg_mask;
	reg_value >>= p_item->reg_addr_offest;
	for (i = 0; p_item->axp_step_tbl[i].step_max_vol != 0; i++) {
		base_step1 += ((p_item->axp_step_tbl[i].step_max_vol -
				p_item->axp_step_tbl[i].step_min_vol + p_item->axp_step_tbl[i].step_val) /
				p_item->axp_step_tbl[i].step_val);
		if (p_item->axp_step_tbl[i].regation)
				reg_value = (~reg_value & (p_item->cfg_reg_mask >> p_item->reg_addr_offest));
		if (reg_value < base_step1) {
			vol =  (reg_value - base_step2) * p_item->axp_step_tbl[i].step_val +
					p_item->axp_step_tbl[i].step_min_vol;
			return vol;
		}
		if (p_item->axp_step_tbl[i].regation)
			reg_value = (~reg_value & (p_item->cfg_reg_mask >> p_item->reg_addr_offest));
		base_step2 += ((p_item->axp_step_tbl[i].step_max_vol -
				p_item->axp_step_tbl[i].step_min_vol + p_item->axp_step_tbl[i].step_val) /
				p_item->axp_step_tbl[i].step_val);
	}

	return -1;
}


static int pmu_set_vol(char *name, int set_vol, int onoff)
{
	int i, temp_vol, src_vol = pmu_get_vol(name);
	u32 step_voltage = 0xffff;
	axp_contrl_info *p_item = NULL;
	p_item = get_ctrl_info_from_tbl(name);
	if (!p_item) {
		return -1;
	}

	for (i = 0; p_item->axp_step_tbl[i].step_val != 0; i++) {
		step_voltage = min(step_voltage, p_item->axp_step_tbl[i].step_val);
	}

	if (step_voltage == 0xffff)
		return -1;

	if (src_vol > set_vol) {
		for (temp_vol = src_vol; temp_vol >= set_vol; temp_vol -= step_voltage) {
			if (pmu_axp221_set_vol(name, temp_vol, onoff))
				return -1;
		}
		udelay(step_voltage*10/25);
	} else if (src_vol < set_vol) {
		for (temp_vol = src_vol; temp_vol <= set_vol; temp_vol += step_voltage) {
			if (pmu_axp221_set_vol(name, temp_vol, onoff))
				return -1;
		}
		udelay(step_voltage*10/25);
	}
	return 0;
}
int axp221_set_ddr_voltage(int set_vol)
{
	return pmu_set_vol(AXP221_SUNXI_DRAM_VOL, set_vol, 1);
}

int axp221_set_pll_voltage(int set_vol)
{
	return pmu_set_vol(AXP221_SUNXI_CPU_VOL, set_vol, 1);
}

int axp221_set_sys_voltage(int set_vol, int onoff)
{
	return pmu_set_vol(AXP221_SUNXI_SYS_VOL, set_vol, onoff);
}

int axp221_set_sys_voltage_ext(char *name, int set_vol, int onoff)
{
	return pmu_set_vol(name, set_vol, onoff);
}

static int axp221_necessary_reg_enable(void)
{
	__attribute__((unused)) u8 reg_value;

#ifdef CONFIG_ARCH_SUN8IW11
	/*set dcdc dvm mode*/
/*	if (pmic_bus_read(AXP221_RUNTIME_ADDR, BOOT_POWER221_DC23_DVM_CTL, &reg_value))
		return -1;
	reg_value |= (0x3f << 2);
	if (pmic_bus_write(AXP221_RUNTIME_ADDR, BOOT_POWER221_DC23_DVM_CTL, reg_value))
		return -1;

	if (pmic_bus_read(AXP221_RUNTIME_ADDR, BOOT_POWER221_HOTOVER_CTL, &reg_value))
		return -1;
	reg_value |= (1<<0);
	if (pmic_bus_write(AXP221_RUNTIME_ADDR, BOOT_POWER221_HOTOVER_CTL, reg_value))
		return -1;
*/
#endif

	return 0;
}

int axp221_reg_read(u8 addr, u8 *val)
{
	return pmic_bus_read(AXP221_RUNTIME_ADDR, addr, val);
}

int axp221_reg_write(u8 addr, u8 val)
{
	return pmic_bus_write(AXP221_RUNTIME_ADDR, addr, val);
}


int axp221_axp_init(u8 power_mode)
{
	pmu_err("axpxp_init\n");
	u8 pmu_type;

	if (pmic_bus_init(AXP221_DEVICE_ADDR, AXP221_RUNTIME_ADDR)) {
		pmu_err("bus init error\n");
		return -1;
	}

	if (pmic_bus_read(AXP221_RUNTIME_ADDR, BOOT_POWER221_VERSION,
			  &pmu_type)) {
		pmu_err("bus read error\n");
		return -1;
	}

	pmu_type &= 0xCF;
	pmu_err(" \n\n pmuu_type = %x\n", pmu_type);
	if (pmu_type == 0x06) {
		/* pmu type AXP221 */
		axp221_necessary_reg_enable();
		printf("PMU: AXP221\n");
#if 0
		pwrok_restart_enable();
		disable_dcdc_pfm_mode();
#endif
		return AXP221_CHIP_ID;
	}
	printf("unknow PMU\n");
	return -1;
}
