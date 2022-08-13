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
        Modifier(Code pCode, bool pBatch) : code(pCode),
                                            batch_mode(pBatch)
        {
        }
        friend std::ostream &
        operator<<(std::ostream &os, const Modifier &mod)
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
}
