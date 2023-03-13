// Validate that __ZENOHCXX_ZENOHPICO and __ZENOHCXX_ZENOHC are mutually exclusive
#if defined(__ZENOHCXX_ZENOHPICO) and defined(__ZENOHCXX_ZENOHC)
#error("Internal include configuration error: both __ZENOHCXX_ZENOHC and __ZENOHCXX_ZENOHPICO defined")
#endif

#include <functional>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "assert.h"
#include "string.h"