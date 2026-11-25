/*
 * libvcodec - private conventions of the VPU driver
 *
 * None of this is in the standard videodev2.h, and the driver's own
 * mvx-v4l2-controls.h is not installed into staging (every user inside the
 * SDK carries its own copy). To keep this library dependency free, only the
 * constants actually used are copied here, each with the place in the driver
 * it comes from.
 *
 * Source: drivers/media/platform/canaan/vpu/
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KD_MVX_H
#define KD_MVX_H

#include <linux/videodev2.h>

/*
 * Base of the private controls. The driver uses V4L2_CTRL_CLASS_MPEG +
 * 0x2000, and the order of enum v4l2_cid_mve_video in mvx-v4l2-controls.h is
 * where the offsets below come from.
 */
#define KD_CID_MVE_BASE (V4L2_CTRL_CLASS_MPEG + 0x2000)

/*
 * Frame rate. Note it is Q16 fixed point: 30 fps is written as 30 << 16.
 * The driver defaults to 30 << 16, range 0 .. 0x10000000
 * (mvx_v4l2_ctrls.c:909).
 */
#define KD_CID_MVE_FRAME_RATE (KD_CID_MVE_BASE + 0)

/*
 * How NALUs are framed in the bitstream, menu entries in nalu_format_str[]
 * of mvx_v4l2_ctrls.c:
 *   0 = start codes (Annex-B, the default)
 *   1 = one NALU per buffer
 *   2/3/4 = 1/2/4 byte length field
 * Writing the bitstream straight to a playable .264/.265 file needs 0.
 */
#define KD_CID_MVE_NALU_FORMAT (KD_CID_MVE_BASE + 1)
#define KD_NALU_FORMAT_START_CODES 0

/*
 * H.265 profile and level go through private controls: the driver does not
 * register the standard V4L2_CID_MPEG_VIDEO_HEVC_PROFILE/LEVEL
 * (mvx_v4l2_ctrls.c:1013 and :1045). H.264 uses the standard controls.
 */
#define KD_CID_MVE_H265_PROFILE (KD_CID_MVE_BASE + 3)
#define KD_CID_MVE_H265_LEVEL (KD_CID_MVE_BASE + 5)

/* indices into h265_profile_str[] */
#define KD_MVE_H265_PROFILE_MAIN 0
#define KD_MVE_H265_PROFILE_MAIN_STILL 1
#define KD_MVE_H265_PROFILE_MAIN_INTRA 2
#define KD_MVE_H265_PROFILE_MAIN_10 3

/*
 * Arbitrary downscale on the decoder output.
 *
 * mvx-v4l2-controls.h:336 and :426. The firmware scales the picture down on
 * its way out of the decoder, so the frames arrive already small and nothing
 * downstream has to resize them - which matters here because the K230 has no
 * scaler anywhere else: the display controller programs its active size
 * straight from the plane's source rectangle and ignores the destination one
 * (canaan_vo.c, canaan_vo_update_video), the GDMA only rotates, and the 2D
 * engine only does colour conversion, OSD and borders.
 *
 * Two rules, both learned the hard way:
 *
 *   1. It has to be set BEFORE the firmware is loaded. mvx_session.c,
 *      mvx_session_set_dsl_frame() returns -EBUSY once is_fw_loaded() is
 *      true, and the firmware is fetched as soon as both queues are
 *      streaming (see dec_probe_output() in kd_dec.c).
 *
 *   2. Down only. There is no upscaler.
 */
struct kd_mvx_dsl_frame {
	unsigned int width;
	unsigned int height;
};

#define KD_VIDIOC_S_MVX_DSL_FRAME \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 6, struct kd_mvx_dsl_frame)

/*
 * The three things this driver is easiest to get wrong, written down once.
 *
 * 1. V4L2_BUF_FLAG_KEYFRAME means something different on input and output
 *
 *    Input (mvx_v4l2_buffer.c:187-238):
 *      - encoder input is a frame     -> MVX_BUFFER_FRAME_FLAG_FORCE_IDR,
 *                                        forces the frame to be coded as IDR
 *      - decoder input is a bitstream -> MVX_BUFFER_EOF, meaning a complete
 *                                        frame ends here
 *
 *    Output (mvx_v4l2_buffer.c:update_vb2_flags):
 *      - set from MVX_BUFFER_EOF, meaning this buffer ends a frame. It does
 *        NOT mean the frame is a key frame.
 *
 *    So the flag cannot be used to tell whether an encoded packet is a key
 *    frame; reserved2 below is what carries that.
 *
 * 2. reserved2 of an output buffer carries the frame type
 *
 *    On single plane output the driver fills in (mvx_v4l2_vidioc.c:1532):
 *      reserved2 = (frame_type << 24) | (src_transform << 16) |
 *                  bitstream_remaining_kb
 *    frame_type is 0=I 1=P 2=B 3=b (mve_protocol_def_v2.h:1444).
 *
 * 3. End of stream is not -EPIPE
 *
 *    The driver never sets vb2's last_buffer_dequeued, so DQBUF will not
 *    return -EPIPE. The only end marker is V4L2_BUF_FLAG_LAST on an output
 *    buffer (set from MVX_BUFFER_EOS); the driver also queues a
 *    V4L2_EVENT_EOS (mvx_v4l2_vidioc.c:1501).
 */
#define KD_RESERVED2_FRAME_TYPE(r) (((r) >> 24) & 0xff)
#define KD_MVX_FRAME_TYPE_I 0

#endif /* KD_MVX_H */
