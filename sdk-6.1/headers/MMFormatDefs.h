/******************************************************************************
 *
 * Copyright (c) 2002-2004 PalmSource, Inc. All rights reserved.
 *
 * File: MMFormatDefs.h
 *
 * Release: Palm OS 6.1
 *
 *****************************************************************************/

#ifndef _FORMAT_DEFS_H_
#define _FORMAT_DEFS_H_

#include <sys/endian.h>

#if defined(__cplusplus) && _SUPPORTS_NAMESPACE
namespace palmos {
namespace media {
#endif

#define P_FORMAT_MAX_KEY_LENGTH	64

enum formatFamily
{
	// do NOT use a value of 0, otherwise that format will look like
	// a generic version of the format type
	P_FORMAT_FAMILY_MPEG12 = 1,
	// P_FORMAT_FAMILY_MPEG2 deprecated; use _MPEG12
	P_FORMAT_FAMILY_MPEG4 = 3,
	P_FORMAT_FAMILY_ATRAC,
	P_FORMAT_FAMILY_H263,
	P_FORMAT_FAMILY_3GPP,
	P_FORMAT_FAMILY_AVI,
	P_FORMAT_FAMILY_QUICKTIME,		// QuickTime is a registered trademark of Apple Computer
	P_FORMAT_FAMILY_ASF,
	P_FORMAT_FAMILY_WAV,
	P_FORMAT_FAMILY_AIFF,
	P_FORMAT_FAMILY_JPEG,
	P_FORMAT_FAMILY_GIF,
	P_FORMAT_FAMILY_BMP,
	P_FORMAT_FAMILY_PNG,
	P_FORMAT_FAMILY_TIFF,
	P_FORMAT_FAMILY_RAW,
	P_FORMAT_FAMILY_YCBCR420,
	P_FORMAT_FAMILY_OGG,
	P_FORMAT_FAMILY_AMR,
	P_FORMAT_FAMILY_G7XX,
	P_FORMAT_FAMILY_H264,
	P_FORMAT_FAMILY_CINEPAK,

	P_FORMAT_FAMILY_USER = 0xfd,
	P_FORMAT_FAMILY_PRIVATE = 0xfe,
	P_FORMAT_FAMILY_PALMOS_INTERNAL = 0xff
};

// the low 8 bits of a valid format type describe a basic class of media data.
// 5 bits are currently used; 3 are to be left cleared for future type expansion.
// bits 8..15 describe the format family.  bits 16..23 describe a subtype if necessary
// to distinguish the format from others in the same family.  bits 24..31 are reserved
// for internal use by the framework and should be left cleared.

#define _MMFORMATTYPE(subtype, family, type) \
	((subtype << 16) | (family << 8) | (type))

enum formatType
{
	// invalid format type
	P_FORMAT_UNKNOWN =					0,

	// wildcard type
	P_FORMAT_ANY_TYPE =					0xff,
	
	// raw and encoded data types
	P_FORMAT_AUDIO_TYPE = 				0x01,
	P_FORMAT_VIDEO_TYPE = 				0x02,
	P_FORMAT_MIDI_TYPE = 				0x04,
	P_FORMAT_STILL_TYPE = 				0x08,

	P_FORMAT_RAW_AUDIO = 				_MMFORMATTYPE(0,	P_FORMAT_FAMILY_RAW,		P_FORMAT_AUDIO_TYPE),
	P_FORMAT_RAW_VIDEO =				_MMFORMATTYPE(0,	P_FORMAT_FAMILY_RAW,		P_FORMAT_VIDEO_TYPE),
	P_FORMAT_MIDI =						_MMFORMATTYPE(0,	P_FORMAT_FAMILY_RAW,		P_FORMAT_MIDI_TYPE),
	P_FORMAT_RAW_STILL =				_MMFORMATTYPE(0,	P_FORMAT_FAMILY_RAW,		P_FORMAT_STILL_TYPE),

