//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_TOKENIZER_H_
#define PXL_TOKENIZER_H_

#include "Pxl/Pxl/interface/pxl/core/macros.hpp"

#include <string>
#include <vector>

namespace pxl
{

class PXL_DLL_EXPORT Tokenizer
{
  public:
    enum CharType
    {
        WHITESPACE,
        ORDINARY,
        DELIM,
        LITERAL
    };

    Tokenizer();
    virtual ~Tokenizer();

    bool hasNext();
    std::string next();

    void setCharType(size_t begin, size_t end, CharType type);
    void setCharType(size_t chr, CharType type);
    void setText(const std::string &str);

  private:
    void findToken();

  private:
    size_t _pos;
    std::string _text;
    std::string _token;
    bool _tokenAvailable;
    std::vector<CharType> _types;
};

} // namespace pxl

#endif
