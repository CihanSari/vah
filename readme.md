# Runtime variant access helper

Single header only variant access helper. Useful for serializing or handling variant types with template specialization. Useful for factory code or replacing inheritance.

[![Build Status](https://csari.visualstudio.com/VariantAccessHelper/_apis/build/status/CihanSari.vah?branchName=master)](https://csari.visualstudio.com/VariantAccessHelper/_build/latest?definitionId=3&branchName=master)

# Example codes
## Serialization example
```c++
#include <sstream>
#include <csari/vah.hpp>
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

void serializationExample() {
  using V = std::variant<std::size_t, char>;
  auto const dataVector = std::vector<V>{std::size_t{42U}, 'a', 'b'};
  auto const dataVectorLoaded =
      loadVariantVector<V>(serializeVariantVector(dataVector));
}
```

## Construction and update example
```cpp
#include <csari/vah.hpp>
void variantLambdaConstructAndUpdate() {
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
}
```