// Original
// https://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal

#include <ostream>

namespace Color
{
enum Code
{
    FG_RED = 31,
    FG_GREEN = 32,
    FG_YELLOW = 33,
    FG_BLUE = 34,
    FG_CYAN = 36,
    FG_ACQUA = 27,
    FG_DEFAULT = 39,
    BG_RED = 41,
    BG_GREEN = 42,
    BG_BLUE = 44,
    BG_CYAN = 46,
    BG_DEFAULT = 49
};

class Modifier
{
    Code code;
    bool batch_mode;

  public:
    Modifier(Code pCode = Code::BG_DEFAULT, bool pBatch = true) : code(pCode), batch_mode(pBatch)
    {
    }
    friend std::ostream &operator<<(std::ostream &os, const Modifier &mod)
    {
        if (mod.batch_mode)
        {
            return os;
        }

        if (mod.code > 30)
        {
            return os << "\033[" << mod.code << "m"; // 8 bits
        }
        return os << "\033[38;5;" << mod.code << "m"; // 256 bits
    }
};

struct Colors
{
    Modifier yellow;
    Modifier green;
    Modifier blue;
    Modifier cyan;
    Modifier acqua;
    Modifier red;
    Modifier def;

    Colors(bool batch_mode)
    {
        yellow = Modifier(Code::FG_YELLOW, batch_mode);
        green = Modifier(Code::FG_GREEN, batch_mode);
        blue = Modifier(Code::FG_BLUE, batch_mode);
        cyan = Modifier(Code::FG_CYAN, batch_mode);
        acqua = Modifier(Code::FG_ACQUA, batch_mode);
        red = Modifier(Code::FG_RED, batch_mode);
        def = Modifier(Code::FG_DEFAULT, batch_mode);
    }
};
} // namespace Color
