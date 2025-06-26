#include <iostream>
#include <type_traits>
#include <functional>
#include <string>
#include <sstream>
#include <vector>

// Utilities
//-----------------------------------------------------------------------------------------------------------

// Helper to detect std::function
template<typename>
struct is_std_function : std::false_type {};
template<typename T>
struct is_std_function<std::function<T>> : std::true_type {};

// Helper to detect if a type has operator()
template<typename T>
struct has_operator_parentheses {
private:
    template<typename U>
    static auto test(int) -> decltype(&U::operator(), std::true_type{});

    template<typename>
    static auto test(...) -> std::false_type
    {
        return std::false_type{};
    }

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

// Primary template
template<typename T, typename = void>
struct is_callable_object : std::false_type {};

// 1. Specialization for function pointers
template<typename Ret, typename... Args>
struct is_callable_object<Ret(*)(Args...), void> : std::true_type {};

// 2. Specialization for std::function
template<typename T>
struct is_callable_object<T, std::enable_if_t<is_std_function<T>::value>> : std::true_type {};

// 3. Specialization for lambdas and functors
template<typename T>
struct is_callable_object<T, std::enable_if_t<
    !is_std_function<T>::value &&       // Not a std::function
    !std::is_pointer_v<T> &&            // Not a pointer
    !std::is_function_v<std::remove_pointer_t<T>> && // Not a function type
    (has_operator_parentheses<T>::value || std::is_invocable_v<T>) // Has operator() or is invocable
    >> : std::true_type {};

template <typename T>
static constexpr bool is_callable_object_v = is_callable_object<T>::value;

template <typename T>
constexpr bool is_empty_callable(const T& v)
{
    static_assert(is_callable_object_v<T>);
    return false;
}

template <typename Tout, typename... Targs>
constexpr bool is_empty_callable(Tout(*_func_ptr)(Targs...))
{
    static_assert(is_callable_object_v<decltype(_func_ptr)>);
    return _func_ptr == nullptr;
}

template <typename Tout, typename... Targs>
bool TryInvoke(Tout(*_func_ptr)(Targs...))
{
    //static_assert(is_callable_object_v<decltype(_func_ptr)>);
    try
    {
        std::invoke(_func_ptr, std::forward<Targs>({})...);
        return true;
    }
    catch (std::bad_function_call)
    {
        std::cerr << "bad_function_call" << std::endl;
        return false;
    }
}

template <typename Tout, typename... Targs>
constexpr bool is_empty_callable(const std::function<Tout(Targs...)>& _func)
{
    static_assert(is_callable_object_v<std::remove_cvref_t<decltype(_func)>>);
    return !_func;
}

template <typename Tout, typename... Targs>
bool TryInvoke(const std::function<Tout(Targs...)>& _func)
{
    //static_assert(is_callable_object_v<decltype(_func)>);
    try
    {
        std::invoke(_func, std::forward<Targs>({})...);
        return true;
    }
    catch (std::bad_function_call)
    {
        std::cerr << "bad_function_call" << std::endl;
        return false;
    }
}

//-----------------------------------------------------------------------------------------------------------

namespace TestUnit
{
    //-----------------------------------------------------------------------------------------------------------
    // Lambdas to test
    //-----------------------------------------------------------------------------------------------------------

    template <typename Tout, typename... Args>
    static auto template_lambda = [](Args...) constexpr
    {
        if constexpr (!std::is_void_v<Tout>)
        {
            return Tout{};
        }
    };

    template <typename... Args>
    static auto noreturn_template_lambda = template_lambda<void, Args...>;

    static auto void_lambda = noreturn_template_lambda<>;
    static auto void_lambda_int = noreturn_template_lambda<int>;
    static auto void_lambda_bool = noreturn_template_lambda<bool>;
    static auto void_lambda_int_bool = noreturn_template_lambda<int, bool>;

    static auto int_lambda = template_lambda<int>;
    static auto int_lambda_int = template_lambda<int, int>;
    static auto int_lambda_bool = template_lambda<int, bool>;
    static auto int_lambda_int_bool = template_lambda<int, int, bool>;

    //-----------------------------------------------------------------------------------------------------------
    // Function pointers to test
    //-----------------------------------------------------------------------------------------------------------

    template <typename Tout, typename... Args>
    using template_func_pointer = Tout(*)(Args...);

    // Function pointers with no return to test
    template <typename... Args>
    using noreturn_template_func_pointer = template_func_pointer<void, Args...>;

    using void_func_ptr = noreturn_template_func_pointer<>;
    using void_func_ptr_int = noreturn_template_func_pointer<int>;
    using void_func_ptr_bool = noreturn_template_func_pointer<bool>;
    using void_func_ptr_int_bool = noreturn_template_func_pointer<int, bool>;

    using int_func_ptr = template_func_pointer<int>;
    using int_func_ptr_int = template_func_pointer<int, int>;
    using int_func_ptr_bool = template_func_pointer<int, bool>;
    using int_func_ptr_int_bool = template_func_pointer<int, int, bool>;

