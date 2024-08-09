#pragma once

#if defined(ZENOHCXX_ZENOHPICO) && defined(ZENOHCXX_ZENOHC)
#error("Only one of ZENOHCXX_ZENOHPICO and ZENOHCXX_ZENOHC should be defined. \
Explictly include zenohpico.hxx and zenohc.hxx to use both libraies in the same program\
under namespaces zenohpico and zenohc respectively.")
#endif
#if !defined(ZENOHCXX_ZENOHPICO) && !defined(ZENOHCXX_ZENOHC)
#error("Either ZENOHCXX_ZENOHPICO or ZENOHCXX_ZENOHC should be defined")
#endif

#if defined(ZENOHCXX_ZENOHPICO)
#include "zenoh-pico.h"
#elif defined(ZENOHCXX_ZENOHC)
#include "zenoh.h"
#endif