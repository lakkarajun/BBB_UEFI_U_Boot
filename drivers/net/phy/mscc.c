/*
 * Driver for Microsemi VSC85xx PHYs
 *
 * Author: John Haechten
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Microsemi Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <miiphy.h>

/* Microsemi PHY ID's */
#define PHY_ID_VSC8530                  0x00070560
#define PHY_ID_VSC8531                  0x00070570
#define PHY_ID_VSC8540                  0x00070760
#define PHY_ID_VSC8541                  0x00070770

/* Microsemi VSC85xx PHY Register Pages */
#define MSCC_EXT_PAGE_ACCESS            31     /* Page Access Register */
#define MSCC_PHY_PAGE_STANDARD		0x0000 /* Standard registers */
#define MSCC_PHY_PAGE_EXTENDED_1	0x0001 /* Extended registers - page 1 */
#define MSCC_PHY_PAGE_EXTENDED_2	0x0002 /* Extended registers - page 2 */
#define MSCC_PHY_PAGE_EXTENDED_3	0x0003 /* Extended registers - page 3 */
#define MSCC_PHY_PAGE_EXTENDED_4	0x0004 /* Extended registers - page 4 */
#define MSCC_PHY_PAGE_GPIO		0x0010 /* GPIO registers */
#define MSCC_PHY_PAGE_TEST		0x2A30 /* TEST Page registers */
#define MSCC_PHY_PAGE_TR		0x52B5 /* Token Ring Page registers */

/* MSCC PHY Auxiliary Control/Status Register */
#define MIIM_AUX_CNTRL_STAT_REG		0x1c
#define MIIM_AUX_CNTRL_STAT_ACTIPHY_TO	0x0004
#define MIIM_AUX_CNTRL_STAT_F_DUPLEX	0x0020
#define MIIM_AUX_CNTRL_STAT_SPEED_MASK	0x0018
#define MIIM_AUX_CNTRL_STAT_SPEED_POS	(3)
#define MIIM_AUX_CNTRL_STAT_SPEED_10M	(0x0)
#define MIIM_AUX_CNTRL_STAT_SPEED_100M	(0x1)
#define MIIM_AUX_CNTRL_STAT_SPEED_1000M	(0x2)

#define MSCC_PHY_EXT_PHY_CNTL_1         23
#define MAC_IF_SELECTION_MASK           0x1800
#define MAC_IF_SELECTION_GMII           0
#define MAC_IF_SELECTION_RMII           1
#define MAC_IF_SELECTION_RGMII          2
#define MAC_IF_SELECTION_POS            11
#
/* Extended Page 2 Registers */
#define MSCC_PHY_RGMII_CNTL             20
#define VSC_FAST_LINK_FAIL2_ENA_MASK    0x8000
#define RGMII_RX_CLK_OUT_POS            11
#define RGMII_RX_CLK_OUT_DIS            1
#define RGMII_RX_CLK_DELAY_POS          4
#define RGMII_RX_CLK_DELAY_MASK         0x0070
#define RGMII_TX_CLK_DELAY_POS          0
#define RGMII_TX_CLK_DELAY_MASK         0x0007

#define MSCC_PHY_WOL_MAC_CONTROL        27
#define EDGE_RATE_CNTL_POS              5
#define EDGE_RATE_CNTL_MASK             0x00E0

#define MSCC_PHY_RESET_TIMEOUT		(100)
#define MSCC_PHY_MICRO_TIMEOUT		(500)


typedef enum {
        VSC_PHY_RGMII_CLK_DELAY_200_PS =  0,     /**< RGMII/GMII Clock Delay (Skew) */
        VSC_PHY_RGMII_CLK_DELAY_800_PS =  1,     /**< RGMII/GMII Clock Delay (Skew) */
        VSC_PHY_RGMII_CLK_DELAY_1100_PS = 2,     /**< RGMII/GMII Clock Delay (Skew) */
        VSC_PHY_RGMII_CLK_DELAY_1700_PS = 3,     /**< RGMII/GMII Clock Delay (Skew) */
        VSC_PHY_RGMII_CLK_DELAY_2000_PS = 4,     /**< RGMII/GMII Clock Delay (Skew) */
        VSC_PHY_RGMII_CLK_DELAY_2300_PS = 5,     /**< RGMII/GMII Clock Delay (Skew) */
        VSC_PHY_RGMII_CLK_DELAY_2600_PS = 6,     /**< RGMII/GMII Clock Delay (Skew) */
        VSC_PHY_RGMII_CLK_DELAY_3400_PS = 7      /**< RGMII/GMII Clock Delay (Skew) */
} vsc_phy_rgmii_gmii_clk_skew;

