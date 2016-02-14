/*
 * LibPAY - The Toolkit for Smart Payment Applications
 *
 * Copyright (C) 2015, 2016  Michael Jung <mijung@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 */

#ifndef __LIBPAY_CORE_H__
#define __LIBPAY_CORE_H__

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(*(x)))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define TLV_TAG_CLASS_MASK  0xC0u
#define TLV_TAG_P_C_MASK    0x20u
#define TLV_TAG_NUMBER_MASK 0x1Fu

#endif						    /* ndef __LIBPAY_CORE_H__ */
