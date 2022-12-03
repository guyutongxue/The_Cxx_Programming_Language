# 大作业报告

本代码提供了：
- 类模板 `Function`
- 函数模板 `operator*(Function, Function)`
- 常量 `placeholder::_` `placeholder::_1` ... `placeholder::_8`

类模板 `Function` 提供对所有可调用对象（不含数据成员指针、成员函数）的包装。基于尚未进入标准的 P0201 `std::polymorphic_value` 提供的类型擦除机制。

函数模板 `operator*(Function, Function)` 提供 `Function` 之间的函数复合。如果右侧操作数返回 `std::tuple`，则支持多元（向量）复合。

命名空间 `placeholder` 中的常量，提供 `Function` 的实参重绑定。`placeholder::_` 提供基于默认顺序的重绑定。`placeholder::_N` 支持乱序的重绑定。

具体示例参见 `main.cpp`。

-----

本程序在 VS 2022 17.4，开启 `/std:c++latest` 下编译通过；内含 C++20 语法。