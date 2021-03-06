/******************************************************************************
 *
 * Board Configuration File
 *
 * - Compiler:          GNU GCC for AVR32
 * - Supported devices: traq|paq hardware version 1.4
 * - AppNote:			N/A
 *
 * - Last Author:		Ryan David ( ryan.david@redline-electronics.com )
 *
 *
 * Copyright (c) 2012 Redline Electronics LLC.
 *
 * This file is part of traq|paq.
 *
 * traq|paq is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * traq|paq is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with traq|paq. If not, see http://www.gnu.org/licenses/.
 *
 ******************************************************************************/

#ifndef CONF_BOARD_H
#define CONF_BOARD_H

// Configure/Deconfigure hardware on the module
/*#define TRAQPAQ_HW_PWM_ENABLED			TRUE
#define TRAQPAQ_HW_EBI_ENABLED			TRUE
#define TRAQPAQ_HW_GPS_ENABLED			TRUE
#define TRAQPAQ_HW_SPI_ENABLED			TRUE
#define TRAQPAQ_HW_TWI_ENABLED			TRUE
#define TRAQPAQ_HW_EXINT_ENABLED		TRUE
#define TRAQPAQ_HW_USB_ENABLED			TRUE
#define TRAQPAQ_HW_DEBUG_ENABLED		TRUE
#define TRAQPAQ_HW_PLL_ENABLED			TRUE
#define TRAQPAQ_HW_WDT_ENABLED			TRUE*/

#define TRAQPAQ_NORMAL_MODE_ON_USB		FALSE
#define TRAQPAQ_DEBUG_ENABLED			TRUE
#define TRAQPAQ_BATTERY_TEST_MODE		FALSE
#define TRAQPAQ_GPS_TTF_TEST_MODE		FALSE
#define TRAQPAQ_UBLOX_GPS				TRUE

#if( (TRAQPAQ_DEBUG_ENABLED == TRUE) && (TRAQPAQ_GPS_ECHO_MODE == TRUE) )
#error "Debug Mode and GPS Echo Mode both turned on!"
#endif


#endif // CONF_BOARD_H
