"""
The ectools module defined several classes for type definitions which are implemented in a generic way
generic and provide possible hooks for functions which are specific to a certain research target.
These classes are intended as abstract base classes to be inherited for a specific implementation of a generalized search.
"""

class ECTypeContainer(object):
    """Abstract base class for all EC type definition container classes.

    This class is the parent class for all type definitions.

    The class contains a member list called "fields" which controls which class members
    are considered when the object is serialized to a dictionary.
    """
    def __init__(self,
                 name,
                 name_tag=None,
                 latex_tag=None,
                 root_tag=None,
                 mpl_tag=None,
                 html_tag=None,
                 countable=True,
                 pluralize=False):
        """Constructor for ECTypeContainer objects

        Args:
            name: The name of the type objects
            name_tag: A short name tag for the type object
            latex_tag: A short name tag for the type object in latex
            latex_tag: A short name tag for the type object used in matplotlib plots
            html_tag: A short name tag for the type object used in html documents
            countable: Boolean to control if this object type can be counted or if its existence is binary
            pluralize: Boolean to indicate if the name of this object should be pluralized
        """
        self.name = name
        self.name_tag = name_tag if name_tag is not None else name
        self.latex_tag = latex_tag if latex_tag is not None else name_tag
        self.root_tag = root_tag if root_tag is not None else latex_tag
        self.mpl_tag = mpl_tag if mpl_tag is not None else latex_tag
        self.html_tag = html_tag if html_tag is not None else name_tag
        self.countable = countable
        self.pluralize = pluralize
        self.fields =  ["name",
                        "name_tag",
                        "latex_tag",
                        "root_tag",
                        "mpl_tag",
                        "html_tag",
                        "countable",
                        "pluralize",
                        ]
    def to_dict(self):
        """Serialize the object to a dictionary

        Returns:
            A dictionary containing all members defined in the fields member variable
        """
        return { f:getattr(self, f) for f in self.fields}

class ECClassType(ECTypeContainer):
    """Abstract base class for EC class types, which should be implemented for a
    specific search.

    """
    def __init__(self, name, name_tag, short_name=None, filter_func=None,  **kwargs):
        """Constructor for ECClassType objects

        Note:
            Calls the parent constructor for ECTypeContainer and adds additional fields

        Args:
            name: The name of the type objects.
            name_tag: A short name tag for the type object.
            short_name: A short name for the class type.
            filter_func: A function used to filter a set of class names to contain only classes of this class type.
            **kwargs: Arbitrary keyword arguments passed to parent constructor.
        """
        # tag is used in construction of class names seperated by a "+".
        # Empty tag is associated to all classes without "+" in name
        self.filter_func = filter_func
        self.short_name = short_name
        super(ECClassType, self).__init__(name, name_tag, **kwargs)
        self.fields.append("class_tag")
        self.fields.append("short_name")

    @property
    def class_tag(self):
        """Property for the current class tag

        Returns:
            A string with the current class_tag
        """
        if not self.name_tag:
            return ""
        else:
            return "+" + self.name_tag


class ECObjectType(ECTypeContainer):
    """Abstract base class for EC object types, i.e. the objects which define a event class """
    def __init__(self, name, name_tag, **kwargs):
        """Constructor for ECClassType objects

        Note:
            Calls the parent constructor for ECTypeContainer and adds additional fields

        Args:
            name: The name of the type objects.
            name_tag: A short name tag for the type object.
            **kwargs: Arbitrary keyword arguments passed to parent constructor.
        """
        super(ECObjectType, self).__init__(name, name_tag, **kwargs)


class ECGroupType(ECTypeContainer):
    """Abstract base class for EC object group types"""
    def __init__(self,
                 name,
                 name_tag,
                 filter_func=None,
                 sort_func=None,
                 description=None,
                 description_latex=None,
                 **kwargs):
        """Constructor for ECGroupType objects

        Note:
            Calls the parent constructor for ECTypeContainer and adds additional fields

        Args:
            name: The name of the type objects.
            name_tag: A short name tag for the type object.
            filter_func: A function used to filter a set of class names to contain only classes of this group type.
            sort_func: A function used to sort a set of class names for this group type.
            sort_func: A description for this object group type.
            **kwargs: Arbitrary keyword arguments passed to parent constructor.
        """
        self.filter_func = filter_func
        self.sort_func = None
        self.description = description
        if description_latex:
            self.description_latex = description_latex
        else:
            self.description_latex = description
        super(ECGroupType, self).__init__(name, name_tag, **kwargs)
        self.fields.append("description")
        self.fields.append("description_latex")

class ECDistributionType(ECTypeContainer):
    """Abstract base class for EC distribution types"""
    def __init__(self, name, name_tag, required_object_types=None,  **kwargs):
        """Constructor for ECDistributionType objects

        Note:
            Calls the parent constructor for ECTypeContainer and adds additional fields

        Args:
            name: The name of the type objects.
            name_tag: A short name tag for the type object.
            required_object_types: A list of ECObjectType names which are required in a class for a distribution to be present.
            **kwargs: Arbitrary keyword arguments passed to parent constructor.
        """
        if required_object_types is None:
            self.required_object_types = []
        else:
            self.required_object_types = required_object_types
        super(ECDistributionType, self).__init__(name, name_tag, **kwargs)
