/*
 *  Copyright (c) 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_TMMBN_H_
#define WEBRTC_MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_TMMBN_H_

#include <vector>

#include "webrtc/base/basictypes.h"
#include "webrtc/base/constructormagic.h"
#include "webrtc/modules/rtp_rtcp/source/rtcp_packet/rtpfb.h"
#include "webrtc/modules/rtp_rtcp/source/rtcp_packet/tmmb_item.h"
#include "webrtc/modules/rtp_rtcp/source/rtcp_utility.h"

namespace webrtc {
namespace rtcp {
// Temporary Maximum Media Stream Bit Rate Notification (TMMBN).
// RFC 5104, Section 4.2.2.
class Tmmbn : public Rtpfb {
 public:
  static const uint8_t kFeedbackMessageType = 4;

  Tmmbn() {}
  ~Tmmbn() override {}

  // Parse assumes header is already parsed and validated.
  bool Parse(const RTCPUtility::RtcpCommonHeader& header,
             const uint8_t* payload);  // Size of the payload is in the header.

  void WithTmmbr(uint32_t ssrc, uint32_t bitrate_kbps, uint16_t overhead) {
    WithTmmbr(TmmbItem(ssrc, bitrate_kbps * 1000, overhead));
  }
  void WithTmmbr(const TmmbItem& item);

  const std::vector<TmmbItem>& items() const { return items_; }

 protected:
  bool Create(uint8_t* packet,
              size_t* index,
              size_t max_length,
              RtcpPacket::PacketReadyCallback* callback) const override;

 private:
  size_t BlockLength() const override {
    return kHeaderLength + kCommonFeedbackLength +
           TmmbItem::kLength * items_.size();
  }

  // Media ssrc is unused, shadow base class setter and getter.
  void To(uint32_t ssrc);
  uint32_t media_ssrc() const;

  std::vector<TmmbItem> items_;

  RTC_DISALLOW_COPY_AND_ASSIGN(Tmmbn);
};
}  // namespace rtcp
}  // namespace webrtc
#endif  // WEBRTC_MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_TMMBN_H_
