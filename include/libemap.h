/**
 * @file libemap.h
 * @brief Top-level public header for the EMAP library.
 *
 * This header serves as the single inclusion point for the EMAP high-level
 * API. It re-exports the public symbols provided by emap_high_level.h and
 * guarantees safe, single inclusion through the use of include guards and
 * an optional #pragma once.
 *
 * Usage:
 * @code
 * #include "libemap.h"
 * // Use EMAP high-level functions and types
 * @endcode
 *
 * @note Clients should include this header rather than internal/private
 * headers to maintain API stability.
 *
 * @see emap_high_level.h
 */
#ifndef LIBEMAP_H
#define LIBEMAP_H

#pragma once
#include "emap_high_level.h"
#include "wrappers.h"

#endif