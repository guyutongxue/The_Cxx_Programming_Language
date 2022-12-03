#include <cassert>
#include <sstream>
#include <string>

#include "./function.hpp"

void naiveFunc(int) {}

void testAdaptability() {
    // 支持包装 原生函数、函数指针、Lambda 表达式、函数对象
    auto lambda = [] { return 42; };
    auto funcPtr = &naiveFunc;
    struct {
        std::unique_ptr<int> operator()(std::unique_ptr<int> x) {
            ++*x;
            return x;
        }
    } functorWithMoveOnlyArg;
    // 支持引用传参
    auto lambdaWithRef = [](int& x) { x++; };
    auto lambdaWithRRef = [](std::unique_ptr<int>&& x) {
        std::unique_ptr<int> temp = std::move(x);
    };

    Function<int()> a = lambda;
    Function<void(int)> b = naiveFunc;
    Function<void(int)> c = funcPtr;
    Function<std::unique_ptr<int>(std::unique_ptr<int>)> d =
        functorWithMoveOnlyArg;
    Function<void(int&)> e = lambdaWithRef;
    Function<void(std::unique_ptr<int> &&)> f = lambdaWithRRef;

    assert(a() == 42);
    b(42);
    c(42);
    auto ptr = std::make_unique<int>();
    ptr = d(std::move(ptr));
    assert(*ptr == 1);
    int zero{};
    e(zero);
    assert(zero == 1);
    f(std::move(ptr));
    assert(!ptr);
}

void testComposition() {
    // 一元复合
    Function<int(int, int)> add = [](int x, int y) { return x + y; };
    Function<int(int)> square = [](int x) { return x * x; };
    auto square_add = square * add;
    assert(square_add(1, 2) == (1 + 2) * (1 + 2));

    // 多元复合：f * g 中 g 返回元组，f 接受多参
    Function<std::tuple<int, const char*, int>(int, int)> div =
        [](int x, int r) { return std::make_tuple(x / r, "...", x % r); };
    Function<std::string(int, const char*, int)> toString = [](auto... s) {
        std::ostringstream ss;
        (ss << ... << s);
        return ss.str();
    };
    assert((toString * div)(123, 7) == "17...4");
}

void testBind() {
    using namespace placeholders;

    // 默认顺序的绑定
    Function<int(int, int)> minus = [](int x, int y) { return x - y; };
    Function<int(int)> minusFrom42 = minus(42, _);
    Function<int(int)> minus42 = minus(_, 42);
    Function<int(int, int)> minusCopy = minus(_, _);
    assert(minusFrom42(20) == 22);
    assert(minus42(56) == 14);
    assert(minusCopy(50, 15) == 35);

    // 多元绑定
    Function<std::string(int, char, int)> expr = [](int a, char b, int c) {
        return std::to_string(a) + b + std::to_string(c);
    };
    Function<std::string(int, int)> addExpr = expr(_, '+', _);
    assert(addExpr(1, 2) == "1+2");

    // 指定顺序的绑定
    Function<bool(int, int)> less = [](int x, int y) { return x < y; };
    Function<bool(int, int)> greater = less(_2, _1);
    assert(greater(5, 3));

    // 多重绑定
    Function<int(int, int, int, int)> sum4 = [](auto... xs) {
        return (... + xs);
    };
    Function<int(int, int, int)> sum3 = sum4(_1, 2000, _3, _2);
    Function<int(int, int)> sum2 = sum3(200, _2, _1);
    Function<int(int)> sum1 = sum2(_, 20);
    assert(sum1(2) == 2222);
}

int main() {
    testAdaptability();
    testComposition();
    testBind();
}