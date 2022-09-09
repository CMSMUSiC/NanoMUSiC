#include "Pxl/Pxl/interface/pxl/core/Tokenizer.hh"

namespace pxl
{
Tokenizer::Tokenizer() : _pos(0), _tokenAvailable(false)
{
    _types.resize(256);
    setCharType(0x0, 0xFF, ORDINARY);
    setCharType(0x0, 0x20, WHITESPACE);
    setCharType(0x7F, 0xFF, WHITESPACE);
}

Tokenizer::~Tokenizer()
{
}

bool Tokenizer::hasNext()
{
    findToken();

    if (_tokenAvailable)
    {
        return true;
    }

    return false;
}

std::string Tokenizer::next()
{
    findToken();

    if (_tokenAvailable)
    {
        _tokenAvailable = false;
        return _token;
    }

    throw "No next token!";
}

void Tokenizer::setCharType(size_t begin, size_t end, CharType type)
{
    if (begin > 255)
    {
        begin = 0;
    }

    if (end > 255)
    {
        end = 255;
    }
    for (size_t i = begin; i <= end; ++i)
    {
        _types[i] = type;
    }
}

void Tokenizer::setCharType(size_t chr, CharType type)
{
    if (chr > 255)
    {
        chr = 255;
    }

    _types[chr] = type;
}

void Tokenizer::setText(const std::string &str)
{
    _text = str;
}

void Tokenizer::findToken()
{
    if (_pos >= _text.length() || _tokenAvailable)
    {
        return;
    }

    _token.clear();
    _tokenAvailable = false;
    bool literal = false;

    while (_pos < _text.length())
    {
        std::string::traits_type::char_type chr = _text.at(_pos);
        CharType type = _types[(size_t)chr];
        _pos++;

        if (type == LITERAL)
        {
            if (literal)
            {
                _tokenAvailable = true;
                literal = false;
            }
            else
            {
                literal = true;
                _token.clear();
            }
        }
        else if (type == ORDINARY || literal)
        {
            _token.append(1, chr);
            _tokenAvailable = true;
        }
        else if (type == DELIM)
        {
            if (literal)
            {
                _token.append(1, chr);
                _tokenAvailable = true;
            }
            else
            {
                _tokenAvailable = true;
                break;
            }
        }
        //		else if (_tokenAvailable && type == WHITESPACE)
        //		{
        //			break;
        //		}
    }
}

} // namespace pxl
