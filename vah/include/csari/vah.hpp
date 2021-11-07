#pragma once
#include <cstddef>
#include <variant>
namespace csari::vah::vahinternal {
using Num = std::size_t;
using std::forward;
using std::get;
using std::in_place_index;
using std::index_sequence;
using std::is_same_v;
using std::make_index_sequence;
using std::variant_size_v;
template <Num idx, class T>
using variant_t = std::variant_alternative_t<idx, T>;
template <Num N>
struct num {
  static constexpr Num value = N;
};
template <class F, Num... Is>
constexpr void forConstexprWithExpander(F func, index_sequence<Is...>) {
  using expander = int[];
  (void)expander{0, ((void)func(num<Is>{}), 0)...};
}
template <Num N, class F>
constexpr void forConstexprWithExpander(F func) {
  forConstexprWithExpander(func, make_index_sequence<N>());
}
template <Num targetIndex, Num index, class V, class... Ts>
constexpr auto constructVariantFromIndexConstexprRecurse(Ts&&... params) -> V {
  if constexpr (index == targetIndex) {
    return V{in_place_index<index>, forward<Ts>(params)...};
  }
  if constexpr (index + 1 < vahinternal::variant_size_v<V>) {
    return constructVariantFromIndexConstexprRecurse<targetIndex, index + 1, V>(
        forward<Ts>(params)...);
  }
  // Index out of bounds
  return V{in_place_index<index>, forward<Ts>(params)...};
}
template <Num index, class V, class... Ts>
auto constructVariantFromIndexRuntimeRecurse(Num const targetIndex,
                                             Ts&&... params) -> V {
  if (index == targetIndex) {
    return V{in_place_index<index>, forward<Ts>(params)...};
  }
  if constexpr (index + 1 < variant_size_v<V>) {
    return constructVariantFromIndexRuntimeRecurse<index + 1, V>(
        targetIndex, forward<Ts>(params)...);
  } else {
    return V{in_place_index<0>, forward<Ts>(params)...};
  }
}

template <class VariantType, class T, Num index = 0>
constexpr auto variantIndexImplementation() -> Num {
  if constexpr (index == variant_size_v<VariantType>) {
    return index;
  } else if constexpr (is_same_v<variant_t<index, VariantType>, T>) {
    return index;
  } else {
    return variantIndexImplementation<VariantType, T, index + 1>();
  }
}

template <class V, class F>
constexpr void performOnData(V& variantData, vahinternal::Num const index,
                             F f) {
  vahinternal::forConstexprWithExpander<vahinternal::variant_size_v<V>>(
      [ index, &variantData, &f ](auto i) constexpr {
        if (i.value == index) {
          f(vahinternal::get<i.value>(variantData));
        }
      });
}
template <class V, class F>
constexpr void performOnData(V const& variantData, vahinternal::Num const index,
                             F f) {
  vahinternal::forConstexprWithExpander<vahinternal::variant_size_v<V>>(
      [ index, &variantData, &f ](auto const i) constexpr {
        if (i.value == index) {
          f(vahinternal::get<i.value>(variantData));
        }
      });
}
}  // namespace csari::vah::vahinternal
namespace csari::vah {
template <class V, class... Ts>
auto constructVariantFromIndexRuntime(vahinternal::Num const targetIndex,
                                      Ts&&... params) -> V {
  return vahinternal::constructVariantFromIndexRuntimeRecurse<0, V>(
      targetIndex, vahinternal::forward<Ts>(params)...);
}
template <vahinternal::Num targetIndex, class V, class... Ts>
constexpr auto constructVariantFromIndexConstexpr(Ts&&... params) -> V {
  return vahinternal::constructVariantFromIndexConstexprRecurse<targetIndex, 0,
                                                                V>(
      vahinternal::forward<Ts>(params)...);
}
template <class V, class F, class... Ts>
auto constructAndPerformOnData(vahinternal::Num const index, F f, Ts&&... args)
    -> V {
  auto v = constructVariantFromIndexRuntime<V>(
      index, vahinternal::forward<Ts>(args)...);
  vahinternal::forConstexprWithExpander<vahinternal::variant_size_v<V>>([&](
      auto i) constexpr {
    if (i.value == index) {
      f(vahinternal::get<i.value>(v));
    }
  });
  return v;
}

template <class V, class F>
constexpr void performOnData(V& variantData, F&& f) {
  vahinternal::performOnData(variantData, variantData.index(),
                             vahinternal::forward<F>(f));
}
template <class V, class F>
constexpr void performOnData(V const& variantData, F&& f) {
  vahinternal::performOnData(variantData, variantData.index(),
                             vahinternal::forward<F>(f));
}

// Get index from the current variant types
template <class VariantType, class T, vahinternal::Num index = 0>
constexpr vahinternal::Num VariantIndex =
    vahinternal::variantIndexImplementation<VariantType, T, index>();
}  // namespace csari::vah