/*
 * Copyright (c) 2011, Takashi TOYOSHIMA <toyoshim@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * - Neither the name of the authors nor the names of its contributors may be
 *   used to endorse or promote products derived from this software with out
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUE
 * NTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#include "wrap.h"
#include "nacl_main.h"

#include <sstream>
#include <string>

#include <pthread.h>
#include <sys/fcntl.h>

#include "ppapi/c/pp_errors.h"
#include "ppapi/c/ppb_file_io.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/file_io.h"
#include "ppapi/cpp/file_ref.h"
#include "ppapi/cpp/file_system.h"
#include "ppapi/cpp/var.h"

class Fd {
  public:
    Fd(bool open) : open(open), offset(0), ref(NULL), io(NULL) {};
    bool open;
    off_t offset;
    pp::FileRef* ref;
    pp::FileIO* io;
};

enum phase {
  PHASE_SYS_OPEN,
  PHASE_IO_OPEN,
  PHASE_SYS_READ,
  PHASE_IO_READ,
  PHASE_SYS_WRITE,
  PHASE_IO_WRITE,
};

struct arguments {
  enum phase phase;
  const char* path;
  int oflag;
  int n;
  void* buf;
  size_t nbytes;
  int result;
  pp::FileRef* ref;
  pp::FileIO* io;
  int32_t open_flags;
  std::stringstream fullpath;
} args;
static pp::Core* core = NULL;
static pp::FileSystem* file = NULL;
static pp::Instance* instance = NULL;
static std::vector<Fd*> fds;
static pthread_mutex_t mutex;

void
wrap_init
(pp::Instance* instance_, const pp::CompletionCallback& callback)
{
  instance = instance_;
  core = pp::Module::Get()->core();
  pthread_mutex_init(&mutex, NULL);
  pthread_mutex_lock(&mutex);
  file = new pp::FileSystem(instance, PP_FILESYSTEMTYPE_LOCALTEMPORARY);
  file->Open(4 * 1024 * 1024, callback);
}

void
wrap_trash
(void)
{
  if (file)
    delete file;
}

void
wrap_main
(void* param, int32_t result)
{
  switch (args.phase) {
    case PHASE_SYS_OPEN: {
      args.phase = PHASE_IO_OPEN;
      args.fullpath.str("");
      args.fullpath << "/" << args.path;
      args.ref = new pp::FileRef(*file, args.fullpath.str().c_str());
      args.io = new pp::FileIO(instance);
      switch (args.oflag) {
        case O_RDONLY:
          args.open_flags = PP_FILEOPENFLAG_READ;
          break;
        case O_WRONLY:
          args.open_flags = PP_FILEOPENFLAG_WRITE;
          break;
        case O_RDWR:
          args.open_flags = PP_FILEOPENFLAG_READ | PP_FILEOPENFLAG_WRITE;
          break;
        default:
          result = -1;
          nacl_puts("wrap_main: unknown open flag");
          pthread_mutex_unlock(&mutex);
          break;
      }
      if (PP_OK_COMPLETIONPENDING != args.io->Open(*args.ref, args.open_flags,
          pp::CompletionCallback(wrap_main, NULL))) {
        nacl_puts("wrap_main: open failed");
        pthread_mutex_unlock(&mutex);
      }
      break; }
    case PHASE_IO_OPEN: {
      if (0 != result) {
        std::stringstream ss;
        ss << "wrap_main: io_open failed with " << result;
        nacl_puts(ss.str().c_str());
        args.result = -1;
      } else {
        args.result = 0;
      }
      pthread_mutex_unlock(&mutex);
      break; }
    case PHASE_SYS_READ: {
      args.phase = PHASE_IO_READ;
      if (PP_OK_COMPLETIONPENDING != fds[args.n]->io->Read(fds[args.n]->offset,
          static_cast<char*>(args.buf), args.nbytes,
          pp::CompletionCallback(wrap_main, NULL))) {
        nacl_puts("wrap_main: read request failed");
        pthread_mutex_unlock(&mutex);
      }
      break; }
    case PHASE_IO_READ: {
      args.result = result;
      if (result > 0)
        fds[args.n]->offset += result;
      pthread_mutex_unlock(&mutex);
      break; }
    case PHASE_SYS_WRITE: {
      args.phase = PHASE_IO_WRITE;
      if (PP_OK_COMPLETIONPENDING != fds[args.n]->io->Write(
          fds[args.n]->offset, static_cast<const char*>(args.buf), args.nbytes,
          pp::CompletionCallback(wrap_main, NULL))) {
        nacl_puts("wrap_main: read request failed");
        pthread_mutex_unlock(&mutex);
      }
      break; }
    case PHASE_IO_WRITE: {
      args.result = result;
      if (result > 0)
        fds[args.n]->offset += result;
      pthread_mutex_unlock(&mutex);
      break; }
  }
}

int
__wrap_open
(const char* path, int oflag, ...)
{
  args.phase = PHASE_SYS_OPEN;
  args.path = path;
  args.oflag = oflag;
  core->CallOnMainThread(0, pp::CompletionCallback(wrap_main, NULL));
  pthread_mutex_lock(&mutex);
  if (args.result < 0) {
    std::stringstream ss;
    ss << "__wrap_open: " << path << "; " << oflag << "\n";
    nacl_puts(ss.str().c_str());
    return -1;
  }
  Fd* fd = new Fd(true);
  fd->ref = args.ref;
  fd->io = args.io;
  fds.push_back(fd);
  return fds.size() - 1;
}

ssize_t
__wrap_read
(int n, void* buf, size_t nbytes)
{
  if (fds.size() <= n) {
    nacl_puts("__wrap_read: invalid descriptor\n");
    goto error;
  }
  if (!fds[n]->open) {
    nacl_puts("__wrap_read: not opened\n");
    goto error;
  }
  args.phase = PHASE_SYS_READ;
  args.n = n;
  args.buf = buf;
  args.nbytes = nbytes;
  core->CallOnMainThread(0, pp::CompletionCallback(wrap_main, NULL));
  pthread_mutex_lock(&mutex);
  return args.result;
error:
  std::stringstream ss;
  ss << " __wrap_read " << n << ", " << buf << ", " << nbytes << "\n";
  nacl_puts(ss.str().c_str());
  return -1;
}

ssize_t
__wrap_write
(int n, const void* buf, size_t nbytes)
{
  if (fds.size() <= n) {
    nacl_puts("__wrap_write: invalid descriptor\n");
    goto error;
  }
  if (!fds[n]->open) {
    nacl_puts("__wrap_write: not opened\n");
    goto error;
  }
  args.phase = PHASE_SYS_WRITE;
  args.n = n;
  args.buf = const_cast<void*>(buf);
  args.nbytes = nbytes;
  core->CallOnMainThread(0, pp::CompletionCallback(wrap_main, NULL));
  pthread_mutex_lock(&mutex);
  return args.result;
error:
  std::stringstream ss;
  ss << " __wrap_write " << n << ", " << buf << ", " << nbytes << "\n";
  nacl_puts(ss.str().c_str());
  return -1;
}

off_t
__wrap_lseek
(int n, off_t offset, int whence)
{
  if (fds.size() <= n) {
    nacl_puts("__wrap_lseek: invalid descriptor\n");
    goto error;
  }
  if (!fds[n]->open) {
    nacl_puts("__wrap_lseek: not opened\n");
    goto error;
  }
  if (SEEK_SET == whence) {
    fds[n]->offset = offset;
    return offset;
  }
  nacl_puts("__wrap_lseek: not supported\n");
error:
  std::stringstream ss;
  ss << "__wrap_lseek: " << n << ", " << offset << ", " << whence << "\n";
  nacl_puts(ss.str().c_str());
  return -1;
}

