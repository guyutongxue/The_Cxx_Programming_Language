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
struct PlaceHolder {
    static constexpr std::size_t value{N};
};

template <typename T>
struct IsPlaceHolder : std::false_type {};

template <std::size_t N>
struct IsPlaceHolder<PlaceHolder<N>> : std::true_type {};

template <typename T>
constexpr bool isPlaceHolder = IsPlaceHolder<std::remove_cvref_t<T>>::value;

template <typename... Args>
constexpr bool hasPlaceHolder = (... || isPlaceHolder<Args>);

namespace placeholders {
constexpr PlaceHolder<0> _;
constexpr PlaceHolder<1> _1;
constexpr PlaceHolder<2> _2;
constexpr PlaceHolder<3> _3;
constexpr PlaceHolder<4> _4;
constexpr PlaceHolder<5> _5;
constexpr PlaceHolder<6> _6;
constexpr PlaceHolder<7> _7;
constexpr PlaceHolder<8> _8;
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
    if constexpr (std::is_same_v<std::remove_cvref_t<Arg1>, PlaceHolder<0>>) {
        return std::tuple_cat(
            std::tuple{PlaceHolder<N>{}},
            translateGenericPlaceholder<N + 1>(std::forward<Args>(args)...));
    } else {
        static_assert(
            !isPlaceHolder<Arg1>,
            "Generic placeholder cannot be used with common placeholder.");
        return std::tuple_cat(
            std::tuple{std::forward<Arg1>(arg1)},
            translateGenericPlaceholder<N>(std::forward<Args>(args)...));
    }
}

template <typename R, typename... Args>
    requires(!hasPlaceHolder<Args...>)
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
        requires(hasPlaceHolder<BindArgs...>)
    auto operator()(BindArgs&&... args) {
        static_assert(sizeof...(BindArgs) == sizeof...(Args));
        static_assert((... && (std::convertible_to<BindArgs, Args> ||
                               isPlaceHolder<BindArgs>)));
        if constexpr ((... || (std::is_same_v<std::remove_cvref_t<BindArgs>,
                                              PlaceHolder<0>>))) {
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

    template <typename... BoundedArgs>
    struct ArgMap {
        std::tuple<BoundedArgs...> boundedArgs_;

        template <typename T>
        constexpr decltype(auto) map(T&& t) {
            if constexpr (isPlaceHolder<T>) {
                constexpr std::size_t phIdx = std::remove_cvref_t<T>::value;
                static_assert(phIdx != 0);
                return std::get<phIdx - 1>(std::move(boundedArgs_));
            } else {
                return std::forward<T>(t);
            }
        }
    };

    template <std::size_t N>
    constexpr decltype(auto) get(std::integral_constant<std::size_t, N>) {
        return std::get<N>(args_);
    }

    template <typename... CallArgs>
    R operator()(CallArgs&&... args) {
        return invoke(std::make_index_sequence<sizeof...(BindArgs)>{},
                      std::forward<CallArgs>(args)...);
    }

    template <typename... CallArgs, std::size_t... Idx>
    R invoke(std::index_sequence<Idx...>, CallArgs&&... cArgs) {
        return f_((ArgMap<CallArgs...>{{std::forward<CallArgs>(cArgs)...}}.map(
            get(std::integral_constant<std::size_t, Idx>{})))...);
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