    // std::function to test
    template <typename Tout, typename... Args>
    using template_std_func = std::function<Tout(Args...)>;

    // std::function with no return to test
    template <typename... Args>
    using noreturn_template_std_func = template_std_func<void, Args...>;

    using void_std_func = noreturn_template_std_func<>;
    using void_std_func_int = noreturn_template_std_func<int>;
    using void_std_func_bool = noreturn_template_std_func<bool>;
    using void_std_func_int_bool = noreturn_template_std_func<int, bool>;
    using int_std_func = template_std_func<int>;
    using int_std_func_int = template_std_func<int, int>;
    using int_std_func_bool = template_std_func<int, bool>;
    using int_std_func_int_bool = template_std_func<int, int, bool>;

    //-----------------------------------------------------------------------------------------------------------
    // Static functions to test
    //-----------------------------------------------------------------------------------------------------------

    static void StaticVoidFunc()
    {
    }

    static void StaticVoidFuncInt(int)
    {
    }

    static void StaticVoidFuncBool(bool)
    {
    }

    static void StaticVoidFuncIntBool(int, bool)
    {
    }

    static int StaticIntFunc()
    {
        return 0;
    }

    static int StaticIntFuncInt(int)
    {
        return 0;
    }

    static int StaticIntFuncBool(bool)
    {
        return 0;
    }

    static int StaticIntFuncIntBool(int, bool)
    {
        return 0;
    }

    //-----------------------------------------------------------------------------------------------------------
    // Classes with operator() to test
    //-----------------------------------------------------------------------------------------------------------

    struct StructWithVoidParenthesesOperator
    {
    public:
        void operator()()
        {
        }
    };

    struct StructWithIntParenthesesOperator
    {
    public:
        int operator()()
        {
            return 0;
        }
    };

    //-----------------------------------------------------------------------------------------------------------
    // is_callable_object test
    //-----------------------------------------------------------------------------------------------------------

    static_assert(is_callable_object_v<decltype(void_lambda)>, "");
    static_assert(is_callable_object_v<decltype(void_lambda_int)>, "");
    static_assert(is_callable_object_v<decltype(void_lambda_bool)>, "");
    static_assert(is_callable_object_v<decltype(void_lambda_int_bool)>, "");
    static_assert(is_callable_object_v<decltype(int_lambda)>, "");
    static_assert(is_callable_object_v<decltype(int_lambda_int)>, "");
    static_assert(is_callable_object_v<decltype(int_lambda_bool)>, "");
    static_assert(is_callable_object_v<decltype(int_lambda_int_bool)>, "");

    static_assert(is_callable_object_v<decltype(+void_lambda)>, "");
    static_assert(is_callable_object_v<decltype(+void_lambda_int)>, "");
    static_assert(is_callable_object_v<decltype(+void_lambda_bool)>, "");
    static_assert(is_callable_object_v<decltype(+void_lambda_int_bool)>, "");
    static_assert(is_callable_object_v<decltype(+int_lambda)>, "");
    static_assert(is_callable_object_v<decltype(+int_lambda_int)>, "");
    static_assert(is_callable_object_v<decltype(+int_lambda_bool)>, "");
    static_assert(is_callable_object_v<decltype(+int_lambda_int_bool)>, "");

    static_assert(is_callable_object_v<void_func_ptr>, "");
    static_assert(is_callable_object_v<void_func_ptr_int>, "");
    static_assert(is_callable_object_v<void_func_ptr_int_bool>, "");
    static_assert(is_callable_object_v<int_func_ptr>, "");
    static_assert(is_callable_object_v<int_func_ptr_int>, "");
    static_assert(is_callable_object_v<int_func_ptr_int_bool>, "");

    static_assert(is_callable_object_v<void_std_func>, "");
    static_assert(is_callable_object_v<void_std_func_int>, "");
    static_assert(is_callable_object_v<void_std_func_int_bool>, "");
    static_assert(is_callable_object_v<int_std_func>, "");
    static_assert(is_callable_object_v<int_std_func_int>, "");
    static_assert(is_callable_object_v<int_std_func_int_bool>, "");

    static_assert(is_callable_object_v<decltype(&StaticVoidFunc)>, "");
    static_assert(is_callable_object_v<decltype(&StaticVoidFuncInt)>, "");
    static_assert(is_callable_object_v<decltype(&StaticVoidFuncIntBool)>, "");
    static_assert(is_callable_object_v<decltype(&StaticIntFunc)>, "");
    static_assert(is_callable_object_v<decltype(&StaticIntFuncInt)>, "");
    static_assert(is_callable_object_v<decltype(&StaticIntFuncIntBool)>, "");

    static_assert(is_callable_object_v<StructWithVoidParenthesesOperator>, "");
    static_assert(is_callable_object_v<StructWithIntParenthesesOperator>, "");

