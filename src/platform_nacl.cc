//
// Copyright (c) 2011, Takashi TOYOSHIMA <toyoshim@gmail.com>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// - Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
//
// - Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// - Neither the name of the authors nor the names of its contributors may be
//   used to endorse or promote products derived from this software with out
//   specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUE
// NTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//

#include "platform_nacl.h"

#include <pthread.h>
#include <setjmp.h>
#include <sys/fcntl.h>

#include <cstdio>
#include <sstream>
#include <string>

#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/input_event.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"

#include "naclfs.h"

#include "machine.h"

static jmp_buf jb;

class CpMega88Instance : public pp::Instance {
public:
  explicit CpMega88Instance(PP_Instance instance)
      : pp::Instance(instance),
        core(pp::Module::Get()->core()),
        naclfs(new naclfs::NaClFs(this)),
        thread_init(false),
        thread_block(false){
    open("/dev/stdin", O_RDONLY);
    open("/dev/stdout", O_WRONLY);
    open("/dev/stderr", O_WRONLY);
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_lock(&mutex);
  }
  virtual ~CpMega88Instance() {
    if (thread_init) {
      pthread_join(thread_main, NULL);
      pthread_mutex_destroy(&mutex);
    }
  }

  void Boot() {
    puts("Booting CP/Mega88 on NaCl");
    printf("  Setting up timer: ");
    core->CallOnMainThread(10, pp::CompletionCallback(TimerCallback, this));
    puts("OK");
    printf("  Setting up main thread: ");
    fflush(stdout);
    pthread_create(&thread_main, NULL, ThreadMain, NULL);
    thread_init = true;
  }

  virtual void HandleMessage(const pp::Var& var_message) {
    if (!var_message.is_string())
      return;
    std::string message = var_message.AsString();
    const char *str = message.c_str();
    switch (str[0]) {
      case 'B': // Boot
        Boot();
        break;
      default:  // Port
        naclfs->HandleMessage(var_message);
        break;
    }
  }

  void Log(const char* log) {
    naclfs->Log(log);
  }

  void Block(void) {
    thread_block = true;
    pthread_mutex_lock(&mutex);
  }

  static void* ThreadMain(void* param) {
    puts("OK");
    setjmp(jb);
    machine_boot();
    return NULL;
  }

  static void TimerCallback(void* param, int32_t result) {
    CpMega88Instance* self = static_cast<CpMega88Instance*>(param);
    if (self->thread_block) {
      self->thread_block = false;
      pthread_mutex_unlock(&self->mutex);
    }
    self->core->CallOnMainThread(10,
        pp::CompletionCallback(TimerCallback, self));
  }

private:
  pthread_t thread_main;
  pthread_mutex_t mutex;
  pp::Core* core;
  naclfs::NaClFs* naclfs;
  bool thread_init;
  bool thread_block;
};

class CpMega88Module : public pp::Module {
public:
  CpMega88Module() : pp::Module() {}
  virtual ~CpMega88Module() {}
  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    if (NULL != CpMega88Module::singleInstance)
      return NULL;
    singleInstance = new CpMega88Instance(instance);
    return singleInstance;
  }
  static CpMega88Instance* singleInstance;
};

CpMega88Instance* CpMega88Module::singleInstance = NULL;

namespace pp {
  Module* CreateModule() {
    return new CpMega88Module();
  }
}  // namespace pp

void
nacl_sleep
(void)
{
  CpMega88Instance* self =
    static_cast<CpMega88Instance*>(CpMega88Module::singleInstance);
  if (NULL == self)
    return;
  self->Block();
}

extern "C" void
platform_reset
(void)
{
  longjmp(jb, 0);
}
