import json
import luigi
import logging

logger = logging.getLogger('luigi-interface')

class StringListParameter(luigi.ListParameter):

    def parse(self, x):
        """
        Parse an individual value from the input.

        :param str x: the value to parse.
        :return: the parsed value.
        """
        logger.info("raw string to parse " + x)
        if not x.startswith("["):
            if "," in x:
                return list(json.loads(json.dumps(x.split(","))))
            return list(x.split(" "))
        logger.info(json.loads(x))
        return list(json.loads(x))


    def serialize(self, x):
        """
        Opposite of :py:meth:`parse`.

        Converts the value ``x`` to a string.

        :param x: the value to serialize.
        """
        return list(json.dumps(x))
