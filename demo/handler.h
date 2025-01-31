// Copyright 2022 The Chromium Authors.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_ANALYSIS_DEMO_HANDLER_H_
#define CONTENT_ANALYSIS_DEMO_HANDLER_H_

#include <fstream>
#include <iostream>
#include <utility>

#include "content_analysis/sdk/analysis_agent.h"

// An AgentEventHandler that dumps requests information to stdout and blocks
// any requests that have the keyword "block" in their data
class Handler : public content_analysis::sdk::AgentEventHandler {
 public:
  using Event = content_analysis::sdk::ContentAnalysisEvent;

  Handler() = default;

 protected:
  // Analyzes one request from Google Chrome and responds back to the browser
  // with either an allow or block verdict.
  void AnalyzeContent(std::unique_ptr<Event> event) {
    // An event represents one content analysis request and response triggered
    // by a user action in Google Chrome.  The agent determines whether the
    // user is allowed to perform the action by examining event->GetRequest().
    // The verdict, which can be "allow" or "block" is written into
    // event->GetResponse().

    std::cout << std::endl << "----------" << std::endl << std::endl;

    DumpRequest(event->GetRequest());

    bool block = false;
    bool success = true;

    if (event->GetRequest().has_text_content()) {
      block = ShouldBlockRequest(
          event->GetRequest().text_content());
    } else if (event->GetRequest().has_file_path()) {
      std::string content;
      success =
          ReadContentFromFile(event->GetRequest().file_path(),
                              &content);
      if (success) {
        block = ShouldBlockRequest(content);
      }
    }

    if (!success) {
      content_analysis::sdk::UpdateResponse(
          event->GetResponse(),
          std::string(),
          content_analysis::sdk::ContentAnalysisResponse::Result::FAILURE);
      std::cout << "  Verdict: failed to reach verdict" << std::endl;
    } else if (block) {
      content_analysis::sdk::SetEventVerdictToBlock(event.get());
      std::cout << "  Verdict: block" << std::endl;
    } else {
      std::cout << "  Verdict: allow" << std::endl;
    }

    std::cout << std::endl;

    // Send the response back to Google Chrome.
    if (event->Send() != 0) {
      std::cout << "[Demo] Error sending response" << std::endl;
    }
  }

 private:
  void OnBrowserConnected(
      const content_analysis::sdk::BrowserInfo& info) override {
    std::cout << std::endl << "==========" << std::endl;
    std::cout << "Browser connected pid=" << info.pid << std::endl;
  }

  void OnBrowserDisconnected(
      const content_analysis::sdk::BrowserInfo& info) override {
    std::cout << std::endl << "Browser disconnected pid=" << info.pid << std::endl;
    std::cout << "==========" << std::endl;
  }

  void OnAnalysisRequested(std::unique_ptr<Event> event) override {
    // If the agent is capable of analyzing content in the background, the
    // events may be handled in background threads.  Having said that, a
    // event should not be assumed to be thread safe, that is, it should not
    // be accessed by more than one thread concurrently.
    //
    // In this example code, the event is handled synchronously.
    AnalyzeContent(std::move(event));
  }
  void OnResponseAcknowledged(
      const content_analysis::sdk::ContentAnalysisAcknowledgement&
          ack) override {
    std::cout << "Ack: " << ack.request_token() << std::endl;
  }

  void DumpRequest(
      const content_analysis::sdk::ContentAnalysisRequest& request) {
    std::string connector = "<Unknown>";
    if (request.has_analysis_connector()) {
      switch (request.analysis_connector())
      {
      case content_analysis::sdk::FILE_DOWNLOADED:
        connector = "download";
        break;
      case content_analysis::sdk::FILE_ATTACHED:
        connector = "attach";
        break;
      case content_analysis::sdk::BULK_DATA_ENTRY:
        connector = "bulk-data-entry";
        break;
      case content_analysis::sdk::PRINT:
        connector = "print";
        break;
      default:
        break;
      }
    }

    std::string url =
        request.has_request_data() && request.request_data().has_url()
        ? request.request_data().url() : "<No URL>";

    std::string filename =
        request.has_request_data() && request.request_data().has_filename()
        ? request.request_data().filename() : "<No filename>";

    std::string digest =
        request.has_request_data() && request.request_data().has_digest()
        ? request.request_data().digest() : "<No digest>";

    std::string file_path =
        request.has_file_path()
        ? request.file_path() : "None, bulk text entry";

    std::cout << "Request: " << request.request_token() << std::endl;
    std::cout << "  Connector: " << connector << std::endl;
    std::cout << "  URL: " << url << std::endl;
    std::cout << "  Filename: " << filename << std::endl;
    std::cout << "  Digest: " << digest << std::endl;
    std::cout << "  Filepath: " << file_path << std::endl;
  }

  bool ReadContentFromFile(const std::string& file_path,
                          std::string* content) {
    std::ifstream file(file_path,
                      std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open())
      return false;

    // Get file size.  This example does not handle files larger than 1MB.
    // Make sure content string can hold the contents of the file.
    int size = file.tellg();
    if (size > 1024 * 1024)
      return false;

    content->resize(size + 1);

    // Read file into string.
    file.seekg(0, std::ios::beg);
    file.read(&(*content)[0], size);
    content->at(size) = 0;
    return true;
  }

  bool ShouldBlockRequest(const std::string& content) {
    // Determines if the request should be blocked.  For this simple example
    // the content is blocked if the string "block" is found.  Otherwise the
    // content is allowed.
    return content.find("block") != std::string::npos;
  }
};

#endif  // CONTENT_ANALYSIS_DEMO_HANDLER_H_