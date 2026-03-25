# HW1 — ApplyFunction

Реализация функции:

```cpp
template <typename T>
void ApplyFunction(std::vector<T>& data,
                   const std::function<void(T&)>& transform,
                   const int threadCount = 1);
```
Сборка из hw1:
```bash
cmake -S . -B build
cmake --build build  
```

Скриншот выполненных тестов находится в папке image_report
