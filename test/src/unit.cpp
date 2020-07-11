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

auto variantStructAccessAndUpdate() -> bool {
  using namespace csari::vah;
  using V = std::variant<float, int, char>;
  auto const indexRuntime = 1U;
  auto var = constructVariantFromIndexRuntime<V>(indexRuntime);
  std::get<int>(var) = 3;
  auto constexpr f = VariantConstexprPerformer{};
  performOnData(var, f);
  return std::get<int>(var) == 94;
}

TEST_CASE("VahVariantStructAccessAndUpdate") {
  using namespace csari::vah;
  auto res = variantStructAccessAndUpdate();
  REQUIRE(res);
}

auto variantLambdaConstructAndUpdate() -> bool {
  using namespace csari::vah;
  using V = std::variant<int, float, char>;
  auto var = constructAndPerformOnData<V>(
      VariantIndex<V, char>, [](auto& val) constexpr {
        using U = std::remove_reference_t<decltype(val)>;
        if constexpr (std::is_same_v<U, int>) {
          val = 98;
        } else if constexpr (std::is_same_v<U, float>) {
          val = 6.28f;
        } else if constexpr (std::is_same_v<U, char>) {
          val = 'r';
        }
      });
  performOnData(
      var, VariantIndex<V, char>, [](auto& val) constexpr {
        using U = std::remove_reference_t<decltype(val)>;
        if constexpr (std::is_same_v<U, int>) {
          val = 48;
        } else if constexpr (std::is_same_v<U, float>) {
          val = 6.28f;
        } else if constexpr (std::is_same_v<U, char>) {
          val = 'k';
        }
      });
  return std::get<char>(var) == 'k';
}

TEST_CASE("VahVariantLambdaConstructAndUpdate") {
  using namespace csari::vah;
  auto const res = variantLambdaConstructAndUpdate();
  REQUIRE(res);
}

auto variantNonDefaultConstructibleLambdaConstructAndUpdate() -> bool {
  using namespace csari::vah;

  struct K1 final {
    constexpr explicit K1(char){};
    int data{4};
  };
  struct K2 final {
    constexpr explicit K2(char){};
    float data{0.4f};
  };
  struct K3 final {
    constexpr explicit K3(char){};
    char data{'q'};
  };
  using V = std::variant<K1, K2, K3>;
  auto var = constructAndPerformOnData<V>(
      VariantIndex<V, K2>, [](auto& val) constexpr {
        using U = std::remove_reference_t<decltype(val)>;
        if constexpr (std::is_same_v<U, K1>) {
          val.data = 98;
        } else if constexpr (std::is_same_v<U, K2>) {
          val.data = 6.28f;
        } else if constexpr (std::is_same_v<U, K3>) {
          val.data = 'k';
        }
      },
      't');
  performOnData(
      var, VariantIndex<V, K2>, [](auto& val) constexpr {
        using U = std::remove_reference_t<decltype(val)>;
        if constexpr (std::is_same_v<U, K1>) {
          val.data = 48;
        } else if constexpr (std::is_same_v<U, K2>) {
          val.data = 14.8f;
        } else if constexpr (std::is_same_v<U, K3>) {
          val.data = 'k';
        }
      });
  return std::get<K2>(var).data == 14.8f;
}

TEST_CASE("VahVariantNonDefaultConstructibleLambdaConstructAndUpdate") {
  using namespace csari::vah;
  auto const res = variantNonDefaultConstructibleLambdaConstructAndUpdate();
  REQUIRE(res);
}

constexpr auto variantConstexprStructAccessAndUpdate() -> bool {
  using namespace csari::vah;
  auto var =
      constructVariantFromIndexConstexpr<1, std::variant<float, int, char>>();
  std::get<int>(var) = 3;
  auto constexpr f = VariantConstexprPerformer{};
  performOnData(var, f);
  return std::get<int>(var) == 94;
}

TEST_CASE("VahVariantConstexprStructAccessAndUpdate") {
  using namespace csari::vah;
  auto constexpr res = variantConstexprStructAccessAndUpdate();
  REQUIRE(res);
}
