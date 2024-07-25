// Copyright 2024 gRPC authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GRPCPP_SUPPORT_GLOBAL_CALLBACK_HOOK_H
#define GRPCPP_SUPPORT_GLOBAL_CALLBACK_HOOK_H

#include <functional>

#include <grpc/grpc.h>
#include <grpc/support/port_platform.h>
#include <grpcpp/support/status.h>

namespace grpc {

class GlobalCallbackHook {
 public:
  virtual ~GlobalCallbackHook() = default;
  virtual void RunCallback(grpc_call* call,
                           std::function<void(Status)> callback,
                           Status status) = 0;
  virtual void RunCallback(grpc_call* call, std::function<void(bool)> callback,
                           bool ok) = 0;
};

/// An exception-safe way of invoking a user-specified callback function
static void CatchingCallbackWithStatus(std::function<void(Status)> callback,
                                       Status status) {
#if GRPC_ALLOW_EXCEPTIONS
  try {
    callback(status);
  } catch (...) {
    // nothing to return or change here, just don't crash the library
  }
#else   // GRPC_ALLOW_EXCEPTIONS
  callback(status);
#endif  // GRPC_ALLOW_EXCEPTIONS
}

static void CatchingCallbackWithOk(std::function<void(bool)> callback,
                                   bool ok) {
#if GRPC_ALLOW_EXCEPTIONS
  try {
    callback(ok);
  } catch (...) {
    // nothing to return or change here, just don't crash the library
  }
#else   // GRPC_ALLOW_EXCEPTIONS
  callback(ok);
#endif  // GRPC_ALLOW_EXCEPTIONS
}

class DefaultGlobalCallbackHook final : public GlobalCallbackHook {
 public:
  void RunCallback(grpc_call* call, std::function<void(Status)> callback,
                   Status status) override {
    CatchingCallbackWithStatus(std::move(callback), std::move(status));
  }
  void RunCallback(grpc_call* call, std::function<void(bool)> callback,
                   bool ok) override {
    CatchingCallbackWithOk(std::move(callback), ok);
  }
};

std::shared_ptr<GlobalCallbackHook> GetGlobalCallbackHook();
void SetGlobalCallbackHook(GlobalCallbackHook* hook);
}  // namespace grpc

#endif  // GRPCPP_SUPPORT_GLOBAL_CALLBACK_HOOK_H
