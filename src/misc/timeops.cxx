// ----------------------------------------------------------------------------
//	timeops.cxx
//
// Copyright (C) 2007
//		Stelios Bounanos, M0GLD
//
// This file is part of fldigi.
//
// fldigi is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// fldigi is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------

#include <config.h>

#include "timeops.h"

struct timespec operator+(const struct timespec &t0, const double &t)
{
        struct timespec r;
        r.tv_sec = t0.tv_sec + static_cast<time_t>(t);
        r.tv_nsec = t0.tv_nsec + static_cast<long>((t - static_cast<time_t>(t)) * 1e9);
        return r;
}

struct timespec operator-(const struct timespec &t0, const struct timespec &t1)
{
        struct timespec r = t0;

        if (r.tv_nsec < t1.tv_nsec) {
                --r.tv_sec;
                r.tv_nsec += 1000000000L;
        }
        r.tv_sec -= t1.tv_sec;
        r.tv_nsec -= t1.tv_nsec;

        return r;
}

bool operator>(const struct timespec &t0, const struct timespec &t1)
{
        if (t0.tv_sec == t1.tv_sec)
                return t0.tv_nsec > t1.tv_nsec;
        else if (t0.tv_sec > t1.tv_sec)
                return true;
        else
                return false;
}