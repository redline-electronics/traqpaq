/*! \file *********************************************************************
 *
 * \brief CONF_EBI EBI/SMC driver for AVR32 UC3.
 *
 * \note The values defined in this file are device-specific. See the device
 *       datasheet for further information.
 *
 * - Compiler:           IAR EWAVR32 and GNU GCC for AVR32
 * - Supported devices:  All AVR32 devices with an SMC module can be used.
 * - AppNote:
 *
 * \author               Atmel Corporation: http://www.atmel.com \n
 *                       Support and FAQ: http://support.atmel.no/
 *
 ******************************************************************************/

/* Copyright (c) 2009 Atmel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an Atmel
 * AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 *
 */

#ifndef _CONF_EBI_H_
#define _CONF_EBI_H_

#include "compiler.h"
#include "board.h"

#define SMC_USE_NCS0                             
#define SMC_COMPONENT_CS0			"smc_lcd.h"

#define EBI_DATA_0                  AVR32_EBI_DATA_0
#define EBI_DATA_1                  AVR32_EBI_DATA_1
#define EBI_DATA_2                  AVR32_EBI_DATA_2
#define EBI_DATA_3                  AVR32_EBI_DATA_3
#define EBI_DATA_4                  AVR32_EBI_DATA_4
#define EBI_DATA_5                  AVR32_EBI_DATA_5
#define EBI_DATA_6                  AVR32_EBI_DATA_6
#define EBI_DATA_7                  AVR32_EBI_DATA_7
#define EBI_DATA_8                  AVR32_EBI_DATA_8
#define EBI_DATA_9                  AVR32_EBI_DATA_9
#define EBI_DATA_10                 AVR32_EBI_DATA_10
#define EBI_DATA_11                 AVR32_EBI_DATA_11
#define EBI_DATA_12                 AVR32_EBI_DATA_12
#define EBI_DATA_13                 AVR32_EBI_DATA_13
#define EBI_DATA_14                 AVR32_EBI_DATA_14
#define EBI_DATA_15                 AVR32_EBI_DATA_15

#define EBI_ADDR_21                 AVR32_EBI_ADDR_21_1
#define EBI_NCS_0                   AVR32_EBI_NCS_0_1
#define EBI_NWE0                    AVR32_EBI_NWE0_0
#define EBI_NRD                     AVR32_EBI_NRD_0

#endif  // _CONF_EBI_H_
