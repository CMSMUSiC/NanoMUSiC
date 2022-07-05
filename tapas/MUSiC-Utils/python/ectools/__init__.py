"""Core library to handle and process event classes in general and their representation as TEventClass root objects.

This module contains definitions to handle event classes represented by the TEventClass
object and abstract classes for the required type definitions to categorize and process larger sets of event classes.
The main functionality is implemented as a set of HelperClasses which provide a wide set of class methods to work
with TEventClass objects. The implementation in this module is expected to be fully generic and does not rely of
specific HEP specific definitions.

The helper classes are expected to be reimplemented for a specific generalized and added in the register module
(see the ecmusic module for an example for such an implementation)

"""
