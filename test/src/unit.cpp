#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <csari/vah.hpp>
#include <iostream>
#include <variant>

struct VariantConstexprPerformer final {
  constexpr void operator()(float& val) const { val = 3.14f; }
  constexpr void operator()(int& val) const { val = 94; }
  constexpr void operator()(char& val) const { val = ';'; }
};

constexpr auto variantConstexprStructAccessAndUpdate() -> bool {
  using namespace csari::vah;
  auto var = std::variant<int, float, char>{3};
  auto constexpr f = VariantConstexprPerformer{};
  performOnData(var, f);
  return std::get<int>(var) == 94;
}

TEST_CASE("VahVariantConstexprStructAccessAndUpdate") {
  using namespace csari::vah;
  auto constexpr res = variantConstexprStructAccessAndUpdate();
  REQUIRE(res);
}

constexpr auto variantConstexprLambdaConstructAndUpdate() -> bool {
  using namespace csari::vah;
  auto var = std::variant<int, float, char>{};
  constructAndPerformOnData(
      var, 0, [](auto& val) constexpr {
        using U = Base<decltype(val)>;
        if constexpr (std::is_same_v<U, int>) {
          val = 98;
        } else if constexpr (std::is_same_v<U, float>) {
          val = 6.28f;
        } else if constexpr (std::is_same_v<U, char>) {
          val = 'k';
        }
      });
  performOnData(
      var, 0, [](auto& val) constexpr {
        using U = Base<decltype(val)>;
        if constexpr (std::is_same_v<U, int>) {
          val = 48;
        } else if constexpr (std::is_same_v<U, float>) {
          val = 6.28f;
        } else if constexpr (std::is_same_v<U, char>) {
          val = 'k';
        }
      });
  return std::get<int>(var) == 48;
}

TEST_CASE("VahVariantConstexprLambdaConstructAndUpdate") {
  using namespace csari::vah;
  auto constexpr res = variantConstexprLambdaConstructAndUpdate();
  REQUIRE(res);
}