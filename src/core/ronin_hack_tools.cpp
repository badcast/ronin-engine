/**
// Hacking private member of class
// issue from
//
https://ru.stackoverflow.com/questions/1524726/%D0%9A%D0%B0%D0%BA-%D0%BF%D0%BE%D0%BB%D1%83%D1%87%D0%B8%D1%82%D1%8C-%D0%B4%D0%BE%D1%81%D1%82%D1%83%D0%BF-%D0%BA-%D0%BF%D1%80%D0%B8%D0%B2%D0%B0%D1%82%D0%BD%D0%BE%D0%BC%D1%83-%D1%87%D0%BB%D0%B5%D0%BD%D1%83-%D0%BA%D0%BB%D0%B0%D1%81%D1%81%D0%B0
template <typename TPtr>
struct HackGet {
    static inline TPtr ptr;
};

template <auto tptr>
struct HackAccess {
    static inline int dummy { (HackGet<decltype(tptr)>::ptr = tptr, 0) };
};

using namespace RoninEngine::Runtime;

template struct HackAccess<&Transform::m_name>;


template struct HackAccess<&Something::topSecretValue>;
template struct HackAccess<&Something::b>;

int main()
{
    Something something{};
    ::std::cout << something.*(HackGet<int Something::*>::ptr);
    std::cout << something.*(HackGet<float Something::*>::ptr);
}

*/
