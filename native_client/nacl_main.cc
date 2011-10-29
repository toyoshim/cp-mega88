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

#include "nacl_main.h"

#include <pthread.h>

#include <cstdio>
#include <sstream>
#include <string>
#include <queue>

#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/input_event.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"

extern "C" int nacl_main(void);

class CpMega88Instance : public pp::Instance {
public:
  explicit CpMega88Instance(PP_Instance instance) : pp::Instance(instance)
  {
    PostMessage(pp::Var("Booting CP/Mega88 on NaCl\n"));
  }
  virtual ~CpMega88Instance() {
    if (thread_init) {
      pthread_join(thread_main, NULL);
      pthread_mutex_destroy(&thread_buffer_mutex);
      pthread_mutex_destroy(&thread_block_mutex);
    }
  }

  bool Init(uint32_t argc, const char* argn[], const char* argv[]) {
    PostMessage(pp::Var("  Setting up timer: "));
    core = pp::Module::Get()->core();
    Log("OK\n");
    if (!SetupTimer()) {
      PostMessage(pp::Var("ERROR\n"));
      return false;
    }
    Log("  Setting up main thread: ");
    if (!CreateMainThread()) {
      Log("ERROR\n");
      return false;
    }
    return true;
  }

  bool SetupTimer() {
    if (NULL == core)
      return false;
    core->CallOnMainThread(10, pp::CompletionCallback(TimerCallback, this));
    return true;
  }

  bool CreateMainThread() { 
    thread_init = false;
    thread_block = false;
    int rc = pthread_mutex_init(&thread_buffer_mutex, NULL);
    if (0 != rc)
      return false;
    rc = pthread_mutex_init(&thread_block_mutex, NULL);
    if (0 != rc) {
      pthread_mutex_destroy(&thread_buffer_mutex);
      return false;
    }
    pthread_mutex_lock(&thread_block_mutex);
    rc = pthread_create(&thread_main, NULL, ThreadMain, this);
    if (0 != rc) {
      pthread_mutex_destroy(&thread_buffer_mutex);
      pthread_mutex_destroy(&thread_block_mutex);
      return false;
    }
    thread_init = true;
    return thread_init;
  }

  virtual void HandleMessage(const pp::Var& var_message) {
    if (!var_message.is_string())
      return;
    std::string message = var_message.AsString();
    pthread_mutex_lock(&thread_buffer_mutex);
    in.push(message.c_str()[0]);
    pthread_mutex_unlock(&thread_buffer_mutex);
  }

  void Log(const char* log) {
    pthread_mutex_lock(&thread_buffer_mutex);
    out << log;
    pthread_mutex_unlock(&thread_buffer_mutex);
  }

  int GetKey(void) {
    if (0 != pthread_mutex_trylock(&thread_buffer_mutex))
      return -1;
    int result = -1;
    if (!in.empty()) {
      uint8_t key = in.front();
      in.pop();
      result = static_cast<int>(key);
    }
    pthread_mutex_unlock(&thread_buffer_mutex);
    return result;
  }

  void Block(void) {
    thread_block = true;
    pthread_mutex_lock(&thread_block_mutex);
  }

  static void* ThreadMain(void* param) {
    CpMega88Instance* self = static_cast<CpMega88Instance*>(param);
    self->Log("OK\n");
    nacl_main();
    return NULL;
  }

  static void TimerCallback(void* param, int32_t result) {
    CpMega88Instance* self = static_cast<CpMega88Instance*>(param);
    if (0 == pthread_mutex_trylock(&self->thread_buffer_mutex)) {
      if (!self->out.str().empty()) {
	self->PostMessage(pp::Var(self->out.str()));
	self->out.str("");
      }
      pthread_mutex_unlock(&self->thread_buffer_mutex);
    }
    if (self->thread_block) {
      self->thread_block = false;
      pthread_mutex_unlock(&self->thread_block_mutex);
    }
    self->SetupTimer();
  }

private:
  pthread_t thread_main;
  bool thread_init;
  pthread_mutex_t thread_buffer_mutex;
  pthread_mutex_t thread_block_mutex;
  volatile bool thread_block;
  pp::Core *core;
  std::stringstream out;
  std::queue<uint8_t> in;
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
nacl_puts
(const char *s)
{
  CpMega88Instance* self =
    static_cast<CpMega88Instance*>(CpMega88Module::singleInstance);
  if (NULL == self)
    return;
  self->Log(s);
}

void
nacl_putc
(char c)
{
  CpMega88Instance* self =
    static_cast<CpMega88Instance*>(CpMega88Module::singleInstance);
  if (NULL == self)
    return;
  char s[2];
  s[0] = c;
  s[1] = 0;
  self->Log(s);
}

int
nacl_getc
(void)
{
  CpMega88Instance* self =
    static_cast<CpMega88Instance*>(CpMega88Module::singleInstance);
  if (NULL == self)
    return -1;
  return self->GetKey();
}

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
