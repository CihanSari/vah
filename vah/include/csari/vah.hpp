#pragma once
namespace csari::vah::vahinternal {
using Num = unsigned __int64;
template <Num N>
struct num {
  static constexpr Num value = N;
};
template <class F, Num... Is>
constexpr void forConstexprWithExpander(F func, std::index_sequence<Is...>) {
  using expander = int[];
  (void)expander{0, ((void)func(num<Is>{}), 0)...};
}
template <Num N, typename F>
constexpr void forConstexprWithExpander(F func) {
  forConstexprWithExpander(func, std::make_index_sequence<N>());
}
template <typename V, Num targetIndex, Num index = 0>
constexpr void constructVariantFromIndex(V& variantData) {
  if constexpr (index == std::variant_size_v<V>) {
    return;
  }
  if constexpr (index == targetIndex) {
    variantData = {std::in_place_index<index>};
  } else {
    constructVariantFromIndex<V, targetIndex, index + 1>(variantData);
  }
}
}  // namespace csari::vah::vahinternal
namespace csari::vah {
using Num = vahinternal::Num;
template <typename T>
using Base = std::remove_const_t<std::remove_reference_t<T>>;
template <typename V, class F>
constexpr void constructAndPerformOnData(V& variantData, Num const index, F f) {
  vahinternal::forConstexprWithExpander<std::variant_size_v<V>>(
      [index, &variantData, &f](auto i) {
        if (i.value == index) {
          vahinternal::constructVariantFromIndex<V, i.value>(variantData);
          f(std::get<i.value>(variantData));
        }
      });
}
template <typename V, class F>
constexpr void constructAndPerformOnData(V const& variantData, Num const index,
                                         F f) {
  static_assert(false, "Variant is constant");
}

template <typename V, typename T, class F>
constexpr void constructAndPerformOnData(V& variantData, F f) {
  variantData = T{};
  f(std::get<T>(variantData));
}

template <typename V, class F>
constexpr void constructAndPerformOnData(V& variantData, F f) {
  static_assert(false, "Variant type is unknown! Provide type index.");
}

template <typename V, class F>
constexpr void constructAndPerformOnData(V const& variantData, F f) {
  static_assert(false, "Variant is constant!");
}

template <typename V, class F>
constexpr void performOnData(V& variantData, Num const index, F f) {
  vahinternal::forConstexprWithExpander<std::variant_size_v<V>>(
      [ index, &variantData, &f ](auto i) constexpr {
        if (i.value == index) {
          f(std::get<i.value>(variantData));
        }
      });
}
template <typename V, class F>
constexpr void performOnData(V const& variantData, Num const index, F f) {
  vahinternal::forConstexprWithExpander<std::variant_size_v<V>>(
      [ index, &variantData, &f ](auto const i) constexpr {
        if (i.value == index) {
          f(std::get<i.value>(variantData));
        }
      });
}

template <typename V, class F>
constexpr void performOnData(V& variantData, F f) {
  performOnData(variantData, variantData.index(), std::move(f));
}
template <typename V, class F>
constexpr void performOnData(V const& variantData, F f) {
  performOnData(variantData, variantData.index(), std::move(f));
}
}  // namespace csari::vah