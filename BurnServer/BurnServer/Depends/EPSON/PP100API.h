/** @file PP100API.h
 * @brief PP100API.dll Header File
 * @author SEIKO EPSON CORPORATION
 */

#ifndef _PP100API_H_
#define _PP100API_H_

#ifdef	__cplusplus
extern "C" {
#endif	/* def __cplusplus	*/

#ifdef PP100API_EXPORTS
   #define PP100_API __declspec(dllexport)
#else
   #define PP100_API __declspec(dllimport)
#endif

/** @defgroup define define
 *  @{
 */
/** @defgroup ReturnCode Return Code
 *  @ingroup define
 *  @{
 */
/** @def PP100API_BASE
 * base value in PP-100 API
 */
#define PP100API_BASE 0
/** @def PP100API_JOB_SETTING_NOT_SET
 * if the key value is not set
 */
#define PP100API_JOB_SETTING_NOT_SET PP100API_BASE + 1
/** @def PP100API_SUCCESS
 * if successful
 */
#define PP100API_SUCCESS PP100API_BASE - 0
/** @def PP100API_FAILURE
 * if failure
 */
#define PP100API_FAILURE PP100API_BASE - 1
/** @def PP100API_NOT_INITIALIZED
 * if the function is called before calling PP100_Initialize()
 */
#define PP100API_NOT_INITIALIZED PP100API_BASE - 2
/** @def PP100API_INVALID_PARAMETER
 * if the parameter is invalid
 */
#define PP100API_INVALID_PARAMETER PP100API_BASE - 3
/** @def PP100API_HOST_NOT_FOUND
 * if the host is not found
 */
#define PP100API_HOST_NOT_FOUND PP100API_BASE - 4
/** @def PP100API_ORDER_FOLDER_NOT_FOUND
 * if the folder that TDBridge is monitoring is not found
 */
#define PP100API_ORDER_FOLDER_NOT_FOUND PP100API_BASE - 5
/** @def PP100API_ACCESS_DENIED
 * if access to the folder is denied
 */
#define PP100API_ACCESS_DENIED PP100API_BASE - 6
/** @def PP100API_MORE_ITEMS
 * if area to receive the result of API funtion is insufficient.
 */
#define PP100API_MORE_ITEMS PP100API_BASE - 7
/** @def PP100API_PUBLISHER_NOT_FOUND
 * if PP-100 series is not found
 */
#define PP100API_PUBLISHER_NOT_FOUND PP100API_BASE - 8
/** @def PP100API_JOB_NOT_CREATED
 * if the job is not created
 */
#define PP100API_JOB_NOT_CREATED PP100API_BASE - 10
/** @def PP100API_NOT_SUPPORTED
 * if the environment is not supported
 */
#define PP100API_NOT_SUPPORTED PP100API_BASE - 11
/** @def PP100API_TDBRIDGE_LESS_VERSION
 * if TDBridge version is less than 5.00
 */
#define PP100API_TDBRIDGE_LESS_VERSION PP100API_BASE - 12
/** @def PP100API_JOB_SUBMITTED
 * if the job is published to the server
 */
#define PP100API_JOB_SUBMITTED PP100API_BASE - 13
/** @def PP100API_JOB_IS_SUBMITTED_BY_TOTAL_DISC_MAKER
 * if the job is submitted by Total Disc Maker directly
 */
#define PP100API_JOB_IS_SUBMITTED_BY_TOTAL_DISC_MAKER PP100API_BASE - 14
/** @def PP100API_JOB_IS_ALREADY_FINISHED
 * if the job finished
 */
#define PP100API_JOB_IS_ALREADY_FINISHED PP100API_BASE - 15
/** @def PP100API_JOB_NOT_RECOGNIZED
 * if the job is not recognized by the server
 */
#define PP100API_JOB_NOT_RECOGNIZED PP100API_BASE - 16
/** @def PP100API_ERROR_CODE_NOT_DEFINED
 * if the error code is not defined
 */
#define PP100API_ERROR_CODE_NOT_DEFINED PP100API_BASE - 17
/** @def PP100API_INFORMATION_CODE_NOT_DEFINED
 * if the information code is not defined
 */
#define PP100API_INFORMATION_CODE_NOT_DEFINED PP100API_BASE - 18
/** @} */ // end of Return Code

/** @defgroup ERRORCodeRefertotheTDBridgeguidefordetails ERROR Code(Refer to the TDBridge guide for details)
 *  @ingroup define
 *  @{
 */
/** @def PP100API_SYS001
 * Accepting the job is failed.
 */
#define PP100API_SYS001 1
/** @def PP100API_SYS002
 * Executing the job failed due to the communication error with the publishing machine.
 */
#define PP100API_SYS002 2
/** @def PP100API_SYS003
 * Executing the job failed because the HDD of the publishing machine does not have enough space.
 */
#define PP100API_SYS003 3
/** @def PP100API_JDF0100
 * The specified job ID already exists.
 */
#define PP100API_JDF0100 100
/** @def PP100API_JDF0101
 * The specified job ID contains more than 40 characters.
 */
#define PP100API_JDF0101 101
/** @def PP100API_JDF0102
 * The specified job ID contains characters other than English one byte characters/numbers, "-", and "_".
 */
#define PP100API_JDF0102 102
/** @def PP100API_JDF0200
 * The specified publishing machine name has not been registered.
 */
#define PP100API_JDF0200 200
/** @def PP100API_JDF0201
 * Multiple publishing machines are detected, but none of them has not been specified using the PUBLISHER key. Or no publishing machine has not been specified in the Environment Setting.
 */
#define PP100API_JDF0201 201
/** @def PP100API_JDF0202
 * The publishing machine specified by the PUBLISHER key is set to an unsupported mode, or a mode cannot be acquired.
 */
#define PP100API_JDF0202 202
/** @def PP100API_JDF0203
 * There is no publishing machine that has been registered.
 */
#define PP100API_JDF0203 203
/** @def PP100API_JDF0300
 * The specified number of discs to be published is exceeding the upper limit of the publisher (the upper limit changes depending on the selected mode and stacker setting).
 */
#define PP100API_JDF0300 300
/** @def PP100API_JDF0400
 * [PP-100 / PP-100N / PP-100AP / PP-100II]
A value other than "2" or "3" or "4" is specified.
[PP-50 / PP-50BD / PP-7050BD]
A value other than "2" or "3" is specified.
 */
#define PP100API_JDF0400 400
/** @def PP100API_JDF0500
 * The specified disc type is not the same with that has been specified for the source stacker.
 */
#define PP100API_JDF0500 500
/** @def PP100API_JDF0501
 * DISC_TYPE key has not been specified.
 */
#define PP100API_JDF0501 501
/** @def PP100API_JDF0502
 * [PP-100 / PP-100N / PP-100AP / PP-100II]
A value other than "CD", "DVD" or "DVD-DL" is specified.
[PP-50 / PP-50BD / PP-7050BD / PP-100II (BD)]
A value other than "CD", "DVD", "DVD-DL", "BD" or "BD-DL" is specified.
 */
#define PP100API_JDF0502 502
/** @def PP100API_JDF0600
 * A value other than number is specified.
 */
#define PP100API_JDF0600 600
/** @def PP100API_JDF0700
 * A value other than "YES" or "NO" is specified.
 */
#define PP100API_JDF0700 700
/** @def PP100API_JDF0800
 * A value other than "YES" or "NO" is specified.
 */
#define PP100API_JDF0800 800
/** @def PP100API_JDF0900
 * When any of the followings is detected
(1) The directory path for both the source and destination has not been specified using the DATA key.
(2) The directory path for both the source and destination has not been specified using the DATA key in the file specified by the DATA_LIST key.
 */
#define PP100API_JDF0900 900
/** @def PP100API_JDF0901
 * A file with the same name can not be specified in the same folder.
 */
#define PP100API_JDF0901 901
/** @def PP100API_JDF0902
 * The specified directory path (including the file name) does not conform to the specified disc format.
 */
#define PP100API_JDF0902 902
/** @def PP100API_JDF0903
 * The source file size is too large for the specified disc type.
 */
#define PP100API_JDF0903 903
/** @def PP100API_JDF0904
 * The destination folder hierarchy is exceeding 128 levels.
 */
#define PP100API_JDF0904 904
/** @def PP100API_JDF0905
 * The drive that stores the specified source file is not ready.
 */
#define PP100API_JDF0905 905
/** @def PP100API_JDF0907
 * The specified source file contains a non-existing content.
 */
#define PP100API_JDF0907 907
/** @def PP100API_JDF0908
 * The specified source file contains a file to which the current user does not have read/write access.
 */
#define PP100API_JDF0908 908
/** @def PP100API_JDF0909
 * Some items of the specified source file are being used.
 */
#define PP100API_JDF0909 909
/** @def PP100API_JDF0910
 * The file specified by the DATA_LIST key does not exist. (including when a folder has been specified)
 */
#define PP100API_JDF0910 910
/** @def PP100API_JDF0911
 * The current user does not have read/write access to the file specified by the DATA_LIST key.
 */
#define PP100API_JDF0911 911
/** @def PP100API_JDF0912
 * The file specified by the DATA_LIST key is being used.
 */
#define PP100API_JDF0912 912
/** @def PP100API_JDF0913
 * The drive that stores the file specified by the DATA_LIST key is not ready.
 */
#define PP100API_JDF0913 913
/** @def PP100API_JDF0914
 * The file specified is over 4095MB and cannot be written with the specified record format.
 */
#define PP100API_JDF0914 914
/** @def PP100API_JDF1000
 * The specified Volume Label does not conform to the specified disc format.
 */
#define PP100API_JDF1000 1000
/** @def PP100API_JDF1100
 * The publisher does not support the specified video file format.
 */
#define PP100API_JDF1100 1100
/** @def PP100API_JDF1101
 * The video signals of the specified video file do not match with the all tracks.
 */
#define PP100API_JDF1101 1101
/** @def PP100API_JDF1103
 * The source video file size is too large for the specified disc type.
 */
#define PP100API_JDF1103 1103
/** @def PP100API_JDF1104
 * The specified video file does not exist.
 */
#define PP100API_JDF1104 1104
/** @def PP100API_JDF1105
 * The current user does not have read/write access to the specified video file.
 */
#define PP100API_JDF1105 1105
/** @def PP100API_JDF1106
 * The specified video file is being used.
 */
#define PP100API_JDF1106 1106
/** @def PP100API_JDF1107
 * The drive that stores the specified video file is not ready.
 */
#define PP100API_JDF1107 1107
/** @def PP100API_JDF1108
 * The duration of playing time of the specified video file is less than one second.
 */
#define PP100API_JDF1108 1108
/** @def PP100API_JDF1150
 * The specified VIDEO_TS folder does not exist.
 */
#define PP100API_JDF1150 1150
/** @def PP100API_JDF1151
 * The specified file path is not for the folder named "VIDEO_TS."
 */
#define PP100API_JDF1151 1151
/** @def PP100API_JDF1152
 * The current user does not have read/write access to the specified VIDEO_TS folder.
 */
#define PP100API_JDF1152 1152
/** @def PP100API_JDF1153
 * The drive for the specified VIDO_TS folder is not ready.
 */
#define PP100API_JDF1153 1153
/** @def PP100API_JDF1154
 * The specified VIDEO_TS folder contains a file to which the current user does not have read/write access.
 */
#define PP100API_JDF1154 1154
/** @def PP100API_JDF1155
 * Some files in the specified VIDEO_TS folder are being used.
 */
#define PP100API_JDF1155 1155
/** @def PP100API_JDF1156
 * The file composition of the specified VIDEO_TS folder is incorrect.
 */
#define PP100API_JDF1156 1156
/** @def PP100API_JDF1157
 * The source DVD-Video data size is too large for the specified disc type.
 */
#define PP100API_JDF1157 1157
/** @def PP100API_JDF1200
 * The specified video title is exceeding 32 characters.
 */
#define PP100API_JDF1200 1200
/** @def PP100API_JDF1201
 * The specified video title contains invalid characters.
 */
#define PP100API_JDF1201 1201
/** @def PP100API_JDF1300
 * The publisher does not support the specified image file format.
 */
#define PP100API_JDF1300 1300
/** @def PP100API_JDF1301
 * The source image file size is too large for the specified disc type.
 */
#define PP100API_JDF1301 1301
/** @def PP100API_JDF1302
 * The specified image file does not exist.
 */
#define PP100API_JDF1302 1302
/** @def PP100API_JDF1303
 * The current user does not have read/write access to the specified image file.
 */
#define PP100API_JDF1303 1303
/** @def PP100API_JDF1304
 * The specified image file is being used.
 */
#define PP100API_JDF1304 1304
/** @def PP100API_JDF1305
 * The drive that stores the specified image file is not ready.
 */
#define PP100API_JDF1305 1305
/** @def PP100API_JDF1306
 * The specified disc type for the image file is not the same with that has been specified for the source stacker.
 */
#define PP100API_JDF1306 1306
/** @def PP100API_JDF1400
 * Disc type CD and data file have been specified, however, a setting other than "ISO9660L2", "JOLIET", or "UDF102" has been specified in the FORMAT key.
 */
#define PP100API_JDF1400 1400
/** @def PP100API_JDF1401
 * Disc type DVD or DVD-DL and data file have been specified, however setting other than "UDF102", "UDF102_BRIDGE", or "UDF150" has been specified using the FORMAT key.
 */
#define PP100API_JDF1401 1401
/** @def PP100API_JDF1402
 * Disc type BD or BD-DL and data file have been specified, however setting other than "UDF102" or "UDF260" has been specified using the FORMAT key.
 */
#define PP100API_JDF1402 1402
/** @def PP100API_JDF1500
 * The publisher does not support the specified label file format.
 */
#define PP100API_JDF1500 1500
/** @def PP100API_JDF1501
 * The specified label file does not exist.
 */
#define PP100API_JDF1501 1501
/** @def PP100API_JDF1502
 * The current user does not have read/write access to the label file specified by the LABEL key.
 */
#define PP100API_JDF1502 1502
/** @def PP100API_JDF1503
 * The specified label file is being used.
 */
#define PP100API_JDF1503 1503
/** @def PP100API_JDF1504
 * The drive that stores the specified label file is not ready.
 */
#define PP100API_JDF1504 1504
/** @def PP100API_JDF1505
 * The path referred from the tdd file specified by the LABEL key does not exist.
 */
#define PP100API_JDF1505 1505
/** @def PP100API_JDF1506
 * The current user does not have read/write access to the path referred from the tdd file specified by the LABEL key.
 */
#define PP100API_JDF1506 1506
/** @def PP100API_JDF1507
 * The drive of the path referred from the tdd file specified by the LABEL key is not ready.
 */
#define PP100API_JDF1507 1507
/** @def PP100API_JDF1600
 * The text data file specified using the REPLACE_FIELD key does not exist.
 */
#define PP100API_JDF1600 1600
/** @def PP100API_JDF1601
 * The current user does not have read/write access to the data file specified by the REPLACE_FIELD key.
 */
#define PP100API_JDF1601 1601
/** @def PP100API_JDF1602
 * The text data file specified using the REPLACE_FIELD key is being used.
 */
#define PP100API_JDF1602 1602
/** @def PP100API_JDF1603
 * The drive that stores the file specified by the REPLACE_FIELD key is not ready.
 */
#define PP100API_JDF1603 1603
/** @def PP100API_JDF1604
 * When any of the followings is detected
(1) The file extension or setting itself contains invalid characters.
(2) The number of characters of the specified character string to be merged is exceeding 1024 (upper limit for the text item).
(3) More than 255 text items have been specified.
 */
#define PP100API_JDF1604 1604
/** @def PP100API_JDF1610
 * The specified barcode value does not conform to the barcode standard for the barcode to be replaced.
 */
#define PP100API_JDF1610 1610
/** @def PP100API_JDF1611
 * No barcode file is specified, or the specified barcode file does not exist.
 */
#define PP100API_JDF1611 1611
/** @def PP100API_JDF1612
 * The current user does not have read/write access to the specified barcode file.
 */
#define PP100API_JDF1612 1612
/** @def PP100API_JDF1613
 * The specified barcode file is being used.
 */
#define PP100API_JDF1613 1613
/** @def PP100API_JDF1614
 * The drive for the specified barcode file is not ready.
 */
#define PP100API_JDF1614 1614
/** @def PP100API_JDF1615
 * The specified keyword for the barcode item is incorrect.
 */
#define PP100API_JDF1615 1615
/** @def PP100API_JDF1616
 * The specified barcode item cannot be printed.
 */
#define PP100API_JDF1616 1616
/** @def PP100API_JDF1620
 * An image file that does not exist has been specified.
 */
#define PP100API_JDF1620 1620
/** @def PP100API_JDF1621
 * The specified image file format is not supported.
 */
#define PP100API_JDF1621 1621
/** @def PP100API_JDF1622
 * No access privilege for the specified image file.
 */
#define PP100API_JDF1622 1622
/** @def PP100API_JDF1623
 * The specified image file is in use.
 */
#define PP100API_JDF1623 1623
/** @def PP100API_JDF1624
 * The drive of the specified image file is not ready.
 */
#define PP100API_JDF1624 1624
/** @def PP100API_JDF1625
 * Specification of keyword for replacement image item is invalid.
 */
#define PP100API_JDF1625 1625
/** @def PP100API_JDF1700
 * The value for AUDIO_TITLE key is exceeding 128 half-size characters (64 two-byte characters).
 */
#define PP100API_JDF1700 1700
/** @def PP100API_JDF1701
 * The value for AUDIO_TITLE key contains characters that can not be changed into multi-byte characters.
 */
#define PP100API_JDF1701 1701
/** @def PP100API_JDF1800
 * The value for AUDIO_PERFORMER key is exceeding 128 half-size characters (64 two-byte characters).
 */
#define PP100API_JDF1800 1800
/** @def PP100API_JDF1801
 * The value on AUDIO_PERFORMER key contains characters that can not be changed into multi-byte characters.
 */
#define PP100API_JDF1801 1801
/** @def PP100API_JDF1900
 * The total size of CD-TEXT is exceeding 3024 byte (corresponding to 3000 half-size characters).
 */
#define PP100API_JDF1900 1900
/** @def PP100API_JDF1901
 * The total size of music data to be written and pregap is too large for the specified disc type.
 */
#define PP100API_JDF1901 1901
/** @def PP100API_JDF1910
 * The specified music file format is not supported.
 */
#define PP100API_JDF1910 1910
/** @def PP100API_JDF1911
 * No music file is specified, or the specified music file does not exist.
 */
#define PP100API_JDF1911 1911
/** @def PP100API_JDF1912
 * The current user does not have the access rights to read/write the specified music file.
 */
#define PP100API_JDF1912 1912
/** @def PP100API_JDF1913
 * The specified music file is being used.
 */
#define PP100API_JDF1913 1913
/** @def PP100API_JDF1914
 * The drive that stores the specified music file is not ready.
 */
#define PP100API_JDF1914 1914
/** @def PP100API_JDF1915
 * The specified music file is protected by copyright.
 */
#define PP100API_JDF1915 1915
/** @def PP100API_JDF1916
 * The duration of playing time of the specified music file is less than four seconds.
 */
#define PP100API_JDF1916 1916
/** @def PP100API_JDF1920
 * The specified track title is exceeding 128 half-size characters (64 two-byte characters).
 */
#define PP100API_JDF1920 1920
/** @def PP100API_JDF1921
 * The specified track title contains characters that can not be changed into multi-byte characters.
 */
#define PP100API_JDF1921 1921
/** @def PP100API_JDF1930
 * The specified track artist name is exceeding 128 half-size characters (64 two-byte characters).
 */
#define PP100API_JDF1930 1930
/** @def PP100API_JDF1931
 * The specified track artist name contains characters that can not be changed into multi-byte characters.
 */
#define PP100API_JDF1931 1931
/** @def PP100API_JDF1940
 * The specified pregap is out of the range from 0 to 269999 (00:00:00 frame to 59:59:74 fame).
 */
#define PP100API_JDF1940 1940
/** @def PP100API_JDF1950
 * The value specified in the ISRC keyword contains characters other than one-byte uppercase characters or one-byte numerals. Or the number of characters is not 12.
 */
#define PP100API_JDF1950 1950
/** @def PP100API_JDF2000
 * Other than a one byte character is specified for the DiscDiamOut keyword, or the value is out of the range from 700 to 1194 (70 mm to 119.4 mm).
 */
#define PP100API_JDF2000 2000
/** @def PP100API_JDF2001
 * Other than a one byte character is specified for the DiscDiamin keyword, or the value is out of the range from 180 to 500 (18 mm to 50 mm).
 */
#define PP100API_JDF2001 2001
/** @def PP100API_JDF2100
 * The value specified in the PLUG_IN_ID key exceeds 40 characters.
 */
#define PP100API_JDF2100 2100
/** @def PP100API_JDF2101
 * The value specified in the PLUG_IN_ID key includes an invalid character.
 */
#define PP100API_JDF2101 2101
/** @def PP100API_JDF2102
 * The publisher does not support the specified PLUG_IN_ID key.
 */
#define PP100API_JDF2102 2102
/** @def PP100API_JDF2200
 * The value specified in the PLUG_IN_PARAMETER exceeds 1024 characters.
 */
#define PP100API_JDF2200 2200
/** @def PP100API_JDF2201
 * The value specified in the PLUG_IN_PARAMETER key includes an invalid character.
 */
#define PP100API_JDF2201 2201
/** @def PP100API_JDF2202
 * The publisher does not support the specified PLUG_IN_ID key.
 */
#define PP100API_JDF2202 2202
/** @def PP100API_JDF2300
 * A setting other than "HIGH" is specified in the PRIORITY key.
 */
#define PP100API_JDF2300 2300
/** @def PP100API_JDF2400
 * Characters other than one-byte numerals are included in the AUDIO_CATALOG_CODE key, or the number of characters is not 13.
 */
#define PP100API_JDF2400 2400
/** @def PP100API_JDF2500
 * A value other than "1", "2", or "3" is set in the LABEL_TYPE key.
 */
#define PP100API_JDF2500 2500
/** @def PP100API_JDF2501
 * "3" is specified in the LABEL_TYPE key, but the PRINT_MODE key is not specified and a value other than "1" is selected in print mode of the environment settings.
 */
#define PP100API_JDF2501 2501
/** @def PP100API_JDF2600
 * A value other than "1", "2", or "3" is set in the PRINT_MODE key.
 */
#define PP100API_JDF2600 2600
/** @def PP100API_JDF2601
 * "2" or "3" is specified in the PRINT_MODE key, but the LABEL_TYPE key is not specified and the label type in the environment settings is "EPSON-certified CD/DVD label".
 */
#define PP100API_JDF2601 2601
/** @def PP100API_JDF2602
 * "2" or "3" is specified in the PRINT_MODE key, but "3" is specified in the LABEL_TYPE key.
 */
#define PP100API_JDF2602 2602
/** @def PP100API_JDF2603
 * "3" is specified in the PRINT_MODE key, but the output device type is not PP-100AP.
 */
#define PP100API_JDF2603 2603
/** @def PP100API_JDF2700
 * A value other than "1", "2", or "AUTO" is specified in the IN_STACKER key.
 */
#define PP100API_JDF2700 2700
/** @def PP100API_JDF2800
 * A value other than "0", or "1" is specified in the MEASURE key.
 */
#define PP100API_JDF2800 2800
/** @def PP100API_JDF2801
 * "1" is not specified on the MEASURE key in the Error Rate Measurement mode.
 */
#define PP100API_JDF2801 2801
/** @def PP100API_JDF2900
 * A value other than "YES", or "NO" is specified in the ARCHIVE_DISC_ONLY key.
 */
#define PP100API_JDF2900 2900
/** @def PP100API_JDF0000
 * [PP-100 / PP-50 / PP-50BD / PP-7050BD / PP-100II]
CD is specified for the disc type, but no value is specified for the data file, video file, image file, track information, label file, or Plug-in information.
[ PP-100N]
CD is specified for the disc type, but no value is specified for the data file, video file, image file, track information, or label file.
 */
#define PP100API_JDF0000 5000
/** @def PP100API_JDF0001
 * [PP-100 / PP-50 / PP-50BD / PP-7050BD / PP-100II]
DVD or DVD-DL is specified for the disc type, but no value is specified for the data file, video file, image file, track information, label file, or Plug-in information.
[ PP-100N]
DVD or DVD-DL is specified for the disc type, but no value is specified for the data file, video file, image file, track information, or label file information.
 */
#define PP100API_JDF0001 5001
/** @def PP100API_JDF0002
 * [PP-100AP]
Print data is not specified.
 */
#define PP100API_JDF0002 5002
/** @def PP100API_JDF0003
 * [PP-50BD / PP-7050BD / PP-100II (BD)]
BD or BD-DL is specified as the disc type, but data file, label file, Plug-In information are not specified.
 */
#define PP100API_JDF0003 5003
/** @def PP100API_JDF0004
 * The Job is submitted in read mode.
 */
#define PP100API_JDF0004 5004
/** @def PP100API_CAN000
 * Picking up a disc from the drive tray or the printer tray is failed.
Remedy:
Turn off Disc producer and turn it back on.
 */
#define PP100API_CAN000 10000
/** @def PP100API_CAN001
 * A disc is dropped while being picked up.
Remedy:
(1) Turn Disc producer off and remove the dropped disc.
(2) Turn Disc producer back on.
 */
#define PP100API_CAN001 10001
/** @def PP100API_CAN002
 * Releasing a disc is failed.
Remedy:
Turn off PP-100/PP-100N and turn it back on.
 */
#define PP100API_CAN002 10002
/** @def PP100API_CAN003
 * Setting a disc in the drive tray or the printer tray must have been made, however, no disc is detected in the tray.
Remedy:
(1) Turn Disc producer off and remove any foreign object inside the machine.
(2) Turn Disc producer back on.
 */
#define PP100API_CAN003 10003
/** @def PP100API_CAN004
 * In Batch mode, the stacker 3 was not empty when the job was started. Therefore, the output stacker has become full during processing the job.
Remedy:
(1) Remove all discs from the stacker 3.
(2) Put new blank discs into the stacker 1 and stacker 2, then restart the job.
 */
#define PP100API_CAN004 10004
/** @def PP100API_CAN005
 * An error occurs while the robotic arm is moving.
Remedy:
(1) Turn Disc producer off and remove any foreign object inside the machine.
(2) Turn Disc producer back on.
 */
#define PP100API_CAN005 10005
/** @def PP100API_CAN006
 * An auto loader fatal error has occurred. The robotic arm can not be moved due to overheating of the motor.
Remedy:
(1) Turn Disc producer off and remove any foreign object inside the machine.
(2) Turn Disc producer back on.
 */
#define PP100API_CAN006 10006
/** @def PP100API_CAN007
 * Controlling the drive tray is failed.
Remedy:
(1) Turn Disc producer off and remove any foreign object from the drive tray.
(2) Turn Disc producer back on.
 */
#define PP100API_CAN007 10007
/** @def PP100API_CAN008
 * Processing the job is impossible because of a fatal error of the drive.
Remedy:
Turn off Disc producer and turn it back on.
 */
#define PP100API_CAN008 10008
/** @def PP100API_CAN009
 * Controlling the printer tray is failed.
Remedy:
(1) Turn Disc producer off and remove any foreign object from the printer tray.
(2) Turn Disc producer back on.
 */
#define PP100API_CAN009 10009
/** @def PP100API_CAN010
 * A printer fatal error has occurred.
Remedy:
Turn off Disc producer and turn it back on.
 */
#define PP100API_CAN010 10010
/** @def PP100API_CAN011
 * A communication with the printer is failed.
Remedy:
Check if the port setting on the printer driver is correct.
 */
#define PP100API_CAN011 10011
/** @def PP100API_CAN012
 * A printer name error has occurred. The printer specified by the EPJ file can not be found.
Remedy:
Check the printer name.
 */
#define PP100API_CAN012 10012
/** @def PP100API_CAN013
 * A printer maintenance error has occurred.
Remedy:
Please contact the store where you purchased Disc producer or our customer service.
 */
#define PP100API_CAN013 10013
/** @def PP100API_CAN014
 * The waste ink pads need to be replaced.
Remedy:
Please contact the store where you purchased Disc producer or our customer service.
 */
#define PP100API_CAN014 10014
/** @def PP100API_CAN015
 * The status error has occurred, and any of the following has been detected.
(1) The disc on the drive tray disappeared.
(2) The disc on the printer tray disappeared.
(3) A disc was placed on the drive tray.
(4) A disc was placed on the printer tray.
(5) Robotic arm got a disc.
Remedy:
(1) Check if there occurred an error inside the chassis.
(2) Turn Disc producer back on.
 */
#define PP100API_CAN015 10015
/** @def PP100API_CAN016
 * A JOB file error has occurred, and one of the following has been detected.
(1) The specified disc image file cannot be accessed.
(2) The specified label data file cannot be accessed.
Remedy:
Set an access privilege for "SYSTEM" account to the shared folder in which the disc image files and label data files are saved, or specify the copied image file on the local computer.
 */
#define PP100API_CAN016 10016
/** @def PP100API_CAN017
 * A non-supported version error has occurred, and any of the following has been detected.
(1) Firmware version of the auto loader is not supported.
(2) Firmware version of the drive 1 is different from the supported version.
(3) Firmware version of the drive 2 is different from the supported version.
(4) Firmware version of the printer is not supported.
Remedy:
Upgrade the firmware of the concerning device.
 */
#define PP100API_CAN017 10017
/** @def PP100API_CAN018
 * The publishing mode specified by EPSON Total Disc Setup when issuing the job and the mode selected on the device are different.
Remedy:
Reset the publishing mode using the EPSON Total Disc Setup.
 */
#define PP100API_CAN018 10018
/** @def PP100API_CAN019
 * Creating a disc image is cancelled due to an invalid source directory path.
Remedy:
Check the files and folders in the specified directory.
 */
#define PP100API_CAN019 10019
/** @def PP100API_CAN020
 * A communication error with the publisher occurs.
Remedy:
Check if the power supply cable and USB cable are securely connected.
 */
#define PP100API_CAN020 10020
/** @def PP100API_CAN021
 * An error rate log save error has occurred.
(1) The drive of the error rate log save folder is not ready.
(2) The path of the error rate log save folder is invalid.
(3) There is not enough space in the error rate log save folder.
(4) The current user does not have read/write access to the error rate log save folder or the file(s) in the folder.
(5) The error rate log save folder or the file(s) in the folder is being used.
(6) The error rate log save folder or the file(s) in the folder is read-only.
Remedy:
Check the error rate log save folder and the file(s) in the folder.
 */
#define PP100API_CAN021 10021
/** @def PP100API_CAN100
 * The plug-in could not start.
Remedy:
Register the plug-in in the registry correctly.
 */
#define PP100API_CAN100 10100
/** @def PP100API_CAN101
 * The plug-in termination status code could not be retrieved, or is invalid.
Remedy:
Specify the defined termination status code.
 */
#define PP100API_CAN101 10101
/** @def PP100API_CAN102
 * The plug-in cancelled the job.
Remedy:
Check the plug-in processing.
 */
#define PP100API_CAN102 10102
/** @def PP100API_CAN103
 * The registry name session is set to one. But because the log-in user could not be identified, the plug-in failed to start.
Remedy:
Check the log-in state.
 */
#define PP100API_CAN103 10103
/** @def PP100API_STP000
 * Due to a disc type error or some other reason, the number of write retries has reached that preset by EPSON Total Disc Setup.
Remedy:
(1) Check the blank discs in the source stacker.
(2) Resume or cancel the paused job using EPSON Total Disc Monitor.
 */
#define PP100API_STP000 10300
/** @def PP100API_STP001
 * Due to a write error, the number of write retries has reached that preset by EPSON Total Disc Setup.
Remedy:
(1) Check the blank discs in the source stacker.
(2) Resume or cancel the paused job using EPSON Total Disc Monitor.
When the error still occurs, please contact the store where you purchased Disc producer or our customer service.
 */
#define PP100API_STP001 10301
/** @def PP100API_STP002
 * Consecutive errors regarding the error rate above threshold level have occurred and the number of continuous error rate measurement of Total Disc Setup has exceeded.
Remedy:
(1) Check the blank discs in the source stacker.
(2) Resume or cancel the paused job using EPSON Total Disc Monitor.
When the error still occurs, please contact the store where you purchased Disc producer or our customer service.
 */
#define PP100API_STP002 10302
/** @def PP100API_RTN000
 * No disc is set in the source stacker specified for the job.
Remedy:
Set blank discs in the source stacker.
 */
#define PP100API_RTN000 10200
/** @def PP100API_RTN001
 * The output stacker specified for the job has become full.
Remedy:
Remove the discs from the output stacker.
 */
#define PP100API_RTN001 10201
/** @def PP100API_RTN002
 * The disc cover is opening.
Remedy:
Close the disc cover.
 */
#define PP100API_RTN002 10202
/** @def PP100API_RTN003
 * The ink cartridge cover is open.
Remedy:
Close the ink cartridge cover.
 */
#define PP100API_RTN003 10203
/** @def PP100API_RTN004
 * The specified label print cannot be made because the ink level is low (the cartridge needs to be replaced).
Remedy:
Replace the ink cartridge(s) with a new one(s).
 */
#define PP100API_RTN004 10204
/** @def PP100API_RTN005
 * No ink cartridge is detected.
Remedy:
Install the ink cartridges.
 */
#define PP100API_RTN005 10205
/** @def PP100API_RTN006
 * The job cannot be made because the stacker has been removed.
Remedy:
Install the stacker.
 */
#define PP100API_RTN006 10206
/** @def PP100API_RTN007
 * The stacker 3 installation status does not match with the specified publishing mode.
Remedy:
Remove the stacker 3 in Standard or External Output mode.
Install the stacker 3 in Batch mode.
 */
#define PP100API_RTN007 10207
/** @def PP100API_RTN008
 * The job cannot be made because the stacker 4 is open.
Remedy:
Close the stacker 4.
 */
#define PP100API_RTN008 10208
/** @def PP100API_RTN009
 * Multiple discs have been transferred to the drive.
Remedy:
(1) Open the disc cover.
(2) Remove the disc from the drive tray, and close the disc cover.
 */
#define PP100API_RTN009 10209
/** @def PP100API_RTN010
 * Multiple discs have been transferred to the printer.
Remedy:
(1) Open the disc cover.
(2) Remove the disc from the drive tray, and close the disc cover.
 */
#define PP100API_RTN010 10210
/** @def PP100API_RTN011
 * Cannot recognize the ink cartridge.
Remedy:
Correctly install the ink cartridge.
 */
#define PP100API_RTN011 10211
/** @def PP100API_RTN012
 * Picking up a disc from the source stacker is failed.
Remedy:
Separate any sticky discs in the source stacker from each other.
 */
#define PP100API_RTN012 10212
/** @def PP100API_RTN013
 * The number of discs in the stacker has exceeded the upper limit.
Remedy:
Remove discs so that the number of discs in the stacker falls within the limit.
 */
#define PP100API_RTN013 10213
/** @def PP100API_RTN014
 * The maintenance box cover is not placed correctly.
Remedy:
Check the status of the cover.
 */
#define PP100API_RTN014 10214
/** @def PP100API_RTN015
 * Time to replace the maintenance box.
Remedy:
Replace the maintenance box with a new box.
 */
#define PP100API_RTN015 10215
/** @def PP100API_RTN016
 * The maintenance box is not installed.
Remedy:
Install the maintenance box.
 */
#define PP100API_RTN016 10216
/** @def PP100API_RTN017
 * Cannot recognize the maintenance box.
Remedy:
Correctly install the maintenance box.
 */
#define PP100API_RTN017 10217
/** @def PP100API_OTH000
 * Cannot obtain the job status. Disc publishing operation can be continued. This code may be temporarily output immediately after a job is issued.
Remedy:
When an INFORMATION Code is output, see "INFORMATION Code List" and perform the remedy according to the code.
When no INFORMATION Code is output, no error exists.
 */
#define PP100API_OTH000 10400
/** @} */ // end of ERROR Code(Refer to the TDBridge guide for details)

/** @defgroup INFORMATIONCodeRefertotheTDBridgeguidefordetails INFORMATION Code(Refer to the TDBridge guide for details)
 *  @ingroup define
 *  @{
 */
/** @def PP100API_HPR000
 * A printer fatal error has occurred.
Remedy:
Turn off PP-100/PP-100N and turn it back on.
 */
#define PP100API_HPR000 1
/** @def PP100API_HPR001
 * A printer maintenance error has occurred.
Remedy:
Please contact the store where you purchased Disc producer or our customer service.
 */
#define PP100API_HPR001 2
/** @def PP100API_HPR002
 * The waste ink pads need to be replaced.
Remedy:
Please contact the store where you purchased Disc producer or our customer service.
 */
#define PP100API_HPR002 3
/** @def PP100API_HPR003
 * A communication with the printer is failed.
Remedy:
Check if the port setting on the printer driver is correct.
 */
#define PP100API_HPR003 4
/** @def PP100API_HPR005
 * The ink cartridge cover is open.
Remedy:
Close the ink cartridge cover.
 */
#define PP100API_HPR005 5
/** @def PP100API_HPR006
 * A printer maintenance error has occurred.
Remedy:
Please contact the store where you purchased Disc producer or our customer service.
 */
#define PP100API_HPR006 6
/** @def PP100API_HPR007
 * The maintenance box is not installed.
Remedy:
Install the maintenance box.
 */
#define PP100API_HPR007 7
/** @def PP100API_HPR008
 * Time to replace the maintenance box.
Remedy:
Replace the maintenance box with a new box.
 */
#define PP100API_HPR008 8
/** @def PP100API_HPR009
 * Cannot recognize the maintenance box.
Remedy:
Correctly install the maintenance box.
 */
#define PP100API_HPR009 9
/** @def PP100API_HPR010
 * The maintenance box cover is not placed correctly.
Remedy:
Check the status of the cover.
 */
#define PP100API_HPR010 10
/** @def PP100API_HDR000
 * Controlling the drive tray is failed.
Remedy:
(1) Turn Disc producer off and remove any foreign object from the drive tray.
(2) Turn Disc producer back on.
 */
#define PP100API_HDR000 100
/** @def PP100API_HDR001
 * Processing the job is impossible because of a fatal error of the drive.
Remedy:
Turn off Disc producer and turn it back on.
 */
#define PP100API_HDR001 101
/** @def PP100API_HDR002
 * The drive is not connected correctly.
Remedy:
Check the connection of the drive.
 */
#define PP100API_HDR002 102
/** @def PP100API_HAR000
 * Picking up a disc from the drive tray or the printer tray is failed.
Remedy:
Turn off Disc producer and turn it back on.
 */
#define PP100API_HAR000 200
/** @def PP100API_HAR001
 * A disc is dropped while being picked up.
Remedy:
(1) Turn Disc producer off and remove the dropped disc.
(2) Turn Disc producer back on.
 */
#define PP100API_HAR001 201
/** @def PP100API_HAR002
 * Releasing a disc is failed.
Remedy:
Turn off Disc producer and turn it back on.
 */
#define PP100API_HAR002 202
/** @def PP100API_HAR003
 * Setting a disc in the drive tray or the printer tray must have been made, however, no disc is detected in the tray.
Remedy:
(1) Turn Disc producer off and remove any foreign object inside the machine.
(2) Turn Disc producer back on.
 */
#define PP100API_HAR003 203
/** @def PP100API_HAR004
 * An error occurs while the robotic arm is moving.
Remedy:
(1) Turn Disc producer off and remove any foreign object inside the machine.
(2) Turn Disc producer back on.
 */
#define PP100API_HAR004 204
/** @def PP100API_HAR005
 * An auto loader fatal error has occurred. The robotic arm can not be moved due to overheating of the motor.
Remedy:
(1) Turn Disc producer off and remove any foreign object inside the machine.
(2) Turn Disc producer back on.
 */
#define PP100API_HAR005 205
/** @def PP100API_HAR007
 * Picking up a disc from the stacker is failed.
Remedy:
(1) Open the disc cover.
(2) Separate any sticky discs in the source stacker from each other, and close the disc cover.
 */
#define PP100API_HAR007 207
/** @def PP100API_HAR008
 * Multiple discs have been transferred to the drive.
Remedy:
(1) Open the disc cover.
(2) Remove the disc from the open drive tray, and close the disc cover.
 */
#define PP100API_HAR008 208
/** @def PP100API_HAR009
 * Multiple discs have been transferred to the printer.
Remedy:
(1) Open the disc cover.
(2) Remove the disc from the open printer tray, and close the disc cover.
 */
#define PP100API_HAR009 209
/** @def PP100API_HIKC000
 * The cyan ink cartridge has been removed.
Remedy:
Install the cyan ink cartridge.
 */
#define PP100API_HIKC000 300
/** @def PP100API_HIKC001
 * The model number of the cyan ink cartridge is different or the model number cannot be recognized.
Remedy:
Replace the cyan ink cartridge with a correct one.
 */
#define PP100API_HIKC001 301
/** @def PP100API_HIKM000
 * The magenta ink cartridge has been removed.
Remedy:
Install the magenta ink cartridge.
 */
#define PP100API_HIKM000 400
/** @def PP100API_HIKM001
 * The model number of the magenta ink cartridge is different or the model number cannot be recognized.
Remedy:
Replace the magenta ink cartridge with a correct one.
 */
#define PP100API_HIKM001 401
/** @def PP100API_HIKY000
 * The yellow ink cartridge has been removed.
Remedy:
Install the yellow ink cartridge.
 */
#define PP100API_HIKY000 500
/** @def PP100API_HIKY001
 * The model number of the yellow ink cartridge is different or the model number cannot be recognized.
Remedy:
Replace the yellow ink cartridge with a correct one.
 */
#define PP100API_HIKY001 501
/** @def PP100API_HIKLC000
 * The light cyan ink cartridge has been removed.
Remedy:
Install the light cyan ink cartridge.
 */
#define PP100API_HIKLC000 600
/** @def PP100API_HIKLC001
 * The model number of the light cyan ink cartridge is different or the model number cannot be recognized.
Remedy:
Replace the light cyan ink cartridge with a correct one.
 */
#define PP100API_HIKLC001 601
/** @def PP100API_HIKLM000
 * The light magenta ink cartridge has been removed.
Remedy:
Install the light magenta ink cartridge.
 */
#define PP100API_HIKLM000 700
/** @def PP100API_HIKLM001
 * The model number of the light magenta ink cartridge is different or the model number cannot be recognized.
Remedy:
Replace the light magenta ink cartridge with a correct one.
 */
#define PP100API_HIKLM001 701
/** @def PP100API_HIKBK000
 * The black ink cartridge has been removed.
Remedy:
Install the black ink cartridge.
 */
#define PP100API_HIKBK000 800
/** @def PP100API_HIKBK001
 * The model number of the black ink cartridge is different or the model number cannot be recognized.
Remedy:
Replace the black ink cartridge with a correct one.
 */
#define PP100API_HIKBK001 801
/** @def PP100API_HST1000
 * The number of discs in the stacker 1 has exceeded the upper limit.
Remedy:
Remove discs so that the number of discs in the stacker 1 falls within the limit.
 */
#define PP100API_HST1000 1000
/** @def PP100API_HST1001
 * The stacker 1 has not been installed.
Remedy:
Install the stacker 1.
 */
#define PP100API_HST1001 1001
/** @def PP100API_HST2000
 * The number of discs in the stacker 2 has exceeded the upper limit.
Remedy:
Remove discs so that the number of discs in the stacker 2 falls within the limit.
 */
#define PP100API_HST2000 2000
/** @def PP100API_HST2001
 * The stacker 2 has not been installed.
Remedy:
Install the stacker 2.
 */
#define PP100API_HST2001 2001
/** @def PP100API_HST3000
 * The number of discs in the stacker 3 has exceeded the upper limit.
Remedy:
Remove discs so that the number of discs in the stacker 3 falls within the limit.
 */
#define PP100API_HST3000 3000
/** @def PP100API_HST3001
 * The output stacker in Standard mode is set to stacker 3, but stacker 3 is not installed, or the output stacker in External Output mode is set to stacker 3, but stacker 3 is not installed, or stacker 3 is not installed in Batch mode.
Remedy:
Install stacker 3.
 */
#define PP100API_HST3001 3001
/** @def PP100API_HST3002
 * The output stacker in Standard mode is set to stacker 4, or the output stacker in External Output mode is set to stacker 4, but stacker 3 is installed.
Remedy:
[PP-100 / PP-100N / PP-100AP / PP-100II]
Remove stacker 3.
 */
#define PP100API_HST3002 3002
/** @def PP100API_HDC000
 * The disc cover is open.
Remedy:
Close the disc cover.
 */
#define PP100API_HDC000 4000
/** @def PP100API_HNW000
 * Either one of the following communication errors occur.
(1) Communication with Disc producer is disabled.
(2) The filter driver has not been attached.
Remedy:
[Other than PP-100N]
(1) Check the power supply of the Disc producer.
(2) Check if the USB cable is securely connected.
(3) Connect the publishing machine, and turn on the power.
[For PP-100N]
(1) Check the power supply of the Disc producer.
(2) Check if the network cable is securely connected.
 */
#define PP100API_HNW000 4100
/** @def PP100API_HNW001
 * A non-supported version error has occurred, and any of the following has been detected.
(1) Firmware version of the auto loader is not supported.
(2) Firmware version of the drive 1 is different from the supported version.
(3) Firmware version of the drive 2 is different from the supported version.
(4) Firmware version of the printer is not supported.
Remedy:
Upgrade the firmware of the concerning device.
 */
#define PP100API_HNW001 4101
/** @} */ // end of INFORMATION Code(Refer to the TDBridge guide for details)

/** @defgroup Publishmode Publish mode
 *  @ingroup define
 *  @{
 */
/** @def PP100API_NORMAL_MODE
 * Normal mode
 */
#define PP100API_NORMAL_MODE 1
/** @def PP100API_OUTPUT_MODE
 * Output mode
 */
#define PP100API_OUTPUT_MODE 2
/** @def PP100API_BATCH_MODE
 * Batch mode
 */
#define PP100API_BATCH_MODE 3
/** @def PP100API_OUTPUT_CONTROL_MODE
 * Output Control mode
 */
#define PP100API_OUTPUT_CONTROL_MODE 4
/** @def PP100API_SECURITY_MODE
 * Security mode
 */
#define PP100API_SECURITY_MODE 5
/** @def PP100API_MEASUREMENT_MODE
 * Measurement mode
 */
#define PP100API_MEASUREMENT_MODE 6
/** @def PP100API_READ_BACK_MODE
 * Read back mode
 */
#define PP100API_READ_BACK_MODE 7
/** @} */ // end of Publish mode

/** @defgroup Stackersetting Stacker setting
 *  @ingroup define
 *  @{
 */
/** @def PP100API_INPUT
 * input
 */
#define PP100API_INPUT 1
/** @def PP100API_OUTPUT
 * output
 */
#define PP100API_OUTPUT 2
/** @def PP100API_NOT_USE_STACKER
 * not use
 */
#define PP100API_NOT_USE_STACKER 3
/** @def PP100API_CD_MINUS_R
 * CD-R
 */
#define PP100API_CD_MINUS_R 10
/** @def PP100API_DVD_MINUS_R
 * DVD-R
 */
#define PP100API_DVD_MINUS_R 20
/** @def PP100API_DVD_PLUS_R
 * DVD+R
 */
#define PP100API_DVD_PLUS_R 21
/** @def PP100API_DVD_R
 * DVD+/-R
 */
#define PP100API_DVD_R 22
/** @def PP100API_DVD_MINUS_R_DL
 * DVD-R DL
 */
#define PP100API_DVD_MINUS_R_DL 30
/** @def PP100API_DVD_PLUS_R_DL
 * DVD+R DL
 */
#define PP100API_DVD_PLUS_R_DL 31
/** @def PP100API_DVD_R_DL
 * DVD+/-R DL
 */
#define PP100API_DVD_R_DL 32
/** @def PP100API_BD_MINUS_R
 * BD-R
 */
#define PP100API_BD_MINUS_R 40
/** @def PP100API_BD_MINUS_R_DL
 * BD-R DL
 */
#define PP100API_BD_MINUS_R_DL 50
/** @} */ // end of Stacker setting

/** @defgroup Publisherstatus Publisher status
 *  @ingroup define
 *  @{
 */
/** @def PP100API_WAITING
 * waiting
 */
#define PP100API_WAITING 1
/** @def PP100API_TRANSFERRING
 * Transferring
 */
#define PP100API_TRANSFERRING 2
/** @def PP100API_PAUSED
 * paused
 */
#define PP100API_PAUSED 3
/** @def PP100API_PRINTING
 * printing
 */
#define PP100API_PRINTING 100
/** @def PP100API_DRYING
 * drying
 */
#define PP100API_DRYING 101
/** @def PP100API_CLEANING
 * cleaning
 */
#define PP100API_CLEANING 102
/** @def PP100API_WRITING
 * writing
 */
#define PP100API_WRITING 200
/** @def PP100API_VERIFYING_WRITE
 * verifying write
 */
#define PP100API_VERIFYING_WRITE 201
/** @def PP100API_UNUSED
 * not use
 */
#define PP100API_UNUSED 202
/** @def PP100API_PLUGIN_PROCESSING
 * Plug-in processing
 */
#define PP100API_PLUGIN_PROCESSING 203
/** @def PP100API_MEASURING
 * measuring
 */
#define PP100API_MEASURING 204
/** @} */ // end of Publisher status

/** @defgroup JobStatusRefertotheTDBridgeguideforstatetransition Job Status(Refer to the TDBridge guide for state transition)
 *  @ingroup define
 *  @{
 */
/** @def PP100API_JOB_PROCESSING_TO_ACCEPT
 * Job is processing to accept
 */
#define PP100API_JOB_PROCESSING_TO_ACCEPT 1
/** @def PP100API_JOB_WAITING
 * Job is waiting
 */
#define PP100API_JOB_WAITING 2
/** @def PP100API_JOB_RUNNING
 * Job is transferring or writing or printing
 */
#define PP100API_JOB_RUNNING 3
/** @def PP100API_JOB_PAUSING
 * Job is pausing
 */
#define PP100API_JOB_PAUSING 4
/** @def PP100API_JOB_RECOVERING
 * Job is recovering
 */
#define PP100API_JOB_RECOVERING 5
/** @def PP100API_JOB_CANCELING
 * Job is canceling
 */
#define PP100API_JOB_CANCELING 6
/** @def PP100API_JOB_PAUSED
 * Job is pausing
 */
#define PP100API_JOB_PAUSED 7
/** @def PP100API_JOB_STANDBY
 * Job is standby
 */
#define PP100API_JOB_STANDBY 8
/** @def PP100API_JOB_PUBLISHED
 * Job is published
 */
#define PP100API_JOB_PUBLISHED 9
/** @def PP100API_JOB_PUBLISHED_WARNING_NO_ERROR_DISC
 * Job is published (warning , no error disc)
 */
#define PP100API_JOB_PUBLISHED_WARNING_NO_ERROR_DISC 10
/** @def PP100API_JOB_PUBLISHED_WARNING_ERROR_DISC
 * Job is finished (warning , error disc)
 */
#define PP100API_JOB_PUBLISHED_WARNING_ERROR_DISC 11
/** @def PP100API_JOB_USER_CANCELLED
 * Job is cancelled by user operation
 */
#define PP100API_JOB_USER_CANCELLED 12
/** @def PP100API_JOB_ERROR_CANCELLED
 * Job is cancelled by error
 */
#define PP100API_JOB_ERROR_CANCELLED 13
/** @def PP100API_JOB_DENIED
 * Job is denied
 */
#define PP100API_JOB_DENIED 14
/** @} */ // end of Job Status(Refer to the TDBridge guide for state transition)

/** @defgroup Stackerrest Stacker rest
 *  @ingroup define
 *  @{
 */
/** @def PP100API_STACKER_NOT_FULL
 * Stacker is not full
 */
#define PP100API_STACKER_NOT_FULL 200
/** @} */ // end of Stacker rest

/** @defgroup Usbconnectionmode Usb connection mode
 *  @ingroup define
 *  @{
 */
/** @def PP100API_USB_20
 * USB2.0
 */
#define PP100API_USB_20 1
/** @def PP100API_USB_30
 * USB3.0
 */
#define PP100API_USB_30 2
/** @} */ // end of Usb connection mode

/** @defgroup JobType Job Type
 *  @ingroup define
 *  @{
 */
/** @def PP100API_JOB_TYPE_WRITE
 * writing job
 */
#define PP100API_JOB_TYPE_WRITE 1
/** @def PP100API_JOB_TYPE_PRINT
 * printing job
 */
#define PP100API_JOB_TYPE_PRINT 2
/** @def PP100API_JOB_TYPE_PUBLISH
 * writing & printing job
 */
#define PP100API_JOB_TYPE_PUBLISH 3
/** @def PP100API_JOB_TYPE_MEASURE
 * measuring job
 */
#define PP100API_JOB_TYPE_MEASURE 4
/** @} */ // end of Job Type

/** @defgroup settingvalueofjob setting value of job
 *  @ingroup define
 *  @{
 */
/** @def PP100API_STACKER1
 * stacker 1
 */
#define PP100API_STACKER1 1
/** @def PP100API_STACKER2
 * stacker 2
 */
#define PP100API_STACKER2 2
/** @def PP100API_STACKER3
 * stacker 3
 */
#define PP100API_STACKER3 3
/** @def PP100API_STACKER4
 * stacker 4
 */
#define PP100API_STACKER4 4
/** @def PP100API_STACKER1_OR_STACKER2
 * stacker 1, stacker2
 */
#define PP100API_STACKER1_OR_STACKER2 12
/** @def PP100API_STACKER2_OR_STACKER3
 * stacker 2, stacker3
 */
#define PP100API_STACKER2_OR_STACKER3 23
/** @def PP100API_STACKER2_OR_STACKER4
 * stacker 2, stacker4
 */
#define PP100API_STACKER2_OR_STACKER4 24
/** @def PP100API_NOSETTING
 * no setting
 */
#define PP100API_NOSETTING 5
/** @def PP100API_CD
 * CD
 */
#define PP100API_CD 1
/** @def PP100API_DVD
 * DVD
 */
#define PP100API_DVD 2
/** @def PP100API_DVD_DL
 * DVD DL
 */
#define PP100API_DVD_DL 3
/** @def PP100API_BD
 * BD
 */
#define PP100API_BD 4
/** @def PP100API_BD_DL
 * BD DL
 */
#define PP100API_BD_DL 5
/** @def PP100API_COMPARE
 * compare
 */
#define PP100API_COMPARE 1
/** @def PP100API_DONT_COMPARE
 * don't compare
 */
#define PP100API_DONT_COMPARE 2
/** @def PP100API_MEASURE
 * measure
 */
#define PP100API_MEASURE 1
/** @def PP100API_DONT_MEASURE
 * don't measure
 */
#define PP100API_DONT_MEASURE 2
/** @def PP100API_ONLY_ARCHIVE_DISC
 * only archive disc
 */
#define PP100API_ONLY_ARCHIVE_DISC 1
/** @def PP100API_NOT_ONLY_ARCHIVE_DISC
 * not only archive disc
 */
#define PP100API_NOT_ONLY_ARCHIVE_DISC 2
/** @def PP100API_CLOSE_DISC
 * close
 */
#define PP100API_CLOSE_DISC 1
/** @def PP100API_DONT_CLOSE_DISC
 * don't close
 */
#define PP100API_DONT_CLOSE_DISC 2
/** @def PP100API_ISO9660L2
 * ISO 9660 level 2
 */
#define PP100API_ISO9660L2 1
/** @def PP100API_JOLIET
 * Joliet
 */
#define PP100API_JOLIET 2
/** @def PP100API_UDF102
 * UDF 1.02
 */
#define PP100API_UDF102 3
/** @def PP100API_UDF102_BRIDGE
 * UDF 1.02 + UDF Bridge
 */
#define PP100API_UDF102_BRIDGE 4
/** @def PP100API_UDF150
 * UDF 1.50
 */
#define PP100API_UDF150 6
/** @def PP100API_UDF260
 * UDF 2.60
 */
#define PP100API_UDF260 5
/** @def PP100API_CD_DVD_LABEL
 * CD/DVD label
 */
#define PP100API_CD_DVD_LABEL 1
/** @def PP100API_CD_DVD_PREMIUM_LABEL
 * CD/DVD Premium Label
 */
#define PP100API_CD_DVD_PREMIUM_LABEL 2
/** @def PP100API_EPSON_SPECIFIED_CD_DVD_LABEL
 * EPSON Specified CD/DVD Label
 */
#define PP100API_EPSON_SPECIFIED_CD_DVD_LABEL 3
/** @def PP100API_PRINTMODE1
 * Print mode 1
 */
#define PP100API_PRINTMODE1 1
/** @def PP100API_PRINTMODE2
 * Print mode 2
 */
#define PP100API_PRINTMODE2 2
/** @def PP100API_PRINTMODE3
 * Print mode 3
 */
#define PP100API_PRINTMODE3 3
/** @def PP100API_NORMAL_PRIORITY
 * normal priority
 */
#define PP100API_NORMAL_PRIORITY 1
/** @def PP100API_HIGH_PRIORITY
 * high priority
 */
#define PP100API_HIGH_PRIORITY 2
/** @} */ // end of setting value of job

/** @defgroup invalidvalue invalid value
 *  @ingroup define
 *  @{
 */
/** @def PP100API_UNKNOWN
 * unknown
 */
#define PP100API_UNKNOWN 0xFFFFFFFF
/** @def PP100API_ERROR
 * error
 */
#define PP100API_ERROR 0xFFFFFFFE
/** @} */ // end of invalid value

/** @} */ // end of define
/** @defgroup structure structure
 *  @{
 */
#pragma pack(1)
/** @struct _PP100_ENUM_PUBLISHER_INFO
 * @brief PP-100 Series information
 */
struct _PP100_ENUM_PUBLISHER_INFO {

    /** @brief the name that is registered by Total Disc Setup */
    wchar_t pPublisherName[65];

    /** @brief drive number */
    unsigned long ulDriveNumber;

    /** @brief connection type [ 0 : USB 1 : Ethernet] */
    unsigned long ulConnectType;

    /** @brief reserved area */
    unsigned char ucReserved[512];

};
/** @brief PP-100 Series information */
typedef _PP100_ENUM_PUBLISHER_INFO PP100_ENUM_PUBLISHER_INFO;
/** @brief PP-100 Series information */
typedef _PP100_ENUM_PUBLISHER_INFO *LPPP100_ENUM_PUBLISHER_INFO;

/** @struct _PP100_INK_STATUS
 * @brief The Ink remaining of PP-100 Series. The percentage of 0 to 100. Unknown:PP100API_ERROR
 */
struct _PP100_INK_STATUS {

    /** @brief Cyan Ink */
    unsigned long ulCyan;

    /** @brief Magenta Ink */
    unsigned long ulMagenta;

    /** @brief Yellow Ink */
    unsigned long ulYellow;

    /** @brief Light-Cyan Ink */
    unsigned long ulLightCyan;

    /** @brief Light-Magenta Ink */
    unsigned long ulLightMagenta;

    /** @brief Black Ink */
    unsigned long ulBlack;

};
/** @brief The Ink remaining of PP-100 Series. The percentage of 0 to 100. Unknown:PP100API_ERROR */
typedef _PP100_INK_STATUS PP100_INK_STATUS;
/** @brief The Ink remaining of PP-100 Series. The percentage of 0 to 100. Unknown:PP100API_ERROR */
typedef _PP100_INK_STATUS *LPPP100_INK_STATUS;

/** @struct _PP100_PUBLISHER_STATUS
 * @brief The Status of PP-100 Series
 */
struct _PP100_PUBLISHER_STATUS {

    /** @brief INFORMATION Code */
    unsigned long ulINFORMATIONCode[50];

    /** @brief The publish mode */
    unsigned long ulMode;

    /** @brief Drive Status */
    unsigned long ulDriveStatus[2];

    /** @brief The Plug-in name of drive */
    wchar_t pDrivePluginName[2][41];

    /** @brief Drive life. The percentage of 0 to 100. Unknown:PP100API_UNKNOWN */
    unsigned long ulDriveLife[2];

    /** @brief Printer Status */
    unsigned long ulPrinterStatus;

    /** @brief Ink remaining */
    PP100_INK_STATUS stInkStatus;

    /** @brief Setting of the stacker */
    unsigned long ulStackerSetting[4];

    /** @brief Disc remaining in the stacker. The percentage of 0 to 100. Unknown:PP100API_ERROR */
    unsigned long ulStackerRest[4];

    /** @brief The minimum printable copies */
    unsigned long ulPrintableCopies;

    /** @brief The printed copies */
    unsigned long ulPrintedCopies;

    /** @brief Free space of maintenance box. The percentage of 0 to 100. Only PP-100AP and PP-100II. Unknown:PP100API_ERROR */
    unsigned long ulMaintenanceBoxFreeSpace;

    /** @brief Serial number of PP-100 Series */
    wchar_t pSerialNumber[33];

    /** @brief Progress of drive. The percentage of 0 to 100. Unknown:PP100API_UNKNOWN */
    unsigned long ulDriveProgress[2];

    /** @brief Total discs published. Only PP-100II. Unknown:PP100API_ERROR */
    unsigned long ulCompleteDiscNum;

    /** @brief usb connection mode */
    unsigned long ulUsbConnectionMode;

    /** @brief reserved area */
    unsigned char ucReserved[496];

};
/** @brief The Status of PP-100 Series */
typedef _PP100_PUBLISHER_STATUS PP100_PUBLISHER_STATUS;
/** @brief The Status of PP-100 Series */
typedef _PP100_PUBLISHER_STATUS *LPPP100_PUBLISHER_STATUS;

/** @struct _PP100_JOB_STATUS
 * @brief The JOB Status of PP-100 Series
 */
struct _PP100_JOB_STATUS {

    /** @brief JOB ID */
    wchar_t pJobID[41];

    /** @brief the name that is registered by Total Disc Setup */
    wchar_t pPublisherName[65];

    /** @brief JOB status */
    unsigned long ulJobStatus;

    /** @brief The factor of error or waiting recovery */
    unsigned long ulErrorCode;

    /** @brief The number of publication */
    unsigned long ulPublicationNumber;

    /** @brief The number of completion */
    unsigned long ulCompletionNumber;

    /** @brief The job type */
    unsigned long ulJobType;

    /** @brief The source stacker */
    unsigned long ulSource;

    /** @brief The destination stacker */
    unsigned long ulDestination;

    /** @brief The job index */
    unsigned long ulJobIndex;

    /** @brief The estimated time to complete job.(in seconds) */
    unsigned long ulEstimateTime;

    /** @brief The number of error disc */
    unsigned long ulErrorNumber;

    /** @brief The publish mode */
    unsigned long ulMode;

    /** @brief 1:The job is submitted by Total Disc Maker directly. 0:The job is submitted by TDBridge. */
    unsigned char ucIsSubmittedByTotalDiscMaker;

    /** @brief The remaining estimated time to complete job.(in seconds) */
    unsigned long ulRemainingEstimateTime;

    /** @brief reserved area */
    unsigned char ucReserved[508];

};
/** @brief The JOB Status of PP-100 Series */
typedef _PP100_JOB_STATUS PP100_JOB_STATUS;
/** @brief The JOB Status of PP-100 Series */
typedef _PP100_JOB_STATUS *LPPP100_JOB_STATUS;

/** @struct _PP100_WRITE_DATA
 * @brief The Write Data of PP-100 Series
 */
struct _PP100_WRITE_DATA {

    /** @brief source data */
    wchar_t pSourceData[256];

    /** @brief destination data */
    wchar_t pDestinationData[256];

};
/** @brief The Write Data of PP-100 Series */
typedef _PP100_WRITE_DATA PP100_WRITE_DATA;
/** @brief The Write Data of PP-100 Series */
typedef _PP100_WRITE_DATA *LPPP100_WRITE_DATA;

/** @struct _PP100_AUDIO_TRACK
 * @brief The AudioCD Track Data of PP-100 Series
 */
struct _PP100_AUDIO_TRACK {

    /** @brief music file path */
    wchar_t pMusicFilePath[256];

    /** @brief track title */
    wchar_t pTrackTitle[129];

    /** @brief track artist name */
    wchar_t pTrackArtistName[129];

    /** @brief pregap[The range of settable values is from 0 to 269999. (00:00:00 frame to 59:59:74 fame)] */
    unsigned long ulPregap;

    /** @brief ISRC */
    wchar_t pISRC[13];

};
/** @brief The AudioCD Track Data of PP-100 Series */
typedef _PP100_AUDIO_TRACK PP100_AUDIO_TRACK;
/** @brief The AudioCD Track Data of PP-100 Series */
typedef _PP100_AUDIO_TRACK *LPPP100_AUDIO_TRACK;

/** @struct _PP100_REPLACE_FIELD_TEXT
 * @brief The print data of string(text , barcode)
 */
struct _PP100_REPLACE_FIELD_TEXT {

    /** @brief the string of item */
    wchar_t pName[1025];

    /** @brief the string to replace */
    wchar_t pValue[1025];

};
/** @brief The print data of string(text , barcode) */
typedef _PP100_REPLACE_FIELD_TEXT PP100_REPLACE_FIELD_TEXT;
/** @brief The print data of string(text , barcode) */
typedef _PP100_REPLACE_FIELD_TEXT *LPP100_REPLACE_FIELD_TEXT;

/** @struct _PP100_REPLACE_FIELD_BINARY
 * @brief The print data of binary
 */
struct _PP100_REPLACE_FIELD_BINARY {

    /** @brief the string of item */
    wchar_t pName[1025];

    /** @brief the file path of binary data to replace */
    wchar_t pPath[256];

};
/** @brief The print data of binary */
typedef _PP100_REPLACE_FIELD_BINARY PP100_REPLACE_FIELD_BINARY;
/** @brief The print data of binary */
typedef _PP100_REPLACE_FIELD_BINARY *LPP100_REPLACE_FIELD_BINARY;

#pragma pack()

/** @} */ // end of structure
/** @defgroup function function
 *  @{
 */
/** @defgroup function function
 *  @{
 */
/** @brief Function to initialize internal data of the PP-100 API
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_SUPPORTED if the environment is not supported
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_Initialize(void);

/** @brief Function to destroy internal data of the PP-100 API
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_Destroy(void);

/** @brief Function to connect to the computer on which TDBridge is running and get the server handle
 * @param[in] pHost[64] Host name or IP address(only IPv4) that TDBridge is running. If pHost is NULL, this function assumes localhost.
 * @param[in] pOrderFolder[256] Folder path that TDBridge is monitoring.If pHost[64] is NULL, this parameter is ignored.
 * @param[out] pHandle buffer for storing server handle
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pHost[64] is not NULL and pOrderFolder[256] is NULL , b. pHandle is NULL
 * @retval PP100API_HOST_NOT_FOUND If pHost[64] is not found
 * @retval PP100API_ORDER_FOLDER_NOT_FOUND If pOrderFolder[256] is not found
 * @retval PP100API_ACCESS_DENIED if access to the folder is denied
 * @retval PP100API_TDBRIDGE_LESS_VERSION if TDBridge version is less than 5.00
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_ConnectServer(wchar_t pHost[64] , wchar_t pOrderFolder[256] , unsigned long* pHandle);

/** @brief Function to disconnect from the server
 * @param[in] ulHandle server handle that got by calling PP100_ConnectServer() function
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If ulHandle is NULL
 * @retval PP100API_HOST_NOT_FOUND If the host specified ulHandle not found
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_DisConnectServer(unsigned long ulHandle);

/** @brief Function to enumerate PP-100 Series that Total Disc Setup recognizes
 * @param[in] ulHandle server handle received by calling PP100_ConnectServer() function
 * @param[out] pEnumPublisherInfo[] Pointer of the array for storing result of this function. The caller need to allocate memory of the array.
 * @param[in,out] pEnumPubilsherInfoNum number of array pEnumPublisherInfo[]
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. ulHandle is NULL , b. pEnumPublisherInfo[] is NULL , c. pEnumPubilsherInfoNum is NULL
 * @retval PP100API_HOST_NOT_FOUND If the host specified ulHandle not found
 * @retval PP100API_ACCESS_DENIED if access to the folder is denied
 * @retval PP100API_TDBRIDGE_LESS_VERSION if TDBridge version is less than 5.00
 * @retval PP100API_MORE_ITEMS If pEnumPubilsherInfoNum is insufficient or NULL, the data will not be copied and pEnumPubilsherInfoNum will be set to number of items
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_EnumPublishers(unsigned long ulHandle , PP100_ENUM_PUBLISHER_INFO pEnumPublisherInfo[] , unsigned long* pEnumPubilsherInfoNum);

/** @brief Function to get the status of the PP-100 Series
 * @param[in] ulHandle server handle that got by calling PP100_ConnectServer() function
 * @param[in] pPublisherName[65] the publisher name registered in Total Disc Setup
 * @param[out] pPublisherStatus buffer for storing status
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. ulHandle is NULL , b. pPublisherName[65] is NULL , c. pPublisherStatus is NULL
 * @retval PP100API_HOST_NOT_FOUND If the host specified ulHandle not found
 * @retval PP100API_PUBLISHER_NOT_FOUND If the PP-100 Series specified pPublisherName[65] is not found
 * @retval PP100API_ACCESS_DENIED if access to the folder is denied
 * @retval PP100API_TDBRIDGE_LESS_VERSION if TDBridge version is less than 5.00
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetPublisherStatus(unsigned long ulHandle , wchar_t pPublisherName[65] , LPPP100_PUBLISHER_STATUS pPublisherStatus);

/** @brief Function to get the status of all jobs or the job specified by the job ID. When completed jobs are over 50, the job statuses are stored recent 50.
 * @param[in] ulHandle server handle that got by calling PP100_ConnectServer() function
 * @param[in] pJobID[41] Job ID If this parameter is NULL, this function get all job status
 * @param[out] pJobStatus[] Buffer for storing job status. The caller need to allocate memory of the array.
 * @param[in,out] pJobStatusNum number of array pJobStatusNum
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. ulHandle is NULL , b. pJobID[41] is NULL , c. pJobStatus[] is NULL , d. pJobStatusNum is NULL
 * @retval PP100API_HOST_NOT_FOUND If the host specified ulHandle not found
 * @retval PP100API_PUBLISHER_NOT_FOUND If the PP-100 Series specified pPublisherName[65] not found
 * @retval PP100API_ACCESS_DENIED if access to the folder is denied
 * @retval PP100API_TDBRIDGE_LESS_VERSION if TDBridge version is less than 5.00
 * @retval PP100API_MORE_ITEMS If pJobStatusNum is insufficient or NULL, the data will not be copied and pJobStatusNum will be set to number of items
 * @retval PP100API_INFORMATION_CODE_NOT_DEFINED if the job specified pJobID[41] is not recognized
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetJobStatus(unsigned long ulHandle , wchar_t pJobID[41] , PP100_JOB_STATUS pJobStatus[] , unsigned long* pJobStatusNum);

/** @brief Function to create a new job
 * @param[out] pJobID[41] Job ID
 * @retval PP100API_SUCCESS if successful. In this case, this function sets pJobID[41] Job ID string
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the condition below a. pJobID[41] is NULL
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_CreateJob(wchar_t pJobID[41]);

/** @brief Function to get the job ID created job in PP100_CreateJob
 * @param[out] ppJobID pointer to array of buffer for storing job id
 * @param[in,out] pJobIDNum number of job
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. ppJobID is NULL , b. pJobIDNum is NULL
 * @retval PP100API_MORE_ITEMS If pJobIDNum is insufficient or NULL, the data will not be copied and pJobIDNum will be set to number of items
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetCreatedJobList(wchar_t** ppJobID , unsigned long* pJobIDNum);

/** @brief Function to copy the job created in PP100_CreateJob and assign another job ID
 * @param[in] pSourceJobID[41] job id of source job
 * @param[out] pDestinationJobID[41] job id of destination job
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pSourceJobID[41] is NULL , b. pDestinationJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pSourceJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_CopyJob(wchar_t pSourceJobID[41] , wchar_t pDestinationJobID[41]);

/** @brief Function to delete the created job in PP100_CreateJob
 * @param[in] pJobID[41] Job ID
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the condition below a. pJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_DeleteJob(wchar_t pJobID[41]);

/** @brief Function to create JDF file from the job created in PP100_CreateJob and copy the JDF file to the server
 * @param[in] ulHandle server handle that got by calling PP100_ConnectServer() function
 * @param[in] pJobID[41] Job ID
 * @param[in] bAutoDelete true:If successful this function deletes the job specified pJobID[41].false:This function doesn't delete the job.So you need to delete the job calling PP100_DeleteJob when job is not needed.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. ulHandle is NULL , b. pJobID[41] is NULL
 * @retval PP100API_HOST_NOT_FOUND If the host specified ulHandle is not found
 * @retval PP100API_ACCESS_DENIED if access to the folder is denied
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_TDBRIDGE_LESS_VERSION if TDBridge version is less than 5.00
 * @retval PP100API_JOB_SUBMITTED if the job is published to the server
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SubmitJob(unsigned long ulHandle , wchar_t pJobID[41] , bool bAutoDelete);

/** @brief Function to create a JCF file for the specified job ID and copy JCF file to the server
 * @param[in] ulHandle server handle received by calling PP100_ConnectServer() function
 * @param[in] pJobID[41] Job ID
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. ulHandle is NULL , b. pJobID[41] is NULL
 * @retval PP100API_HOST_NOT_FOUND If the host specified ulHandle not found
 * @retval PP100API_ACCESS_DENIED if access to the folder is denied
 * @retval PP100API_TDBRIDGE_LESS_VERSION if TDBridge version is less than 5.00
 * @retval PP100API_JOB_IS_SUBMITTED_BY_TOTAL_DISC_MAKER if failed to cancel the job specified pJobID[41] because the job is submitted by Total Disc Maker directly.
 * @retval PP100API_INFORMATION_CODE_NOT_DEFINED if the job specified pJobID[41] is not recognized
 * @retval PP100API_JOB_IS_ALREADY_FINISHED if the job specified pJobID[41] finished
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_CancelJob(unsigned long ulHandle , wchar_t pJobID[41]);

/** @brief Function to set PUBLISHER key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] pPublisher[65] the machine name. If this parameter is NULL, this function delete the key.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the condition below a. pJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetPublisher(wchar_t pJobID[41] , wchar_t pPublisher[65]);

/** @brief Function to get PUBLISHER key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pPublisher[65].
 * @param[in] pJobID[41] Job ID
 * @param[out] pPublisher[65] the machine name
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pPublisher[65] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetPublisher(wchar_t pJobID[41] , wchar_t pPublisher[65]);

/** @brief Function to set COPIES key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] ulNumber the number to publish. If this parameter is NULL, this function delete the key.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the condition below a. pJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetCopies(wchar_t pJobID[41] , unsigned long ulNumber);

/** @brief Function to get COPIES key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pNumber.
 * @param[in] pJobID[41] Job ID
 * @param[out] pNumber the number to publish
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pNumber is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetCopies(wchar_t pJobID[41] , unsigned long* pNumber);

/** @brief Function to set OUT_STACKER key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] ulOutStacker the output stacker. If this parameter is NULL, this function delete the key.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the condition below a. pJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetOutStacker(wchar_t pJobID[41] , unsigned long ulOutStacker);

/** @brief Function to get OUT_STACKER key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pOutStacker.
 * @param[in] pJobID[41] Job ID
 * @param[out] pOutStacker the output stacker
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below c. pJobID[41] is NULL , d. pOutStacker is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetOutStacker(wchar_t pJobID[41] , unsigned long* pOutStacker);

/** @brief Function to set IN_STACKER key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] ulInStacker the input stacker. If this parameter is NULL, this function delete the key.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the condition below c. pJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetInStacker(wchar_t pJobID[41] , unsigned long ulInStacker);

/** @brief Function to get IN_STACKER key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pInStacker.
 * @param[in] pJobID[41] Job ID
 * @param[out] pInStacker the input stacker
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below c. pJobID[41] is NULL , d. pInStacker is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetInStacker(wchar_t pJobID[41] , unsigned long* pInStacker);

/** @brief Function to set DISC_TYPE key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] ulDiscType the kind of disc. If this parameter is NULL, this function delete the key.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the condition below a. pJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetDiscType(wchar_t pJobID[41] , unsigned long ulDiscType);

/** @brief Function to get DISC_TYPE key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pDiscType.
 * @param[in] pJobID[41] Job ID
 * @param[out] pDiscType the kind of disc
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pDiscType is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetDiscType(wchar_t pJobID[41] , unsigned long* pDiscType);

/** @brief Function to set WRITING_SPEED key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] fWritingSpeed the writing speed.It is positive value.It is rounded to one decimal place. If this parameter is NULL, this function delete the key.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. fWritingSpeed is less zero.
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetWritingSpeed(wchar_t pJobID[41] , float fWritingSpeed);

/** @brief Function to get WRITING_SPEED key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pWritingSpeed.
 * @param[in] pJobID[41] Job ID
 * @param[out] pWritingSpeed the writing speed.It is positive value.It is rounded to one decimal place.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pWritingSpeed is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetWritingSpeed(wchar_t pJobID[41] , float* pWritingSpeed);

/** @brief Function to set COMPARE key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] ulCompare the compare setting. If this parameter is NULL, this function delete the key.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the condition below a. pJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetCompare(wchar_t pJobID[41] , unsigned long ulCompare);

/** @brief Function to get COMPARE key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pCompare.
 * @param[in] pJobID[41] Job ID
 * @param[out] pCompare the compare setting
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pCompare is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetCompare(wchar_t pJobID[41] , unsigned long* pCompare);

/** @brief Function to set MEASURE key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] ulMeasure the measure setting. If this parameter is NULL, this function delete the key.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the condition below a. pJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetMeasure(wchar_t pJobID[41] , unsigned long ulMeasure);

/** @brief Function to get MEASURE key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pMeasure.
 * @param[in] pJobID[41] Job ID
 * @param[out] pMeasure the measure setting
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pMeasure is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetMeasure(wchar_t pJobID[41] , unsigned long* pMeasure);

/** @brief Function to set ARCHIVE_DISC_ONLY key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] ulArchiveDiscOnly the archive disc only setting. If this parameter is NULL, this function delete the key.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the condition below a. pJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetArchiveDiscOnly(wchar_t pJobID[41] , unsigned long ulArchiveDiscOnly);

/** @brief Function to get ARCHIVE_DISC_ONLY key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pArchiveDiscOnly.
 * @param[in] pJobID[41] Job ID
 * @param[out] pArchiveDiscOnly the archive disc only setting
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pMeasure is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetArchiveDiscOnly(wchar_t pJobID[41] , unsigned long* pArchiveDiscOnly);

/** @brief Function to set CLOSE_DISC key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] ulCloseDisc the disc close setting. If this parameter is NULL, this function delete the key.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the condition below a. pJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetCloseDisc(wchar_t pJobID[41] , unsigned long ulCloseDisc);

/** @brief Function to get CLOSE_DISC key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pCloseDisc.
 * @param[in] pJobID[41] Job ID
 * @param[out] pCloseDisc the disc close setting
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pCloseDisc is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetCloseDisc(wchar_t pJobID[41] , unsigned long* pCloseDisc);

/** @brief Function to set FORMAT key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] ulWritingFormat the writing format. If this parameter is NULL, this function delete the key.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the condition below a. pJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetFormat(wchar_t pJobID[41] , unsigned long ulWritingFormat);

/** @brief Function to get FORMAT key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pWritingFormat.
 * @param[in] pJobID[41] Job ID
 * @param[out] pWritingFormat the writing format
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pWritingFormat is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetFormat(wchar_t pJobID[41] , unsigned long* pWritingFormat);

/** @brief Function to delete DATA key then set DATA key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] pWriteData[] the writing data
 * @param[in] ulWriteDataNum number of array pWriteData[]
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pWriteData[] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetData(wchar_t pJobID[41] , PP100_WRITE_DATA pWriteData[] , unsigned long ulWriteDataNum);

/** @brief Function to add DATA key to existing key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] pWriteData[] the writing data
 * @param[in] ulWriteDataNum number of array pWriteData[]
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pWriteData[] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_AddData(wchar_t pJobID[41] , PP100_WRITE_DATA pWriteData[] , unsigned long ulWriteDataNum);

/** @brief Function to delete DATA key of the job.Function confirms the existence of appointed data and deletes data when there is it and returns success when there is not it.
 * @param[in] pJobID[41] Job ID
 * @param[in] pWriteData[] the writing data
 * @param[in] ulWriteDataNum number of array pWriteData[]
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pWriteData[] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_DeleteData(wchar_t pJobID[41] , PP100_WRITE_DATA pWriteData[] , unsigned long ulWriteDataNum);

/** @brief Function to get DATA key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pWriteData[].
 * @param[in] pJobID[41] Job ID
 * @param[out] pWriteData[] buffer for storing writing data
 * @param[in,out] pWriteDataNum number of array pWriteData[]
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pWriteData[] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_MORE_ITEMS If pWriteDataNum is insufficient or NULL, the data will not be copied and pWriteDataNum will be set to number of items
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetData(wchar_t pJobID[41] , PP100_WRITE_DATA pWriteData[] , unsigned long* pWriteDataNum);

/** @brief Function to set VOLUME_LABEL key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] pVolumeLabel[63] the volume label. If this parameter is NULL, this function delete the key.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the condition below a. pJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetVolumeLabel(wchar_t pJobID[41] , wchar_t pVolumeLabel[63]);

/** @brief Function to get VOLUME_LABEL key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pVolumeLabel[63].
 * @param[in] pJobID[41] Job ID
 * @param[out] pVolumeLabel[63] the volume label
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pVolumeLabel[63] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetVolumeLabel(wchar_t pJobID[41] , wchar_t pVolumeLabel[63]);

/** @brief Function to set VIDEO key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] pVideoPath[256] the path to video file or VIDEO_TS folder. If this parameter is NULL, this function delete the key.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the condition below a. pJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetVideo(wchar_t pJobID[41] , wchar_t pVideoPath[256]);

/** @brief Function to get VIDEO key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pVideoPath[256].
 * @param[in] pJobID[41] Job ID
 * @param[out] pVideoPath[256] the path to video file or VIDEO_TS folder
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pVideoPath[256] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetVideo(wchar_t pJobID[41] , wchar_t pVideoPath[256]);

/** @brief Function to set VIDEO_TITLE key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] pVideoTitle[33] the video title. If this parameter is NULL, this function delete the key.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the condition below a. pJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetVideoTitle(wchar_t pJobID[41] , wchar_t pVideoTitle[33]);

/** @brief Function to get VIDEO_TITLE key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pVideoTitle[33].
 * @param[in] pJobID[41] Job ID
 * @param[out] pVideoTitle[33] the video title
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pVideoTitle[33] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetVideoTitle(wchar_t pJobID[41] , wchar_t pVideoTitle[33]);

/** @brief Function to set IMAGE key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] pImage[256] the image file. If this parameter is NULL, this function delete the key.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parametermatches one of the condition below a. pJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetImage(wchar_t pJobID[41] , wchar_t pImage[256]);

/** @brief Function to get IMAGE key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pImage[256].
 * @param[in] pJobID[41] Job ID
 * @param[out] pImage[256] the image file
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pImage[256] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetImage(wchar_t pJobID[41] , wchar_t pImage[256]);

/** @brief Function to delete AUDIO_TRACK key then set AUDIO_TRACK key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] pAudioTrack[] the audio track
 * @param[in] ulAudioTrackNum number of array pAudioTrack[]
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pAudioTrack[] is NULL
 * @retval PP100API_JOB_NOT_CREATED The job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetAudioTrack(wchar_t pJobID[41] , PP100_AUDIO_TRACK pAudioTrack[] , unsigned long ulAudioTrackNum);

/** @brief Function to add AUDIO_TRACK key to existing key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] pAudioTrack[] the audio track
 * @param[in] ulAudioTrackNum number of array pAudioTrack[]
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pAudioTrack[] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_AddAudioTrack(wchar_t pJobID[41] , PP100_AUDIO_TRACK pAudioTrack[] , unsigned long ulAudioTrackNum);

/** @brief Function to delete AUDIO_TRACK key of the job.Function confirms the existence of appointed data and deletes data when there is it and returns success when there is not it.
 * @param[in] pJobID[41] Job ID
 * @param[in] pAudioTrack[] the audio track
 * @param[in] ulAudioTrackNum number of array pAudioTrack[]
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pAudioTrack[] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_DeleteAudioTrack(wchar_t pJobID[41] , PP100_AUDIO_TRACK pAudioTrack[] , unsigned long ulAudioTrackNum);

/** @brief Function to get AUDIO_TRACK key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pAudioTrack[].
 * @param[in] pJobID[41] Job ID
 * @param[out] pAudioTrack[] buffer for storing audio track
 * @param[in,out] pAudioTrackNum number of array pAudioTrack[]
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pAudioTrack[] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_MORE_ITEMS If pAudioTrackNum is insufficient or NULL, the data will not be copied and pAudioTrackNum will be set to number of items
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetAudioTrack(wchar_t pJobID[41] , PP100_AUDIO_TRACK pAudioTrack[] , unsigned long* pAudioTrackNum);

/** @brief Function to set AUDIO_TITLE key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] pAudioTitle[129] the album title. If this parameter is NULL, this function delete the key.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the condition below a. pJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetAudioTitle(wchar_t pJobID[41] , wchar_t pAudioTitle[129]);

/** @brief Function to get AUDIO_TITLE key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pAudioTitle[129].
 * @param[in] pJobID[41] Job ID
 * @param[out] pAudioTitle[129] the album title
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pAudioTitle[129] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetAudioTitle(wchar_t pJobID[41] , wchar_t pAudioTitle[129]);

/** @brief Function to set AUDIO_CATALOG_CODE key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] pAudioCatalogCode[14] the catalog code. If this parameter is NULL, this function delete the key.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the condition below a. pJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetAudioCatalogCode(wchar_t pJobID[41] , wchar_t pAudioCatalogCode[14]);

/** @brief Function to get AUDIO_CATALOG_CODE key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pAudioCatalogCode[14].
 * @param[in] pJobID[41] Job ID
 * @param[out] pAudioCatalogCode[14] the catalog code
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pAudioCatalogCode[14] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetAudioCatalogCode(wchar_t pJobID[41] , wchar_t pAudioCatalogCode[14]);

/** @brief Function to set AUDIO_PERFORMER key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] pAudioPerformer[129] the artist name. If this parameter is NULL, this function delete the key.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the condition below a. pJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetAudioPerformer(wchar_t pJobID[41] , wchar_t pAudioPerformer[129]);

/** @brief Function to get AUDIO_PERFORMER key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pAudioPerformer[129].
 * @param[in] pJobID[41] Job ID
 * @param[out] pAudioPerformer[129] the artist name
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pAudioPerformer[129] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetAudioPerformer(wchar_t pJobID[41] , wchar_t pAudioPerformer[129]);

/** @brief Function to set LABEL key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] pLabel[256] the label file. If this parameter is NULL, this function delete the key.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the condition below a. pJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetLabel(wchar_t pJobID[41] , wchar_t pLabel[256]);

/** @brief Function to get LABEL key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pLabel[256].
 * @param[in] pJobID[41] Job ID
 * @param[out] pLabel[256] the label file
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pLabel[256] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetLabel(wchar_t pJobID[41] , wchar_t pLabel[256]);

/** @brief Function to set LABEL_TYPE key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] ulLabelType the label type. If this parameter is NULL, this function delete the key.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the condition below a. pJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetLabelType(wchar_t pJobID[41] , unsigned long ulLabelType);

/** @brief Function to get LABEL_TYPE key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pLabelType.
 * @param[in] pJobID[41] Job ID
 * @param[out] pLabelType the label type
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pLabelType is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetLabelType(wchar_t pJobID[41] , unsigned long* pLabelType);

/** @brief Function to delete REPLACE_FIELD key then set REPLACE_FIELD key of the job(text , barcode string)
 * @param[in] pJobID[41] Job ID
 * @param[in] pReplaceFieldText[] the array of replace field(text , barcode string)
 * @param[in] ulReplaceFieldTextNum number of array pReplaceFieldText[]
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pReplaceFieldText[] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetReplaceFieldText(wchar_t pJobID[41] , PP100_REPLACE_FIELD_TEXT pReplaceFieldText[] , unsigned long ulReplaceFieldTextNum);

/** @brief Function to add REPLACE_FIELD key to existing key of the job(text , barcode string)
 * @param[in] pJobID[41] Job ID
 * @param[in] pReplaceFieldText[] the array of replace field(text , barcode string)
 * @param[in] ulReplaceFieldTextNum number of array pReplaceFieldText[]
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pReplaceFieldText[] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_AddReplaceFieldText(wchar_t pJobID[41] , PP100_REPLACE_FIELD_TEXT pReplaceFieldText[] , unsigned long ulReplaceFieldTextNum);

/** @brief Function to delete REPLACE_FIELD key of the job.Function confirms the existence of appointed data and deletes data when there is it and returns success when there is not it.(text , barcode string)
 * @param[in] pJobID[41] Job ID
 * @param[in] pReplaceFieldText[] the array of replace field(text , barcode string)
 * @param[in] ulReplaceFieldTextNum number of array pReplaceFieldText[]
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pReplaceFieldText[] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_DeleteReplaceFieldText(wchar_t pJobID[41] , PP100_REPLACE_FIELD_TEXT pReplaceFieldText[] , unsigned long ulReplaceFieldTextNum);

/** @brief Function to get REPLACE_FIELD key of the job(text , barcode string). When the return value is only PP100API_SUCCESS , the value is stored in pReplaceFieldText[].
 * @param[in] pJobID[41] Job ID
 * @param[out] pReplaceFieldText[] the array of replace field(text , barcode string)
 * @param[in,out] pReplaceFieldTextNum number of array pReplaceFieldText[]
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pReplaceFieldText[] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_MORE_ITEMS If pReplaceFieldTextNum is insufficient or NULL, the data will not be copied and pReplaceFieldTextNum will be set to number of items
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetReplaceFieldText(wchar_t pJobID[41] , PP100_REPLACE_FIELD_TEXT pReplaceFieldText[] , unsigned long* pReplaceFieldTextNum);

/** @brief Function to delete REPLACE_FIELD key then set REPLACE_FIELD key of the job(binary file path)
 * @param[in] pJobID[41] Job ID
 * @param[in] pReplaceFieldBinary[] the array of replace field(binary file path)
 * @param[in] ulReplaceFieldBinaryNum number of array pReplaceFieldBinary[]
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pReplaceFieldBinary[] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetReplaceFieldBinary(wchar_t pJobID[41] , PP100_REPLACE_FIELD_BINARY pReplaceFieldBinary[] , unsigned long ulReplaceFieldBinaryNum);

/** @brief Function to add REPLACE_FIELD key to existing key of the job(binary file path)
 * @param[in] pJobID[41] Job ID
 * @param[in] pReplaceFieldBinary[] the array of replace field(binary file path)
 * @param[in] ulReplaceFieldBinaryNum number of array pReplaceFieldBinary[]
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pReplaceFieldBinary[] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_AddReplaceFieldBinary(wchar_t pJobID[41] , PP100_REPLACE_FIELD_BINARY pReplaceFieldBinary[] , unsigned long ulReplaceFieldBinaryNum);

/** @brief Function to delete REPLACE_FIELD key of the job.Function confirms the existence of appointed data and deletes data when there is it and returns success when there is not it.(barcode binary file path)
 * @param[in] pJobID[41] Job ID
 * @param[in] pReplaceFieldBinary[] the array of replace field(file path)
 * @param[in] ulReplaceFieldBinaryNum number of array pReplaceFieldBinary[]
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pReplaceFieldBinary[] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_DeleteReplaceFieldBinary(wchar_t pJobID[41] , PP100_REPLACE_FIELD_BINARY pReplaceFieldBinary[] , unsigned long ulReplaceFieldBinaryNum);

/** @brief Function to get REPLACE_FIELD key of the job(binary file path). When the return value is only PP100API_SUCCESS , the value is stored in pReplaceFieldBinary[].
 * @param[in] pJobID[41] Job ID
 * @param[out] pReplaceFieldBinary[] the array of replace field(binary file path)
 * @param[in,out] pReplaceFieldBinaryNum number of array pReplaceFieldBinary[]
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pReplaceFieldBinary[] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_MORE_ITEMS If pReplaceFieldBinaryNum is insufficient or NULL, the data will not be copied and pReplaceFieldBinaryNum will be set to number of items
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetReplaceFieldBinary(wchar_t pJobID[41] , PP100_REPLACE_FIELD_BINARY pReplaceFieldBinary[] , unsigned long* pReplaceFieldBinaryNum);

/** @brief Function to set LABEL_AREA key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] ulDiscDiamOut the outside diameter(If you don't set, this value is 0.If you set, this value is by a tenth of a millimeter from 700 to 1194)
 * @param[in] ulDiscDiamIn the inside diameter(If you don't set, this value is 0.If you set, this value is by a tenth of a millimeter from 180 to 500)
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the condition below a. pJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetLabelArea(wchar_t pJobID[41] , unsigned long ulDiscDiamOut , unsigned long ulDiscDiamIn);

/** @brief Function to get LABEL_AREA key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pDiscDiamOut and pDiscDiamIn.
 * @param[in] pJobID[41] Job ID
 * @param[out] pDiscDiamOut the outside diameter
 * @param[out] pDiscDiamIn the inside diameter
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pDiscDiamOut is NULL , c.pDiscDiamIn is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetLabelArea(wchar_t pJobID[41] , unsigned long* pDiscDiamOut , unsigned long* pDiscDiamIn);

/** @brief Function to set PRINT_MODE key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] ulPrintMode the print mode. If this parameter is NULL, this function delete the key.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the condition below a. pJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetPrintMode(wchar_t pJobID[41] , unsigned long ulPrintMode);

/** @brief Function to get PRINT_MODE key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pPrintMode.
 * @param[in] pJobID[41] Job ID
 * @param[out] pPrintMode the label type
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pPrintMode is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetPrintMode(wchar_t pJobID[41] , unsigned long* pPrintMode);

/** @brief Function to set PLUG_IN_ID key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] pPlugInID[41] Plugin ID. If this parameter is NULL, this function delete the key.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the condition below a. pJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetPlugInID(wchar_t pJobID[41] , wchar_t pPlugInID[41]);

/** @brief Function to get PLUG_IN_ID key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pPlugInID[41].
 * @param[in] pJobID[41] Job ID
 * @param[out] pPlugInID[41] Plugin ID
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pPlugInID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetPlugInID(wchar_t pJobID[41] , wchar_t pPlugInID[41]);

/** @brief Function to set PLUG_IN_PARAMETER key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] pPlugInParameter[1025] plugin parameter. If this parameter is NULL, this function delete the key.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the condition below a. pJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetPlugInParameter(wchar_t pJobID[41] , wchar_t pPlugInParameter[1025]);

/** @brief Function to get PLUG_IN_PARAMETER key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pPlugInParameter[1025].
 * @param[in] pJobID[41] Job ID
 * @param[out] pPlugInParameter[1025] plugin parameter
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pPlugInParameter[1025] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetPlugInParameter(wchar_t pJobID[41] , wchar_t pPlugInParameter[1025]);

/** @brief Function to set PRIORITY key of the job
 * @param[in] pJobID[41] Job ID
 * @param[in] ulPriority priority. If this parameter is NULL, this function delete the key.
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the condition below a. pJobID[41] is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_SetPriority(wchar_t pJobID[41] , unsigned long ulPriority);

/** @brief Function to get PRIORITY key of the job. When the return value is only PP100API_SUCCESS , the value is stored in pPriority.
 * @param[in] pJobID[41] Job ID
 * @param[out] pPriority priority
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_NOT_INITIALIZED if the function is called before calling PP100_Initialize()
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. pJobID[41] is NULL , b. pPriority is NULL
 * @retval PP100API_JOB_NOT_CREATED If the job specified pJobID[41] is not created
 * @retval PP100API_JOB_SETTING_NOT_SET if the key value is not set
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_GetPriority(wchar_t pJobID[41] , unsigned long* pPriority);

/** @brief Function to get string of the error code. When the return value is only PP100API_SUCCESS , the value is stored in pErrorString[1025].
 * @param[in] ulErrorCode ErrorCode
 * @param[out] pErrorString[1025] the string of error code
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. ulErrorCode is NULL
 * @retval PP100API_ERROR_CODE_NOT_DEFINED If the error code specified ulErrorCode is not defined
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_ConvertErrorCodeToString(unsigned long ulErrorCode , wchar_t pErrorString[1025]);

/** @brief Function to get String of the information code. When the return value is only PP100API_SUCCESS , the value is stored in pInformationString[1025].
 * @param[in] ulInformationCode InformationCode
 * @param[out] pInformationString[1025] the string of information code
 * @retval PP100API_SUCCESS if successful
 * @retval PP100API_INVALID_PARAMETER If parameter matches one of the conditions below a. ulInformationCode is NULL
 * @retval PP100API_INFORMATION_CODE_NOT_DEFINED If the error code specified ulInformationCode is not defined
 * @retval PP100API_FAILURE if failure
 */
PP100_API long _stdcall PP100_ConvertInformationCodeToString(unsigned long ulInformationCode , wchar_t pInformationString[1025]);

/** @} */ // end of function
/** @brief typedef for PP100_Initialize() function */
typedef long (_stdcall *PP100_INITIALIZE)(void);

/** @brief typedef for PP100_Destroy() function */
typedef long (_stdcall *PP100_DESTROY)(void);

/** @brief typedef for PP100_ConnectServer() function */
typedef long (_stdcall *PP100_CONNECTSERVER)(wchar_t pHost[64] , wchar_t pOrderFolder[256] , unsigned long* pHandle);

/** @brief typedef for PP100_DisConnectServer() function */
typedef long (_stdcall *PP100_DISCONNECTSERVER)(unsigned long ulHandle);

/** @brief typedef for PP100_EnumPublishers() function */
typedef long (_stdcall *PP100_ENUMPUBLISHERS)(unsigned long ulHandle , PP100_ENUM_PUBLISHER_INFO pEnumPublisherInfo[] , unsigned long* pEnumPubilsherInfoNum);

/** @brief typedef for PP100_GetPublisherStatus() function */
typedef long (_stdcall *PP100_GETPUBLISHERSTATUS)(unsigned long ulHandle , wchar_t pPublisherName[65] , LPPP100_PUBLISHER_STATUS pPublisherStatus);

/** @brief typedef for PP100_GetJobStatus() function */
typedef long (_stdcall *PP100_GETJOBSTATUS)(unsigned long ulHandle , wchar_t pJobID[41] , PP100_JOB_STATUS pJobStatus[] , unsigned long* pJobStatusNum);

/** @brief typedef for PP100_CreateJob() function */
typedef long (_stdcall *PP100_CREATEJOB)(wchar_t pJobID[41]);

/** @brief typedef for PP100_GetCreatedJobList() function */
typedef long (_stdcall *PP100_GETCREATEDJOBLIST)(wchar_t** ppJobID , unsigned long* pJobIDNum);

/** @brief typedef for PP100_CopyJob() function */
typedef long (_stdcall *PP100_COPYJOB)(wchar_t pSourceJobID[41] , wchar_t pDestinationJobID[41]);

/** @brief typedef for PP100_DeleteJob() function */
typedef long (_stdcall *PP100_DELETEJOB)(wchar_t pJobID[41]);

/** @brief typedef for PP100_SubmitJob() function */
typedef long (_stdcall *PP100_SUBMITJOB)(unsigned long ulHandle , wchar_t pJobID[41] , bool bAutoDelete);

/** @brief typedef for PP100_CancelJob() function */
typedef long (_stdcall *PP100_CANCELJOB)(unsigned long ulHandle , wchar_t pJobID[41]);

/** @brief typedef for PP100_SetPublisher() function */
typedef long (_stdcall *PP100_SETPUBLISHER)(wchar_t pJobID[41] , wchar_t pPublisher[65]);

/** @brief typedef for PP100_GetPublisher() function */
typedef long (_stdcall *PP100_GETPUBLISHER)(wchar_t pJobID[41] , wchar_t pPublisher[65]);

/** @brief typedef for PP100_SetCopies() function */
typedef long (_stdcall *PP100_SETCOPIES)(wchar_t pJobID[41] , unsigned long ulNumber);

/** @brief typedef for PP100_GetCopies() function */
typedef long (_stdcall *PP100_GETCOPIES)(wchar_t pJobID[41] , unsigned long* pNumber);

/** @brief typedef for PP100_SetOutStacker() function */
typedef long (_stdcall *PP100_SETOUTSTACKER)(wchar_t pJobID[41] , unsigned long ulOutStacker);

/** @brief typedef for PP100_GetOutStacker() function */
typedef long (_stdcall *PP100_GETOUTSTACKER)(wchar_t pJobID[41] , unsigned long* pOutStacker);

/** @brief typedef for PP100_SetInStacker() function */
typedef long (_stdcall *PP100_SETINSTACKER)(wchar_t pJobID[41] , unsigned long ulInStacker);

/** @brief typedef for PP100_GetInStacker() function */
typedef long (_stdcall *PP100_GETINSTACKER)(wchar_t pJobID[41] , unsigned long* pInStacker);

/** @brief typedef for PP100_SetDiscType() function */
typedef long (_stdcall *PP100_SETDISCTYPE)(wchar_t pJobID[41] , unsigned long ulDiscType);

/** @brief typedef for PP100_GetDiscType() function */
typedef long (_stdcall *PP100_GETDISCTYPE)(wchar_t pJobID[41] , unsigned long* pDiscType);

/** @brief typedef for PP100_SetWritingSpeed() function */
typedef long (_stdcall *PP100_SETWRITINGSPEED)(wchar_t pJobID[41] , float fWritingSpeed);

/** @brief typedef for PP100_GetWritingSpeed() function */
typedef long (_stdcall *PP100_GETWRITINGSPEED)(wchar_t pJobID[41] , float* pWritingSpeed);

/** @brief typedef for PP100_SetCompare() function */
typedef long (_stdcall *PP100_SETCOMPARE)(wchar_t pJobID[41] , unsigned long ulCompare);

/** @brief typedef for PP100_GetCompare() function */
typedef long (_stdcall *PP100_GETCOMPARE)(wchar_t pJobID[41] , unsigned long* pCompare);

/** @brief typedef for PP100_SetMeasure() function */
typedef long (_stdcall *PP100_SETMEASURE)(wchar_t pJobID[41] , unsigned long ulMeasure);

/** @brief typedef for PP100_GetMeasure() function */
typedef long (_stdcall *PP100_GETMEASURE)(wchar_t pJobID[41] , unsigned long* pMeasure);

/** @brief typedef for PP100_SetArchiveDiscOnly() function */
typedef long (_stdcall *PP100_SETARCHIVEDISCONLY)(wchar_t pJobID[41] , unsigned long ulArchiveDiscOnly);

/** @brief typedef for PP100_GetArchiveDiscOnly() function */
typedef long (_stdcall *PP100_GETARCHIVEDISCONLY)(wchar_t pJobID[41] , unsigned long* pArchiveDiscOnly);

/** @brief typedef for PP100_SetCloseDisc() function */
typedef long (_stdcall *PP100_SETCLOSEDISC)(wchar_t pJobID[41] , unsigned long ulCloseDisc);

/** @brief typedef for PP100_GetCloseDisc() function */
typedef long (_stdcall *PP100_GETCLOSEDISC)(wchar_t pJobID[41] , unsigned long* pCloseDisc);

/** @brief typedef for PP100_SetFormat() function */
typedef long (_stdcall *PP100_SETFORMAT)(wchar_t pJobID[41] , unsigned long ulWritingFormat);

/** @brief typedef for PP100_GetFormat() function */
typedef long (_stdcall *PP100_GETFORMAT)(wchar_t pJobID[41] , unsigned long* pWritingFormat);

/** @brief typedef for PP100_SetData() function */
typedef long (_stdcall *PP100_SETDATA)(wchar_t pJobID[41] , PP100_WRITE_DATA pWriteData[] , unsigned long ulWriteDataNum);

/** @brief typedef for PP100_AddData() function */
typedef long (_stdcall *PP100_ADDDATA)(wchar_t pJobID[41] , PP100_WRITE_DATA pWriteData[] , unsigned long ulWriteDataNum);

/** @brief typedef for PP100_DeleteData() function */
typedef long (_stdcall *PP100_DELETEDATA)(wchar_t pJobID[41] , PP100_WRITE_DATA pWriteData[] , unsigned long ulWriteDataNum);

/** @brief typedef for PP100_GetData() function */
typedef long (_stdcall *PP100_GETDATA)(wchar_t pJobID[41] , PP100_WRITE_DATA pWriteData[] , unsigned long* pWriteDataNum);

/** @brief typedef for PP100_SetVolumeLabel() function */
typedef long (_stdcall *PP100_SETVOLUMELABEL)(wchar_t pJobID[41] , wchar_t pVolumeLabel[63]);

/** @brief typedef for PP100_GetVolumeLabel() function */
typedef long (_stdcall *PP100_GETVOLUMELABEL)(wchar_t pJobID[41] , wchar_t pVolumeLabel[63]);

/** @brief typedef for PP100_SetVideo() function */
typedef long (_stdcall *PP100_SETVIDEO)(wchar_t pJobID[41] , wchar_t pVideoPath[256]);

/** @brief typedef for PP100_GetVideo() function */
typedef long (_stdcall *PP100_GETVIDEO)(wchar_t pJobID[41] , wchar_t pVideoPath[256]);

/** @brief typedef for PP100_SetVideoTitle() function */
typedef long (_stdcall *PP100_SETVIDEOTITLE)(wchar_t pJobID[41] , wchar_t pVideoTitle[33]);

/** @brief typedef for PP100_GetVideoTitle() function */
typedef long (_stdcall *PP100_GETVIDEOTITLE)(wchar_t pJobID[41] , wchar_t pVideoTitle[33]);

/** @brief typedef for PP100_SetImage() function */
typedef long (_stdcall *PP100_SETIMAGE)(wchar_t pJobID[41] , wchar_t pImage[256]);

/** @brief typedef for PP100_GetImage() function */
typedef long (_stdcall *PP100_GETIMAGE)(wchar_t pJobID[41] , wchar_t pImage[256]);

/** @brief typedef for PP100_SetAudioTrack() function */
typedef long (_stdcall *PP100_SETAUDIOTRACK)(wchar_t pJobID[41] , PP100_AUDIO_TRACK pAudioTrack[] , unsigned long ulAudioTrackNum);

/** @brief typedef for PP100_AddAudioTrack() function */
typedef long (_stdcall *PP100_ADDAUDIOTRACK)(wchar_t pJobID[41] , PP100_AUDIO_TRACK pAudioTrack[] , unsigned long ulAudioTrackNum);

/** @brief typedef for PP100_DeleteAudioTrack() function */
typedef long (_stdcall *PP100_DELETEAUDIOTRACK)(wchar_t pJobID[41] , PP100_AUDIO_TRACK pAudioTrack[] , unsigned long ulAudioTrackNum);

/** @brief typedef for PP100_GetAudioTrack() function */
typedef long (_stdcall *PP100_GETAUDIOTRACK)(wchar_t pJobID[41] , PP100_AUDIO_TRACK pAudioTrack[] , unsigned long* pAudioTrackNum);

/** @brief typedef for PP100_SetAudioTitle() function */
typedef long (_stdcall *PP100_SETAUDIOTITLE)(wchar_t pJobID[41] , wchar_t pAudioTitle[129]);

/** @brief typedef for PP100_GetAudioTitle() function */
typedef long (_stdcall *PP100_GETAUDIOTITLE)(wchar_t pJobID[41] , wchar_t pAudioTitle[129]);

/** @brief typedef for PP100_SetAudioCatalogCode() function */
typedef long (_stdcall *PP100_SETAUDIOCATALOGCODE)(wchar_t pJobID[41] , wchar_t pAudioCatalogCode[14]);

/** @brief typedef for PP100_GetAudioCatalogCode() function */
typedef long (_stdcall *PP100_GETAUDIOCATALOGCODE)(wchar_t pJobID[41] , wchar_t pAudioCatalogCode[14]);

/** @brief typedef for PP100_SetAudioPerformer() function */
typedef long (_stdcall *PP100_SETAUDIOPERFORMER)(wchar_t pJobID[41] , wchar_t pAudioPerformer[129]);

/** @brief typedef for PP100_GetAudioPerformer() function */
typedef long (_stdcall *PP100_GETAUDIOPERFORMER)(wchar_t pJobID[41] , wchar_t pAudioPerformer[129]);

/** @brief typedef for PP100_SetLabel() function */
typedef long (_stdcall *PP100_SETLABEL)(wchar_t pJobID[41] , wchar_t pLabel[256]);

/** @brief typedef for PP100_GetLabel() function */
typedef long (_stdcall *PP100_GETLABEL)(wchar_t pJobID[41] , wchar_t pLabel[256]);

/** @brief typedef for PP100_SetLabelType() function */
typedef long (_stdcall *PP100_SETLABELTYPE)(wchar_t pJobID[41] , unsigned long ulLabelType);

/** @brief typedef for PP100_GetLabelType() function */
typedef long (_stdcall *PP100_GETLABELTYPE)(wchar_t pJobID[41] , unsigned long* pLabelType);

/** @brief typedef for PP100_SetReplaceFieldText() function */
typedef long (_stdcall *PP100_SETREPLACEFIELDTEXT)(wchar_t pJobID[41] , PP100_REPLACE_FIELD_TEXT pReplaceFieldText[] , unsigned long ulReplaceFieldTextNum);

/** @brief typedef for PP100_AddReplaceFieldText() function */
typedef long (_stdcall *PP100_ADDREPLACEFIELDTEXT)(wchar_t pJobID[41] , PP100_REPLACE_FIELD_TEXT pReplaceFieldText[] , unsigned long ulReplaceFieldTextNum);

/** @brief typedef for PP100_DeleteReplaceFieldText() function */
typedef long (_stdcall *PP100_DELETEREPLACEFIELDTEXT)(wchar_t pJobID[41] , PP100_REPLACE_FIELD_TEXT pReplaceFieldText[] , unsigned long ulReplaceFieldTextNum);

/** @brief typedef for PP100_GetReplaceFieldText() function */
typedef long (_stdcall *PP100_GETREPLACEFIELDTEXT)(wchar_t pJobID[41] , PP100_REPLACE_FIELD_TEXT pReplaceFieldText[] , unsigned long* pReplaceFieldTextNum);

/** @brief typedef for PP100_SetReplaceFieldBinary() function */
typedef long (_stdcall *PP100_SETREPLACEFIELDBINARY)(wchar_t pJobID[41] , PP100_REPLACE_FIELD_BINARY pReplaceFieldBinary[] , unsigned long ulReplaceFieldBinaryNum);

/** @brief typedef for PP100_AddReplaceFieldBinary() function */
typedef long (_stdcall *PP100_ADDREPLACEFIELDBINARY)(wchar_t pJobID[41] , PP100_REPLACE_FIELD_BINARY pReplaceFieldBinary[] , unsigned long ulReplaceFieldBinaryNum);

/** @brief typedef for PP100_DeleteReplaceFieldBinary() function */
typedef long (_stdcall *PP100_DELETEREPLACEFIELDBINARY)(wchar_t pJobID[41] , PP100_REPLACE_FIELD_BINARY pReplaceFieldBinary[] , unsigned long ulReplaceFieldBinaryNum);

/** @brief typedef for PP100_GetReplaceFieldBinary() function */
typedef long (_stdcall *PP100_GETREPLACEFIELDBINARY)(wchar_t pJobID[41] , PP100_REPLACE_FIELD_BINARY pReplaceFieldBinary[] , unsigned long* pReplaceFieldBinaryNum);

/** @brief typedef for PP100_SetLabelArea() function */
typedef long (_stdcall *PP100_SETLABELAREA)(wchar_t pJobID[41] , unsigned long ulDiscDiamOut , unsigned long ulDiscDiamIn);

/** @brief typedef for PP100_GetLabelArea() function */
typedef long (_stdcall *PP100_GETLABELAREA)(wchar_t pJobID[41] , unsigned long* pDiscDiamOut , unsigned long* pDiscDiamIn);

/** @brief typedef for PP100_SetPrintMode() function */
typedef long (_stdcall *PP100_SETPRINTMODE)(wchar_t pJobID[41] , unsigned long ulPrintMode);

/** @brief typedef for PP100_GetPrintMode() function */
typedef long (_stdcall *PP100_GETPRINTMODE)(wchar_t pJobID[41] , unsigned long* pPrintMode);

/** @brief typedef for PP100_SetPlugInID() function */
typedef long (_stdcall *PP100_SETPLUGINID)(wchar_t pJobID[41] , wchar_t pPlugInID[41]);

/** @brief typedef for PP100_GetPlugInID() function */
typedef long (_stdcall *PP100_GETPLUGINID)(wchar_t pJobID[41] , wchar_t pPlugInID[41]);

/** @brief typedef for PP100_SetPlugInParameter() function */
typedef long (_stdcall *PP100_SETPLUGINPARAMETER)(wchar_t pJobID[41] , wchar_t pPlugInParameter[1025]);

/** @brief typedef for PP100_GetPlugInParameter() function */
typedef long (_stdcall *PP100_GETPLUGINPARAMETER)(wchar_t pJobID[41] , wchar_t pPlugInParameter[1025]);

/** @brief typedef for PP100_SetPriority() function */
typedef long (_stdcall *PP100_SETPRIORITY)(wchar_t pJobID[41] , unsigned long ulPriority);

/** @brief typedef for PP100_GetPriority() function */
typedef long (_stdcall *PP100_GETPRIORITY)(wchar_t pJobID[41] , unsigned long* pPriority);

/** @brief typedef for PP100_ConvertErrorCodeToString() function */
typedef long (_stdcall *PP100_CONVERTERRORCODETOSTRING)(unsigned long ulErrorCode , wchar_t pErrorString[1025]);

/** @brief typedef for PP100_ConvertInformationCodeToString() function */
typedef long (_stdcall *PP100_CONVERTINFORMATIONCODETOSTRING)(unsigned long ulInformationCode , wchar_t pInformationString[1025]);

/** @def FNSTR_PP100_INITIALIZE
 * function name(PP100_Initialize()) for GetProcAddress function
 */
#define FNSTR_PP100_INITIALIZE "PP100_Initialize"

/** @def FNSTR_PP100_DESTROY
 * function name(PP100_Destroy()) for GetProcAddress function
 */
#define FNSTR_PP100_DESTROY "PP100_Destroy"

/** @def FNSTR_PP100_CONNECTSERVER
 * function name(PP100_ConnectServer()) for GetProcAddress function
 */
#define FNSTR_PP100_CONNECTSERVER "PP100_ConnectServer"

/** @def FNSTR_PP100_DISCONNECTSERVER
 * function name(PP100_DisConnectServer()) for GetProcAddress function
 */
#define FNSTR_PP100_DISCONNECTSERVER "PP100_DisConnectServer"

/** @def FNSTR_PP100_ENUMPUBLISHERS
 * function name(PP100_EnumPublishers()) for GetProcAddress function
 */
#define FNSTR_PP100_ENUMPUBLISHERS "PP100_EnumPublishers"

/** @def FNSTR_PP100_GETPUBLISHERSTATUS
 * function name(PP100_GetPublisherStatus()) for GetProcAddress function
 */
#define FNSTR_PP100_GETPUBLISHERSTATUS "PP100_GetPublisherStatus"

/** @def FNSTR_PP100_GETJOBSTATUS
 * function name(PP100_GetJobStatus()) for GetProcAddress function
 */
#define FNSTR_PP100_GETJOBSTATUS "PP100_GetJobStatus"

/** @def FNSTR_PP100_CREATEJOB
 * function name(PP100_CreateJob()) for GetProcAddress function
 */
#define FNSTR_PP100_CREATEJOB "PP100_CreateJob"

/** @def FNSTR_PP100_GETCREATEDJOBLIST
 * function name(PP100_GetCreatedJobList()) for GetProcAddress function
 */
#define FNSTR_PP100_GETCREATEDJOBLIST "PP100_GetCreatedJobList"

/** @def FNSTR_PP100_COPYJOB
 * function name(PP100_CopyJob()) for GetProcAddress function
 */
#define FNSTR_PP100_COPYJOB "PP100_CopyJob"

/** @def FNSTR_PP100_DELETEJOB
 * function name(PP100_DeleteJob()) for GetProcAddress function
 */
#define FNSTR_PP100_DELETEJOB "PP100_DeleteJob"

/** @def FNSTR_PP100_SUBMITJOB
 * function name(PP100_SubmitJob()) for GetProcAddress function
 */
#define FNSTR_PP100_SUBMITJOB "PP100_SubmitJob"

/** @def FNSTR_PP100_CANCELJOB
 * function name(PP100_CancelJob()) for GetProcAddress function
 */
#define FNSTR_PP100_CANCELJOB "PP100_CancelJob"

/** @def FNSTR_PP100_SETPUBLISHER
 * function name(PP100_SetPublisher()) for GetProcAddress function
 */
#define FNSTR_PP100_SETPUBLISHER "PP100_SetPublisher"

/** @def FNSTR_PP100_GETPUBLISHER
 * function name(PP100_GetPublisher()) for GetProcAddress function
 */
#define FNSTR_PP100_GETPUBLISHER "PP100_GetPublisher"

/** @def FNSTR_PP100_SETCOPIES
 * function name(PP100_SetCopies()) for GetProcAddress function
 */
#define FNSTR_PP100_SETCOPIES "PP100_SetCopies"

/** @def FNSTR_PP100_GETCOPIES
 * function name(PP100_GetCopies()) for GetProcAddress function
 */
#define FNSTR_PP100_GETCOPIES "PP100_GetCopies"

/** @def FNSTR_PP100_SETOUTSTACKER
 * function name(PP100_SetOutStacker()) for GetProcAddress function
 */
#define FNSTR_PP100_SETOUTSTACKER "PP100_SetOutStacker"

/** @def FNSTR_PP100_GETOUTSTACKER
 * function name(PP100_GetOutStacker()) for GetProcAddress function
 */
#define FNSTR_PP100_GETOUTSTACKER "PP100_GetOutStacker"

/** @def FNSTR_PP100_SETINSTACKER
 * function name(PP100_SetInStacker()) for GetProcAddress function
 */
#define FNSTR_PP100_SETINSTACKER "PP100_SetInStacker"

/** @def FNSTR_PP100_GETINSTACKER
 * function name(PP100_GetInStacker()) for GetProcAddress function
 */
#define FNSTR_PP100_GETINSTACKER "PP100_GetInStacker"

/** @def FNSTR_PP100_SETDISCTYPE
 * function name(PP100_SetDiscType()) for GetProcAddress function
 */
#define FNSTR_PP100_SETDISCTYPE "PP100_SetDiscType"

/** @def FNSTR_PP100_GETDISCTYPE
 * function name(PP100_GetDiscType()) for GetProcAddress function
 */
#define FNSTR_PP100_GETDISCTYPE "PP100_GetDiscType"

/** @def FNSTR_PP100_SETWRITINGSPEED
 * function name(PP100_SetWritingSpeed()) for GetProcAddress function
 */
#define FNSTR_PP100_SETWRITINGSPEED "PP100_SetWritingSpeed"

/** @def FNSTR_PP100_GETWRITINGSPEED
 * function name(PP100_GetWritingSpeed()) for GetProcAddress function
 */
#define FNSTR_PP100_GETWRITINGSPEED "PP100_GetWritingSpeed"

/** @def FNSTR_PP100_SETCOMPARE
 * function name(PP100_SetCompare()) for GetProcAddress function
 */
#define FNSTR_PP100_SETCOMPARE "PP100_SetCompare"

/** @def FNSTR_PP100_GETCOMPARE
 * function name(PP100_GetCompare()) for GetProcAddress function
 */
#define FNSTR_PP100_GETCOMPARE "PP100_GetCompare"

/** @def FNSTR_PP100_SETMEASURE
 * function name(PP100_SetMeasure()) for GetProcAddress function
 */
#define FNSTR_PP100_SETMEASURE "PP100_SetMeasure"

/** @def FNSTR_PP100_GETMEASURE
 * function name(PP100_GetMeasure()) for GetProcAddress function
 */
#define FNSTR_PP100_GETMEASURE "PP100_GetMeasure"

/** @def FNSTR_PP100_SETARCHIVEDISCONLY
 * function name(PP100_SetArchiveDiscOnly()) for GetProcAddress function
 */
#define FNSTR_PP100_SETARCHIVEDISCONLY "PP100_SetArchiveDiscOnly"

/** @def FNSTR_PP100_GETARCHIVEDISCONLY
 * function name(PP100_GetArchiveDiscOnly()) for GetProcAddress function
 */
#define FNSTR_PP100_GETARCHIVEDISCONLY "PP100_GetArchiveDiscOnly"

/** @def FNSTR_PP100_SETCLOSEDISC
 * function name(PP100_SetCloseDisc()) for GetProcAddress function
 */
#define FNSTR_PP100_SETCLOSEDISC "PP100_SetCloseDisc"

/** @def FNSTR_PP100_GETCLOSEDISC
 * function name(PP100_GetCloseDisc()) for GetProcAddress function
 */
#define FNSTR_PP100_GETCLOSEDISC "PP100_GetCloseDisc"

/** @def FNSTR_PP100_SETFORMAT
 * function name(PP100_SetFormat()) for GetProcAddress function
 */
#define FNSTR_PP100_SETFORMAT "PP100_SetFormat"

/** @def FNSTR_PP100_GETFORMAT
 * function name(PP100_GetFormat()) for GetProcAddress function
 */
#define FNSTR_PP100_GETFORMAT "PP100_GetFormat"

/** @def FNSTR_PP100_SETDATA
 * function name(PP100_SetData()) for GetProcAddress function
 */
#define FNSTR_PP100_SETDATA "PP100_SetData"

/** @def FNSTR_PP100_ADDDATA
 * function name(PP100_AddData()) for GetProcAddress function
 */
#define FNSTR_PP100_ADDDATA "PP100_AddData"

/** @def FNSTR_PP100_DELETEDATA
 * function name(PP100_DeleteData()) for GetProcAddress function
 */
#define FNSTR_PP100_DELETEDATA "PP100_DeleteData"

/** @def FNSTR_PP100_GETDATA
 * function name(PP100_GetData()) for GetProcAddress function
 */
#define FNSTR_PP100_GETDATA "PP100_GetData"

/** @def FNSTR_PP100_SETVOLUMELABEL
 * function name(PP100_SetVolumeLabel()) for GetProcAddress function
 */
#define FNSTR_PP100_SETVOLUMELABEL "PP100_SetVolumeLabel"

/** @def FNSTR_PP100_GETVOLUMELABEL
 * function name(PP100_GetVolumeLabel()) for GetProcAddress function
 */
#define FNSTR_PP100_GETVOLUMELABEL "PP100_GetVolumeLabel"

/** @def FNSTR_PP100_SETVIDEO
 * function name(PP100_SetVideo()) for GetProcAddress function
 */
#define FNSTR_PP100_SETVIDEO "PP100_SetVideo"

/** @def FNSTR_PP100_GETVIDEO
 * function name(PP100_GetVideo()) for GetProcAddress function
 */
#define FNSTR_PP100_GETVIDEO "PP100_GetVideo"

/** @def FNSTR_PP100_SETVIDEOTITLE
 * function name(PP100_SetVideoTitle()) for GetProcAddress function
 */
#define FNSTR_PP100_SETVIDEOTITLE "PP100_SetVideoTitle"

/** @def FNSTR_PP100_GETVIDEOTITLE
 * function name(PP100_GetVideoTitle()) for GetProcAddress function
 */
#define FNSTR_PP100_GETVIDEOTITLE "PP100_GetVideoTitle"

/** @def FNSTR_PP100_SETIMAGE
 * function name(PP100_SetImage()) for GetProcAddress function
 */
#define FNSTR_PP100_SETIMAGE "PP100_SetImage"

/** @def FNSTR_PP100_GETIMAGE
 * function name(PP100_GetImage()) for GetProcAddress function
 */
#define FNSTR_PP100_GETIMAGE "PP100_GetImage"

/** @def FNSTR_PP100_SETAUDIOTRACK
 * function name(PP100_SetAudioTrack()) for GetProcAddress function
 */
#define FNSTR_PP100_SETAUDIOTRACK "PP100_SetAudioTrack"

/** @def FNSTR_PP100_ADDAUDIOTRACK
 * function name(PP100_AddAudioTrack()) for GetProcAddress function
 */
#define FNSTR_PP100_ADDAUDIOTRACK "PP100_AddAudioTrack"

/** @def FNSTR_PP100_DELETEAUDIOTRACK
 * function name(PP100_DeleteAudioTrack()) for GetProcAddress function
 */
#define FNSTR_PP100_DELETEAUDIOTRACK "PP100_DeleteAudioTrack"

/** @def FNSTR_PP100_GETAUDIOTRACK
 * function name(PP100_GetAudioTrack()) for GetProcAddress function
 */
#define FNSTR_PP100_GETAUDIOTRACK "PP100_GetAudioTrack"

/** @def FNSTR_PP100_SETAUDIOTITLE
 * function name(PP100_SetAudioTitle()) for GetProcAddress function
 */
#define FNSTR_PP100_SETAUDIOTITLE "PP100_SetAudioTitle"

/** @def FNSTR_PP100_GETAUDIOTITLE
 * function name(PP100_GetAudioTitle()) for GetProcAddress function
 */
#define FNSTR_PP100_GETAUDIOTITLE "PP100_GetAudioTitle"

/** @def FNSTR_PP100_SETAUDIOCATALOGCODE
 * function name(PP100_SetAudioCatalogCode()) for GetProcAddress function
 */
#define FNSTR_PP100_SETAUDIOCATALOGCODE "PP100_SetAudioCatalogCode"

/** @def FNSTR_PP100_GETAUDIOCATALOGCODE
 * function name(PP100_GetAudioCatalogCode()) for GetProcAddress function
 */
#define FNSTR_PP100_GETAUDIOCATALOGCODE "PP100_GetAudioCatalogCode"

/** @def FNSTR_PP100_SETAUDIOPERFORMER
 * function name(PP100_SetAudioPerformer()) for GetProcAddress function
 */
#define FNSTR_PP100_SETAUDIOPERFORMER "PP100_SetAudioPerformer"

/** @def FNSTR_PP100_GETAUDIOPERFORMER
 * function name(PP100_GetAudioPerformer()) for GetProcAddress function
 */
#define FNSTR_PP100_GETAUDIOPERFORMER "PP100_GetAudioPerformer"

/** @def FNSTR_PP100_SETLABEL
 * function name(PP100_SetLabel()) for GetProcAddress function
 */
#define FNSTR_PP100_SETLABEL "PP100_SetLabel"

/** @def FNSTR_PP100_GETLABEL
 * function name(PP100_GetLabel()) for GetProcAddress function
 */
#define FNSTR_PP100_GETLABEL "PP100_GetLabel"

/** @def FNSTR_PP100_SETLABELTYPE
 * function name(PP100_SetLabelType()) for GetProcAddress function
 */
#define FNSTR_PP100_SETLABELTYPE "PP100_SetLabelType"

/** @def FNSTR_PP100_GETLABELTYPE
 * function name(PP100_GetLabelType()) for GetProcAddress function
 */
#define FNSTR_PP100_GETLABELTYPE "PP100_GetLabelType"

/** @def FNSTR_PP100_SETREPLACEFIELDTEXT
 * function name(PP100_SetReplaceFieldText()) for GetProcAddress function
 */
#define FNSTR_PP100_SETREPLACEFIELDTEXT "PP100_SetReplaceFieldText"

/** @def FNSTR_PP100_ADDREPLACEFIELDTEXT
 * function name(PP100_AddReplaceFieldText()) for GetProcAddress function
 */
#define FNSTR_PP100_ADDREPLACEFIELDTEXT "PP100_AddReplaceFieldText"

/** @def FNSTR_PP100_DELETEREPLACEFIELDTEXT
 * function name(PP100_DeleteReplaceFieldText()) for GetProcAddress function
 */
#define FNSTR_PP100_DELETEREPLACEFIELDTEXT "PP100_DeleteReplaceFieldText"

/** @def FNSTR_PP100_GETREPLACEFIELDTEXT
 * function name(PP100_GetReplaceFieldText()) for GetProcAddress function
 */
#define FNSTR_PP100_GETREPLACEFIELDTEXT "PP100_GetReplaceFieldText"

/** @def FNSTR_PP100_SETREPLACEFIELDBINARY
 * function name(PP100_SetReplaceFieldBinary()) for GetProcAddress function
 */
#define FNSTR_PP100_SETREPLACEFIELDBINARY "PP100_SetReplaceFieldBinary"

/** @def FNSTR_PP100_ADDREPLACEFIELDBINARY
 * function name(PP100_AddReplaceFieldBinary()) for GetProcAddress function
 */
#define FNSTR_PP100_ADDREPLACEFIELDBINARY "PP100_AddReplaceFieldBinary"

/** @def FNSTR_PP100_DELETEREPLACEFIELDBINARY
 * function name(PP100_DeleteReplaceFieldBinary()) for GetProcAddress function
 */
#define FNSTR_PP100_DELETEREPLACEFIELDBINARY "PP100_DeleteReplaceFieldBinary"

/** @def FNSTR_PP100_GETREPLACEFIELDBINARY
 * function name(PP100_GetReplaceFieldBinary()) for GetProcAddress function
 */
#define FNSTR_PP100_GETREPLACEFIELDBINARY "PP100_GetReplaceFieldBinary"

/** @def FNSTR_PP100_SETLABELAREA
 * function name(PP100_SetLabelArea()) for GetProcAddress function
 */
#define FNSTR_PP100_SETLABELAREA "PP100_SetLabelArea"

/** @def FNSTR_PP100_GETLABELAREA
 * function name(PP100_GetLabelArea()) for GetProcAddress function
 */
#define FNSTR_PP100_GETLABELAREA "PP100_GetLabelArea"

/** @def FNSTR_PP100_SETPRINTMODE
 * function name(PP100_SetPrintMode()) for GetProcAddress function
 */
#define FNSTR_PP100_SETPRINTMODE "PP100_SetPrintMode"

/** @def FNSTR_PP100_GETPRINTMODE
 * function name(PP100_GetPrintMode()) for GetProcAddress function
 */
#define FNSTR_PP100_GETPRINTMODE "PP100_GetPrintMode"

/** @def FNSTR_PP100_SETPLUGINID
 * function name(PP100_SetPlugInID()) for GetProcAddress function
 */
#define FNSTR_PP100_SETPLUGINID "PP100_SetPlugInID"

/** @def FNSTR_PP100_GETPLUGINID
 * function name(PP100_GetPlugInID()) for GetProcAddress function
 */
#define FNSTR_PP100_GETPLUGINID "PP100_GetPlugInID"

/** @def FNSTR_PP100_SETPLUGINPARAMETER
 * function name(PP100_SetPlugInParameter()) for GetProcAddress function
 */
#define FNSTR_PP100_SETPLUGINPARAMETER "PP100_SetPlugInParameter"

/** @def FNSTR_PP100_GETPLUGINPARAMETER
 * function name(PP100_GetPlugInParameter()) for GetProcAddress function
 */
#define FNSTR_PP100_GETPLUGINPARAMETER "PP100_GetPlugInParameter"

/** @def FNSTR_PP100_SETPRIORITY
 * function name(PP100_SetPriority()) for GetProcAddress function
 */
#define FNSTR_PP100_SETPRIORITY "PP100_SetPriority"

/** @def FNSTR_PP100_GETPRIORITY
 * function name(PP100_GetPriority()) for GetProcAddress function
 */
#define FNSTR_PP100_GETPRIORITY "PP100_GetPriority"

/** @def FNSTR_PP100_CONVERTERRORCODETOSTRING
 * function name(PP100_ConvertErrorCodeToString()) for GetProcAddress function
 */
#define FNSTR_PP100_CONVERTERRORCODETOSTRING "PP100_ConvertErrorCodeToString"

/** @def FNSTR_PP100_CONVERTINFORMATIONCODETOSTRING
 * function name(PP100_ConvertInformationCodeToString()) for GetProcAddress function
 */
#define FNSTR_PP100_CONVERTINFORMATIONCODETOSTRING "PP100_ConvertInformationCodeToString"

/** @} */ // end of function
#ifdef	__cplusplus
}
#endif	/* def __cplusplus	*/

#endif // _PP100API_H_
