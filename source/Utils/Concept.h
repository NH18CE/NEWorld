#pragma once
struct Uncopyable {
    Uncopyable() = default;
    Uncopyable(const Uncopyable&) = delete;
    Uncopyable(Uncopyable&&) = default;
    Uncopyable& operator=(const Uncopyable&) = delete;
    Uncopyable& operator=(Uncopyable&&) = default;
};

class Singletion :Uncopyable {
protected:
    Singletion() = default;
public:
    Singletion(Singletion&&) = delete;
    Singletion& operator=(Singletion&&) = delete;
};

