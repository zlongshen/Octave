/*

Copyright (C) 2002-2015 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

#if ! defined (octave_quit_h)
#define octave_quit_h 1

#include "octave-config.h"

#include <setjmp.h>

/* The signal header is just needed for the sig_atomic_t type.  */
#if defined (__cplusplus)
#  include <csignal>
#  include <string>
extern "C" {
#else
#  include <signal.h>
#endif

#if defined (OCTAVE_HAVE_SIG_JUMP)

typedef sigjmp_buf octave_jmp_buf;

#define octave_set_current_context sigsetjmp (current_context, 1)

#else

typedef jmp_buf octave_jmp_buf;

#define octave_set_current_context setjmp (current_context)

#endif

OCTAVE_API extern octave_jmp_buf current_context;

OCTAVE_API extern void octave_save_current_context (void *);

OCTAVE_API extern void octave_restore_current_context (void *);

OCTAVE_NORETURN OCTAVE_API extern void octave_jump_to_enclosing_context (void);

#if defined (__cplusplus)
class
octave_execution_exception
{
public:

  octave_execution_exception (void) : m_stack_trace () { }

  octave_execution_exception (const octave_execution_exception& x)
    : m_stack_trace (x.m_stack_trace) { }

  octave_execution_exception& operator = (const octave_execution_exception& x)
  {
    if (&x != this)
      m_stack_trace = x.m_stack_trace;

    return *this;
  }

  ~octave_execution_exception (void) { }

  virtual void set_stack_trace (const std::string& st)
  {
    m_stack_trace = st;
  }

  virtual void set_stack_trace (void)
  {
    m_stack_trace = "";
  }

  virtual std::string info (void) const
  {
    return m_stack_trace;
  }

private:

  std::string m_stack_trace;
};

class
octave_interrupt_exception
{
};
#endif

enum octave_exception
{
  octave_no_exception = 0,
  octave_exec_exception = 1,
  octave_alloc_exception = 2
};

OCTAVE_API extern sig_atomic_t octave_interrupt_immediately;

/*
  > 0: interrupt pending
    0: no interrupt pending
  < 0: handling interrupt
*/
OCTAVE_API extern sig_atomic_t octave_interrupt_state;

OCTAVE_API extern sig_atomic_t octave_exception_state;

OCTAVE_API extern volatile sig_atomic_t octave_signal_caught;

OCTAVE_API extern void octave_handle_signal (void);

OCTAVE_NORETURN OCTAVE_API extern void octave_throw_interrupt_exception (void);

OCTAVE_NORETURN OCTAVE_API extern void octave_throw_execution_exception (void);

OCTAVE_NORETURN OCTAVE_API extern void octave_throw_bad_alloc (void);

OCTAVE_API extern void octave_rethrow_exception (void);

#if defined (__cplusplus)
inline void octave_quit (void)
{
  if (octave_signal_caught)
    {
      octave_signal_caught = 0;
      octave_handle_signal ();
    }
};

#define OCTAVE_QUIT octave_quit ()

#else

#define OCTAVE_QUIT \
  do \
    { \
      if (octave_signal_caught) \
        { \
          octave_signal_caught = 0; \
          octave_handle_signal (); \
        } \
    } \
  while (0)
#endif

/* Normally, you just want to use

     BEGIN_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;
     ... some code that calls a "foreign" function ...
     END_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE;

   but sometimes it is useful to do something like

     BEGIN_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE_1;
     ... custom code here, normally ending in a call to
         octave_rethrow_exception ...
     BEGIN_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE_2;

   so that you can perform extra clean up operations before throwing
   the interrupt exception.  */

#define BEGIN_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE \
  BEGIN_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE_1; \
  octave_rethrow_exception (); \
  BEGIN_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE_2

#define BEGIN_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE_1 \
  do \
    { \
      octave_jmp_buf saved_context; \
 \
      octave_save_current_context (saved_context); \
 \
      if (octave_set_current_context) \
        { \
          octave_restore_current_context (saved_context)

#define BEGIN_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE_2 \
        } \
      else \
        { \
          octave_interrupt_immediately++

#define END_INTERRUPT_IMMEDIATELY_IN_FOREIGN_CODE \
          octave_interrupt_immediately--; \
          octave_restore_current_context (saved_context); \
        } \
    } \
  while (0)

#if defined (__cplusplus)

#define BEGIN_INTERRUPT_WITH_EXCEPTIONS \
  sig_atomic_t saved_octave_interrupt_immediately = octave_interrupt_immediately; \
 \
  try \
    { \
      octave_interrupt_immediately = 0;

#define END_INTERRUPT_WITH_EXCEPTIONS \
    } \
  catch (const octave_interrupt_exception&) \
    { \
      octave_interrupt_immediately = saved_octave_interrupt_immediately; \
      octave_jump_to_enclosing_context (); \
    } \
  catch (const octave_execution_exception&) \
    { \
      octave_interrupt_immediately = saved_octave_interrupt_immediately; \
      octave_exception_state = octave_exec_exception; \
      octave_jump_to_enclosing_context (); \
    } \
  catch (const std::bad_alloc&) \
    { \
      octave_interrupt_immediately = saved_octave_interrupt_immediately; \
      octave_exception_state = octave_alloc_exception; \
      octave_jump_to_enclosing_context (); \
    } \
 \
  octave_interrupt_immediately = saved_octave_interrupt_immediately
#endif

#if defined (__cplusplus)
}

/* These should only be declared for C++ code, and should also be
   outside of any extern "C" block.  */

extern OCTAVE_API void (*octave_signal_hook) (void);
extern OCTAVE_API void (*octave_interrupt_hook) (void);
extern OCTAVE_API void (*octave_bad_alloc_hook) (void);

#endif

#endif
