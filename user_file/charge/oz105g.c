/*
 *
 */


/**
 * header files
 */
#include"oz105g.h"


/**
 * global definitions and macros ...
 */

/* parameter configuration
 */
#define CONFIG_CV		4200
#define CONFIG_CC	2000
#define CONFIG_ILIMIT	500//2000
#define CONFIG_EOC	200

#define CONFIG_WAKEUP_VOLT	3000
#define CONFIG_WAKEUP_CURR	400
#define CONFIG_RECHG_VOLT	100
#define CONFIG_VSYS_MIN		3400


#define OZ105C_WRITE_ADDR	0x20
#define OZ105C_READ_ADDR	0x21


/* debug related
 */
#define OZ105C_DEBUG	1

#if defined(OZ105C_DEBUG)

#define oz105c_debug(fmt, args...) 	\
	printf("[oz105c debug]: [%s] " fmt "\n", __FUNCTION__, ##args)
#define oz015c_debug_na(fmt, args...)	do {} while (0)
#else
#define oz105c_debug(fmt, args...) 	do {} while (0)
#define oz015c_debug_na(fmt, args...)	do {} while (0)
#endif


/* Voltage Registers (R/W) 
 */
#define	REG_CHARGER_VOLTAGE		0x00
#define	REG_T34_CHG_VOLTAGE		0x01
#define	REG_T45_CHG_VOLTAGE		0x02
 #define CHG_VOLT_STEP			25	//step 25mV

#define	REG_WAKEUP_VOLTAGE		0x03
 #define WK_VOLT_STEP			100	//step 100mV
#define	REG_RECHARGE_HYSTERESIS		0x04
#define RECHG_VOLT_STEP			50	//step 50mV
#define	REG_MIN_VSYS_VOLTAGE		0x05
 #define VSYS_VOLT_STEP			200	//step 200mV

/* Current Registers (R/W) 
 */
#define	REG_CHARGE_CURRENT		0x10
 #define CHG_CURRT_STEP			100	//step 100mA

#define	REG_WAKEUP_CURRENT		0x11
 #define WK_CURRT_STEP			10	//step 10mA

#define	REG_END_CHARGE_CURRENT		0x12
 #define EOC_CURRT_STEP			10	//step 10mA

#define	REG_VBUS_LIMIT_CURRENT		0x13
 #define VBUS_ILMT_STEP			100	//step 100mA

/* Protection Register (R/W) 
 */
#define	REG_SAFETY_TIMER 	0x20
 #define WAKEUP_TIMER_MASK 		0x0F
 #define WK_TIMER_15MIN			0x01	//15min wakeup timer
 #define WK_TIMER_30MIN			0x02	//30min wakeup timer
 #define WK_TIMER_45MIN			0x03	//45min wakeup timer
 #define WK_TIMER_60MIN			0x04	//60min wakeup timer
 #define WK_TIMER_75MIN			0x05	//60min wakeup timer
 #define WK_TIMER_90MIN			0x06	//60min wakeup timer
 #define WK_TIMER_105MIN		0x07	//60min wakeup timer
 #define CC_TIMER_MASK			0xF0
 #define CC_TIMER_120MIN		0x10	//120min CC charge timer
 #define CC_TIMER_180MIN		0x20	//180min CC charge timer
 #define CC_TIMER_240MIN		0x30	//240min CC charge timer
 #define CC_TIMER_300MIN		0x40	//300min CC charge timer
 #define CC_TIMER_390MIN		0x50	//390min CC charge timer
 #define CC_TIMER_480MIN		0x60	//480min CC charge timer
 #define CC_TIMER_570MIN		0x70	//570min CC charge timer

/* Charger Control Register (R/W) 
 */
#define	REG_CHARGER_CONTROL		0x30
 #define RTHM_SELECT				0x01	//0:100K, 1:10K

/* Status Registers (R) 
 */
#define	REG_VBUS_STATUS			0x40
 #define	VSYS_OVP_FLAG			0x01	//VSYS OVP event flag
 #define	VBUS_UVP_FLAG			0x10	//VBUS UVP event flag
 #define	VBUS_OK_FLAG			0x20	//VBUS OK flag
 #define	VBUS_OVP_FLAG			0x40	//VBUS OVP event flag
 #define	VDC_PR_FLAG                     0x80    // 1 when VDC < VDC threshold for system priority

#define	REG_CHARGER_STATUS		0x41
 #define	CHARGER_INIT			0x01	//Before init flag
 #define	IN_WAKEUP_STATE			0x02	//In Wakeup State
 #define	IN_CC_STATE			0x04	//In CC Charge State
 #define	IN_CV_STATE			0x08	//In CV Charge State
 #define	CHARGE_FULL_STATE		0x10	//Charge Full State
 #define	WK_TIMER_FLAG			0x20	//WK CHG Timer Overflow
 #define	CC_TIMER_FLAG			0x40	//CC CHG Timer Overflow 

#define	REG_THM_STATUS			0x42
 #define	THM_T1_STATE			0x01	//T1 > THM state 
 #define	THM_T12_STATE			0x02	//THM in T12 state 
 #define	THM_T23_STATE			0x04	//THM in T23 state 
 #define	THM_T34_STATE			0x08	//THM in T34 state 
 #define	THM_T45_STATE			0x10	//THM in T45 state 
 #define	THM_T5_STATE			0x20	//THM > T5 state 
 #define	INT_OTP_FLAG			0x40	//Internal OTP event


 void oz105c_i2c_Init(void)
 {
	 twi_master_init();
 }
 

/*
 * return 0 if succeed, return negative if error
 * device address 0x10, read address 0x21
 */
static int oz105c_read_byte(unsigned char reg, unsigned char *val)
{
	bool transfer_succeeded;
	transfer_succeeded = twi_master_transfer(OZ105C_WRITE_ADDR, &reg, 1, TWI_DONT_ISSUE_STOP);	//write
	transfer_succeeded &=twi_master_transfer(OZ105C_READ_ADDR, val, 1, TWI_ISSUE_STOP);	//read

	if (transfer_succeeded)
		return 0;
	else
		return -1;
}

/*
 * return 0 if succeed, return negative if error
 * device address 0x10, read address 0x20
 */

static int oz105c_write_byte(unsigned char reg, unsigned char val)
{
	bool transfer_succeeded;
	uint8_t w2_data[2];
	w2_data[0] = reg;
	w2_data[1] = val;
	transfer_succeeded =  twi_master_transfer(OZ105C_WRITE_ADDR, w2_data, 2, TWI_ISSUE_STOP);
	if (transfer_succeeded)
		return 0;
	else
		return -1;
}

static int oz105c_update_bits(unsigned char reg, unsigned char mask, unsigned char val)
{
	int ret;
	unsigned char tmp = 0;

	ret = oz105c_read_byte(reg, &tmp);

	//oz105c_debug("reg--0x%x,%d",reg,tmp);
	if (ret < 0) {
		oz105c_debug("oz105c read byte error:"
				" %d (reg 0x%02x--%d\r\n)", ret,reg,tmp);
		return ret;
	}

	if ((tmp & mask) != val) {
		tmp &= ~mask;
		tmp |= val & mask;
		return oz105c_write_byte(reg, tmp);
	} else {
		return 0;
	}
}

static int oz105c_update_reg(unsigned char reg, unsigned char val)
{
	int ret;
	unsigned char tmp;

	ret = oz105c_read_byte(reg, &tmp);
	if (ret < 0) {
#if 0
		oz105c_debug("oz105c read byte error:"
				" %d (reg 0x%02x)", ret);
#else
		oz105c_debug("oz105c read byte error:"
				" %d", ret);
#endif
		return ret;
	}

	if (val != tmp)
		ret = oz105c_write_byte(reg, val);

	return ret;
}

int oz105c_set_min_vsys(int min_vsys_mv)
{
	unsigned char val;

	if (min_vsys_mv < 1800)
		min_vsys_mv = 1800;
	else if (min_vsys_mv > 3600)
		min_vsys_mv = 3600;

	val = min_vsys_mv / VSYS_VOLT_STEP;

	return oz105c_update_bits(REG_MIN_VSYS_VOLTAGE, 0x1f, val);
}

int oz105c_get_min_vsys(void)
{
	unsigned char val;
	int ret;

	ret = oz105c_read_byte(REG_MIN_VSYS_VOLTAGE, &val);
	if (ret < 0)
		return ret;

	return ( (val & 0x1f) * VSYS_VOLT_STEP);
}

static int __oz105c_set_chg_volt(unsigned char reg, int chgvolt_mv)
{

//	int ret;
	unsigned char chg_volt;

	if (chgvolt_mv < 4000)
		chgvolt_mv = 4000;
	else if (chgvolt_mv > 4600)
		chgvolt_mv = 4600;

	chg_volt = (chgvolt_mv - 4000) / CHG_VOLT_STEP ;

	return oz105c_update_bits(reg, 0x1f, chg_volt);
}

static int __oz105c_get_chg_volt(unsigned char reg)
{
	int ret;
	unsigned char val;

	ret = oz105c_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	return ((int)val * CHG_VOLT_STEP);

}

#define oz105c_set_chg_volt(mv)	__oz105c_set_chg_volt(REG_CHARGER_VOLTAGE, mv)
#define oz105c_get_chg_volt()	__oz105c_get_chg_volt(REG_CHARGER_VOLTAGE)

#define oz105c_set_t34_volt(mv)	__oz105c_set_chg_volt(REG_T34_CHG_VOLTAGE, mv)
#define oz105c_get_t34_volt()	__oz105c_get_chg_volt(REG_T34_CHG_VOLTAGE)

#define oz105c_set_t45_volt(mv)	__oz105c_set_chg_volt(REG_T45_CHG_VOLTAGE, mv)
#define oz105c_get_t45_volt()	__oz105c_get_chg_volt(REG_T45_CHG_VOLTAGE)

int oz105c_set_wakeup_volt(int wakeup_mv)
{
	unsigned char val;

	if (wakeup_mv < 1500)
		wakeup_mv = 1500;
	else if (wakeup_mv > 3000)
		wakeup_mv = 3000;

	val = wakeup_mv / WK_VOLT_STEP;

	return oz105c_update_reg(REG_WAKEUP_VOLTAGE, val);
}

int oz105c_set_rechg_hystersis(int hyst_mv)
{
	unsigned char val;

	if (hyst_mv > 200)
		hyst_mv = 200;
	else if (hyst_mv < 0)
		hyst_mv = 0;

	val = hyst_mv / RECHG_VOLT_STEP;

	return oz105c_update_reg(REG_RECHARGE_HYSTERESIS, val);	
}

int oz105c_set_eoc_current(int eoc_ma)
{
	unsigned char val;

	if (eoc_ma <= 0)
		eoc_ma = 0;
	else if (eoc_ma > 320)
		eoc_ma = 320;

	val = eoc_ma / EOC_CURRT_STEP;

	return oz105c_update_reg(REG_END_CHARGE_CURRENT, val);
}

int oz105c_get_eoc_current(void)
{
	int ret;
	unsigned char val;

	ret = oz105c_read_byte(REG_END_CHARGE_CURRENT, &val);
	if (ret < 0)
		return ret;

	return (val * EOC_CURRT_STEP);
}


int oz105c_set_charger_current(int chg_ma)
{
	unsigned char val;

	if (chg_ma > 4000)
		chg_ma = 4000;
	else if (chg_ma < 600 && chg_ma > 0)
		chg_ma = 600;
	else if (chg_ma < 0)
		chg_ma = 0;

	val = chg_ma / CHG_CURRT_STEP;

	return oz105c_update_reg(REG_CHARGE_CURRENT, val);
}

int oz105c_get_charger_current(void)
{
	int ret;
	unsigned char val;

	ret = oz105c_read_byte(REG_CHARGE_CURRENT, &val);
	if (ret < 0)
		return ret;

	return (val * CHG_CURRT_STEP);
}


int oz105c_set_wakeup_current(int wak_ma)
{
	unsigned char val;

	if (wak_ma < 100)
		wak_ma = 100;
	if (wak_ma > 400)
		wak_ma = 400;

	val = wak_ma / WK_CURRT_STEP;
	return oz105c_update_reg(REG_WAKEUP_CURRENT, val);
}

int oz105c_set_vbus_current(int ilmt_ma)
{
//	int ret;
	unsigned char val;

	if(ilmt_ma < 300)
		val = 0x01;		//100mA
	else if(ilmt_ma >= 300 && ilmt_ma < 600)
		val = 0x05;		//500mA
	else if(ilmt_ma >= 600 && ilmt_ma < 800)
		val = 0x0c;	//700mA
	else if(ilmt_ma >= 800 && ilmt_ma < 950)
		val = 0x09;	//900mA
	else if(ilmt_ma >= 950 && ilmt_ma < 1100)
		val = 0x10;	//1000mA
	else if(ilmt_ma >= 1100 && ilmt_ma < 1300)
		val = 0x12;	//1200mA
	else if(ilmt_ma >= 1300 && ilmt_ma < 1450)
		val = 0x0e;	//1400mA
	else if(ilmt_ma >= 1450 && ilmt_ma < 1600)
		val = 0x0f;	//1500mA
	else if(ilmt_ma >= 1600 && ilmt_ma < 1800)
		val = 0x11;	//1700mA
	else if(ilmt_ma >= 1800 && ilmt_ma < 1950)
		val = 0x13;	//1900mA
	else if(ilmt_ma >= 1950)
		val = 0x14;	//2000mA

	return oz105c_update_reg(REG_VBUS_LIMIT_CURRENT, val);
}

int oz105c_get_vbus_current(void)
{
	unsigned char val;
	int ret;

	ret = oz105c_read_byte(REG_VBUS_LIMIT_CURRENT, &val);
	if (ret < 0)
		return ret;

	switch(val) {
		case 0x00: return 0;
		case 0x01: ;
		case 0x02: ;
		case 0x03: return 100;
		case 0x04: ;
		case 0x05: ;
		case 0x06: ;
		case 0x07: return 500;
		case 0x08: ;
		case 0x09: ;
		case 0x0a: ;
		case 0x0b: return 900;
		case 0x0c: ;
		case 0x0d: return 700;
		case 0x0e: return 1400;
		case 0x0f: return 1500;
		case 0x10: return 1000;
		case 0x11: return 1700;
		case 0x12: return 1200;
		case 0x13: return 1900;
		case 0x14: return 2000;

		default:
			return -1;
	}
}

int oz105c_set_safety_cc_timer(int tmin)
{
	unsigned char val;
	unsigned char Data;
	int ret;

	switch (tmin) {
	case 0:
		val = 0;
		break;
	case 120:
		val = CC_TIMER_120MIN;
		break;
	case 180:
		val = CC_TIMER_180MIN;
		break;
	case 240:
		val = CC_TIMER_240MIN;
		break;
	case 300:
		val = CC_TIMER_300MIN;
		break;
	case 390:
		val = CC_TIMER_390MIN;
		break;
	case 480:
		val = CC_TIMER_480MIN;
		break;
	case 570:
		val = CC_TIMER_570MIN;
		break;
	default:
		oz105c_debug("invalid value");
		val = CC_TIMER_180MIN;
		break;
	}

	ret = oz105c_read_byte(REG_SAFETY_TIMER, &Data);
	if (ret < 0)
		return ret;

	if ((Data & CC_TIMER_MASK) != val) {
		Data &= WAKEUP_TIMER_MASK;
		Data |= val;
		ret = oz105c_write_byte(REG_SAFETY_TIMER, Data);
	}

	return ret;
}

int oz105c_set_safety_wk_timer(int tmin)
{
	unsigned char val, data;
	int ret;

	//Notice: with x0h, saftety timer function is disabled
	switch (tmin) {
	case 0:		
		val = 0; 
		break;
	case 15:
		val = WK_TIMER_15MIN;
		break;
	case 30:
		val = WK_TIMER_30MIN;
		break;
	case 45:
		val = WK_TIMER_45MIN;
		break;
	case 60:
		val = WK_TIMER_60MIN;
		break;
	case 75:
		val = WK_TIMER_75MIN;
		break;
	case 90:
		val = WK_TIMER_90MIN;
		break;
	case 105:
		val = WK_TIMER_105MIN;
		break;
	default:
		oz105c_debug("invalid value");
		val = WK_TIMER_30MIN;
	}

	ret = oz105c_read_byte(REG_SAFETY_TIMER, &data);
	if (ret < 0)
		return ret;

	if ((data & WAKEUP_TIMER_MASK) != val) {
		data &= CC_TIMER_MASK;
		data |= val;
		ret = oz105c_write_byte(REG_SAFETY_TIMER, data);
	}

	return ret;
}

int oz105c_enable_otg(int enable)
{
	unsigned char val, mask;

	val = (!enable) ? 0 : 1;

	if (val)
		mask = 0x06;
	else
		mask = 0x04;

	return oz105c_update_bits(REG_CHARGER_CONTROL, mask, val << 2);
}


int oz105c_init(void)
{
//	int ret = 0;

	oz105c_debug();

	/* 
	 * basic configuration, do not change them
	 */

	/* write rthm  100k (0) */
	oz105c_update_bits(REG_CHARGER_CONTROL, RTHM_SELECT, 0);
	
	/* set charger PWM TON time */
	oz105c_update_bits(REG_CHARGER_CONTROL, 0x18, 0x0 << 3);

	/* min vsys */
	oz105c_set_min_vsys(CONFIG_VSYS_MIN);
	
	/* EOC */
	oz105c_set_eoc_current(CONFIG_EOC);

	/* disable CC TIMER and WAKEUP TIMER */
	oz105c_set_safety_cc_timer(0);
	oz105c_set_safety_wk_timer(0);

	/* recharger hysteresis: 100mV */
	oz105c_set_rechg_hystersis(CONFIG_RECHG_VOLT);

	/* CV T34 T45 CHARGE VOLTAGE set to 4400mV */
	oz105c_set_t34_volt(CONFIG_CV);
	oz105c_set_t45_volt(CONFIG_CV);
	oz105c_set_chg_volt(CONFIG_CV);

	/* wakeup volatge: defualt to 3000mV */
	oz105c_set_wakeup_volt(CONFIG_WAKEUP_VOLT);

	/* wakeup current: defualt to 400mA*/
	oz105c_set_wakeup_current(CONFIG_WAKEUP_CURR);

	/* charge current: set to 2000mA */
	oz105c_set_charger_current(CONFIG_CC);

	/* vbus current: set to 2000mA */
	oz105c_set_vbus_current(CONFIG_ILIMIT);
	
	return 0;
}

#define OZ105C_DUMP_REGS

int oz105c_dump(void)
{
	int ret;

#if defined(OZ105C_DUMP_REGS)
	int i;
	unsigned char data;
#endif

	ret = oz105c_get_chg_volt();
	printf("CV: %d(mv),	", ret);

	ret = oz105c_get_chg_volt();
	printf("CV_T34: %d(mv),	", ret);

	ret = oz105c_get_chg_volt();
	printf("CV_T45: %d(mv).\n", ret);

	ret = oz105c_get_vbus_current();
	printf("ilimit: %d(ma), ", ret);

	ret = oz105c_get_charger_current();
	printf("CC: %d(ma), ", ret);

	ret = oz105c_get_eoc_current();
	printf("EOC: %d(ma).\n", ret);

	ret = oz105c_get_min_vsys();
	printf("vsysmin: %d(mv)\n", ret);
	
#if defined(OZ105C_DUMP_REGS)
    	for (i=REG_CHARGER_VOLTAGE; i<=REG_MIN_VSYS_VOLTAGE; i++) {
		oz105c_read_byte(i, &data);
		printf("[0x%02x]=0x%02x ", i, data);        
    	}
    	for (i=REG_CHARGE_CURRENT; i<=REG_VBUS_LIMIT_CURRENT; i++) {
		oz105c_read_byte(i, &data);
		printf("[0x%02x]=0x%02x ", i, data);        
	}

	oz105c_read_byte(REG_SAFETY_TIMER, &data);
	printf("[0x%02x]=0x%02x ", REG_SAFETY_TIMER, data);        

	oz105c_read_byte(REG_CHARGER_CONTROL, &data);
	printf("[0x%02x]=0x%02x ", REG_CHARGER_CONTROL, data);        

    	for (i=REG_VBUS_STATUS; i<=REG_THM_STATUS; i++) {
		oz105c_read_byte(i, &data);
		printf("[0x%02x]=0x%02x ", i, data);        
    	}
    
	printf("\n");
#endif
			return ret;
}



