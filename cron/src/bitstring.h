/*
 * Copyright (c) 1989, 1993
 *  The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Paul Vixie.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *  This product includes software developed by the University of
 *  California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *  @(#)bitstring.h  8.1 (Berkeley) 7/19/93
 */

#ifndef _BITSTRING_H_
#define  _BITSTRING_H_

typedef  unsigned char bitstr_t;

/* internal macros */
        /* byte of the bitstring bit is in */
#define  _bit_byte(bit) \
  ((bit) >> 3)

        /* mask for the bit within its byte */
#define  _bit_mask(bit) \
  (1 << ((bit)&0x7))

/* external macros */
        /* bytes in a bitstring of nbits bits */
#define  bitstr_size(nbits) \
  ((((nbits) - 1) >> 3) + 1)

        /* allocate a bitstring */
#define  bit_alloc(nbits) \
  (bitstr_t *)calloc(1, \
      (unsigned int)bitstr_size(nbits) * sizeof(bitstr_t))

        /* allocate a bitstring on the stack */
#define  bit_decl(name, nbits) \
  (name)[bitstr_size(nbits)]

        /* is bit N of bitstring name set? */
#define  bit_test(name, bit) \
  ((name)[_bit_byte(bit)] & _bit_mask(bit))

        /* set bit N of bitstring name */
#define  bit_set(name, bit) \
  (name)[_bit_byte(bit)] |= _bit_mask(bit)

        /* clear bit N of bitstring name */
#define  bit_clear(name, bit) \
  (name)[_bit_byte(bit)] &= ~_bit_mask(bit)

        /* clear bits start ... stop in bitstring */
#define  bit_nclear(name, start, stop) { \
  register bitstr_t *_name = name; \
  register int _start = start, _stop = stop; \
  register int _startbyte = _bit_byte(_start); \
  register int _stopbyte = _bit_byte(_stop); \
  if (_startbyte == _stopbyte) { \
    _name[_startbyte] &= ((0xff >> (8 - (_start&0x7))) | \
              (0xff << ((_stop&0x7) + 1))); \
  } else { \
    _name[_startbyte] &= 0xff >> (8 - (_start&0x7)); \
    while (++_startbyte < _stopbyte) \
      _name[_startbyte] = 0; \
    _name[_stopbyte] &= 0xff << ((_stop&0x7) + 1); \
  } \
}

        /* set bits start ... stop in bitstring */
#define  bit_nset(name, start, stop) { \
  register bitstr_t *_name = name; \
  register int _start = start, _stop = stop; \
  register int _startbyte = _bit_byte(_start); \
  register int _stopbyte = _bit_byte(_stop); \
  if (_startbyte == _stopbyte) { \
    _name[_startbyte] |= ((0xff << (_start&0x7)) & \
            (0xff >> (7 - (_stop&0x7)))); \
  } else { \
    _name[_startbyte] |= 0xff << ((_start)&0x7); \
    while (++_startbyte < _stopbyte) \
          _name[_startbyte] = 0xff; \
    _name[_stopbyte] |= 0xff >> (7 - (_stop&0x7)); \
  } \
}

        /* find first bit clear in name */
#define  bit_ffc(name, nbits, value) { \
  register bitstr_t *_name = name; \
  register int _byte, _nbits = nbits; \
  register int _stopbyte = _bit_byte(_nbits), _value = -1; \
  for (_byte = 0; _byte <= _stopbyte; ++_byte) \
    if (_name[_byte] != 0xff) { \
      _value = _byte << 3; \
      for (_stopbyte = _name[_byte]; (_stopbyte&0x1); \
          ++_value, _stopbyte >>= 1); \
      break; \
    } \
  *(value) = _value; \
}

        /* find first bit set in name */
#define  bit_ffs(name, nbits, value) { \
  register bitstr_t *_name = name; \
  register int _byte, _nbits = nbits; \
  register int _stopbyte = _bit_byte(_nbits), _value = -1; \
  for (_byte = 0; _byte <= _stopbyte; ++_byte) \
    if (_name[_byte]) { \
      _value = _byte << 3; \
      for (_stopbyte = _name[_byte]; !(_stopbyte&0x1); \
          ++_value, _stopbyte >>= 1); \
      break; \
    } \
  *(value) = _value; \
}

#endif /* !_BITSTRING_H_ */
