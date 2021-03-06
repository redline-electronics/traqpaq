/******************************************************************************
 *
 * Memory Interface Include
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


#ifndef DATAFLASH_H_
#define DATAFLASH_H_

#include "hal.h"

#define FLASH_VERSION					"1.20"

#define flash_clr_wp()					gpio_set_gpio_pin(DATAFLASH_WP)
#define flash_set_wp()					gpio_clr_gpio_pin(DATAFLASH_WP)

#define flash_clr_hold()				gpio_set_gpio_pin(DATAFLASH_HOLD)
#define flash_set_hold()				gpio_clr_gpio_pin(DATAFLASH_HOLD)

#define flash_set_busy_flag()			flash.flags.isBusy = TRUE
#define flash_clr_busy_flag()			flash.flags.isBusy = FALSE
#define flash_busy_flag()				flash.flags.isBusy

#define flash_set_full_flag()			flash.flags.isFull = TRUE
#define flash_clr_full_flag()			flash.flags.isFull = FALSE
#define flash_full_flag()				flash.flags.isFull

// Device IDs for AT25DF321
#define FLASH_ATMEL_AT25DF321_MAN_ID		0x1F
#define FLASH_ATMEL_AT25DF321_ID0			0x47
#define FLASH_ATMEL_AT25DF321_ID1			0x01

// Device IDs for AT25DF161
#define FLASH_ATMEL_AT25DF161_MAN_ID		0x1F
#define FLASH_ATMEL_AT25DF161_ID0			0x46
#define FLASH_ATMEL_AT25DF161_ID1			0x02


// Read Commands
#define DATAFLASH_CMD_READ_ARRAY			0x03	// Up to 50MHz operation

// Program and Erase Commands
#define FLASH_CMD_BLOCK_ERASE_4KB			0x20
#define DATAFLASH_CMD_BLOCK_ERASE_32KB		0x52
#define DATAFLASH_CMD_BLOCK_ERASE_64KB		0xD8
#define DATAFLASH_CMD_CHIP_ERASE			0x60	// 0xC7 is alternate command
#define DATAFLASH_CMD_PAGE_PROGRAM			0x02
#define DATAFLASH_CMD_PGM_ERASE_SUSPEND		0xB0	// Pause a previous Program or Erase command
#define DATAFLASH_CMD_PGM_ERASE_RESUME		0xD0	// Resume a previous Program or Erase command

// Protection Commands
#define DATAFLASH_CMD_WRITE_ENABLE			0x06
#define DATAFLASH_CMD_WRITE_DISABLE			0x04
#define DATAFLASH_CMD_PROTECT_SECTOR		0x36
#define DATAFLASH_CMD_UNPROTECT_SECTOR		0x39
#define DATAFLASH_CMD_READ_PROTECTION		0x3C

// Security Commands
#define DATAFLASH_CMD_SECTOR_LOCKDOWN		0x33
#define DATAFLASH_CMD_FREEZE_SECTOR_LOCKDOWN 0x34
#define DATAFLASH_CMD_READ_SECTOR_LOCKDOWN	0x35
#define DATAFLASH_CMD_PROGRAM_OTP			0x9B
#define DATAFLASH_CMD_READ_OTP				0x77

// Status Register Commands
#define DATAFLASH_CMD_READ_STATUS			0x05
#define DATAFLASH_CMD_WRITE_STATUS1			0x01
#define DATAFLASH_CMD_WRITE_STATUS2			0x31

// Miscellaneous Commands
#define DATAFLASH_CMD_RESET					0xF0
#define DATAFLASH_CMD_READ_DEVICE_ID		0x9F
#define DATAFLASH_CMD_DEEP_POWER_DOWN		0xB9
#define DATAFLASH_CMD_WAKEUP				0xAB


// Dummy Bytes returned from device for specific commands
#define DATAFLASH_CMD_DUMMY					0xFF
#define DATAFLASH_OTP_RESERVED				0xAA

#define DATAFLASH_SECTOR_PROTECTED			1	// Sector is protected and cannot be programmed or erased. This is the default state
#define DATAFLASH_SECTOR_UNPROTECTED		0	// Sector is unprotected and can be programmed and erased

#define DATAFLASH_STATUS1_SPRL_LOCKED		1	// Sector Protection Registers are locked
#define DATAFLASH_STATUS1_SPRL_UNLOCKED		0	// Sector Protection Registers are unlocked

#define DATAFLASH_STATUS1_EPE_SUCCESSFUL	0	// Erase or Program operation was sucessful
#define DATAFLASH_STATUS1_EPE_UNSUCCESSFUL	1	// Erase or program error was detected

#define DATAFLASH_STATUS1_WPP_ASSERTED		0	// Write Protect is asserted
#define DATAFLASH_STATUS1_WPP_DEASSERTED	1	// Write Protect is deasserted

#define DATAFLASH_STATUS1_SWP_ALL_UNPROTECT	0	// All sectors are software unprotected
#define DATAFLASH_STATUS1_SWP_SOME_PROTECT	1	// Some sectors are software protected
#define DATAFLASH_STATUS1_SWP_RESERVED		2	// Reserved for future use
#define DATAFLASH_STATUS1_SWP_ALL_PROTECT	3	// All sectors are software protected

#define DATAFLASH_STATUS1_WEL_WRITE_DISABLED	0	// Device is not write enabled
#define DATAFLASH_STATUS1_WEL_WRITE_ENABLED	1	// Device is write enabled

#define DATAFLASH_STATUS1_RDY_READY			0	// Device is ready
#define DATAFLASH_STATUS1_RDY_BUSY			1	// Device is busy with an internal operation

#define DATAFLASH_STATUS2_RSTE_ENABLED		1	// Reset command is enabled
#define DATAFLASH_STATUS2_RSTE_DISABLED		0	// Reset command is disabled (default)

#define DATAFLASH_STATUS2_SLE_DISABLED		0	// Sector lockdown and Freeze Sector Lockdown state commands are disabled
#define DATAFLASH_STATUS2_SLE_ENABLED		1	// Sector lockdown and Freeze Sector Lockdown state commands are enabled

#define DATAFLASH_STATUS2_PS_NOT_SUSPENDED	0	// No sectors are program suspended
#define DATAFLASH_STATUS2_PS_SUSPENDED		1	// A sector is program suspended

#define DATAFLASH_STATUS2_ES_NOT_SUSPENDED	0	// No sectors are erase suspended (default)
#define DATAFLASH_STATUS2_ES_SUSPENDED		1	// A sector is erase suspended

#define DATAFLASH_STATUS2_RDY_READY			0	// Device is ready
#define DATAFLASH_STATUS2_RDY_BUSY			1	// Device is busy with an internal operation		



#define DATAFLASH_WR_STATUS1_SPRL_BIT		7
#define DATAFLASH_WR_STATUS1_SPRL_MASK		(0b1 << DATAFLASH_WR_STATUS1_SPRL_BIT)
#define DATAFLASH_WR_STATUS1_SPRL_LOCKED	1	// Sector Protection Registers are locked
#define DATAFLASH_WR_STATUS1_SPRL_UNLOCKED	0	// Sector Protection Registers are unlocked


#define DATAFLASH_WR_STATUS1_GLOBAL_BIT			2
#define DATAFLASH_WR_STATUS1_GLOBAL_MASK		(0b1111 << DATAFLASH_WR_STATUS1_GLOBAL_BIT)
#define DATAFLASH_WR_STATUS1_GLOBAL_LOCKED		0b1111	// All sectors locked
#define DATAFLASH_WR_STATUS1_GLOBAL_UNLOCKED	0b0000	// All sectors unlocked


#define DATAFLASH_WR_STATUS2_RSTE_BIT		4
#define DATAFLASH_WR_STATUS2_RSTE_MASK		(0b1 << DATAFLASH_WR_STATUS2_RSTE_BIT)
#define DATAFLASH_WR_STATUS2_RSTE_ENABLED	1	// Reset command is enabled
#define DATAFLASH_WR_STATUS2_RSTE_DISABLED	0	// Reset command is enabled (default)

#define DATAFLASH_WR_STATUS2_SLE_BIT		3
#define DATAFLASH_WR_STATUS2_SLE_MASK		(0b1 << DATAFLASH_WR_STATUS2_SLE_BIT)
#define DATAFLASH_WR_STATUS2_SLE_ENABLED	1	// Sector lockdown and Freeze Sector Lockdown state commands are enabled
#define DATAFLASH_WR_STATUS2_SLE_DISABLED	0	// Sector lockdown and Freeze Sector Lockdown state commands are disabled

#define DATAFLASH_RESPONSE_OK				1
#define DATAFLASH_RESPONSE_FAILURE			0

#define DATAFLASH_STATUS_GLOBAL_UNPROTECT	0x00


#define FLASH_4KB							4096	// Number of bytes in 4KB
#define DATAFLASH_32KB						32768	// Number of bytes in 32KB


#define DATAFLASH_PDCA_CHECK_TIME			5	// Time in milliseconds to suspend task before checking PDCA status again
#define DATAFLASH_STATUS_CHECK_TIME			10	// Time in milliseconds to suspend task before checking status register again
#define DATAFLASH_PROGRAM_TIME				2
#define DATAFLASH_ERASE_TIME				20

struct tDataflashStatusRegisters{
	unsigned SPRL	: 1;		// Sector Protection Registers Locked
	unsigned RES0	: 1;		// Reserved for future use
	unsigned EPE	: 1;		// Erase/Program Error
	unsigned WPP	: 1;		// Write Protect Pin Status
	unsigned SWP	: 2;		// Software Protection Status
	unsigned WEL	: 1;		// Write Enable Latch Status
	unsigned BSY0	: 1;		// Ready/Busy Status
	
	unsigned RES1	: 3;		// Reserved for future use
	unsigned RSTE	: 1;		// Reset Enabled
	unsigned SLE	: 1;		// Sector Lockdown Enabled
	unsigned PS		: 1;		// Program Suspend Status
	unsigned ES		: 1;		// Erase Suspend Status
	unsigned BSY1	: 1;		// Ready/Busy Status
};


union tDataflashStatus {
	struct tDataflashStatusRegisters registers;
	unsigned char raw[2];
};

struct tFlashFlags {
	unsigned char isBusy;
	unsigned char isFull;	
};

enum tFlashDevice {
	ATMEL_AT25DF161,
	ATMEL_AT25DF321,
	UNKNOWN_DEVICE
};

struct tFlashLayout {
	unsigned int userPrefsStart;
	unsigned int userPrefsEnd;
	
	unsigned int trackListStart;
	unsigned int trackListEnd;
	
	unsigned int recordTableStart;
	unsigned int recordTableEnd;
	
	unsigned int recordDataStart;
	unsigned int recordDataEnd;
};

struct tFlash {
	enum tFlashDevice device;
	union tDataflashStatus status;
	struct tFlashFlags flags;
	struct tFlashLayout layout;
};

void flash_task_init( void );
void flash_task( void *pvParameters );
enum tFlashDevice  flash_initDevice( void );
union tDataflashStatus flash_readStatus( void );
unsigned char flash_GlobalUnprotect( void );
unsigned char flash_WriteEnable( void );
unsigned char flash_WriteDisable( void );
unsigned char flash_UpdateSector(unsigned long startAddress, unsigned short length, unsigned char *bufferPointer);
unsigned char flash_ReadToBuffer(unsigned long startAddress, unsigned short length, unsigned char *bufferPointer);
unsigned char flash_WriteFromBuffer(unsigned long startAddress, unsigned short length, unsigned char *bufferPointer);
unsigned char flash_ReadOTP(unsigned char startAddress, unsigned char length, unsigned char *bufferPointer);
unsigned char flash_WriteOTP(unsigned char startAddress, unsigned char length, unsigned char *bufferPointer);
unsigned char flash_eraseBlock(unsigned char blockSize, unsigned long startAddress);
unsigned char flash_chipErase( void );
unsigned char flash_powerDown( void );
unsigned char flash_wakeUp( void );
unsigned char flash_is_busy( void );
unsigned short flash_calculate_otp_crc( void );
unsigned short flash_calculate_userPrefs_crc( void );
unsigned char flash_send_request(unsigned char command, unsigned char *pointer, unsigned short length, unsigned int index, unsigned char resume, unsigned char delay);
unsigned char flash_eraseRecordedData( void );
unsigned char flash_operation_failed( void );
unsigned char flash_eraseTracks( void );

#endif /* DATAFLASH_H_ */