	P_FORMAT_MPEG12_AUDIO =				_MMFORMATTYPE(0,	P_FORMAT_FAMILY_MPEG12,		P_FORMAT_AUDIO_TYPE),
	P_FORMAT_MPEG12_VIDEO =				_MMFORMATTYPE(0,	P_FORMAT_FAMILY_MPEG12,		P_FORMAT_VIDEO_TYPE),
	
	P_FORMAT_MPEG4_AUDIO_TF =			_MMFORMATTYPE(0x01,	P_FORMAT_FAMILY_MPEG4,		P_FORMAT_AUDIO_TYPE),
	P_FORMAT_MPEG4_AUDIO_CELP =			_MMFORMATTYPE(0x02, P_FORMAT_FAMILY_MPEG4,		P_FORMAT_AUDIO_TYPE),
	P_FORMAT_MPEG4_AUDIO_PARAMETRIC =	_MMFORMATTYPE(0x03, P_FORMAT_FAMILY_MPEG4,		P_FORMAT_AUDIO_TYPE),
	P_FORMAT_MPEG4_AUDIO_TTS =			_MMFORMATTYPE(0x04, P_FORMAT_FAMILY_MPEG4,		P_FORMAT_AUDIO_TYPE),
	P_FORMAT_MPEG4_AUDIO_STRUCTURED =	_MMFORMATTYPE(0x05, P_FORMAT_FAMILY_MPEG4,		P_FORMAT_AUDIO_TYPE),
	
	P_FORMAT_MPEG4_VIDEO =				_MMFORMATTYPE(0,	P_FORMAT_FAMILY_MPEG4,		P_FORMAT_VIDEO_TYPE),
	P_FORMAT_DIVX_4_VIDEO =				_MMFORMATTYPE(1,	P_FORMAT_FAMILY_MPEG4,		P_FORMAT_VIDEO_TYPE),
	P_FORMAT_DIVX_5_VIDEO =				_MMFORMATTYPE(2,	P_FORMAT_FAMILY_MPEG4,		P_FORMAT_VIDEO_TYPE),
	
	P_FORMAT_ATRAC_AUDIO =				_MMFORMATTYPE(0,	P_FORMAT_FAMILY_ATRAC,		P_FORMAT_AUDIO_TYPE),
	
	P_FORMAT_H263_VIDEO =				_MMFORMATTYPE(0,	P_FORMAT_FAMILY_H263,		P_FORMAT_VIDEO_TYPE),
	
	P_FORMAT_MSADPCM_AUDIO =			_MMFORMATTYPE(0x02,	P_FORMAT_FAMILY_WAV,		P_FORMAT_AUDIO_TYPE),
	P_FORMAT_DVI_ADPCM_AUDIO =			_MMFORMATTYPE(0x11,	P_FORMAT_FAMILY_WAV,		P_FORMAT_AUDIO_TYPE),
	
	P_FORMAT_JPEG_STILL =				_MMFORMATTYPE(0,	P_FORMAT_FAMILY_JPEG,		P_FORMAT_STILL_TYPE),
	P_FORMAT_BMP_STILL =				_MMFORMATTYPE(0,	P_FORMAT_FAMILY_BMP,		P_FORMAT_STILL_TYPE),
	P_FORMAT_PNG_STILL =				_MMFORMATTYPE(0,	P_FORMAT_FAMILY_PNG,		P_FORMAT_STILL_TYPE),
	P_FORMAT_TIFF_STILL =				_MMFORMATTYPE(0,	P_FORMAT_FAMILY_TIFF,		P_FORMAT_STILL_TYPE),
	
	P_FORMAT_YCBCR420_PLANAR_VIDEO =	_MMFORMATTYPE(0,	P_FORMAT_FAMILY_YCBCR420,	P_FORMAT_VIDEO_TYPE),
	
