/*
 *
 * Copyright (c) 2003 The Regents of the University of California.  All
 * rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Neither the name of the University nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */



/**
 * \file queue.h a queue template
 *
 *  \brief implements a queue template in regular C.
 *
 *  To use it,
 *  you must have two structures, a container structure and an element
 *  structure.  Call them "c" and "e".  You can add the list to the
 *  container like this:
 *
 *  struct c {
 *    // list declaration: field name 'list', element type struct e
 *    QUEUE_DECL(list,struct e);
 *    int other_fields;
 *    ...
 *  }
 *
 *  struct e {
 *    // element declaration: field name 'ptrs', element type struct e
 *    QUEUE_ELEMENT_DECL(ptrs,struct e);
 *    int other_fields;
 *    ...
 *  };
 *
 *  Through the magic of macros, this will turn into:
 *
 *  struct c {
 *    struct e * list_head;
 *    struct e * list_tail;
 *    int other_fields;
 *    ...
 *  }
 *
 *  struct e {
 *    struct e * ptrs_next;
 *    struct e * ptrs_prev;
 *    int other_fields;
 *    ...
 *  };
 *
 *  The tokens 'ptrs' and 'list' are supplied in order to give the
 *  fields unique names; this way it is possible to have a single
 *  container contain multiple lists, without generating a field name
 *  conflict.
 *
 *  Then, you must instantiate the functions that operate on the lists.
 *  You do this with two more macros:
 *
 *  // function declarations:
 *  //   function prefix=       my_list_*
 *  //   element field name=    ptrs
 *  //   container field name=  list
 *  //   element type=          struct e
 *  //   container type=        struct c
 *
 *  QUEUE_INLINE_INSTANTIATIONS(my_list,ptrs,list,struct e,struct c);
 *  QUEUE_FUNCTION_INSTANTIATIONS(my_list,ptrs,list,struct e,struct c);
 *
 *  The INLINE macro generates inline functions and prototypes... these
 *  need to be in a .h file.  The FUNCTION macro generates actual
 *  functions...  these need to be in a .c file.
 *
 *  These macros will generate the following functions:
 *
 *  void my_list_init(struct c *);
 *
 *  int my_list_empty(struct c *);
 *  struct e *my_list_top(struct c *);
 *  struct e *my_list_last(struct c *);
 *
 *  int my_list_qlen(struct c *);
 *  int my_list_element_qlen(struct e *);
 *
 *  struct e *my_list_push(struct c *, struct e *nelement);
 *  struct e *my_list_pop(struct c *);
 *
 *  struct e *my_list_push_front(struct c *, struct e *nelement);
 *  struct e *my_list_pop_back(struct c *);
 *
 *  struct e *my_list_next(struct e *);
 *  struct e *my_list_prev(struct e *);
 *
 *  struct e *my_list_remove(struct c *, struct e *pos);
 *  struct e *my_list_remove_inc(struct c *, struct e *pos);
 *  struct e *my_list_remove_dec(struct c *, struct e *pos);
 *  struct e *my_list_insert_pre(struct c *, struct e *pos, struct e *nelement);
 *  struct e *my_list_insert_post(struct c *, struct e *pos, struct e *nelement);
 *
 *  void my_list_clear_elt_ptrs(struct e *elt);
 *  void my_list_swap_q(struct c *c1, struct c *c2);
 *  void my_list_swap_ptrs(struct e *e1, struct e *e2);
 *
 */

#include <stdlib.h>

#ifndef _QUEUE_H_
#define _QUEUE_H_

#define QUEUE_DECL(name,type) \
type *name##_head; \
type *name##_tail; \
int name##_len

#define QUEUE_ELEMENT_DECL(name,type) \
type *name##_next; \
type *name##_prev; \
int name##_flags

#define IN_QUEUE  (1 << 0)

#define QUEUE_FUNCTION_INSTANTIATIONS(name,iname,cname,type,containertype) \
 \
