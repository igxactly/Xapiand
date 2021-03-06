from datetime import date, datetime
from decimal import Decimal

try:
    import simplejson as json
except ImportError:
    import json
import msgpack
import uuid

from .exceptions import SerializationError, ImproperlyConfigured
from .compat import text_type, binary_type


class Serializer(object):
    def default(self, data):
        if isinstance(data, (date, datetime)):
            return data.isoformat()
        elif isinstance(data, Decimal):
            return float(data)
        elif isinstance(data, uuid.UUID):
            return str(data)
        raise TypeError("Unable to serialize %r (type: %s)" % (data, type(data)))


class TextSerializer(Serializer):
    mimetype = 'text/plain'

    def loads(self, s):
        return s.decode('utf-8')

    def dumps(self, data):
        if isinstance(data, text_type):
            return data.encode('utf-8')
        if isinstance(data, binary_type):
            return data
        raise SerializationError('Cannot serialize %r into text.' % data)


class MsgPackSerializer(Serializer):
    mimetype = 'application/x-msgpack'

    def loads(self, s):
        try:
            return msgpack.loads(s)
        except (ValueError, TypeError) as e:
            raise SerializationError(s, e)

    def dumps(self, data):
        if isinstance(data, text_type):
            return data.encode('utf-8')
        if isinstance(data, binary_type):
            return data
        try:
            return msgpack.dumps(
                data,
                default=self.default,
            )
        except (ValueError, TypeError) as e:
            raise SerializationError(data, e)


class JSONSerializer(Serializer):
    mimetype = 'application/json'

    def loads(self, s):
        try:
            return json.loads(s)
        except (ValueError, TypeError) as e:
            raise SerializationError(s, e)

    def dumps(self, data):
        if isinstance(data, text_type):
            return data.encode('utf-8')
        if isinstance(data, binary_type):
            return data
        try:
            return json.dumps(
                data,
                default=self.default,
                ensure_ascii=False,
                separators=(',', ':'),
            )
        except (ValueError, TypeError) as e:
            raise SerializationError(data, e)


DEFAULT_SERIALIZERS = {
    MsgPackSerializer.mimetype: MsgPackSerializer(),
    JSONSerializer.mimetype: JSONSerializer(),
    TextSerializer.mimetype: TextSerializer(),
}


class Deserializer(object):
    def __init__(self, serializers, default_mimetype='application/x-msgpack'):
        try:
            self.default = serializers[default_mimetype]
        except KeyError:
            raise ImproperlyConfigured('Cannot find default serializer (%s)' % default_mimetype)
        self.serializers = serializers

    def loads(self, s, mimetype=None):
        if not mimetype:
            deserializer = self.default
        else:
            # split out charset
            mimetype, _, _ = mimetype.partition(';')
            try:
                deserializer = self.serializers[mimetype]
            except KeyError:
                raise SerializationError('Unknown mimetype, unable to deserialize: %s' % mimetype)

        return deserializer.loads(s)
