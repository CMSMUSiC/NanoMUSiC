#include "ROOT/RVec.hxx"
#include <array>
#include <functional>
#include <iostream>
#include <utility>
#include <vector>

using namespace ROOT::VecOps;

struct foo
{
    std::reference_wrapper<const int> a;
    std::reference_wrapper<const RVec<float>> b;

    foo(const int &_a = 0, const RVec<float> &_b = {}) : a{_a}, b{_b}
    {
    }
};

// template <typename T>
// auto make_foo(T &&x)
// {
//     RVec<float> temp = {33., 55.};
//     return foo(std::forward<T>(x), temp);
// }

void dummy(const int &oi)
{

    // float n = 44.;
    // auto bar1 = foo(oi, n);
    // auto bar1 = make_foo(oi);

    RVec<float> aaa{6., 1., 5.};
    auto bar1 = foo(oi, aaa);
    std::cout << bar1.a << std::endl;
    std::cout << bar1.b.get() << std::endl;
    std::cout << (bar1.b.get() > 4) << std::endl;

    RVec<float> bbb{1, 2, 3};
    bar1.b = std::ref(bbb);
    std::cout << bar1.a << std::endl;
    std::cout << bar1.b.get() << std::endl;

    // // auto bar2 = foo();
    // // std::cout << bar2.a << std::endl;

    auto bar3 = bar1;
    std::cout << bar3.a << std::endl;
    std::cout << bar3.b.get() << std::endl;

    auto bar4 = foo();
    std::cout << bar4.a << std::endl;
    std::cout << bar4.b.get() << std::endl;

    // const int ppp = 44;
    // bar1.a = std::ref(ppp);
    // std::cout << bar1.a << std::endl;
    // std::cout << bar1.b.get() << std::endl;
    // std::cout << bar3.a << std::endl;
    // std::cout << bar3.b.get() << std::endl;

    // std::array<foo, 3> _arr;
    // for (auto &&f : _arr)
    // {
    //     std::cout << "--> " << f.a << std::endl;
    //     std::cout << "--> " << f.b.get() << std::endl;
    // }

    // _arr[0] = bar1;
    // _arr[2] = bar3;
    // for (auto &&f : _arr)
    // {
    //     std::cout << "### " << f.a << std::endl;
    //     std::cout << "### " << f.b.get() << std::endl;
    // }

    // // std::cout << foo(3).a << std::endl;
    // // auto bar = foo(oi);
    // // std::cout << bar.a << std::endl;
    // // std::cout << foo(3).a << std::endl;

    // // auto b = bar.a + 2;
    // // std::cout << b << std::endl;
}

int main()
{
    int oi = 99;
    dummy(oi);

    return 0;
}