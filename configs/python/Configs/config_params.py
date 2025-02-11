import ROOT
import os
import sys
from typing import Any, Tuple
from functools import partial

from pydantic.types import OptionalIntFloat
from .optional import Optional


def setter(value, obj, field):
    setattr(obj, field, value)
    return obj


def is_convertible_imp(type_py: str, type_cpp: str) -> Tuple[bool, str, str]:
    if type_py == type_cpp:
        return True, type_py, type_cpp

    match type_py:
        case "str":
            match type_cpp:
                case "string":
                    return True, type_py, type_cpp
                case _:
                    return False, type_py, type_cpp
        case "float":
            match type_cpp:
                case "double":
                    return True, type_py, type_cpp
                case _:
                    return False, type_py, type_cpp
        case _:
            return False, type_py, type_cpp


def is_convertible(obj_py: Any, type_cpp: str) -> Tuple[bool, str, str]:
    if type(obj_py).__name__ == "Optional":
        if not (type_cpp.startswith("optional")):
            return False, "Optional", type_cpp

        if obj_py.is_null():
            return True, "", ""

        return is_convertible_imp(
            type(obj_py.resolve()).__name__,
            type_cpp.replace("optional", "").replace("<", "").replace(">", ""),
        )

    if type(obj_py).__name__ == "list":
        if not (type_cpp.startswith("vector")):
            return False, "list", type_cpp

        if len(obj_py) == 0:
            return True, "", ""

        list_types = set()
        for item in obj_py:
            list_types.add(type(item).__name__)

        if len(list_types) == 1:
            return is_convertible_imp(
                list_types.pop(),
                type_cpp.replace("vector", "").replace("<", "").replace(">", ""),
            )

        return False, "List of Any", type_cpp

    type_py = type(obj_py).__name__
    return is_convertible_imp(type_py, type_cpp)


class ConfigParamsBuilder:
    def __init__(
        self,
        name: str,
        ConfigType,
    ):
        self.cp = ConfigType()
        self.config_type_name = type(self.cp).__name__
        self.name = name

        for field in dir(self.cp):
            if not field.startswith("__"):
                if not type(getattr(self.cp, field)).__name__ == "CPPOverload":
                    if field != "name":
                        setattr(self, field, None)
                        setattr(
                            self,
                            f"set_{field}",
                            partial(setter, obj=self, field=field),
                        )

    def update_name(self, name: str) -> "ConfigParamsBuilder":
        self.name = name

        return self

    def set_field(self, name: str, value: Any) -> "ConfigParamsBuilder":
        setattr(self, name, value)

        return self

    def build(self) -> "self.ConfigType":
        for field in dir(self.cp):
            if not field.startswith("__"):
                if not type(getattr(self.cp, field)).__name__ == "CPPOverload":
                    if getattr(self, field) == None:
                        print(
                            f'ERROR: Could not build {self.config_type_name}. "{field}", of type "{type(getattr(self.cp, field)).__name__}",  is not set.',
                            file=sys.stderr,
                        )
                        sys.exit(-1)
                    convertible, type_py, type_cpp = is_convertible(
                        obj_py=getattr(self, field),
                        type_cpp=type(getattr(self.cp, field)).__name__,
                    )
                    if not convertible:
                        print(
                            f'ERROR: Could not build {self.config_type_name}.\nField "{field}" is set with incompatible type. Set with "{type_py}", but should be "{type_cpp}".',
                            file=sys.stderr,
                        )
                        sys.exit(-1)

                    if isinstance(getattr(self, field), Optional):
                        setattr(self.cp, field, getattr(self, field).resolve())
                    else:
                        setattr(self.cp, field, getattr(self, field))

        return self.cp


ROOT.gROOT.ProcessLine(f".L {os.environ['MUSIC_BASE']}/lib/libConfigParams.so")
ConfigParamsCpp = ROOT.ConfigParams


# Custom types
GlobalParamsCpp = ROOT.GlobalParams
GlobalParamsBuilder = partial(ConfigParamsBuilder, ConfigType=GlobalParamsCpp)

JetParamsCpp = ROOT.JetParams
JetParamsBuilder = partial(ConfigParamsBuilder, ConfigType=JetParamsCpp)


if __name__ == "__main__":
    jcf = JetParamsBuilder("Jet Params").set_jet_config_a(123.0).build()
    print(jcf)

    print()

    gcf = (
        GlobalParamsBuilder("Global Params")
        .set_config_a(123.0)
        .set_jet_config(jcf)
        .set_config_b(123.0)
        .set_config_c(123)
        .set_config_d("asd")
        .build()
    )
    # gcf = GlobalParamsBuilder("foo").build()  # should fail

    gcf2 = (
        GlobalParamsBuilder("Global Params")
        .set_config_a(123.0)
        .set_jet_config(
            JetParamsBuilder("Jet Params 2").set_jet_config_a(9999.0).build(),
        )
        .set_config_b(123.0)
        .set_config_c(123)
        .set_config_d("asd")
        .build()
    )
    print(gcf2)
    # gcf = GlobalParamsBuilder("foo").build()  # should fail