typedef enum {
        VSC_PHY_CLK_SLEW_RATE_0 =  0,     /**< MAC i/f Clock Edge Rage Control (Slew), See Reg27E2  */
        VSC_PHY_CLK_SLEW_RATE_1 =  1,     /**< MAC i/f Clock Edge Rage Control (Slew), See Reg27E2  */
        VSC_PHY_CLK_SLEW_RATE_2 =  2,     /**< MAC i/f Clock Edge Rage Control (Slew), See Reg27E2  */
        VSC_PHY_CLK_SLEW_RATE_3 =  3,     /**< MAC i/f Clock Edge Rage Control (Slew), See Reg27E2  */
        VSC_PHY_CLK_SLEW_RATE_4 =  4,     /**< MAC i/f Clock Edge Rage Control (Slew), See Reg27E2  */
        VSC_PHY_CLK_SLEW_RATE_5 =  5,     /**< MAC i/f Clock Edge Rage Control (Slew), See Reg27E2  */
        VSC_PHY_CLK_SLEW_RATE_6 =  6,     /**< MAC i/f Clock Edge Rage Control (Slew), See Reg27E2  */
        VSC_PHY_CLK_SLEW_RATE_7 =  7,     /**< MAC i/f Clock Edge Rage Control (Slew), See Reg27E2  */
} vsc_phy_clk_slew;


static int mscc_parse_status(struct phy_device *phydev)
{
	u16 speed;
	u16 mii_reg;

	mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_AUX_CNTRL_STAT_REG);

	if (mii_reg & MIIM_AUX_CNTRL_STAT_F_DUPLEX)
		phydev->duplex = DUPLEX_FULL;
	else
		phydev->duplex = DUPLEX_HALF;

	speed = mii_reg & MIIM_AUX_CNTRL_STAT_SPEED_MASK;
	speed = speed >> MIIM_AUX_CNTRL_STAT_SPEED_POS;

	switch (speed) {
	case MIIM_AUX_CNTRL_STAT_SPEED_1000M:
		phydev->speed = SPEED_1000;
		break;
	case MIIM_AUX_CNTRL_STAT_SPEED_100M:
		phydev->speed = SPEED_100;
		break;
	case MIIM_AUX_CNTRL_STAT_SPEED_10M:
		phydev->speed = SPEED_10;
		break;
	default:
		phydev->speed = SPEED_10;
		break;
	}

	return (0);
}

static int mscc_startup(struct phy_device *phydev)
{
	int ret;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;
	return mscc_parse_status(phydev);
}

static int mscc_phy_soft_reset(struct phy_device *phydev)
{
        int     rc = 0;
        u16     timeout = MSCC_PHY_RESET_TIMEOUT;
        u16     reg_val = 0;

	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS, MSCC_PHY_PAGE_STANDARD);

	reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);
        reg_val |= BMCR_RESET;
        phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, reg_val);

	reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);

        while ((reg_val & BMCR_RESET) && (timeout > 0)) {
		reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);
                timeout--;
                udelay(1000);   /* 1 ms */
        }

        if (timeout == 0) {
                rc = -ETIME;
        }

        return (rc);
}