type * name##_insert_pre(containertype *c, type *pos, type *nitem) \
{ \
  if (nitem && !name##_in_queue(nitem)) { \
    if (pos) { \
      nitem->iname##_next = pos; \
      nitem->iname##_prev = pos->iname##_prev; \
      if (pos->iname##_prev) \
        pos->iname##_prev->iname##_next = nitem; \
      else \
        c->cname##_head = nitem; \
      pos->iname##_prev = nitem; \
    } else if (c->cname##_head == NULL) { \
      nitem->iname##_next = nitem->iname##_prev = NULL; \
      c->cname##_head = c->cname##_tail = nitem; \
    } else abort(); \
    nitem->iname##_flags |= IN_QUEUE; \
    c->cname##_len++; \
  } \
  return nitem; \
} \
 \
type * name##_insert_post(containertype *c, type *pos, type *nitem) \
{ \
  if (nitem && !name##_in_queue(nitem)) { \
    if (pos) { \
      nitem->iname##_prev = pos; \
      nitem->iname##_next = pos->iname##_next; \
      if (pos->iname##_next) \
        pos->iname##_next->iname##_prev = nitem; \
      else \
        c->cname##_tail = nitem; \
      pos->iname##_next = nitem; \
    } \
    else if (c->cname##_tail == NULL) { \
      nitem->iname##_next = nitem->iname##_prev = NULL; \
      c->cname##_head = c->cname##_tail = nitem; \
    } else abort(); \
    nitem->iname##_flags |= IN_QUEUE; \
    c->cname##_len++; \
  } \
  return nitem; \
} \
 \
type * name##_remove(containertype *c, type *pos) \
{ \
  if (pos && name##_in_queue(pos)) { \
    if (pos->iname##_prev) \
      pos->iname##_prev->iname##_next = pos->iname##_next; \
    else \
      c->cname##_head = pos->iname##_next; \
    if (pos->iname##_next) \
      pos->iname##_next->iname##_prev = pos->iname##_prev; \
    else \
      c->cname##_tail = pos->iname##_prev; \
    pos->iname##_prev = pos->iname##_next = NULL; \
    pos->iname##_flags &= ~IN_QUEUE; \
    c->cname##_len--; \
  } \
  return pos; \
}



#define QUEUE_INLINE_INSTANTIATIONS(name,iname,cname,type,containertype) \
 \
type * name##_remove(containertype *c, type *pos); \
type * name##_insert_post(containertype *c, type *pos, type *nitem); \
type * name##_insert_pre(containertype *c, type *pos, type *nitem); \
 \
static inline \
int name##_in_queue(type *item) \
{ return item && (item->iname##_flags & IN_QUEUE); } \
 \
static inline void name##_init(containertype *c) \
  { c->cname##_tail = c->cname##_head = NULL; c->cname##_len = 0; } \
static inline void name##_el_init(type *c) \
  { c->iname##_next = c->iname##_prev = NULL; c->iname##_flags = 0; } \
static inline int name##_element_qlen(type *p) { return p==NULL?0:1+name##_element_qlen(p->iname##_next);} \
static inline int name##_qlen(containertype *c) { return (c==NULL)?0:(c->cname##_len); } \
static inline type * name##_top(containertype *c) { return (c==NULL)?NULL:c->cname##_head; } \
static inline type * name##_last(containertype *c) { return (c==NULL)?NULL:c->cname##_tail; } \
static inline int name##_empty(containertype *c) { return (c->cname##_head == NULL); } \
static inline type * name##_pop(containertype *c) \
  { return (name##_remove(c, name##_top(c))); } \
static inline type * name##_push(containertype *c, type *nitem) \
  { return (name##_insert_post(c, name##_last(c), nitem)); } \
static inline type * name##_next(type *p) { return (p==NULL)?NULL:p->iname##_next; } \
static inline type * name##_prev(type *p) { return (p==NULL)?NULL:p->iname##_prev; } \
static inline type * name##_remove_inc(containertype *c, type *p) \
  { type *t = name##_next(p); name##_remove(c,p); return t; } \
static inline type * name##_remove_dec(containertype *c, type *p) \
  { type *t = name##_prev(p); name##_remove(c,p); return t; } \
static inline type * name##_push_front(containertype *c, type *p) \
  { if (name##_top(c)) return name##_insert_pre(c, name##_top(c), p); \
    else return name##_push(c, p); } \
static inline type * name##_pop_back(containertype *c) \
  { type *retval = name##_last(c); if (retval) name##_remove(c, retval); return retval; } \
 \
static inline void name##_clear_elt_ptrs(type *e) \
{ e->iname##_prev = e->iname##_next = NULL; } \
 \
static inline void name##_swap_q(containertype *c1, containertype *c2) \
{ \
  type *tmp; \
  tmp = c1->cname##_head; c1->cname##_head = c2->cname##_head; c2->cname##_head = tmp; \
  tmp = c1->cname##_tail; c1->cname##_tail = c2->cname##_tail; c2->cname##_tail = tmp; \
} \
static inline void name##_swap_ptrs(type *e1, type *e2) \
{ \
  type *tmp; \
  tmp = e1->iname##_next; e1->iname##_next = e2->iname##_next; e2->iname##_next = tmp; \
  tmp = e1->iname##_prev; e1->iname##_prev = e2->iname##_prev; e2->iname##_prev = tmp; \
} \
static inline void name##_merge(containertype *c1, containertype *c2) \
{ \
  if (c2->cname##_head && c2->cname##_tail) { \
    if (c1->cname##_head && c1->cname##_tail) { \
       c1->cname##_tail->iname##_next = c2->cname##_head; \
       c2->cname##_head->iname##_prev = c1->cname##_tail; \
       c1->cname##_tail = c2->cname##_tail; \
    } else { \
       c1->cname##_head = c2->cname##_head; \
       c1->cname##_tail = c2->cname##_tail; \
    } \
    c2->cname##_head = c2->cname##_tail = NULL; \
    c1->cname##_len += c2->cname##_len; \
    c2->cname##_len = 0; \
  } \
}


#define QUEUE_INST(a,b,c,d,e) \
   QUEUE_INLINE_INSTANTIATIONS(a,b,c,d,e); \
   QUEUE_FUNCTION_INSTANTIATIONS(a,b,c,d,e);

#endif
