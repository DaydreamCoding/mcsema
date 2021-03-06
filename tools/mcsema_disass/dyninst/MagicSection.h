/*
 * Copyright (c) 2018 Trail of Bits, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <CodeObject.h>

#include <CFG.pb.h>

#include <glog/logging.h>

struct ExternalFunction;

struct MagicSection {
  mcsema::ExternalVariable *WriteExternalVariable(mcsema::Module &module,
                                                  const std::string &name="");
  mcsema::ExternalFunction *WriteExternalFunction(mcsema::Module &module,
                             ExternalFunction &function);

  Dyninst::Address AllocSpace(uint64_t byte_width=8);

  Dyninst::Address GetAllocated(Dyninst::Address ea);

  bool AllocSpace(Dyninst::Address real, Dyninst::Address original) {
    real_to_imag.insert({real, original});
    return true;
  }

  mcsema::ExternalFunction *GetExternalFunction(Dyninst::Address real_ea) {
    auto ea = real_to_imag.find(real_ea);
    if (ea == real_to_imag.end()) {
      LOG(INFO) << "Addr was not even allocated";
      return nullptr;
    }

    for (auto &func : ext_funcs) {
      if (static_cast<Dyninst::Address>(func->ea()) == ea->second) {
        return func;
      }
    }
    LOG(WARNING) << "Did not find external function in MagicSection despite"
                 << " that addr was allocated";
    return nullptr;
  }

  //TODO(lukas): Rework as ctor
  void init(Dyninst::Address start_ea, int ptr_byte_size=8) {
    this->start_ea = start_ea;
    this->ptr_byte_size = ptr_byte_size;
  }

  std::string name = "magic_section";
  std::stringstream data;
  Dyninst::Address start_ea = 0;
  uint64_t size = 0;
  int ptr_byte_size = 8;

  std::vector<mcsema::ExternalVariable *> ext_vars;
  std::vector<mcsema::ExternalFunction *> ext_funcs;

  // This will serve when searching for function xrefs, IDA uses everywhere
  // imaginary address while Dyninst catches the .plt stub one
  std::unordered_map<Dyninst::Address, Dyninst::Address> real_to_imag;

};
