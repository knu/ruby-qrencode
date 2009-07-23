/* -*- c-file-style: "ruby" -*- */

/*
 * qrencode.c - Ruby bindings to libqrencode, a QR code encoder library
 *
 * Copyright (c) 2009 Akinori MUSHA <knu@iDaemons.org>
 *
 * All rights reserved.  You can redistribute and/or modify it under the same
 * terms as Ruby.
 */

#include "ruby.h"
#ifdef HAVE_RUBY_ENCODING_H
#include "ruby/encoding.h"
#endif
#include <qrencode.h>

static VALUE cQRcode;
static VALUE sym_level, sym_version, sym_mode, sym_casesensitive;

static void
cQRcode_free(QRcode *qrcode)
{
    if (qrcode != NULL)
	QRcode_free(qrcode);
}

static VALUE
cQRcode_allocate(VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, cQRcode_free, NULL);
}

static VALUE
cQRcode_initialize(int argc, VALUE *argv, VALUE object)
{
    VALUE string, options, value;
    int level = QR_ECLEVEL_L;
    int version = 0;
    int mode = QR_MODE_8;
    int casesensitive = 1;
    QRcode *qrcode;

    if (rb_scan_args(argc, argv, "11", &string, &options) > 1) {
	StringValue(string);
        options = rb_convert_type(options, T_HASH, "Hash", "to_hash");

        value = rb_hash_aref(options, sym_level);
        if (!NIL_P(value)) {
            switch (level = NUM2INT(value)) {
            case QR_ECLEVEL_L:
            case QR_ECLEVEL_M:
            case QR_ECLEVEL_Q:
            case QR_ECLEVEL_H:
                break;
            default:
                rb_raise(rb_eArgError, "invalid level value: %d", level);
            }
        }

        value = rb_hash_aref(options, sym_version);
        if (!NIL_P(value)) {
            version = NUM2INT(value);
            if (version < 0 || 40 < version)
                rb_raise(rb_eArgError, "invalid version value: %d", version);
        }

        value = rb_hash_aref(options, sym_mode);
        if (!NIL_P(value)) {
            switch (mode = NUM2INT(value)) {
            case QR_MODE_NUM:
            case QR_MODE_AN:
            case QR_MODE_8:
            case QR_MODE_KANJI:
                break;
            default:
                rb_raise(rb_eArgError, "invalid mode value: %d", mode);
            }
        }

        casesensitive = rb_hash_aref(options, sym_casesensitive);
        if (!NIL_P(value)) {
            casesensitive = RTEST(value);
        } else {
            switch (mode) {
            case QR_MODE_8:
            case QR_MODE_KANJI:
                casesensitive = 0;
                break;
            }
        }
    }

#ifdef HAVE_RUBY_ENCODING_H
    if (mode == QR_MODE_KANJI)
        string = rb_str_encode(string, rb_str_new2("CP932"), 0, Qnil);
#endif

    qrcode = QRcode_encodeString(RSTRING_PTR(string), version, level, mode, casesensitive);
    if (qrcode == NULL)
        rb_sys_fail("QRcode_encodeString()");

    DATA_PTR(object) = qrcode;

    return object;
}

static QRcode *
get_QRcode_struct(VALUE object)
{
    QRcode *qrcode;

    Data_Get_Struct(object, QRcode, qrcode);

    if (qrcode == NULL)
	rb_raise(rb_eRuntimeError, "uninitialized data");

    return qrcode;
}

static VALUE
cQRcode_version(VALUE object)
{
    QRcode *qrcode = get_QRcode_struct(object);

    return INT2FIX(qrcode->version);
}

static VALUE
cQRcode_width(VALUE object)
{
    QRcode *qrcode = get_QRcode_struct(object);

    return INT2FIX(qrcode->width);
}

static VALUE
cQRcode_aref(VALUE object, VALUE x, VALUE y)
{
    QRcode *qrcode = get_QRcode_struct(object);
    int width, ix, iy;

    width = qrcode->width;

    ix = NUM2INT(x);
    if (ix < 0 || width <= ix)
        rb_raise(rb_eIndexError, "X value out of range (%d..%d): %d", 0, width - 1, ix);
    iy = NUM2INT(y);
    if (iy < 0 || width <= iy)
        rb_raise(rb_eIndexError, "Y value out of range (%d..%d): %d", 0, width - 1, iy);

    return (qrcode->data[ix + iy * width] & 1) ? Qtrue : Qfalse;
}

void
Init_qrencode()
{
    cQRcode = rb_define_class("QRcode", rb_cObject);

    rb_define_const(cQRcode, "LEVEL_L", INT2FIX(QR_ECLEVEL_L));
    rb_define_const(cQRcode, "LEVEL_M", INT2FIX(QR_ECLEVEL_M));
    rb_define_const(cQRcode, "LEVEL_Q", INT2FIX(QR_ECLEVEL_Q));
    rb_define_const(cQRcode, "LEVEL_H", INT2FIX(QR_ECLEVEL_H));

    rb_define_const(cQRcode, "MODE_NUM",   INT2FIX(QR_MODE_NUM));
    rb_define_const(cQRcode, "MODE_AN",    INT2FIX(QR_MODE_AN));
    rb_define_const(cQRcode, "MODE_8",     INT2FIX(QR_MODE_8));
    rb_define_const(cQRcode, "MODE_KANJI", INT2FIX(QR_MODE_KANJI));

    rb_define_alloc_func(cQRcode, cQRcode_allocate);

    sym_level         = ID2SYM(rb_intern("level"));
    sym_version       = ID2SYM(rb_intern("version"));
    sym_mode          = ID2SYM(rb_intern("mode"));
    sym_casesensitive = ID2SYM(rb_intern("casesensitive"));

    rb_define_method(cQRcode, "initialize", cQRcode_initialize, -1);

    rb_define_method(cQRcode, "version", cQRcode_version, 0);
    rb_define_method(cQRcode, "width", cQRcode_width, 0);
    rb_define_method(cQRcode, "[]", cQRcode_aref, 2);
}
