#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <csari/vah.hpp>
#include <sstream>

struct VariantConstexprPerformer final {
  constexpr VariantConstexprPerformer(char const cVal = 'c',
                                      float const fVal = 3.14f,
                                      int const iVal = 94)
      : cVal{cVal}, fVal{fVal}, iVal{iVal} {}

  constexpr void operator()(char& val) const { val = cVal; }
  constexpr void operator()(float& val) const { val = fVal; }
  constexpr void operator()(int& val) const { val = iVal; }
  char cVal;
  float fVal;
  int iVal;
};

TEST_CASE("VahVariantStructAccessAndUpdate") {
  using namespace csari::vah;
  using V = std::variant<float, int, char>;
  auto const indexRuntime = 1U;
  // initialize index 1 (integer) with default value
  auto var = constructVariantFromIndexRuntime<V>(indexRuntime);
  // Create a performer that will update the value depending on the data type on
  // runtime
  auto constexpr performer = VariantConstexprPerformer{};
  performOnData(var, performer);
  REQUIRE(std::get<int>(var) == 94);
}

TEST_CASE("VahVariantLambdaConstructAndUpdate") {
  using namespace csari::vah;
  using V = std::variant<int, float, char>;
  auto constexpr constructPerformer = VariantConstexprPerformer{'r', 6.28f, 98};
  // Constructor needs to know which type to construct
  auto constexpr VariantCharIndex = VariantIndex<V, char>;
  auto var = constructAndPerformOnData<V>(VariantCharIndex, constructPerformer);
  REQUIRE(std::get<char>(var) == 'r');

  auto constexpr updatePerformer = VariantConstexprPerformer{'k', 3.14f, 94};
  performOnData(var, updatePerformer);
  REQUIRE(std::get<char>(var) == 'k');
}

TEST_CASE("VahVariantNonDefaultConstructibleLambdaConstructAndUpdate") {
  using namespace csari::vah;
  using namespace csari::vah;

  struct KCtorArgs final {};

  struct K1 final {
    constexpr explicit K1(KCtorArgs){};
    int data{4};
  };
  struct K2 final {
    constexpr explicit K2(KCtorArgs){};
    float data{0.4f};
  };
  struct K3 final {
    constexpr explicit K3(KCtorArgs){};
    char data{'q'};
  };
  auto constexpr constructPerformer = [](auto& val) constexpr {
    using U = std::remove_reference_t<decltype(val)>;
    if constexpr (std::is_same_v<U, K1>) {
      val.data = 98;
    } else if constexpr (std::is_same_v<U, K2>) {
      val.data = 6.28f;
    } else if constexpr (std::is_same_v<U, K3>) {
      val.data = 'k';
    }
  };
  using V = std::variant<K1, K2, K3>;
  auto var = constructAndPerformOnData<V>(VariantIndex<V, K2>,
                                          constructPerformer, KCtorArgs{});
  performOnData(
      var, [](auto& val) constexpr {
        using U = std::remove_reference_t<decltype(val)>;
        if constexpr (std::is_same_v<U, K1>) {
          val.data = 48;
        } else if constexpr (std::is_same_v<U, K2>) {
          val.data = 14.8f;
        } else if constexpr (std::is_same_v<U, K3>) {
          val.data = 'k';
        }
      });
  REQUIRE(std::get<K2>(var).data == 14.8f);
}

TEST_CASE("VahVariantConstexprStructAccessAndUpdate") {
  using namespace csari::vah;
  auto var =
      constructVariantFromIndexConstexpr<1, std::variant<float, int, char>>();
  std::get<int>(var) = 3;
  auto constexpr f = VariantConstexprPerformer{};
  performOnData(var, f);
  REQUIRE(std::get<int>(var) == 94);
}

template <class V>
auto serializeVariantVector(std::vector<V> const& vecVar) -> std::string {
  auto ss = std::stringstream{};
  auto const fWriteStream = [&ss](auto const& val) {
    ss.write(reinterpret_cast<char const*>(&val), sizeof(val));
  };
  fWriteStream(size(vecVar));
  std::for_each(begin(vecVar), end(vecVar), [&fWriteStream](V const& var) {
    fWriteStream(var.index());
    csari::vah::performOnData(
        var, [&fWriteStream](auto& val) { fWriteStream(val); });
  });
  return ss.str();
}

template <class V>
auto loadVariantVector(std::string&& serializedData) -> std::vector<V> {
  auto ss = std::stringstream{std::forward<std::string>(serializedData)};
  auto const fReadStream = [&ss](auto& val) {
    ss.read(reinterpret_cast<char*>(&val), sizeof(val));
  };
  auto dataVectorLoaded = std::vector<V>{};
  auto nElements = std::size_t{};
  fReadStream(nElements);
  dataVectorLoaded.reserve(nElements);
  std::generate_n(std::back_inserter(dataVectorLoaded), nElements,
                  [&fReadStream] {
                    auto index = std::size_t{};
                    fReadStream(index);
                    return csari::vah::constructAndPerformOnData<V>(
                        index, [&fReadStream](auto& val) { fReadStream(val); });
                  });
  return dataVectorLoaded;
}

TEST_CASE("VahSerializationTest") {
  using V = std::variant<std::size_t, char>;
  auto const dataVector = std::vector<V>{std::size_t{42U}, 'a', 'b'};
  auto const dataVectorLoaded =
      loadVariantVector<V>(serializeVariantVector(dataVector));
  REQUIRE(size(dataVector) == size(dataVectorLoaded));
  REQUIRE(equal(begin(dataVector), end(dataVector), begin(dataVectorLoaded)));
}
