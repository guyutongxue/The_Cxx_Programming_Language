#include <concepts>
#include <stdexcept>
#include <tuple>
#include <type_traits>

#include "./polymorphic_value.h"

using namespace isocpp_p0201;

template <typename R, typename... Args>
class ICallable {
public:
    virtual R invoke(Args...) = 0;
    virtual ~ICallable() = default;
};

template <typename F, typename R, typename... Args>
class Callable : public ICallable<R, Args...> {
    using FImpl = std::decay_t<F>;
    static_assert(std::is_copy_constructible_v<FImpl>);

public:
    template <std::convertible_to<FImpl> F1>
    Callable(F1&& f) : f_{std::forward<F1>(f)} {}

    R invoke(Args... args) {
        return f_(std::forward<Args>(args)...);
    }

private:
    FImpl f_;
};

template <typename T>
struct ReturnType;

template <typename R, typename... Args>
struct ReturnType<R(Args...)> {
    using type = R;
};

template <std::size_t N>
struct Placeholder {
    static constexpr std::size_t value{N};
};

template <typename T>
struct IsPlaceholder : std::false_type {};

template <std::size_t N>
struct IsPlaceholder<Placeholder<N>> : std::true_type {};

template <typename T>
constexpr bool isPlaceholder = IsPlaceholder<std::remove_cvref_t<T>>::value;

template <typename... Args>
constexpr bool hasPlaceholder = (... || isPlaceholder<Args>);

namespace placeholders {
constexpr Placeholder<0> _;
constexpr Placeholder<1> _1;
constexpr Placeholder<2> _2;
constexpr Placeholder<3> _3;
constexpr Placeholder<4> _4;
constexpr Placeholder<5> _5;
constexpr Placeholder<6> _6;
constexpr Placeholder<7> _7;
constexpr Placeholder<8> _8;
}  // namespace placeholders

template <typename T>
class Function;

template <typename F, typename... BindArgs>
    requires(std::is_function_v<F>)
class Binder;

template <typename F, typename... BindArgs>
    requires(std::is_function_v<F>)
Binder<F, BindArgs...> makeBinder(Function<F>, std::tuple<BindArgs...>);

template <std::size_t N>
auto translateGenericPlaceholder() {
    return std::tuple<>{};
}

template <std::size_t N, typename Arg1, typename... Args>
auto translateGenericPlaceholder(Arg1&& arg1, Args&&... args) {
    if constexpr (std::is_same_v<std::remove_cvref_t<Arg1>, Placeholder<0>>) {
        return std::tuple_cat(
            std::tuple{Placeholder<N>{}},
            translateGenericPlaceholder<N + 1>(std::forward<Args>(args)...));
    } else {
        static_assert(
            !isPlaceholder<Arg1>,
            "Generic placeholder cannot be used with common placeholder.");
        return std::tuple_cat(
            std::tuple{std::forward<Arg1>(arg1)},
            translateGenericPlaceholder<N>(std::forward<Args>(args)...));
    }
}

template <typename R, typename... Args>
    requires(!hasPlaceholder<Args...>)
class Function<R(Args...)> {
public:
    template <typename F>
        requires(!std::is_same_v<std::remove_cvref_t<F>, Function<R(Args...)>>)
    Function(F&& f) {
        pf_ =
            make_polymorphic_value<ICallable<R, Args...>,
                                   Callable<F, R, Args...>>(std::forward<F>(f));
    }

    R operator()(Args... args) {
        if (!pf_) {
            throw std::runtime_error("Call to an empty Function!");
        }
        return pf_->invoke(std::forward<Args>(args)...);
    }

    template <typename... BindArgs>
        requires(hasPlaceholder<BindArgs...>)
    auto operator()(BindArgs&&... args) {
        static_assert(sizeof...(BindArgs) == sizeof...(Args));
        static_assert((... && (std::convertible_to<BindArgs, Args> ||
                               isPlaceholder<BindArgs>)));
        if constexpr ((... || (std::is_same_v<std::remove_cvref_t<BindArgs>,
                                              Placeholder<0>>))) {
            return makeBinder(*this, translateGenericPlaceholder<1>(
                                         std::forward<BindArgs>(args)...));
        } else {
            return makeBinder(*this, std::tuple<BindArgs...>{
                                         std::forward<BindArgs>(args)...});
        }
    }

private:
    polymorphic_value<ICallable<R, Args...>> pf_;
};

template <typename F, typename... BindArgs>
    requires(std::is_function_v<F>)
struct Binder {
    Function<F> f_;
    std::tuple<BindArgs...> args_;

    using R = typename ReturnType<F>::type;

    template <typename... CallArgs, typename T>
    constexpr decltype(auto) map(std::tuple<CallArgs...> callArgs, T&& t) {
        if constexpr (isPlaceholder<T>) {
            constexpr std::size_t phIdx = std::remove_cvref_t<T>::value;
            static_assert(phIdx != 0);
            return std::get<phIdx - 1>(std::move(callArgs));
        } else {
            return std::forward<T>(t);
        }
    }

    template <typename... CallArgs>
    R operator()(CallArgs&&... args) {
        return invoke(std::make_index_sequence<sizeof...(BindArgs)>{},
                      std::forward<CallArgs>(args)...);
    }

    template <typename... CallArgs, std::size_t... Idx>
    R invoke(std::index_sequence<Idx...>, CallArgs&&... cArgs) {
        return f_(map(std::tuple<CallArgs...>{std::forward<CallArgs>(cArgs)...},
                      std::get<Idx>(args_))...);
    }
};

template <typename F, typename... BindArgs>
    requires(std::is_function_v<F>)
Binder<F, BindArgs...> makeBinder(Function<F> f, std::tuple<BindArgs...> args) {
    return Binder<F, BindArgs...>{std::move(f), std::move(args)};
}

template <typename R2, typename R1, typename... Args>
Function<R2(Args...)> operator*(Function<R2(R1)> lhs,
                                Function<R1(Args...)> rhs) {
    return [lhs = std::move(lhs), rhs = std::move(rhs)](Args... args) mutable {
        return lhs(rhs(std::forward<Args>(args)...));
    };
}

template <typename F, typename T, std::size_t... Idx>
auto apply(F&& f, T&& t, std::index_sequence<Idx...>) {
    return std::forward<F>(f)(std::get<Idx>(std::forward<T>(t))...);
}

template <typename R2, typename... R1s, typename... Args>
Function<R2(Args...)> operator*(Function<R2(R1s...)> lhs,
                                Function<std::tuple<R1s...>(Args...)> rhs) {
    return [lhs = std::move(lhs), rhs = std::move(rhs)](Args... args) mutable {
        return apply(std::move(lhs), rhs(std::forward<Args>(args)...),
                     std::make_index_sequence<sizeof...(R1s)>{});
    };
}
