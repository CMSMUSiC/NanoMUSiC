import ROOT


class Optional:
    def __init__(self, opt_type=None, value=None):
        if value != None:
            self.value = value
            self.opt_type = type(self.value)

        else:
            if opt_type == None:
                raise ValueError("Can not infer type on Null.")
            self.opt_type = opt_type
            self.value = None

    def __str__(self):
        return str(self.value)

    @classmethod
    def Value(cls, value) -> "Optional":
        return cls(value=value)

    @classmethod
    def Null(cls, opt_type) -> "Optional":
        return cls(opt_type=opt_type, value=None)

    def has_value(self) -> bool:
        return self.value is not None

    def is_null(self) -> bool:
        return self.value is None

    def unwrap(self):
        if self.value is None:
            raise ValueError("Called unwrap on Null value.")
        return self.value

    def unwrap_or(self, default):
        return self.value if self.value is not None else default

    def resolve(self):
        return self.unwrap_or(ROOT.std.optional[self.opt_type]())

    def get_type(self):
        return type(self.opt_type)


if __name__ == "__main__":
    x = Optional.Value(5)
    y = Optional.Null()
    print(x.unwrap())  # 5
    print(y.unwrap_or(0))  # 0
    # print(y.unwrap())  # 0
