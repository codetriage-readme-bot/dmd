/**
 * Compiler implementation of the
 * $(LINK2 http://www.dlang.org, D programming language).
 *
 * Copyright:   Copyright (C) 1984-1998 by Symantec
 *              Copyright (C) 2000-2018 by The D Language Foundation, All Rights Reserved
 * Authors:     $(LINK2 http://www.digitalmars.com, Walter Bright)
 * License:     $(LINK2 http://www.boost.org/LICENSE_1_0.txt, Boost License 1.0)
 * Source:      $(LINK2 https://github.com/dlang/dmd/blob/master/src/dmd/backend/outbuf.h, backend/outbuf.h)
 */

//#pragma once

#include        <string.h>
#include        <assert.h>

#include <stddef.h>     // for size_t

#if __APPLE__ && __i386__
    /* size_t is 'unsigned long', which makes it mangle differently
     * than D's 'uint'
     */
    typedef unsigned d_size_t;
#else
    typedef size_t d_size_t;
#endif

// Output buffer

// (This used to be called OutBuffer, we renamed it to avoid name conflicts with Mars.)

struct Outbuffer
{
    unsigned char *buf;         // the buffer itself
    unsigned char *pend;        // pointer past the end of the buffer
    unsigned char *p;           // current position in buffer
    unsigned char *origbuf;     // external buffer

    Outbuffer();

    Outbuffer(d_size_t initialSize);

    Outbuffer(unsigned char *bufx, d_size_t bufxlen, unsigned incx) :
        buf(bufx), pend(bufx + bufxlen), p(bufx), origbuf(bufx) { }

    ~Outbuffer();

    void reset()
    {
        p = buf;
    }

    // Reserve nbytes in buffer
    void reserve(unsigned nbytes)
    {
        if (pend - p < nbytes)
            enlarge(nbytes);
    }

    // Reserve nbytes in buffer
    void enlarge(unsigned nbytes);

    // Write n zeros; return pointer to start of zeros
    void *writezeros(uint n)
    {
        if (pend - p < n)
            reserve(n);
        void *pstart = memset(p,0,n);
        p += n;
        return pstart;
    }

    // Position buffer to accept the specified number of bytes at offset
    void position(d_size_t offset, d_size_t nbytes)
    {
        if (offset + nbytes > pend - buf)
        {
            enlarge(offset + nbytes - (p - buf));
        }
        p = buf + offset;
    #if DEBUG
        assert(buf <= p);
        assert(p <= pend);
        assert(p + nbytes <= pend);
    #endif
    }

    // Write an array to the buffer, no reserve check
    void writen(const void *b, d_size_t len)
    {
        //if (p + len > pend) assert(0);
        memcpy(p,b,len);
        p += len;
    }

    // Clear bytes, no reserve check
    void clearn(d_size_t len)
    {
        for (d_size_t i = 0; i < len; i++)
            *p++ = 0;
    }

    // Write an array to the buffer.
    void write(const void *b, unsigned len)
    {
        if (pend - p < len)
            reserve(len);
        memcpy(p,b,len);
        p += len;
    }

    void write(Outbuffer *b) { write(b->buf,b->p - b->buf); }

    /**
     * Flushes the stream. This will write any buffered
     * output bytes.
     */
    void flush() { }

    /**
     * Writes an 8 bit byte, no reserve check.
     */
    void writeByten(unsigned char v)
    {
        *p++ = v;
    }

    /**
     * Writes an 8 bit byte.
     */
    void writeByte(int v)
    {
        if (pend == p)
            reserve(1);
        *p++ = v;
    }

    /**
     * Writes a 16 bit little-end short, no reserve check.
     */
    void writeWordn(int v)
    {
#if _WIN32
        *(unsigned short *)p = v;
#else
        p[0] = v;
        p[1] = v >> 8;
#endif
        p += 2;
    }


    /**
     * Writes a 16 bit little-end short.
     */
    void writeWord(int v)
    {
        reserve(2);
        writeWordn(v);
    }


    /**
     * Writes a 16 bit big-end short.
     */
    void writeShort(int v)
    {
        if (pend - p < 2)
            reserve(2);
#if 0
        p[0] = ((unsigned char *)&v)[1];
        p[1] = v;
#else
        unsigned char *q = p;
        q[0] = v >> 8;
        q[1] = v;
#endif
        p += 2;
    }

    /**
     * Writes a 16 bit char.
     */
    void writeChar(int v)
    {
        writeShort(v);
    }

    /**
     * Writes a 32 bit int.
     */
    void write32(int v);

    /**
     * Writes a 64 bit long.
     */
    void write64(long long v)
    {
        if (pend - p < 8)
            reserve(8);
        *(long long *)p = v;
        p += 8;
    }

    /**
     * Writes a 32 bit float.
     */
    void writeFloat(float v);

    /**
     * Writes a 64 bit double.
     */
    void writeDouble(double v);

    void write(const char *s);

    void write(const unsigned char *s);

    void writeString(const char *s);

    void prependBytes(const char *s);

    void prepend(const void *b, d_size_t len);

    void bracket(char c1,char c2);

    /**
     * Returns the number of bytes written.
     */
    d_size_t size()
    {
        return p - buf;
    }

    char *toString();

    void setsize(unsigned size)
    {
        p = buf + size;
        //debug assert(buf <= p);
        //debug assert(p <= pend);
    }


    void writesLEB128(int value);
    void writeuLEB128(unsigned value);

};
