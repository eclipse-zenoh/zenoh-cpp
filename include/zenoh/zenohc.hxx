#pragma once

#if defined(ZENOHCXX_ZENOHPICO) && defined(ZENOHCXX_ZENOHC)
#error("Only one of ZENOHCXX_ZENOHPICO and ZENOHCXX_ZENOHC should be defined.")
#endif
#if !defined(ZENOHCXX_ZENOHPICO) && !defined(ZENOHCXX_ZENOHC)
#error("Either ZENOHCXX_ZENOHPICO or ZENOHCXX_ZENOHC should be defined")
#endif

#if defined(ZENOHCXX_ZENOHPICO)
#include "zenoh-pico.h"
// use same macro to check for unstable features
#elif defined(ZENOHCXX_ZENOHC)
#include "zenoh.h"
#endif