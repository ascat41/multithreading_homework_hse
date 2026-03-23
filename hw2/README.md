# HW2 — BufferedChannel

Реализация класса:

```cpp
template <class T>
class BufferedChannel {
public:
    explicit BufferedChannel(int size);
    
    void Send(const T& value);           // Отправить значение в канал
    std::optional<T> Recv();             // Получить значение из канала
    void Close();                        // Закрыть канал
};
```

Сборка из buffered_channel:
```bash
cmake -S . -B build
cmake --build build  
```