	P_FORMAT_OGG_VORBIS_AUDIO =			_MMFORMATTYPE(0x01,	P_FORMAT_FAMILY_OGG,		P_FORMAT_AUDIO_TYPE),
	
	P_FORMAT_AMR_TS26_071_AUDIO =		_MMFORMATTYPE(0x01,	P_FORMAT_FAMILY_AMR,		P_FORMAT_AUDIO_TYPE),
	P_FORMAT_AMR_TS26_171_AUDIO =		_MMFORMATTYPE(0x02,	P_FORMAT_FAMILY_AMR,		P_FORMAT_AUDIO_TYPE),
	
	P_FORMAT_G711_AUDIO =				_MMFORMATTYPE(0x01, P_FORMAT_FAMILY_G7XX,		P_FORMAT_AUDIO_TYPE),
	P_FORMAT_G722_AUDIO =				_MMFORMATTYPE(0x02, P_FORMAT_FAMILY_G7XX,		P_FORMAT_AUDIO_TYPE),
	P_FORMAT_G723_AUDIO =				_MMFORMATTYPE(0x03, P_FORMAT_FAMILY_G7XX,		P_FORMAT_AUDIO_TYPE),
	P_FORMAT_G723_1_AUDIO =				_MMFORMATTYPE(0x04, P_FORMAT_FAMILY_G7XX,		P_FORMAT_AUDIO_TYPE),
	P_FORMAT_G726_AUDIO =				_MMFORMATTYPE(0x05, P_FORMAT_FAMILY_G7XX,		P_FORMAT_AUDIO_TYPE),
	P_FORMAT_G728_AUDIO =				_MMFORMATTYPE(0x06, P_FORMAT_FAMILY_G7XX,		P_FORMAT_AUDIO_TYPE),
	P_FORMAT_G729_AUDIO =				_MMFORMATTYPE(0x07, P_FORMAT_FAMILY_G7XX,		P_FORMAT_AUDIO_TYPE),
	
	P_FORMAT_H264_VIDEO =				_MMFORMATTYPE(0,	P_FORMAT_FAMILY_H264,		P_FORMAT_VIDEO_TYPE),

	P_FORMAT_CINEPAK_VIDEO =			_MMFORMATTYPE(0,	P_FORMAT_FAMILY_CINEPAK,	P_FORMAT_VIDEO_TYPE),
	
	// interleaved or otherwise framed types (file formats, network streaming formats, etc.)
	P_FORMAT_STREAM_TYPE =				0x10,

