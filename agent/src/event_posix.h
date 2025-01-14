// Copyright 2022 The Chromium Authors.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_ANALYSIS_SRC_EVENT_POSIX_H_
#define CONTENT_ANALYSIS_SRC_EVENT_POSIX_H_

#include "event_base.h"

namespace content_analysis {
namespace sdk {

// ContentAnalysisEvent implementaton for linux.
class ContentAnalysisEventPosix : public ContentAnalysisEventBase {
 public:
   ContentAnalysisEventPosix(const BrowserInfo& browser_info,
                             ContentAnalysisRequest request);

  // ContentAnalysisEvent:
  int Send() override;

  // TODO(rogerta): Fill in implementation.
};

}  // namespace sdk
}  // namespace content_analysis

#endif  // CONTENT_ANALYSIS_SRC_EVENT_POSIX_H_