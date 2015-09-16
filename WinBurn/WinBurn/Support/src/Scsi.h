/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2009 Christian Kindahl
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

// SCSI statuses.
#define SCSISTAT_GOOD							0x00
#define SCSISTAT_CHECK_CONDITION				0x02
#define SCSISTAT_CONDITION_MET					0x04
#define SCSISTAT_BUSY							0x08
#define SCSISTAT_INTERMEDIATE					0x10
#define SCSISTAT_INTERMEDIATE_COND_MET			0x14
#define SCSISTAT_RESERVATION_CONFLICT			0x18
#define SCSISTAT_COMMAND_TERMINATED				0x22
#define SCSISTAT_QUEUE_FULL						0x28

// SCSI commands.
#define SCSI_INQUIRY							0x12
#define SCSI_READ_CAPACITY						0x25
#define SCSI_READ_TOC_PMA_ATIP					0x43
#define SCSI_GET_CONFIGURATION					0x46
#define SCSI_READ_DISC_INFORMATION				0x51
#define SCSI_READ_DISC_STRUCTURE				0xAD
#define SCSI_READ_FORMAT_CAPACITIES				0x23
#define SCSI_FORMAT_UNIT						0x04
#define SCSI_GET_EVENT_STATUS_NOTIFICATION		0x4A
#define SCSI_TEST_UNIT_READY					0x00
#define SCSI_START_STOP_UNIT					0x1B
#define SCSI_CLOSE_TRACK_SESSION				0x5B
#define SCSI_PREVENTALLOW_MEDIUM_REMOVAL		0x1E
#define SCSI_GET_PERFORMANCE					0xAC
#define SCSI_SET_CD_SPEED						0xBB
#define SCSI_BLANK								0xA1
#define SCSI_MODE_SENSE10						0x5A
#define SCSI_MODE_SELECT10						0x55
#define SCSI_REQUEST_SENSE						0x03
#define SCSI_READ_CD							0xBE
#define SCSI_READ_TRACK_INFORMATION				0x52

// Profiles.
#define PROFILE_NONE							0x0000
#define PROFILE_NONREMOVABLE					0x0001
#define PROFILE_REMOVABLE						0x0002
#define PROFILE_MOPTIC_E						0x0003
#define PROFILE_OPTIC_WO						0x0004
#define PROFILE_AS_MO							0x0005
#define PROFILE_CDROM							0x0008
#define PROFILE_CDR								0x0009
#define PROFILE_CDRW							0x000A
#define PROFILE_DVDROM							0x0010
#define PROFILE_DVDMINUSR_SEQ					0x0011
#define PROFILE_DVDRAM							0x0012
#define PROFILE_DVDMINUSRW_RESTOV				0x0013
#define PROFILE_DVDMINUSRW_SEQ					0x0014
#define PROFILE_DVDMINUSR_DL_SEQ				0x0015
#define PROFILE_DVDMINUSR_DL_JUMP				0x0016
#define PROFILE_DVDPLUSRW						0x001A
#define PROFILE_DVDPLUSR						0x001B
//#define PROFILE_DDCDROM						0x0020
//#define PROFILE_DDCDR							0x0021
//#define PROFILE_DDCDRW						0x0022
#define PROFILE_DVDPLUSRW_DL					0x002A
#define PROFILE_DVDPLUSR_DL						0x002B
#define PROFILE_BDROM							0x0040
#define PROFILE_BDR_SRM							0x0041
#define PROFILE_BDR_RRM							0x0042
#define PROFILE_BDRE							0x0043
#define PROFILE_HDDVDROM						0x0050
#define PROFILE_HDDVDR							0x0051
#define PROFILE_HDDVDRAM						0x0052
#define PROFILE_NONSTANDARD						0xFFFF

// Sense keys.
#define	SENSEKEY_NO_SENSE						0x00
#define SENSEKEY_RECOVERED_ERROR				0x01
#define SENSEKEY_NOT_READY						0x02
#define SENSEKEY_MEDIUM_ERROR					0x03		// Non recoverable error.
#define SENSEKEY_HARDWARE_ERROR					0x04		// Non recoverable error.
#define SENSEKEY_ILLEGAL_REQUEST				0x05
#define SENSEKEY_UNIT_ATTENTION					0x06
#define SENSEKEY_DATA_PROTECT					0x07
#define SENSEKEY_BLANK_CHECK					0x08
#define SENSEKEY_VENDOR_SPECIFIC				0x09
#define SENSEKEY_COPY_ABORTED					0x0A
#define SENSEKEY_ABORTED_COMMAND				0x0B
#define SENSEKEY_EQUAL							0x0C		// Obsolete.
#define SENSEKEY_VOLUME_OVERFLOW				0x0D
#define SENSEKEY_MISCOMPARE						0x0E
#define SENSEKEY_RESERVED						0x0F

// Data block types (mode page 5).
#define DATABLOCK_RAW							0x00	// 2352 bytes raw data.
#define DATABLOCK_RAW_PQ						0x01	// Raw data with P and Q Sub-channel.
#define DATABLOCK_RAW_PW						0x02	// Raw data with P-W Sub-channel.
#define DATABLOCK_RAW_PW_R						0x03	// Raw data with P-W Sub-channel.
#define DATABLOCK_MODE1							0x08	// Mode 1 (ISO/IEC 10149): 2048 bytes of user data.
#define DATABLOCK_MODE2							0x09	// Mode 2 (ISO/IEC 10149): 2336 bytes of user data.
#define DATABLOCK_MODE2_XA						0x0A	// Mode 2 (CD-ROM XA, form 1): 2048 bytes of user data, sub-header from write parameters.
#define DATABLOCK_MODE2_XA_F1					0x0B	// Mode 2 (CD-ROM XA, form 1): 8 bytes of sub-header, 2 048 bytes of user data.
#define DATABLOCK_MODE2_XA_F2					0x0C	// Mode 2 (CD-ROM XA, form 2): 2324 bytes of user data, sub-header is taken from write parameters.
#define DATABLOCK_MODE2_XA_MIX					0x0D	// Mode 2 (CD-ROM XA, form 1, form 2, or mixed form): 8 bytes of sub-header, 2324 bytes of user data.
