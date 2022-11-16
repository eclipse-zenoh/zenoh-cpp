#pragma once

#include "zenoh.h"

namespace zenoh
{

struct GetOptions : public z_get_options_t
{
    GetOptions() : z_get_options_t(z_get_options_default()) {}
    GetOptions& set_target(z_query_target_t v) { target = v; return *this; }
    GetOptions& set_consolidation(z_consolidation_mode_t v) { consolidation.mode = v; return *this; }
};
    

}