static int vsc8531_vsc8541_mac_config(struct phy_device *phydev)
{
        u16     reg_val = 0;

        // For VSC8530/31 the only MAC modes are RMII/RGMII.
        // For VSC8540/41 the only MAC modes are (G)MII and RMII/RGMII.
        // Setup MAC Configuration
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS, MSCC_PHY_PAGE_STANDARD);

        switch (phydev->interface) {
        case PHY_INTERFACE_MODE_MII:
        case PHY_INTERFACE_MODE_GMII:
		reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_EXT_PHY_CNTL_1);
		reg_val &= ~(MAC_IF_SELECTION_MASK);
                reg_val |= MAC_IF_SELECTION_GMII << MAC_IF_SELECTION_POS;
		phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_EXT_PHY_CNTL_1, reg_val); // Set Reg23.12:11 = 0
		phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS, MSCC_PHY_PAGE_EXTENDED_2);
		reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_RGMII_CNTL);
                reg_val &= ~(0x1 << RGMII_RX_CLK_OUT_POS);
                reg_val |= (0x1 << RGMII_RX_CLK_OUT_POS);
		phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_RGMII_CNTL, reg_val); // Set Reg20E2.11 = 1
		printf("PHY 8531 config = (G)MII \n");
                break;

        case PHY_INTERFACE_MODE_RMII:
                reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_EXT_PHY_CNTL_1);
                reg_val &= ~(MAC_IF_SELECTION_MASK);
                reg_val |= MAC_IF_SELECTION_RMII << MAC_IF_SELECTION_POS;
                phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_EXT_PHY_CNTL_1, reg_val); // Set Reg23.12:11 = 1
                phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS, MSCC_PHY_PAGE_EXTENDED_2);
                reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_RGMII_CNTL);
                reg_val &= ~(0x1 << RGMII_RX_CLK_OUT_POS);
                phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_RGMII_CNTL, reg_val); // Clear Reg20E2.11 = 1
		printf("PHY 8531 config = RMII \n");
                break;

        case PHY_INTERFACE_MODE_RGMII:
		reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_EXT_PHY_CNTL_1);
                reg_val &= ~(MAC_IF_SELECTION_MASK);
                reg_val |= MAC_IF_SELECTION_RGMII << MAC_IF_SELECTION_POS;
                phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_EXT_PHY_CNTL_1, reg_val); // Set Reg23.12:11 = 2
                phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS, MSCC_PHY_PAGE_EXTENDED_2);
                reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_RGMII_CNTL);
                reg_val &= ~(0x1 << RGMII_RX_CLK_OUT_POS);
                phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_RGMII_CNTL, reg_val); // Clear Reg20E2.11
		printf("PHY 8531 config = RGMII \n");
                break;

        default:
                return (-EINVAL);
	}

	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS, MSCC_PHY_PAGE_STANDARD);
	return (0);
}

static int vsc8531_config(struct phy_device *phydev)
{
	u16	rx_clk_skew	= VSC_PHY_RGMII_CLK_DELAY_1700_PS;
	u16	tx_clk_skew	= VSC_PHY_RGMII_CLK_DELAY_800_PS;
	u16	edge_rate	= VSC_PHY_CLK_SLEW_RATE_4;
	u16	reg_val;

        if ((phydev->interface == PHY_INTERFACE_MODE_RMII) || (phydev->interface == PHY_INTERFACE_MODE_RGMII)) {
		vsc8531_vsc8541_mac_config(phydev);
		mscc_phy_soft_reset(phydev);
        } else {
		printf("PHY 8531 MAC i/f config Error: mac i/f = 0x%x \n", phydev->interface);
                return (-EINVAL);
        }

	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS, MSCC_PHY_PAGE_EXTENDED_2);
	reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_RGMII_CNTL);
        reg_val &= ~(RGMII_RX_CLK_DELAY_MASK | RGMII_TX_CLK_DELAY_MASK);
        reg_val |= (rx_clk_skew << RGMII_RX_CLK_DELAY_POS) | (tx_clk_skew << RGMII_TX_CLK_DELAY_POS);
        phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_RGMII_CNTL, reg_val);

        reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_WOL_MAC_CONTROL);
        reg_val &= ~(EDGE_RATE_CNTL_MASK);
        reg_val |= edge_rate << EDGE_RATE_CNTL_POS;
        phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_WOL_MAC_CONTROL, reg_val);
        phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS, MSCC_PHY_PAGE_STANDARD);

	printf("printf: PHY VSC8530/VSC8531 MAC i/f config complete - going to ANEG \n");

        genphy_config_aneg(phydev);

        return (0);
}