	P_FORMAT_MPEG12_STREAM =			_MMFORMATTYPE(0,	P_FORMAT_FAMILY_MPEG12,		P_FORMAT_STREAM_TYPE),
	P_FORMAT_MPEG4_STREAM =				_MMFORMATTYPE(0, 	P_FORMAT_FAMILY_MPEG4,		P_FORMAT_STREAM_TYPE),
	P_FORMAT_AAC_ADTS_STREAM =			_MMFORMATTYPE(1,	P_FORMAT_FAMILY_MPEG4,		P_FORMAT_STREAM_TYPE),
	P_FORMAT_AAC_RAW_STREAM =			_MMFORMATTYPE(2,	P_FORMAT_FAMILY_MPEG4,		P_FORMAT_STREAM_TYPE),
	P_FORMAT_MQV_STREAM =				_MMFORMATTYPE(0x01,	P_FORMAT_FAMILY_MPEG4,		P_FORMAT_STREAM_TYPE),
	P_FORMAT_ATRAC_STREAM =				_MMFORMATTYPE(0,	P_FORMAT_FAMILY_ATRAC,		P_FORMAT_STREAM_TYPE),
	P_FORMAT_AVI_STREAM =				_MMFORMATTYPE(0,	P_FORMAT_FAMILY_AVI,		P_FORMAT_STREAM_TYPE),
	P_FORMAT_QUICKTIME_STREAM =			_MMFORMATTYPE(0,	P_FORMAT_FAMILY_QUICKTIME,	P_FORMAT_STREAM_TYPE),
	P_FORMAT_ASF_STREAM =				_MMFORMATTYPE(0,	P_FORMAT_FAMILY_ASF,		P_FORMAT_STREAM_TYPE),
	P_FORMAT_WAV_STREAM =				_MMFORMATTYPE(0,	P_FORMAT_FAMILY_WAV,		P_FORMAT_STREAM_TYPE),
	P_FORMAT_AIFF_STREAM =				_MMFORMATTYPE(0,	P_FORMAT_FAMILY_AIFF,		P_FORMAT_STREAM_TYPE),
	P_FORMAT_BMP_STREAM =				_MMFORMATTYPE(0,	P_FORMAT_FAMILY_BMP,		P_FORMAT_STREAM_TYPE),
	P_FORMAT_JPEG_STREAM =				_MMFORMATTYPE(0,	P_FORMAT_FAMILY_JPEG,		P_FORMAT_STREAM_TYPE),
	P_FORMAT_PNG_STREAM =				_MMFORMATTYPE(0,	P_FORMAT_FAMILY_PNG,		P_FORMAT_STREAM_TYPE),
	P_FORMAT_TIFF_STREAM =				_MMFORMATTYPE(0,	P_FORMAT_FAMILY_TIFF,		P_FORMAT_STREAM_TYPE),
	P_FORMAT_OGG_STREAM =				_MMFORMATTYPE(0,	P_FORMAT_FAMILY_OGG,		P_FORMAT_STREAM_TYPE)
};

// ---------------------------------------------------------------------------
// general keys

// P_FORMATKEY_SYNCHRONOUS			(int32_t: 0 or 1)
// - if the value is 1, indicates that buffers must be handled synchronously
// (ie. their data must be processed immediately or copied.)

#define P_FORMATKEY_SYNCHRONOUS		"sync"

// ---------------------------------------------------------------------------
// keys for P_FORMAT_RAW_AUDIO
//
// P_FORMATKEY_RAW_AUDIO_TYPE		(int32_t:fmtRawAudioType)
// P_FORMATKEY_CHANNEL_USAGE		(int32_t:fmtAudioChannelUsage)
// P_FORMATKEY_FRAME_RATE			(float)
// P_FORMATKEY_BUFFER_FRAMES		(int32_t)
//
// P_FORMATKEY_BYTE_ORDER			(int32_t:LITTLE_ENDIAN, BIG_ENDIAN, HOST_ENDIAN)
// only if raw_audio_type != P_AUDIO_INT8 && raw_audio_type != P_AUDIO_UINT8
//
// P_FORMATKEY_RAW_AUDIO_BITS		(int32_t)
// only if raw_audio_type == P_AUDIO_INT32

enum fmtRawAudioType
{
	P_AUDIO_INT8		= 0x01,
	P_AUDIO_UINT8		= 0x11,
	P_AUDIO_INT16		= 0x02,
	P_AUDIO_INT32		= 0x04,
	P_AUDIO_FLOAT		= 0x24,
	// use this to retrieve bytes-per-sample information for a raw audio type
	P_AUDIO_SIZE_MASK	= 0x0f
};

enum fmtAudioChannelUsage
{
	P_STEREO					= 0x02,
	P_MONO						= 0x01,
	P_DOLBY_PRO_LOGIC_STEREO	= 0x12,
	P_DOLBY_5_1_SURROUND		= 0x26,
	P_DTS_SURROUND				= 0x36,
	P_CHANNEL_COUNT_MASK		= 0x0f
};

#define P_FORMATKEY_BYTE_ORDER		"byte_order"
#define P_FORMATKEY_RAW_AUDIO_TYPE	"raw_audio_type"
#define P_FORMATKEY_RAW_AUDIO_BITS	"raw_audio_bits"
#define P_FORMATKEY_CHANNEL_USAGE	"channel_usage"
#define P_FORMATKEY_FRAME_RATE		"frame_rate"
#define P_FORMATKEY_BUFFER_FRAMES	"buffer_frames"

// ---------------------------------------------------------------------------
// keys for P_FORMAT_RAW_VIDEO 
// 
// P_FORMATKEY_WIDTH 				(int32_t)
// P_FORMATKEY_HEIGHT				(int32_t)
// P_FORMATKEY_STRIDE				(int32_t) stride in pixels (>= WIDTH)
// P_FORMATKEY_PIXEL_FORMAT			(int32_t:fmtPixelFormat)
// P_FORMATKEY_U_PLANE_OFFSET		(int32_t) for planar pixel formats,
// 									the offset of the U (Cb) plane from the
// 									start of the buffer.
// P_FORMATKEY_V_PLANE_OFFSET		(int32_t) for planar pixel formats,
// 									the offset of the V (Cr) plane from the
// 									start of the buffer.
//
// The following conventions are used when PIXEL_FORMAT is P_PIXEL_FORMAT_YCbCr420_PL:
// - the Y plane data begins at the start of the buffer.  if
// P_FORMATKEY_U_PLANE_OFFSET and P_FORMATKEY_V_PLANE_OFFSET have values greater
// than zero, the U and V plane data are located at the given offsets; otherwise
// the Y, U, and V plane data are contiguous.
// - the sample size for all planes is one byte.
// - the U and V planes have half the horizontal and vertical resolution of the
// Y plane.  this applies to both the width and stride of the buffer data.

enum fmtPixelFormat
{
	P_PIXEL_FORMAT_NONE					= 0,

