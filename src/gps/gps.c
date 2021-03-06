/******************************************************************************
 *
 * GPS Interface
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
#include "asf.h"
#include "hal.h"
#include "math.h"
#include "lcd/itoa.h"
#include "string.h"

xQueueHandle gpsRxdQueue;
xQueueHandle gpsManagerQueue;
xTimerHandle xReceiverDeadTimer, xReceiverCfgTimer;

struct tGPSInfo gpsInfo;

__attribute__((__interrupt__)) static void ISR_gps_rxd(void){
	int rxd;
	
	switch( usart_read_char(GPS_USART, &rxd) ){
		case( USART_RX_ERROR ):
			usart_reset_status(GPS_USART);
			incrementErrorCount(gpsInfo.error.rxDataError);
			break;
			
		case( USART_SUCCESS ):
			xQueueSendFromISR(gpsRxdQueue, &rxd, pdFALSE);
			break;
		
		case( USART_RX_EMPTY ):
			incrementErrorCount(gpsInfo.error.rxDataError);
			break;
	}
}

void gps_task_init( void ){
	struct tGPSRequest request;
	
	// Initialize information about the GPS receiver
	gpsInfo.serial_number_valid = FALSE;
	gpsInfo.sw_date_valid = FALSE;
	gpsInfo.sw_version_valid = FALSE;
	gpsInfo.part_number_valid = FALSE;
	
	gpsInfo.mode = 0;
	gpsInfo.satellites = 0;
	gpsInfo.record_flag = FALSE;
	
	gpsInfo.error.checksumErrors = 0;
	gpsInfo.error.ggaMsgTimeouts = 0;
	gpsInfo.error.rmcMsgTimeouts = 0;
	gpsInfo.error.unrecognizedMsgs = 0;
	gpsInfo.error.rxDataError = 0;
	gpsInfo.error.resetCount = 0;
	
	gpsInfo.lastCmd.class = 0;
	gpsInfo.lastCmd.id = 0;
	gpsInfo.lastCmd.response = GPS_RESPONSE_UNKNOWN;
	
	gpsInfo.status = GPS_STATUS_UNKNOWN;
	
	gpsInfo.current_location.heading = 0;
	gpsInfo.current_location.latitude = 0;
	gpsInfo.current_location.longitude = 0;
	
	// Set up the GPS manager task
	#if( TRAQPAQ_GPS_EXTERNAL_LOGGING == FALSE )
	if(systemFlags.button.powerOnMethod == POWER_ON_MODE_BUTTON){
		gpsRxdQueue		= xQueueCreate( GPS_RXD_QUEUE_SIZE,     sizeof(int)     );
		gpsManagerQueue = xQueueCreate( GPS_MANAGER_QUEUE_SIZE, sizeof(request) );

		INTC_register_interrupt( (__int_handler) &ISR_gps_rxd, AVR32_USART3_IRQ, AVR32_INTC_INT0);

		xTaskCreate(gps_task, configTSK_GPS_TASK_NAME, configTSK_GPS_TASK_STACK_SIZE, NULL, configTSK_GPS_TASK_PRIORITY, configTSK_GPS_TASK_HANDLE);
	}
	#else
	gpio_set_gpio_pin(GPS_RESET);
	debug_log(DEBUG_PRIORITY_INFO, DEBUG_SENDER_GPS, "External logging mode");
	#endif
}


void gps_task( void *pvParameters ){
	unsigned char i;
	unsigned int rxdChar;										// Temporary storage for received character queue
	unsigned char recordIndex = 0;								// Index in formatted data struct
	unsigned char oldMode = 0;
	unsigned char calc_xsumA, calc_xsumB;
	
	unsigned int lapTime = 0, oldLapTime = 0;
	unsigned int datestamp = 0;
	
	struct tRecordDataPage gpsData;							// Formatted GPS Data
	struct tGPSLine finishLine;								// Formatted coordinate pairs for "finish line"
	struct tTracklist trackList;
	struct tGPSRequest request;
	enum tGPSStateMachine parserState = GPS_STATUS_UNKNOWN;
	struct tGPSMessage gpsMessage;
	struct tGPSChecksum gpsChecksum;
	struct tGPSRxdMessages gpsRxdMessages;
	
	gpsMessage.frameNumber = 0;			// Reset the Rx frame counter
	
	// Reset the flags for each solution
	gpsRxdMessages.NAV_POSLLH = FALSE;
	gpsRxdMessages.NAV_SOL = FALSE;
	gpsRxdMessages.NAV_VELNED = FALSE;

	// Make sure the battery isn't low before continuing
	fuel_low_battery_check();
	
	debug_log(DEBUG_PRIORITY_INFO, DEBUG_SENDER_GPS, "Task Started");
	
	// Pull the GPS out of reset and enable the ISR
	gps_enable_rxrdy_isr();
	gps_reset();
	
	xReceiverDeadTimer = xTimerCreate( "gpsDeadTimer",
										(GPS_DEAD_STARTUP_TIME / portTICK_RATE_MS),
										FALSE,
										GPS_DEAD_TIMER_ID,
										gps_dead );
										
	xReceiverCfgTimer = xTimerCreate( "gpsCfgTimer",
										(GPS_MSG_TX_TIME / portTICK_RATE_MS),
										FALSE,
										GPS_CFG_MSG_TIMER_ID,
										gps_configure );
										 
	// Kick off the timer
	xTimerStart(xReceiverDeadTimer, pdFALSE);
	xTimerStart(xReceiverCfgTimer, pdFALSE);
	
	while(TRUE){
		// Check for pending requests
		if( xQueueReceive(gpsManagerQueue, &request, pdFALSE) == pdTRUE ){
			switch(request.command){
				case(GPS_MGR_REQUEST_START_RECORDING):
					flash_send_request(FLASH_MGR_SET_DATESTAMP, NULL, NULL, datestamp, FALSE, pdFALSE);
					lapTime = 0;
					oldLapTime = 0xFFFFFFFF;
					gpsInfo.record_flag = TRUE;
					break;
					
				case(GPS_MGR_REQUEST_STOP_RECORDING):
					gpsInfo.record_flag = FALSE;
					recordIndex = 0;
					flash_send_request(FLASH_MGR_END_CURRENT_RECORD, NULL, NULL, NULL, FALSE, 20);
					break;
					
				case(GPS_MGR_REQUEST_SET_FINISH_POINT):
					// Load the track, and then tell the flash that we are using it
					flash_send_request(FLASH_MGR_READ_TRACK, &trackList, sizeof(trackList), request.data, TRUE, 20);
					flash_send_request(FLASH_MGR_SET_TRACK, NULL, NULL, request.data, FALSE, 20);
					finishLine = gps_find_finish_line(trackList.latitude, trackList.longitude, trackList.heading);
					break;
					
				case(GPS_MGR_REQUEST_CREATE_NEW_TRACK):
					itoa(gpsData.utc, &(trackList.name), 10, FALSE);
					trackList.heading = gpsInfo.current_location.heading;
					trackList.longitude = gpsInfo.current_location.longitude;
					trackList.latitude = gpsInfo.current_location.latitude;
					trackList.isEmpty = FALSE;
					trackList.reserved = 0xA5;
					flash_send_request(FLASH_MGR_ADD_TRACK, &trackList, NULL, NULL, FALSE, pdFALSE);
					break;
					
				case(GPS_MGR_REQUEST_SHUTDOWN):					
					gpio_clr_gpio_pin(GPS_RESET);	// Put the GPS into reset
					debug_log(DEBUG_PRIORITY_INFO, DEBUG_SENDER_GPS, "Task shut down");
					wdt_send_request(WDT_REQUEST_GPS_SHUTDOWN_COMPLETE, NULL);
					vTaskSuspend(NULL);
					break;
				
				case(GPS_MGR_REQUEST_LATITUDE):
					*(request.pointer) = gpsInfo.current_location.latitude;
					break;
					
				case(GPS_MGR_REQUEST_LONGITUDE):
					*(request.pointer) = gpsInfo.current_location.longitude;
					break;
					
				case(GPS_MGR_REQUEST_COURSE):
					*(request.pointer) = gpsInfo.current_location.heading;
					break;
					
				case(GPS_MGR_REQUEST_RECORD_STATUS):
					*(request.pointer) = gpsInfo.record_flag;
					break;
				
			}
			
			// Resume requesting task if it has been suspended
			if(request.resume == TRUE){
				vTaskResume(request.handle);
			}
		}
		
		// Service the received characters
		if( (parserState != GPS_STATE_RX_COMPLETE) && (xQueueReceive(gpsRxdQueue, &rxdChar, 20) == TRUE) ){
		
			switch(parserState){
				case(GPS_STATE_UNKNOWN):
					if(rxdChar == GPS_CHAR_SYNC1) {
						parserState = GPS_STATE_SYNC1;
					}						
					break;
				
				case(GPS_STATE_SYNC1):
					if(rxdChar == GPS_CHAR_SYNC2){
						parserState = GPS_STATE_CLASS;
					}else{
						parserState = GPS_STATE_UNKNOWN;
					}
					break;
				
				case(GPS_STATE_CLASS):
					gpsMessage.class = (unsigned char)rxdChar;
					parserState = GPS_STATE_ID;
					break;
				
				case(GPS_STATE_ID):
					gpsMessage.id = (unsigned char)rxdChar;
					parserState = GPS_STATE_LENGTH1;
					break;
				
				case(GPS_STATE_LENGTH1):
					gpsMessage.length = (unsigned char)rxdChar;
					parserState = GPS_STATE_LENGTH2;
					break;
				
				case(GPS_STATE_LENGTH2):
					gpsMessage.length += ((unsigned char)rxdChar << 8);
					
					if( gpsMessage.length >= GPS_MSG_MAX_LENGTH ) {
						gpsMessage.length = GPS_MSG_MAX_LENGTH;
					}						
					
					gpsMessage.rxCount = 0;
					parserState = GPS_STATE_PAYLOAD;
					break;
				
				case(GPS_STATE_PAYLOAD):
					if(gpsMessage.rxCount == gpsMessage.length) {	
						// This received character is checksumA
						gpsMessage.xsumA = (unsigned char)rxdChar;
						
						// Skip to checksumB
						parserState = GPS_STATE_XSUMB;
						
					}else{
						gpsMessage.messages.raw[gpsMessage.rxCount++] = (unsigned char)rxdChar;
					}
					break;
				

				case(GPS_STATE_XSUMB):
					gpsMessage.xsumB = (unsigned char)rxdChar;
					parserState = GPS_STATE_RX_COMPLETE;
					break;
			}	
		}
		
		
		if( parserState == GPS_STATE_RX_COMPLETE ){
			
			gpsChecksum = gps_calculateChecksum(&gpsMessage.class, sizeof(gpsMessage.class) + sizeof(gpsMessage.id) + sizeof(gpsMessage.length) + gpsMessage.length);
			
			if( (gpsChecksum.xsumA != gpsMessage.xsumA) ) {//|| (gpsChecksum.xsumB != gpsMessage.xsumB) ){		// TODO: Figure out why checksum_B does not work
				// Checksums did not match!
				parserState = GPS_STATE_UNKNOWN;
				incrementErrorCount(gpsInfo.error.checksumErrors);
				debug_log(DEBUG_PRIORITY_WARNING, DEBUG_SENDER_GPS, "Dropped packet");
				
			}else{
				// Checksums matched! Good message!
				gpsMessage.frameNumber++;
				debug_tgl_pin0();
				
				switch(gpsMessage.class){
					
					/////////////////////////////
					// UBX Message Class : ACK //
					/////////////////////////////
					case(UBX_CLASS_ACK):
						switch(gpsMessage.id){
							
							// *** ACK-NAK ***
							case(UBX_ACK_NAK):
								debug_log(DEBUG_PRIORITY_INFO, DEBUG_SENDER_GPS, "Receiver responded NAK");
								gpsInfo.lastCmd.class = gpsMessage.messages.ACK_NAK.clsID;
								gpsInfo.lastCmd.id = gpsMessage.messages.ACK_NAK.msgID;
								gpsInfo.lastCmd.response = GPS_RESPONSE_NAK;
								
								// Kick off the next configuration step
								if(gpsInfo.status == GPS_STATUS_UNKNOWN) gps_configure(NULL);
																
								break;
								
							// *** ACK-ACK ***
							case(UBX_ACK_ACK):
								debug_log(DEBUG_PRIORITY_INFO, DEBUG_SENDER_GPS, "Receiver responded ACK");
								gpsInfo.lastCmd.class = gpsMessage.messages.ACK_ACK.clsID;
								gpsInfo.lastCmd.id = gpsMessage.messages.ACK_ACK.msgID;
								gpsInfo.lastCmd.response = GPS_RESPONSE_ACK;
								
								// Kick off the next configuration step
								if(gpsInfo.status == GPS_STATUS_UNKNOWN) gps_configure(NULL);					
								
								break;
								
							// *** Unknown ***
							default:
								incrementErrorCount(gpsInfo.error.unrecognizedMsgs);
								break;
						}
						break;
						
				
					/////////////////////////////
					// UBX Message Class : NAV //
					/////////////////////////////
					case(UBX_CLASS_NAV):
						switch(gpsMessage.id){
							
							// *** NAV-PVT ***
							case(UBX_NAV_PVT):									
								break;
							
							// *** NAV-CLOCK ***
							case(UBX_NAV_CLOCK):
								break;
								
							// *** NAV-DGPS ***
							case(UBX_NAV_DGPS):
								break;
							
							// *** NAV-DOP ***
							case(UBX_NAV_DOP):
								break;
								
							// *** NAV-POSECEF ***
							case(UBX_NAV_POSECEF):
								break;
								
							// *** NAV-POSLLH ***
							case(UBX_NAV_POSLLH):
								gpsRxdMessages.NAV_POSLLH = TRUE;
								
								gpsData.utc = gps_flip_endian4(gpsMessage.messages.NAV_PVT.iTOW);
								
								gpsData.data[recordIndex].latitude = gps_flip_endian4(gpsMessage.messages.NAV_POSLLH.lat);
								gpsData.data[recordIndex].longitude = gps_flip_endian4(gpsMessage.messages.NAV_POSLLH.lon);
								gpsData.data[recordIndex].altitude = (gps_flip_endian4(gpsMessage.messages.NAV_POSLLH.hMSL) / 100) & 0xFFFF;
								
									
								// Copy over the current position info
								gpsInfo.current_location.latitude = gpsData.data[recordIndex].latitude;
								gpsInfo.current_location.longitude = gpsData.data[recordIndex].longitude;
								
								break;
							
							// *** NAV-PVT ***
							case(UBX_NAV_SBAS):
								break;
								
							// *** NAV-SOL ***
							case(UBX_NAV_SOL):
								gpsRxdMessages.NAV_SOL = TRUE;
								
								gpsData.utc = gps_flip_endian4(gpsMessage.messages.NAV_SOL.iTOW);
								gpsData.hdop = gps_flip_endian2(gpsMessage.messages.NAV_SOL.pDOP);
								gpsData.currentMode = gpsMessage.messages.NAV_SOL.gpsFix;
								gpsData.satellites = gpsMessage.messages.NAV_SOL.numSV;
								datestamp = gps_flip_endian2(gpsMessage.messages.NAV_SOL.week);

								// Copy over the current position info
								gpsInfo.satellites = gpsData.satellites;
								gpsInfo.mode = gpsData.currentMode;

								break;
								
							// *** NAV-STATUS ***
							case(UBX_NAV_STATUS):
								break;
								
							// *** NAV-SVINFO ***
							case(UBX_NAV_SVINFO):
								break;
								
							// *** NAV-TIMEGPS ***
							case(UBX_NAV_TIMEGPS):
								break;
								
							// *** NAV-TIMEUTC ***
							case(UBX_NAV_TIMEUTC):
								break;
								
							// *** NAV-VELECEF ***
							case(UBX_NAV_VELECEF):
								break;
								
							// *** NAV-VELNED ***
							case(UBX_NAV_VELNED):
								gpsRxdMessages.NAV_VELNED = TRUE;
								
								gpsData.data[recordIndex].speed = (unsigned short)gps_flip_endian4(gpsMessage.messages.NAV_VELNED.gSpeed);
								gpsData.data[recordIndex].heading = (unsigned short)gps_flip_endian4(gpsMessage.messages.NAV_VELNED.heading);
								
								// Copy the information over to the current location
								gpsInfo.current_location.heading = gpsData.data[recordIndex].heading;
								break;
								
							// *** Unknown NAV Message ***
							default:
								incrementErrorCount(gpsInfo.error.unrecognizedMsgs);
								break;
						}
						break;
					
					/////////////////////////////
					// UBX Message Class : RXM //
					/////////////////////////////
					case(UBX_CLASS_RXM):
						incrementErrorCount(gpsInfo.error.unrecognizedMsgs);
						break;
						
					/////////////////////////////
					// UBX Message Class : INF //
					/////////////////////////////
					case(UBX_CLASS_INF):
						switch(gpsMessage.id){
							
							// *** INF-DEBUG ***
							case(UBX_INF_DEBUG):
								debug_log(DEBUG_PRIORITY_INFO, DEBUG_SENDER_GPS, &gpsMessage.messages.raw);
								break;
							
							// *** INF-ERROR ***	
							case(UBX_INF_ERROR):
								debug_log(DEBUG_PRIORITY_CRITICAL, DEBUG_SENDER_GPS, &gpsMessage.messages.raw);
								break;
								
							// *** INF-NOTICE ***
							case(UBX_INF_NOTICE):
								debug_log(DEBUG_PRIORITY_INFO, DEBUG_SENDER_GPS, &gpsMessage.messages.raw);
								break;
								
							// *** INF-TEST ***
							case(UBX_INF_TEST):
								debug_log(DEBUG_PRIORITY_INFO, DEBUG_SENDER_GPS, &gpsMessage.messages.raw);
								break;
								
							// *** INF-WARNING ***
							case(UBX_INF_WARNING):
								debug_log(DEBUG_PRIORITY_WARNING, DEBUG_SENDER_GPS, &gpsMessage.messages.raw);
								break;
								
							// *** Unknown ***
							default:
								incrementErrorCount(gpsInfo.error.unrecognizedMsgs);
								break;
						}
						break;
						
					/////////////////////////////
					// UBX Message Class : CFG //
					/////////////////////////////
					case(UBX_CLASS_CFG):
						switch(gpsMessage.id) {
							
							// *** CFG-USB ***
							case(UBX_CFG_USB):
								gpsInfo.sw_version_valid = TRUE;
								memcpy(&gpsInfo.serial_number, &gpsMessage.messages.CFG_USB.serialNumber, sizeof(gpsInfo.serial_number));
								break;
								
							// *** CFG-ANT ***
							case(UBX_CFG_ANT):
								break;
								
							// *** CFG-CFG ***
							case(UBX_CFG_CFG):
								break;
								
							// *** CFG-DAT ***
							case(UBX_CFG_DAT):
								break;
								
							// *** CFG-GNSS ***
							case(UBX_CFG_GNSS):
								break;
								
							// *** CFG-INF ***
							case(UBX_CFG_INF):
								break;
								
							// *** CFG-ITFM ***
							case(UBX_CFG_ITFM):
								break;
							
							// *** CFG-MSG ***
							case(UBX_CFG_MSG):
								break;
								
							// *** CFG-NAV5 ***
							case(UBX_CFG_NAV5):
								break;
								
							// *** CFG-NAVX5 ***
							case(UBX_CFG_NAVX5):
								break;
							
							// *** CFG-NMEA ***
							case(UBX_CFG_NMEA):
								break;
								
							// *** CFG-NVS ***
							case(UBX_CFG_NVS):
								break;
								
							// *** CFG- ***
							case(UBX_CFG_PM2):
								break;
								
							// *** CFG-PRT ***
							case(UBX_CFG_PRT):
								break;
								
							// *** CFG-RATE ***
							case(UBX_CFG_RATE):
								break;
								
							// *** CFG-RINV ***
							case(UBX_CFG_RINV):
								break;
								
							// *** CFG-RST ***
							case(UBX_CFG_RST):
								break;
								
							// *** CFG-RXM ***
							case(UBX_CFG_RXM):
								break;
								
							// *** CFG-SBAS ***
							case(UBX_CFG_SBAS):
								break;
								
							// *** CFG-TP5 ***
							case(UBX_CFG_TP5):
								break;
								
							// *** Unknown Message ***
							default:
								incrementErrorCount(gpsInfo.error.unrecognizedMsgs);
								break;
						}
						break;									
							
					/////////////////////////////
					// UBX Message Class : MON //
					/////////////////////////////
					case(UBX_CLASS_MON):
						switch(gpsMessage.id){
							
							// *** MON-VER ***
							case(UBX_MON_VER):
								gpsInfo.status = GPS_STATUS_STARTED;
								xTimerStop(xReceiverDeadTimer, pdFALSE);
								debug_log(DEBUG_PRIORITY_INFO, DEBUG_SENDER_GPS, "GPS Receiver Started Successfully");
								
								gpsInfo.sw_version_valid = TRUE;
								memcpy(&gpsInfo.sw_version, &gpsMessage.messages.MON_VER.swVersion, sizeof(gpsInfo.sw_version));
								
								gpsInfo.part_number_valid = TRUE;
								memcpy(&gpsInfo.part_number, &gpsMessage.messages.MON_VER.hwVersion, sizeof(gpsInfo.part_number));
											
								break;
								
							// *** MON-HW2 ***
							case(UBX_MON_HW2):
								debug_log(DEBUG_PRIORITY_INFO, DEBUG_SENDER_GPS, "Received Debug Info");
								gpsInfo.debug.ofsI = gpsMessage.messages.MON_HW2.ofsI;
								gpsInfo.debug.magI = gpsMessage.messages.MON_HW2.magI;
								gpsInfo.debug.ofsQ = gpsMessage.messages.MON_HW2.ofsQ;
								gpsInfo.debug.magQ = gpsMessage.messages.MON_HW2.magQ;
								break;
								
							// *** MON-HW ***
							case(UBX_MON_HW):
								break;
								
							// *** MON-IO ***
							case(UBX_MON_IO):
								break;
								
							// *** MON-MSGPP ***
							case(UBX_MON_MSGPP):
								break;
								
							// *** MON-RXBUFF ***
							case(UBX_MON_RXBUF):
								break;
								
							// *** MON-RXR ***
							case(UBX_MON_RXR):
								break;
								
							// *** MON-TXBUF ***
							case(UBX_MON_TXBUF):
								break;
								
							// *** Unknown ***
							default:
								incrementErrorCount(gpsInfo.error.unrecognizedMsgs);
								break;
						
						}
						break;
						
					/////////////////////////////
					// UBX Message Class : AID //
					/////////////////////////////
					case(UBX_CLASS_AID):
						incrementErrorCount(gpsInfo.error.unrecognizedMsgs);
						break;
					
					/////////////////////////////
					// UBX Message Class : TIM //
					/////////////////////////////
					case(UBX_CLASS_TIM):
						incrementErrorCount(gpsInfo.error.unrecognizedMsgs);
						break;
					
					/////////////////////////////////
					// UBX Message Class : Unknown //
					/////////////////////////////////
					default:
						incrementErrorCount(gpsInfo.error.unrecognizedMsgs);
						break;
						
				}						
			}
			
			// Get ready for another message
			parserState = GPS_STATE_UNKNOWN;
			
		}
		
		if(gpsRxdMessages.NAV_POSLLH && gpsRxdMessages.NAV_SOL && gpsRxdMessages.NAV_VELNED){
			gpsRxdMessages.NAV_POSLLH = FALSE;
			gpsRxdMessages.NAV_SOL = FALSE;
			gpsRxdMessages.NAV_VELNED = FALSE;
			
			if(gpsInfo.record_flag){
				recordIndex++;
				if(recordIndex == RECORD_DATA_PER_PAGE){
					debug_tgl_pin1();
					flash_send_request(FLASH_MGR_ADD_RECORD_DATA, &gpsData, sizeof(gpsData), NULL, TRUE, 20);
					recordIndex = 0;
				}
			}

		}
		
		
	}		
}


void gps_reset( void ){
	gpio_clr_gpio_pin(GPS_RESET);
	vTaskDelay( (portTickType)TASK_DELAY_MS(GPS_RESET_TIME) );
	gpio_set_gpio_pin(GPS_RESET);
	vTaskDelay( (portTickType)TASK_DELAY_MS(GPS_RESET_TIME) );
}

unsigned char gps_intersection(signed int x1, signed int y1, signed int x2, signed int y2, signed int x3, signed int y3, signed int x4, signed int y4, unsigned short travelHeading, unsigned short finishHeading){
	// (x1, y1) and (x2, y2) are points for line along traveled path
    // (x3, y3) and (y4, x3) are points for the threshold line
	
	// Temporary storage
	float ua, ub, denominator;
	signed short angleDiff;
	
	// Calculate the denominator
    denominator = (x2 - x1)*(y4 - y3) - (y2 - y1)*(x4 - x3);
    
    ua = ((float)((x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3))) / denominator;
    ub = ((float)((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3))) / denominator;

    // Calulate the point of intersection
    //long = x1+ua*(x2 - x1);
    //lat = y1+ua*(y2 - y1);
	
	if( (ua >= 0) && (ua <= 1) && (ub >= 0) && (ub <= 1) ){
		// Paths intersected!
		
		// Check course
		// GOTCHA: Angles are with two assumed decimal points!
		angleDiff = travelHeading - finishHeading;
		
		if(angleDiff > 18000){
			angleDiff -= 36000;
		}else if(angleDiff < -18000){
			angleDiff += 36000;
		}
		
		if( abs(angleDiff) <= THRESHOLD_ANGLE ){
			return TRUE;
		}else{
			return FALSE;
		}
	}
	
	return FALSE;
}

struct tGPSLine gps_find_finish_line(signed int latitude, signed int longitude, unsigned short heading){
	float angle;
	int	vect;
	
	struct tGPSLine finish;
	
	// Add 90 degrees to heading, make sure it is between 0 and 360,
	// and finally shift the decimal back in
	angle = deg2rad((float)((heading + 9000) % 36000) / (float)100);
	
	// Copy the heading on the point to the heading for the line
	finish.heading = heading;
	
	// Project the THRESHOLD_DISTANCE along the perpendicular angle
	vect = (int)(sin(angle) * THRESHOLD_DISTANCE * 60.0 * 1000000.0);
	finish.startLongitude	= longitude + vect;
	finish.endLongitude		= longitude - vect;
	
	vect = (int)(cos(angle) * THRESHOLD_DISTANCE * 60.0 * 1000000.0);
	finish.startLatitude	= latitude + vect;
	finish.endLatitude		= latitude - vect;
	
	return finish;
}

void gps_set_messaging_rate(unsigned char rate){
	
}

void gps_set_messages( void ){
	
}

void gps_cold_start( void ){
	
}

void gps_warm_start( void ){
	
}

void gps_send_request(enum tGpsCommand command, unsigned int *pointer, unsigned char data, unsigned char delay, unsigned char resume){
	struct tGPSRequest request;
	request.command = command;
	request.pointer = pointer;
	request.data = data;
	request.resume = resume;
	
	if(resume == TRUE){
		request.handle = xTaskGetCurrentTaskHandle();
	}
	
	xQueueSend(gpsManagerQueue, &request, delay);
	
	if(resume == TRUE){
		vTaskSuspend(NULL);
	}
}

void gps_messageTimeout( xTimerHandle xTimer ){
	
}

void gps_getReceiverInfo( xTimerHandle xTimer ){
	
}

void gps_dead( xTimerHandle xTimer ){
	int rxdChar;
	
	if( gpsInfo.error.resetCount < GPS_RESET_MAX_TRIES  ){
		
		// Lets kick the receiver and see if it comes back properly
		gpsInfo.status = GPS_STATUS_SICK;
		incrementErrorCount(gpsInfo.error.resetCount);
		debug_log(DEBUG_PRIORITY_CRITICAL, DEBUG_SENDER_GPS, "Receiver is not talking - kicking receiver");
		gps_reset();
		
		// Reset the timer and lets try again
		xTimerReset(xReceiverDeadTimer, pdFALSE);
		
	}else{
		// Something is up. The only thing possible is incorrect baud rate.  Lets try fixing it
		gpsInfo.status = GPS_STATUS_DEAD;
		debug_log(DEBUG_PRIORITY_CRITICAL, DEBUG_SENDER_GPS, "Resetting didn't help - Drastic measures!");
	}
}


void gps_configure( xTimerHandle xTimer ){
	unsigned char i;
	static unsigned char cfgStep = 0;
	
	struct tUbxCfgMsg cfgMsg;
	struct tUbxCfgRate cfgRate;
	
	switch(cfgStep){
		case(0):
			debug_log(DEBUG_PRIORITY_INFO, DEBUG_SENDER_GPS, "Starting to configure receiver");
			
			// By default, turn off all on all ports
			for(i = 0; i < UBX_NEO_6_PORTS; i++) {
				cfgMsg.rate[i] = 0;
			}
			
			// Turn on the UBX_NAV_POSLLH message
			cfgMsg.msgClass	= UBX_CLASS_NAV;
			cfgMsg.msgID	= UBX_NAV_POSLLH;
			cfgMsg.rate[USART_1_PORT] = TRUE;
			gps_sendPacket(UBX_CLASS_CFG, UBX_CFG_MSG, &cfgMsg, sizeof(cfgMsg));
			
			break;
			
		case(1):
			// Turn on the UBX_NAV_VELNED message
			cfgMsg.msgClass	= UBX_CLASS_NAV;
			cfgMsg.msgID	= UBX_NAV_VELNED;
			cfgMsg.rate[USART_1_PORT] = TRUE;
			gps_sendPacket(UBX_CLASS_CFG, UBX_CFG_MSG, &cfgMsg, sizeof(cfgMsg));
			break;
			
		case(2):
			// Turn on the UBX_NAV_SOL message
			cfgMsg.msgClass	= UBX_CLASS_NAV;
			cfgMsg.msgID	= UBX_NAV_SOL;
			cfgMsg.rate[USART_1_PORT] = TRUE;
			gps_sendPacket(UBX_CLASS_CFG, UBX_CFG_MSG, &cfgMsg, sizeof(cfgMsg));
			break;
			
		case(3):
			// Turn off the NMEA GGA message
			cfgMsg.msgClass	= NMEA;
			cfgMsg.msgID	= NMEA_GGA;
			cfgMsg.rate[USART_1_PORT] = FALSE;
			gps_sendPacket(UBX_CLASS_CFG, UBX_CFG_MSG, &cfgMsg, sizeof(cfgMsg));
			break;
			
		case(4):
			// Turn off the NMEA GLL message
			cfgMsg.msgClass	= NMEA;
			cfgMsg.msgID	= NMEA_GLL;
			cfgMsg.rate[USART_1_PORT] = FALSE;
			gps_sendPacket(UBX_CLASS_CFG, UBX_CFG_MSG, &cfgMsg, sizeof(cfgMsg));
			break;
			
		case(5):
			// Turn off the NMEA GSA message
			cfgMsg.msgClass	= NMEA;
			cfgMsg.msgID	= NMEA_GSA;
			cfgMsg.rate[USART_1_PORT] = FALSE;
			gps_sendPacket(UBX_CLASS_CFG, UBX_CFG_MSG, &cfgMsg, sizeof(cfgMsg));
			
		case(6):
			// Turn off the NMEA GSV message
			cfgMsg.msgClass	= NMEA;
			cfgMsg.msgID	= NMEA_GSV;
			cfgMsg.rate[USART_1_PORT] = FALSE;
			gps_sendPacket(UBX_CLASS_CFG, UBX_CFG_MSG, &cfgMsg, sizeof(cfgMsg));
			break;
			
		case(7):
			// Turn off the NMEA RMC message
			cfgMsg.msgClass	= NMEA;
			cfgMsg.msgID	= NMEA_RMC;
			cfgMsg.rate[USART_1_PORT] = FALSE;
			gps_sendPacket(UBX_CLASS_CFG, UBX_CFG_MSG, &cfgMsg, sizeof(cfgMsg));
			break;
			
		case(8):
			// Turn off the NMEA VTG message
			cfgMsg.msgClass	= NMEA;
			cfgMsg.msgID	= NMEA_VTG;
			cfgMsg.rate[USART_1_PORT] = FALSE;
			gps_sendPacket(UBX_CLASS_CFG, UBX_CFG_MSG, &cfgMsg, sizeof(cfgMsg));
			break;
			
		case(9):
			// Set messaging rate
			cfgRate.measRate = gps_flip_endian2(GPS_UBX_MSG_RATE);
			cfgRate.navRate = gps_flip_endian2(GPS_UBX_NAV_RATE);
			cfgRate.timeRef = gps_flip_endian2(GPS_UBX_TIME_REF);
			gps_sendPacket(UBX_CLASS_CFG, UBX_CFG_RATE, &cfgRate, sizeof(cfgRate));
			break;
		
		// NO MORE CASES AFTER THIS
		case(10):
			// Get the HW and SW versions
			gps_sendPacket(UBX_CLASS_MON, UBX_MON_VER, NULL, NULL);
			break;
			
		default:
			debug_log(DEBUG_PRIORITY_INFO, DEBUG_SENDER_GPS, "Finished Configuring Receiver");
			gpsInfo.status = GPS_STATUS_CONFIGURED;
			break;
	}
	
	cfgStep += 1;
}


void gps_setSbasMode(unsigned char enableSBAS){
	
}

void gps_sendPacket(unsigned char msgClass, unsigned char msgID, unsigned char *data, unsigned char length) {
	unsigned char i;
	unsigned char xsumA = 0, xsumB = 0;
	
	// Send sync characters
	usart_putchar(GPS_USART, GPS_CHAR_SYNC1);
	usart_putchar(GPS_USART, GPS_CHAR_SYNC2);
	
	// Send msg class and id fields
	usart_putchar(GPS_USART, msgClass);
	xsumA += msgClass;
	xsumB += xsumA;
		
	usart_putchar(GPS_USART, msgID);
	xsumA += msgID;
	xsumB += xsumA;
	
	// Send message length, little endian!
	usart_putchar(GPS_USART, (length & 0xFF));
	xsumA += (length & 0xFF);
	xsumB += xsumA;
	
	usart_putchar(GPS_USART, (length >> 8) & 0xFF);
	xsumA += (length >> 8) & 0xFF;
	xsumB += xsumA;
	
	// Send out the message payload
	for (i = 0; i < length; i++) {
		xsumA += *data;
		xsumB += xsumA;
		
		usart_putchar(GPS_USART, *data);
	
		data++;
	}
	
	// Finally send out the checksums
	usart_putchar(GPS_USART, xsumA);
	usart_putchar(GPS_USART, xsumB);
}

struct tGPSChecksum gps_calculateChecksum(unsigned char *data, unsigned char length){
	unsigned char i;
	
	struct tGPSChecksum checksum;
	
	checksum.xsumA = 0;
	checksum.xsumB = 0;
	
	for (i = 0; i < length; i++) {
		checksum.xsumA += *data;
		checksum.xsumB += checksum.xsumA;
		
		data++;
	}
	
	return checksum;
}