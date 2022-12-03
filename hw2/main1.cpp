#include <concepts>
#include <iostream>

namespace detail {
template <std::integral Int>
constexpr static int next_pow2(Int x) {
    [[assume(x >= 0)]];
    return x == 1 ? 1
                  : 1 << (CHAR_BIT * sizeof(Int) -
                          std::countl_zero(static_cast<std::make_unsigned_t<Int>>(x - 1)));
}
}  // namespace detail

template <typename T>
struct Vector {
public:
    Vector() : m_data{}, m_capacity{}, m_size{} {}
    Vector(int size) : Vector(size, T{}) {}
    Vector(int size, const T& value) {
        m_size = size;
        m_capacity = detail::next_pow2(size);
        m_data = new T[m_capacity];
        for (int i{0}; i < m_capacity; i++) {
            m_data[i] = value;
        }
    }
    ~Vector() {
        delete[] m_data;
    }

private:
    T* m_data;
    int m_capacity;
    int m_size;
};

int main() {
    Vector<int> a;
    Vector<std::string> b(10);
    Vector<double> c(10, 3.14);
}