static int vsc8541_config(struct phy_device *phydev)
{
        u16     rx_clk_skew     = VSC_PHY_RGMII_CLK_DELAY_800_PS;
        u16     tx_clk_skew     = VSC_PHY_RGMII_CLK_DELAY_800_PS;
        u16     edge_rate       = VSC_PHY_CLK_SLEW_RATE_4;
        u16     reg_val;

        if ((phydev->interface == PHY_INTERFACE_MODE_RMII) ||
                (phydev->interface == PHY_INTERFACE_MODE_RGMII) ||
                (phydev->interface == PHY_INTERFACE_MODE_MII) ||
                (phydev->interface == PHY_INTERFACE_MODE_GMII)) {
                vsc8531_vsc8541_mac_config(phydev);
                mscc_phy_soft_reset(phydev);
        } else {
		printf("PHY 8541 MAC i/f config Error: mac i/f = 0x%x \n", phydev->interface);
                return (-EINVAL);
        }

        phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS, MSCC_PHY_PAGE_EXTENDED_2);
        reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_RGMII_CNTL);
        reg_val &= ~(RGMII_RX_CLK_DELAY_MASK | RGMII_TX_CLK_DELAY_MASK);
        reg_val |= (rx_clk_skew << RGMII_RX_CLK_DELAY_POS) | (tx_clk_skew << RGMII_TX_CLK_DELAY_POS);
        phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_RGMII_CNTL, reg_val);

        reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_WOL_MAC_CONTROL);
        reg_val &= ~(EDGE_RATE_CNTL_MASK);
        reg_val |= edge_rate << EDGE_RATE_CNTL_POS;
        phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_WOL_MAC_CONTROL, reg_val);
        phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS, MSCC_PHY_PAGE_STANDARD);

	printf("printf: PHY VSC8540/VSC8541 MAC i/f config complete - going to ANEG \n");

        genphy_config_aneg(phydev);

        return (0);
}

static struct phy_driver VSC8530_driver = {
        .name = "Microsemi VSC8530",
        .uid = PHY_ID_VSC8530,
        .mask = 0x000ffff0,
        .features = (PHY_BASIC_FEATURES | SUPPORTED_Pause | SUPPORTED_Asym_Pause),
        .config = &vsc8531_config,
        .startup = &mscc_startup,
        .shutdown = &genphy_shutdown,
};

static struct phy_driver VSC8531_driver = {
        .name = "Microsemi VSC8531",
        .uid = PHY_ID_VSC8531,
        .mask = 0x000ffff0,
        .features = (PHY_GBIT_FEATURES | SUPPORTED_Pause | SUPPORTED_Asym_Pause),
        .config = &vsc8531_config,
        .startup = &mscc_startup,
        .shutdown = &genphy_shutdown,
};

static struct phy_driver VSC8540_driver = {
        .name = "Microsemi VSC8540",
        .uid = PHY_ID_VSC8540,
        .mask = 0x000ffff0,
        .features = (PHY_BASIC_FEATURES | SUPPORTED_Pause | SUPPORTED_Asym_Pause),
        .config = &vsc8541_config,
        .startup = &mscc_startup,
        .shutdown = &genphy_shutdown,
};

static struct phy_driver VSC8541_driver = {
        .name = "Microsemi VSC8541",
        .uid = PHY_ID_VSC8541,
        .mask = 0x000ffff0,
        .features = (PHY_GBIT_FEATURES | SUPPORTED_Pause | SUPPORTED_Asym_Pause),
        .config = &vsc8541_config,
        .startup = &mscc_startup,
        .shutdown = &genphy_shutdown,
};

int phy_mscc_init(void)
{
	phy_register(&VSC8530_driver);
	phy_register(&VSC8531_driver);
	phy_register(&VSC8540_driver);
	phy_register(&VSC8541_driver);

	return 0;
}