	// RGB formats
	P_PIXEL_FORMAT_BGR_565_16			= 1,
	P_PIXEL_FORMAT_BGR_565_16_BE		= 2,
	P_PIXEL_FORMAT_BGRA_8888_32			= 3,
	P_PIXEL_FORMAT_BGR_888_32			= 4,
	P_PIXEL_FORMAT_BGR_888_24			= 5,
	P_PIXEL_FORMAT_BGRA_4444_16			= 6,
	P_PIXEL_FORMAT_BGRA_5551_16			= 7,
	P_PIXEL_FORMAT_BGR_555_15			= 8,
	P_PIXEL_FORMAT_BGR_233_8			= 9,
	
	// Planar YCbCr formats
	P_PIXEL_FORMAT_YCbCr420_PL			= 10,
	P_PIXEL_FORMAT_YCbCr411_PL			= 11,
	P_PIXEL_FORMAT_YCbCr422_PL			= 12,
	P_PIXEL_FORMAT_YCbCr444_PL			= 13,

	// Interleaved YCbCr formats
	P_PIXEL_FORMAT_YCbCr420_UYVY		= 14,
	P_PIXEL_FORMAT_YCbCr420_YUYV		= 15,
	P_PIXEL_FORMAT_YCbCr420_VYUY		= 16,
	P_PIXEL_FORMAT_YCbCr420_YVYU		= 17,

	P_PIXEL_FORMAT_YCbCr411_UYYVYY		= 18,

	P_PIXEL_FORMAT_YCbCr422_UYVY		= 19,
	P_PIXEL_FORMAT_YCbCr422_YUYV		= 20,
	P_PIXEL_FORMAT_YCbCr422_VYUY		= 21,
	P_PIXEL_FORMAT_YCbCr422_YVYU		= 22,

