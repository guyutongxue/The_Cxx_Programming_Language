#include <iostream>
#include <string>
#include <iterator>
#include <ranges>

template <typename T>
struct Vector {
public:
    Vector();
    Vector(int size);
    Vector(int size, const T& value);
    Vector(const Vector<T>& other);
    ~Vector();
    Vector<T>& operator=(const Vector<T>& other);
    const T& operator[](int index) const;
    T& operator[](int index);
    int size() const;
    void push_back(const T& element);
    void pop_back();

    using iterator = T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    struct safe_skip_iterator {
        const Vector& target;
        std::ptrdiff_t count{0};

        friend bool operator==(const safe_skip_iterator& lhs, const safe_skip_iterator& rhs) {
            return lhs.count == rhs.count;
        }
        safe_skip_iterator& operator++() {
            count += 2;
            return *this;
        }
        const T& operator*() {
            return target[count];
        }
    };

    iterator begin() const {
        return m_data;
    }

    iterator end() const {
        return m_data + m_size;
    }

    reverse_iterator rbegin() const {
        return reverse_iterator(m_data + m_size);
    }

    reverse_iterator rend() const {
        return reverse_iterator(m_data);
    }

    auto sbegin() {
        return stride_view().begin();
    }

    auto send() {
        return stride_view().end();
    }

    safe_skip_iterator ssbegin() const {
        return {*this, 0};
    }

    safe_skip_iterator ssend() const {
        return {*this, m_size + m_size % 2};
    }

    friend std::ostream& operator<<(std::ostream& out, const Vector& rhs) {
        out << "[ ";
        for (const auto& i : rhs) {
            out << i << " ";
        }
        return out << "]";
    }

private:
    T* m_data;
    int m_capacity;
    int m_size;

    auto stride_view() const {
        return *this | std::views::stride(2) | std::views::common;
    }
};

template <typename T>
Vector<T>::Vector() {
    m_data = nullptr;
    m_capacity = 0;
    m_size = 0;
}

template <typename T>
Vector<T>::Vector(int size) {
    m_data = new T[size];
    m_capacity = size;
    m_size = size;
}

template <typename T>
Vector<T>::Vector(int size, const T& value) {
    m_data = new T[size];
    for (int i = 0; i < size; i++) {
        m_data[i] = value;
    }
    m_capacity = size;
    m_size = size;
}

template <typename T>
Vector<T>::~Vector() {
    if (m_data) delete[] m_data;
}

template <typename T>
Vector<T>::Vector(const Vector<T>& other) {
    if (other.m_size > 0) {
        m_data = new T[other.m_size];
        m_size = other.m_size;
        m_capacity = other.m_size;
        for (int i = 0; i < m_size; i++) {
            m_data[i] = other.m_data[i];
        }
    } else {
        m_data = nullptr;
        m_capacity = 0;
        m_size = 0;
    }
}

template <typename T>
Vector<T>& Vector<T>::operator=(const Vector<T>& other) {
    if (m_data) delete[] m_data;
    if (other.m_size > 0) {
        m_data = new T[other.m_size];
        m_size = other.m_size;
        m_capacity = other.m_size;
        for (int i = 0; i < m_size; i++) {
            m_data[i] = other.m_data[i];
        }
    } else {
        m_data = nullptr;
        m_capacity = 0;
        m_size = 0;
    }
    return *this;
}

template <typename T>
const T& Vector<T>::operator[](int index) const {
    return m_data[index];
}

template <typename T>
T& Vector<T>::operator[](int index) {
    return m_data[index];
}

template <typename T>
int Vector<T>::size() const {
    return m_size;
}

template <typename T>
void Vector<T>::push_back(const T& element) {
    if (m_capacity > m_size) {
        m_data[m_size] = element;
        ++m_size;
    } else {
        ++m_size;
        int new_capacity = m_capacity * 2;
        if (new_capacity < m_size) new_capacity = m_size;
        T* new_data = new T[new_capacity];
        for (int i = 0; i < m_size - 1; i++) {
            new_data[i] = m_data[i];
        }
        new_data[m_size - 1] = element;
        m_capacity = new_capacity;
        delete[] m_data;
        m_data = new_data;
    }
}

template <typename T>
void Vector<T>::pop_back() {
    if (m_size > 0) --m_size;
}

using std::cout, std::endl;

int main() {
    Vector<int> a;
    Vector<std::string> b;

    for (int i = 0; i < 5; ++i) {
        a.push_back(i + 1);
    }

    for (int i = 0; i < 6; ++i) {
        b.push_back(std::to_string(i + 1));
    }

    // now a = { 1, 2, 3, 4, 5 }
    // now b = { "1", "2", "3", "4", "5", "6" }

    cout << "----------------" << endl;

    // 1. cout, ostream

    cout << "1. cout, ostream" << endl;

    cout << a << endl; // [ 1 2 3 4 5 ]
    cout << b << endl; // [ 1 2 3 4 5 6 ]

    cout << "----------------" << endl;

    cout << "2. begin, end" << endl;

    cout << "[ ";
    for (auto itor = a.begin(); itor != a.end(); ++itor) {
        cout << *itor << " "; // [ 1 2 3 4 5 ]
    }
    cout << "]" << endl;

    cout << "[ ";
    for (auto i : b) {
        cout << i << " "; // [ 1 2 3 4 5 6 ]
    }
    cout << "]" << endl;

    cout << "----------------" << endl;

    cout << "3. rbegin, rend" << endl;

    cout << "[ ";
    for (auto itor = a.rbegin(); itor != a.rend(); ++itor) {
        cout << *itor << " "; // [ 5 4 3 2 1 ]
    }
    cout << "]" << endl;

    cout << "----------------" << endl;

    cout << "4 sbegin, send" << endl;

    cout << "[ ";
    for (auto itor = a.sbegin(); itor != a.send(); ++itor) {
        cout << *itor << " "; // [ 1 3 5 ]
    }
    cout << "]" << endl;

    cout << "[ ";
    for (auto itor = b.sbegin(); itor != b.send(); ++itor) {
        cout << *itor << " "; // [ 1 3 5 ]
    }
    cout << "]" << endl;

    cout << "----------------" << endl;

    cout << "5. safe sbegin, send" << endl;

    Vector<int> dummy1 = a;
    cout << "[ ";
    for (auto itor = a.ssbegin(); itor != a.ssend(); ++itor) {
        a = dummy1;
        cout << *itor << " "; // [ 1 3 5 ]
    }
    cout << "]" << endl;

    Vector<std::string> dummy2 = b;
    cout << "[ ";
    for (auto itor = b.ssbegin(); itor != b.ssend(); ++itor) {
        for (int i = 0; i < 5; ++i) b.push_back(std::to_string(i));
        for (int i = 0; i < 5; ++i) b.pop_back();
        cout << *itor << " "; // [ 1 3 5 ]
    }
    cout << "]" << endl;

    cout << "----------------" << endl;

    return 0;
}