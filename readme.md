# Runtime variant access helper

Single header only variant access helper. Useful for serializing or handling variant types with template specialization. Useful for factory code or replacing inheritance.

[![Build Status](https://csari.visualstudio.com/VariantAccessHelper/_apis/build/status/CihanSari.vah?branchName=master)](https://csari.visualstudio.com/VariantAccessHelper/_build/latest?definitionId=3&branchName=master)

# Example codes
## Serialization example
This example is just provided as a showcase and shouldn't be used in production. Use your own serialization library in combination of the vah for best results.

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
void variantConstructAndUpdate(std::size_t index = 1U) {
  using namespace csari::vah;
  using V = std::variant<float, int, char>;
  // initialize index 1 (integer) with default value
  auto var = constructVariantFromIndexRuntime<V>(index);
  performOnData(var, [](auto& val) constexpr {
        using U = std::remove_reference_t<decltype(val)>;
        if constexpr (std::is_same_v<U, float>) {
          val = 48;
        } else if constexpr (std::is_same_v<U, int>) {
          val = 14.8f;
        } else if constexpr (std::is_same_v<U, char>) {
          val = 'k';
        }
      });
}
```