	P_PIXEL_FORMAT_YCbCr444_YUV			= 23,
	P_PIXEL_FORMAT_YCbCr444_UYV			= 24
};

#define P_FORMATKEY_WIDTH			"width"
#define P_FORMATKEY_HEIGHT			"height"
#define P_FORMATKEY_STRIDE			"stride"
#define P_FORMATKEY_PIXEL_FORMAT	"pixel_format"
#define P_FORMATKEY_U_PLANE_OFFSET	"u_offset"
#define P_FORMATKEY_V_PLANE_OFFSET	"v_offset"

// ---------------------------------------------------------------------------
// (no keys for P_FORMAT_MIDI)

// ---------------------------------------------------------------------------
// keys for P_FORMAT_RAW_STILL 
// 
// P_FORMATKEY_WIDTH 				(int32)
// P_FORMATKEY_HEIGHT				(int32)
// P_FORMATKEY_STRIDE				(int32) stride in pixels (>= WIDTH)
// P_FORMATKEY_PIXEL_FORMAT			(int32_t:fmtPixelFormat)
// P_FORMATKEY_U_PLANE_OFFSET		(int32_t) for planar pixel formats,
// 									the offset of the U (Cb) plane from the
// 									start of the buffer.
// P_FORMATKEY_V_PLANE_OFFSET		(int32_t) for planar pixel formats,
// 									the offset of the V (Cr) plane from the
// 									start of the buffer.
// P_FORMATKEY_VIDEO_ORIENTATION	(int32_t:fmtVideoOrientation)

enum fmtVideoOrientation
{
	P_VIDEO_TOP_LEFT_RIGHT = 1,		// This is the typical progressive scan format
	P_VIDEO_BOTTOM_LEFT_RIGHT		// This is how BMP and TGA might scan
};

#define P_FORMATKEY_VIDEO_ORIENTATION	"video_orientation"

// ---------------------------------------------------------------------------
// keys for P_FORMAT_MSADPCM_AUDIO
//
// P_FORMATKEY_CHANNEL_USAGE					(fmtAudioChannelUsage)
// P_FORMATKEY_FRAME_RATE						(float)
// P_FORMATKEY_BUFFER_FRAMES					(int32_t) frames per encoded block
// P_FORMATKEY_MSADPCM_BITS_PER_SAMPLE			(int32_t) number of bits per (mono) sample
// P_FORMATKEY_MSADPCM_COEFS					(raw, variable size): 16-bit coefficient table (HOST-ENDIAN!!!)

#define P_FORMATKEY_MSADPCM_BITS_PER_SAMPLE		"msadpcm_sample_bits"
#define P_FORMATKEY_MSADPCM_COEFS				"msadpcm_coefs"

// ---------------------------------------------------------------------------
// keys for P_FORMAT_MPEG12_AUDIO
//
// P_FORMATKEY_CHANNEL_USAGE					(fmtAudioChannelUsage)
// P_FORMATKEY_FRAME_RATE						(float)
// P_FORMATKEY_BUFFER_FRAMES					(int32_t) frames per encoded block
// P_FORMATKEY_ENCODED_BIT_RATE					(int32_t) bits per second of encoded data
// P_FORMATKEY_MPEG12_AUDIO_REVISION
// P_FORMATKEY_MPEG12_AUDIO_LAYER
// P_FORMATKEY_MPEG12_AUDIO_CHANNEL_MODE

enum fmtMPEG12AudioRevision
{
	P_MPEG12_AUDIO_REV_MPEG1,
	P_MPEG12_AUDIO_REV_MPEG2,
	P_MPEG12_AUDIO_REV_MPEG2_5
};

enum fmtMPEG12AudioLayer
{
	P_MPEG12_AUDIO_LAYER_I,
	P_MPEG12_AUDIO_LAYER_II,
	P_MPEG12_AUDIO_LAYER_III
};

enum fmtMPEG12AudioChannelMode
{
	P_MPEG12_AUDIO_STEREO				= 2,
	P_MPEG12_AUDIO_JOINT_STEREO			= 0x82,
	P_MPEG12_AUDIO_DUAL_CHANNEL			= 0x42,
	P_MPEG12_AUDIO_MONO					= 1,

