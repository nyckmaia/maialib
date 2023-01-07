#ifndef NLOHMANN_JSON_QT6_HACK_INCLUDED
#define NLOHMANN_JSON_QT6_HACK_INCLUDED

#include <QMetaType>
#include "nlohmann/json.hpp"

namespace QTypeTraits {
  template <> struct has_operator_equal<nlohmann::json> : std::true_type { };
  template <> struct has_operator_less_than<nlohmann::json> : std::true_type { };
}

#endif
