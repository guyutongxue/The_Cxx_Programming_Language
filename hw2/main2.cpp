#include <concepts>
#include <iostream>
#include <string>

namespace detail {
template <std::integral Int>
constexpr static int next_pow2(Int x) {
    [[assume(x > 0)]];
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
        if (size > 0) {
            m_capacity = detail::next_pow2(size);
            m_data = new T[m_capacity];
            for (int i{0}; i < m_capacity; i++) {
                m_data[i] = value;
            }
        } else {
            m_capacity = 0;
            m_data = nullptr;
        }
    }
    Vector(const Vector<T>& other) {
        m_size = other.m_size;
        m_capacity = other.m_capacity;
        if (m_capacity > 0) {
            m_data = new T[m_capacity];
            for (int i{0}; i < m_capacity; i++) {
                m_data[i] = other.m_data[i];
            }
        } else {
            m_data = nullptr;
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
    const Vector<int> a(10);
    Vector<int> b(a);
    const Vector<std::string> c;
    Vector<std::string> d(c);
    const Vector<std::string>& e = d;
    Vector<std::string> f(e);
}