    static_assert(!is_callable_object_v<void>, "");
    static_assert(!is_callable_object_v<void*>, "");
    static_assert(!is_callable_object_v<bool>, "");
    static_assert(!is_callable_object_v<bool*>, "");
    static_assert(!is_callable_object_v<int>, "");
    static_assert(!is_callable_object_v<int*>, "");

    static_assert(!is_empty_callable(void_lambda), "");
    static_assert(!is_empty_callable(void_lambda_bool), "");
    static_assert(!is_empty_callable(void_lambda_int), "");
    static_assert(!is_empty_callable(void_lambda_int_bool), "");
    static_assert(!is_empty_callable(int_lambda), "");
    static_assert(!is_empty_callable(int_lambda_bool), "");
    static_assert(!is_empty_callable(int_lambda_int), "");
    static_assert(!is_empty_callable(int_lambda_int_bool), "");

    static_assert(!is_empty_callable(+void_lambda), "");
    static_assert(!is_empty_callable(+void_lambda_bool), "");
    static_assert(!is_empty_callable(+void_lambda_int), "");
    static_assert(!is_empty_callable(+void_lambda_int_bool), "");
    static_assert(!is_empty_callable(+int_lambda), "");
    static_assert(!is_empty_callable(+int_lambda_bool), "");
    static_assert(!is_empty_callable(+int_lambda_int), "");
    static_assert(!is_empty_callable(+int_lambda_int_bool), "");

    static_assert(is_empty_callable(void_func_ptr{}), "");
    static_assert(is_empty_callable(void_func_ptr_int{}), "");
    static_assert(is_empty_callable(void_func_ptr_bool{}), "");
    static_assert(is_empty_callable(void_func_ptr_int_bool{}), "");
    static_assert(is_empty_callable(int_func_ptr{}), "");
    static_assert(is_empty_callable(int_func_ptr_bool{}), "");
    static_assert(is_empty_callable(int_func_ptr_int{}), "");
    static_assert(is_empty_callable(int_func_ptr_int_bool{}), "");

    static void RunTestGeneric(const std::vector<std::function<bool()>> &test_collection)
    {
        std::stringstream ss;

        for (size_t i = 0; i < test_collection.size(); i++)
        {
            std::cout << "Test #" << +(i + 1) << ": ";

            const bool res = std::invoke(test_collection[i]);

            std::cout << (res ? "Pass" : "Fail") << std::endl;
        }
    }

    static void RunTest()
    {
        static const std::vector<std::function<bool()>> tests = {
            []() { return !is_empty_callable(void_std_func(void_lambda)); },
            []() { return !is_empty_callable(void_std_func_int(void_lambda_int)); },
            []() { return !is_empty_callable(void_std_func_bool(void_lambda_bool)); },
            []() { return !is_empty_callable(void_std_func_int_bool(void_lambda_int_bool)); },
            []() { return !is_empty_callable(int_std_func(int_lambda)); },
            []() { return !is_empty_callable(int_std_func_int(int_lambda_int)); },
            []() { return !is_empty_callable(int_std_func_bool(int_lambda_bool)); },
            []() { return !is_empty_callable(int_std_func_int_bool(int_lambda_int_bool)); },

            []() { return !is_empty_callable(void_std_func(&StaticVoidFunc)); },
            []() { return !is_empty_callable(void_std_func_int(&StaticVoidFuncInt)); },
            []() { return !is_empty_callable(void_std_func_bool(&StaticVoidFuncBool)); },
            []() { return !is_empty_callable(void_std_func_int_bool(&StaticVoidFuncIntBool)); },
            []() { return !is_empty_callable(int_std_func(&StaticIntFunc)); },
            []() { return !is_empty_callable(int_std_func_int(&StaticIntFuncInt)); },
            []() { return !is_empty_callable(int_std_func_bool(&StaticIntFuncBool)); },
            []() { return !is_empty_callable(int_std_func_int_bool(&StaticIntFuncIntBool)); },

            []() { return is_empty_callable(void_std_func{}); },
            []() { return is_empty_callable(void_std_func_int{}); },
            []() { return is_empty_callable(void_std_func_bool{}); },
            []() { return is_empty_callable(void_std_func_int_bool{}); },
            []() { return is_empty_callable(int_std_func{}); },
            []() { return is_empty_callable(int_std_func_int{}); },
            []() { return is_empty_callable(int_std_func_bool{}); },
            []() { return is_empty_callable(int_std_func_int_bool{}); },

            []() { return TryInvoke(void_std_func{}); },
            []() { return TryInvoke(void_std_func_int{}); },
            []() { return TryInvoke(void_std_func_bool{}); },
            []() { return TryInvoke(void_std_func_int_bool{}); },
            []() { return TryInvoke(int_std_func{}); },
            []() { return TryInvoke(int_std_func_int{}); },
            []() { return TryInvoke(int_std_func_bool{}); },
            []() { return TryInvoke(int_std_func_int_bool{}); }
        };

        RunTestGeneric(tests);
    }
};

int main()
{
    TestUnit::RunTest();

    return 0;
}
