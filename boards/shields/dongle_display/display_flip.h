/*
 * Copyright (c) 2026 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/* Send the saved orientation to the panel. Call from the display work queue. */
void zmk_dongle_display_flip_apply(void);