	// use this to extract the channel-count information
	// from the channel mode spec
	P_MPEG12_AUDIO_CHANNEL_COUNT_MASK 	= 0x0f
};

// the following type is only used internally but still
// convenient to define here, enum values are relied on, so
// don't change them if you don't know what you're doing

enum fmtMPEG12AudioEmphasis
{
	P_MPEG12_AUDIO_EMPHASIS_NONE 		= 0,
	P_MPEG12_AUDIO_EMPHASIS_50_15ms 	= 1,
	P_MPEG12_AUDIO_EMPHASIS_CCITT_J17	= 3
};

#define P_FORMATKEY_ENCODED_BIT_RATE			"enc_bit_rate"

#define P_FORMATKEY_MPEG12_AUDIO_REVISION		"mpeg12_audio_rev"
#define P_FORMATKEY_MPEG12_AUDIO_LAYER			"mpeg12_audio_layer"
#define P_FORMATKEY_MPEG12_AUDIO_CHANNEL_MODE	"mpeg12_audio_channel_mode"

// ---------------------------------------------------------------------------
// keys for P_FORMAT_MPEG12_VIDEO
//
// P_FORMATKEY_WIDTH 										(int32_t)
// P_FORMATKEY_HEIGHT										(int32_t)
// P_FORMATKEY_FRAME_RATE									(float)
// P_FORMATKEY_ENCODED_BIT_RATE								(int32_t) bits per second of encoded data


// ---------------------------------------------------------------------------
// keys for P_FORMAT_DVI_ADPCM_AUDIO 
//
// P_FORMATKEY_CHANNEL_USAGE				(fmtAudioChannelUsage)
// P_FORMATKEY_FRAME_RATE					(float)
//
// P_FORMATKEY_BUFFER_FRAMES				(int32_t) frames per encoded block
// P_FORMATKEY_DVIADPCM_BITS_PER_SAMPLE		(int32_t) number of bits 
// 													  per (mono) sample
//

#define P_FORMATKEY_DVIADPCM_BITS_PER_SAMPLE	"dviadpcm_sample_bits"

// ---------------------------------------------------------------------------
// keys for P_FORMAT_MPEG4_AUDIO_TF
//

// P_FORMATKEY_MPEG4AUDIO_OBJECT_PROFILE					(int32_t : fmtMPEG4AudioObjectProfile)
// P_FORMATKEY_MPEG4AUDIO_TF_CODING_TYPE					(int32_t : fmtMPEG4AudioTFCoding)
// P_FORMATKEY_MPEG4AUDIO_TF_FRAME_LENGTH					(int32_t)
// P_FORMATKEY_MPEG4AUDIO_TF_CORE_CODER_DELAY				(int32_t)
// P_FORMATKEY_MPEG4AUDIO_TF_LSLAYER_LENGTH					(int32_t)
// P_FORMATKEY_MPEG4AUDIO_TF_PCE							(raw)
// P_FORMATKEY_FRAME_RATE									(float)
// P_FORMATKEY_CHANNEL_USAGE								(fmtAudioChannelUsage)
// P_FORMATKEY_BUFFER_FRAMES								(int32_t) frames per encoded block
// P_FORMATKEY_ENCODED_BIT_RATE								(int32_t) bits per second of encoded data

enum fmtMPEG4AudioObjectProfile
{
	P_MPEG4AUDIO_AAC_MAIN = 1,
	P_MPEG4AUDIO_AAC_LC,
	P_MPEG4AUDIO_AAC_SSR,
	P_MPEG4AUDIO_AAC_LTP,
	// (5 reserved)
	P_MPEG4AUDIO_AAC_SCALABLE = 6,
	P_MPEG4AUDIO_TWINVQ,
	P_MPEG4AUDIO_CELP,
	P_MPEG4AUDIO_HVXC,
	// (10, 11 reserved)
	P_MPEG4AUDIO_TTSI = 12,
	P_MPEG4AUDIO_MAIN_SYNTHETIC,
	P_MPEG4AUDIO_WAVETABLE,
	P_MPEG4AUDIO_GENERAL_MIDI,
	P_MPEG4AUDIO_ALGORITHMIC,
	// MPEG4 version 2 formats follow (error-resistant forms)
	P_MPEG4AUDIO_ER_AAC_LC,
	P_MPEG4AUDIO_ER_AAC_LTP,
	P_MPEG4AUDIO_ER_AAC_SCALABLE,
	P_MPEG4AUDIO_ER_TWINVQ,
	P_MPEG4AUDIO_ER_BSAC,
	P_MPEG4AUDIO_ER_AAC_LD,
	P_MPEG4AUDIO_ER_CELP,
	P_MPEG4AUDIO_ER_HVXC,
	P_MPEG4AUDIO_ER_HILN,
	P_MPEG4AUDIO_ER_PARAMETRIC
};

enum fmtMPEG4AudioTFCoding
{
	P_MPEG4AUDIO_TF_AAC_SCALABLE,
	P_MPEG4AUDIO_TF_BSAC,
	P_MPEG4AUDIO_TF_TWINVQ,
	P_MPEG4AUDIO_TF_AAC_NON_SCALABLE
};

#define P_FORMATKEY_MPEG4AUDIO_OBJECT_PROFILE "mpeg4audio_object_profile"
#define P_FORMATKEY_MPEG4AUDIO_TF_CODING "mpeg4audio_tf_coding"
#define P_FORMATKEY_MPEG4AUDIO_TF_FRAME_LENGTH "mpeg4audio_tf_frame_length"
#define P_FORMATKEY_MPEG4AUDIO_TF_CORE_CODER_DELAY "mpeg4audio_tf_core_coder_delay"
#define P_FORMATKEY_MPEG4AUDIO_TF_LSLAYER_LENGTH "mpeg4audio_tf_lslayer_length"
#define P_FORMATKEY_MPEG4AUDIO_TF_PCE "mpeg4audio_tf_pce"

// ---------------------------------------------------------------------------
// keys for P_FORMAT_MPEG4_VIDEO
//

// P_FORMATKEY_WIDTH 										(int32_t)
// P_FORMATKEY_HEIGHT										(int32_t)
// P_FORMATKEY_MPEG4VIDEO_VOP_TIME_INC_RESOLUTION			(int32_t)
// P_FORMATKEY_FRAME_RATE									(float)
// P_FORMATKEY_ENCODED_BIT_RATE								(int32_t) bits per second of encoded data

#define P_FORMATKEY_MPEG4VIDEO_VOP_TIME_INC_RESOLUTION "mpeg4video_vop_time_inc_res"

// ---------------------------------------------------------------------------
// keys for P_FORMAT_YCBCR420_PLANAR_VIDEO
//
// ***
// *** DEPRECATED: use P_FORMAT_RAW_VIDEO with the P_FORMAT_YCbCr420_PL
// *** pixel format instead of this format type.
// ***
//
// P_FORMATKEY_WIDTH 							(int32_t, multiple of 4)
// P_FORMATKEY_HEIGHT							(int32_t, multiple of 4)
// P_FORMATKEY_FRAME_RATE						(float)
// P_FORMATKEY_YCBCR420_PLANAR_VIDEO_Y_STRIDE	(int32_t)
// P_FORMATKEY_YCBCR420_PLANAR_VIDEO_UV_STRIDE	(int32_t)
//
// buffer data consists of all data for the Y plane, followed by
// the U plane, then the V plane.  the sample size for all planes
// is one byte.  the U and V planes have half the horizontal and
// vertical resolution of the Y plane.

#define P_FORMATKEY_YCBCR420_PLANAR_VIDEO_Y_STRIDE "ycbcr420_y_stride"
#define P_FORMATKEY_YCBCR420_PLANAR_VIDEO_UV_STRIDE "ycbcr420_uv_stride"

#if defined(__cplusplus) && _SUPPORTS_NAMESPACE
}; };
#endif	/* palmos::media */
#endif	/* _FORMAT_DEFS_H_ */
