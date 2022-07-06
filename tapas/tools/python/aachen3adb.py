import sys
import os
import logging
import functools
import subprocess
import time
import datetime
import dateutil.parser
import dateutil.tz
import cookielib
import json
import re
import requests
import collections

import gridlib.util

log = logging.getLogger(__name__)


## Management of the CERN SSO process
class CERNSingleSignOn():
    ## Default constructor
    #
    # @param self Object pointer
    # @param cookies_path Path to the SSO cookies file
    # @param certificate Path to CERN user certificate file
    # @param key Path to key file for CERN user certificate
    def __init__(self, cookies_path, certificate=None, key=None):
        self.cookies_path = os.path.abspath(os.path.expanduser(cookies_path))
        self.certificate = self.check_path(certificate)
        self.key = self.check_path(key)

        # Cookie cache
        self._cookiejar = None


    ## Validate that path points to file
    #
    # @param self Object pointer
    # @param path Path which to validate
    def check_path(self, path):
        # Check if path is supplied
        if not path:
            return None

        # Check if path exists
        path = os.path.abspath(os.path.expanduser(path))
        log.debug('File {} seems to exist, return path.'.format(path))
        if not os.path.exists(path):
            log.info('File {} does not exist, using kinit login.'.format(path))
            return None

        # Valid path
        return path

    ## Check if the Kerberos login is still valid
    #
    # Note that if the certificate and key are set, the login will return True,
    # as it is unnecessary.
    #
    # @param self Object pointer
    # @param min_seconds Minimal amount of seconds left for the login
    def kerberos_valid(self, min_seconds=7200):
        # Simply return True if Kerberos login is not necessary
        if self.certificate and self.key:
            return True

        # Call klist to determine Kerberos authentification status
        call = ['klist']
        process = subprocess.Popen(call, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True)
        stdout, stderr = process.communicate()

        # Compare current time to expiration date
        for line in reversed(stdout.split('\n')):
            if 'CERN.CH@CERN.CH' in line:
                line = line.split('  ')
                # Parse creation and expiration times
                creation = datetime.datetime.strptime(line[0], '%m/%d/%y %H:%M:%S')
                expiration = datetime.datetime.strptime(line[1], '%m/%d/%y %H:%M:%S')
                log.debug('Login from {} expires at {}'.format(
                    creation.strftime('%H:%M:%S %d-%m-%y'),
                    expiration.strftime('%H:%M:%S %d-%m-%y')
                ))
                # Determine remaining seconds
                remaining = int((expiration - datetime.datetime.now()).total_seconds())
                log.info('Time remaining for CERN login: {}h {}m'.format(
                    remaining / 3600,  # hours
                    remaining % 3600 / 60,  # minutes
                ))
                # Compare to minimal amount of seconds allowed
                return remaining > min_seconds

        # Fail the check if no valid authentification was found
        return False

    ## Call kinit for CERN authentification
    #
    # This function uses either the provided username or the environment
    # variable $CERNUSERNAME. If neither of those is set, it will raise an
    # exception.
    #
    # @param self Object pointer
    # @param username CERN username; Taken from if $CERNUSERNAME if None
    # @param retry Number of retries when user mistypes the password
    def kerberos_call(self, username=None, retry=3):
        # Retrieve username if not set
        if not username:
            username = gridlib.util.get_username_cern()

        call = ['kinit', username + '@CERN.CH']
        # Inform user and call kinit
        print('Calling kinit - Enter your CERN password')
        for _ in range(retry):
            process = subprocess.call(call)
            log.debug('Returncode of kinit: {}'.format(process))
            # Return true if successful
            if process == 0:
                return True
            # Retry if necessary
            print('Authentification failed, try again.')

        # Fail authentification attempt
        raise RuntimeError('Authentification failed.')

    ## Load the CERN SSO cookies
    #
    # This function loads the cookies from cookies_path defined in its parent
    # class.
    #
    # @param self Object pointer
    def cookies_load(self):
        # Load cookies (even expired ones)
        cookiejar = cookielib.MozillaCookieJar(self.cookies_path)
        cookiejar.load(ignore_discard=True, ignore_expires=True)
        # Cache cookies
        self._cookiejar = cookiejar

    ## Get CERN SSO cookies for a specific URL
    #
    # Get CERN SSO cookies for specific URL from Kerberos and 'reprocess' it
    # for general use.
    #
    # @param url URL for which to create the SSO cookies
    def cookies_create(self, url):
        log.debug('Creating CERN SSO cookies for {}'.format(url))
        if self.certificate and self.key:
            # Use exported CERN user certificate to authentificate
            call = [
                'cern-get-sso-cookie', '--reprocess',
                '--cert', self.certificate,
                '--key', self.key,
                '--url', url,
                '--outfile', self.cookies_path
            ]
        else:
            # Use Kerberos to authentificate
            call = [
                'cern-get-sso-cookie', '--reprocess', '--krb',
                '--url', url,
                '--outfile', self.cookies_path
            ]
        # Execute the call
        log.debug('I am launching the command {} .'.format(call))
        process = subprocess.Popen(call, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = process.communicate()
        # Check if retrieval was successful
        if process.returncode != 0:
            raise RuntimeError(
                'Cannot create cookies, authentification failed. \n{}\n{}'.format(
                    'stdout: ' + stdout,
                    'stderr: ' + stderr
                )
            )
        # Load newly created cookies and reduce maximum lifetime to 8h
        self.cookies_load()
        now = time.time()
        for cookie in self.cookies:
            if (cookie.name.startswith('_shibsession_')
                and cookie.expires - now > 8 * 60 * 60):
                cookie.expires = int(now + 8 * 60 * 60)
        self.cookies.save(ignore_discard=True, ignore_expires=True)
        return True

    ## Puts the cookies in a jar and returns it
    #
    # The CookieJar class object can be sent via a HTTP request, for example
    # using the python requests package. The CookieJar is also cached in this
    # object to avoid creating it multiple times.
    #
    # @param self Object pointer
    @property
    def cookies(self):
        if not self._cookiejar:
            self.cookies_load()
        return self._cookiejar

    ## Checks whether the Cookies in the object's CookieJar are still valid
    #
    # @param self Object pointer
    def cookies_valid(self):
        # Check if cookies exist
        if not os.path.exists(self.cookies_path):
            return False
        # Check expiration date of authorization cookie
        for cookie in self.cookies:
            if cookie.name.startswith('_shibsession_') and cookie.is_expired():
                return False
        return True


## A custom JSON encoder to implement unspecified formats
#
# This encoder is based on the default JSON encoder, but also allows for
# datetime object encoding. Datetime string representation is not specified in
# the JSON standard, which is the reason for the missing implementation in the
# standard library.
class CustomJSONEncoder(json.JSONEncoder):

    ## Default serialization for JSON fields
    #
    # This modified default serialzation allows for the conversion of Datetime
    # objects into strings (which is not specified in the JSON).
    #
    # @param self Object pointer
    # @param obj Object which to encode
    def default(self, obj):
        # Trim microseconds, add time zone and format datetime to ISO8601
        if isinstance(obj, datetime.datetime):
            if obj.microsecond:
                obj = obj.replace(microsecond=0)
            if not obj.tzinfo:
                obj = obj.replace(tzinfo=dateutil.tz.tzlocal())
            return obj.isoformat()
        # If not datetime, leave it up to the default encoder
        return json.JSONEncoder.default(self, obj)


## A hook to convert datetime strings to objects for the JSON decoder
#
# This is function is meant to be passed to the JSON library's load statement.
# Nested dictionaries (either directly or via lists) are parsed recursively.
#
# @param content JSON content which to decode
def datetime_hook(content):
    isoformat = re.compile('^[0-9]{4}-[0-9]{2}-[0-9]{2}'  # Date
                           'T[0-9]{2}:[0-9]{2}:[0-9]{2}(.[0-9]{6})?'  # Time
                           '\+[0-9]{2}:[0-9]{2}$')  # Time zone
    for k, v in content.items():
        # Convert strings matching ISO8601 format
        if isinstance(v, basestring) and isoformat.match(v):
            content[k] = dateutil.parser.parse(v)
        # Parse dict recursively
        elif isinstance(v, dict):
            content[k] = datetime_hook(v)
        # Parse dicts within list recursively
        elif isinstance(v, list):
            for i, item in enumerate(v):
                if isinstance(item, dict):
                    v[i] = datetime_hook(item)

    return content


## Exception when query yields no result
class EntryNotFoundException(Exception):
    # Pattern with which to check the general Exception's message
    pattern = 'No \w* entry found for:'


## Default exception to be raised for database errors
class DatabaseException(Exception):
    ## Default constructor
    #
    # @param self Object pointer
    # @param message Message which to display
    # @param status_code HTML status code
    # @param payload HTML response's JSON payload (as dictionary)
    def __init__(self, message, status_code=400, payload=None):
        Exception.__init__(self)
        # Raise different exceptions for particular cases
        if re.search(EntryNotFoundException.pattern, message):
            raise EntryNotFoundException(message)

        self.message = message
        self.status_code = status_code
        self.payload = payload

    ## String representation of the exception
    #
    # @param self Object pointer
    def __str__(self):
        s = 'HTTP Error {} - {}'.format(self.status_code, self.message)
        if self.payload:
            s += '\n{}'.format(self.payload)
        return s


## Decorator to catch and retry in case of an requests exception
#
# Tries to perform the command `tries` times. The inital attempt counts as the
# first try. Each subsequent try is exponentially delayed, starting with
# `delay`.
#
# @param tries Number of times to attempt to request
# @param delay Initial delay for subsequent requests in seconds
def retry(tries, delay=5):
    def decorator(func):
        @functools.wraps(func)
        def wrapper(*args, **kwargs):
            # Create local copies
            _tries, _delay = tries - 1, delay
            # Retry loop
            while _tries:
                try:
                    return func(*args, **kwargs)
                except (requests.exceptions.ConnectionError,
                        requests.exceptions.Timeout) as err:
                    log.warning('A {} occured, retrying in {} ...'.format(
                        type(err).__name__, _delay
                    ))
                    time.sleep(_delay)
                    _delay *= tries
                    _tries -= 1
            # Last function call without exception handling
            return func(*args, **kwargs)
        return wrapper
    return decorator


## Parses the python requests reponse object and handles potential errors
#
# If there were no issues, this decorator will return the JSON payload of the
# response object. In case an error has occured, a DatabaseException with the
# proper information is raised.
#
# @param func Function which to wrap
def validate_response(func):
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        # Call func
        response = func(*args, **kwargs)

        # Check if there is JSON content in the response
        if response.headers['Content-Type'] == 'application/json':
            payload = response.json(object_hook=datetime_hook)
        else:
            payload = None

        # Raise exception if request failed
        if not response.ok:
            # Use server message or assume internal server error
            if payload is not None and 'message' in payload:
                message = payload.pop('message')
            else:
                message = 'Internal server error'
            raise DatabaseException(message, response.status_code, payload)
        # Raise exception if request went well, but has no content
        elif response.ok and payload is None:
            raise DatabaseException('No JSON in response. Authorization failed.', 401)

        # Return payload if both response and payload are good
        return payload
    return wrapper


## Class managing the connection to the aachen3a-db
class DatabaseConnection():
    ## Authorize the user via his CERN login
    def __init__(self, url='https://cms-project-aachen3a-db.web.cern.ch',
                 username=None,
                 certificate='~/private/CERN_UserCertificate.pem',
                 key='~/private/CERN_UserCertificate.key',
                 cookies_path='~/.cache/tapas/sso-cookies.txt'):
        # CERN URL
        self.url = url
        self.username = username
        # Authentification files
        self.cookies_path = os.path.abspath(os.path.expanduser(cookies_path))
        self.certificate = certificate
        self.key = key
        # Persistent HTTP session, to be updated with auth cookies
        self._session = None
        # Default header for data transmission; consistent with en- and decode
        self.headers = {'Content-Type': 'application/json'}
        # columns cache
        self._columns = {}


    ## Session property
    #
    # Session is created once it is first requested
    #
    # @param self Object pointer
    @property
    def session(self):
        if not self._session:
            self._session = requests.Session()
        return self._session

    ## Delete the current session
    #
    # @param self Object pointer
    def clear_session(self):
        self._session = None

    ## Authorize the user via his CERN login
    #
    # @param self Object pointer
    # @param force_cookies Force the creation of cookies
    # @param retries Number of remaining retries, should the authorization fail
    def authorize(self, force_cookies=False, retries=1):
        # Create CERN authentification object
        sso = CERNSingleSignOn(self.cookies_path, self.certificate, self.key)

        # Ensure that login is valid
        if not sso.kerberos_valid():
            sso.kerberos_call(self.username)
        # Ensure that cookies are valid
        if force_cookies or not sso.cookies_valid():
            sso.cookies_create(self.url)

        # Load SSO cookies into persistent session and test connection
        self.session.cookies = sso.cookies
        if not self.test():
            if retries > 0:
                self.authorize(True, retries - 1)
            else:
                raise DatabaseException('Failed to establish connection')

        # Signal success
        return True

    ## Function decorator ensuring that authorization is available
    #
    # Note that his cannot be used outside of this class, as it uses class
    # instance variables.
    #
    # @param func Function which to wrap
    def ensure_auth(func):
        @functools.wraps(func)
        def wrapper(self, *args, **kwargs):
            # Check if cookie exists at all
            if not self.session.cookies:
                self.authorize()
            # Check expiration date of each cookie
            else:
                # Check expiration date of authorization cookie
                for cookie in self.session.cookies:
                    if cookie.name.startswith('_shibsession_') and cookie.is_expired():
                        self.authorize()
                        break
            # Execute the original function call
            return func(self, *args, **kwargs)

        return wrapper

    ## Test whether the session can connect successfully
    #
    # @param self Object pointer
    def test(self):
        # Check if cookies are valid
        if not self.session.cookies:
            return False
        # Execute test GET request
        url = self.url + '/api/v1.0/test'
        response = self.session.get(url, json={'connected': True})
        # Check if response was successful
        if not response.ok:
            return False
        # Check if response contains JSON header
        if response.headers['Content-Type'] != self.headers['Content-Type']:
            return False
        # Signal success
        return response.json()['connected']

    ## Encodes the data into the default transmission format
    #
    # The current implementation of the communication uses JSON. The
    # CustomJSONEncoder is utilized to allow for additional (in the JSON
    # standard unspecified) conversions.
    #
    # @param self Object pointer
    # @param data Data which to encode
    def encode(self, data):
        return json.dumps(data, cls=CustomJSONEncoder)

    ## Get columns for all sample and skim tables
    #
    # @param self Object pointer
    @retry(3)
    @validate_response
    @ensure_auth
    def get_columns(self):
        url = self.url + '/api/v1.0/columns'
        if not self._columns:
            self._columns = self.session.get(url)
        return self._columns

    ## Simple GET request / Dataset retrieval
    #
    # @param self Object pointer
    # @param entry Classname of the entry
    # @param name_or_id ID or name of the entry; Name is only valid for samples
    @retry(3)
    @validate_response
    @ensure_auth
    def get(self, entry, name_or_id):
        url = self.url + '/api/v1.0/{0}/{1}'.format(entry, name_or_id)
        # Execute GET request
        return self.session.get(url)

    ## Elaborate GET request / Dataset retrieval
    #
    # @param self Object pointer
    # @param data Dictionary containing the column filters
    # @param latest If True, returns only the latest sample + skim pair
    # @param limit Number of results, which to return
    # @param offset Number of results, which to skip before returning
    @retry(3)
    @validate_response
    @ensure_auth
    def get_query(self, data, latest=False, limit=None, offset=None):
        url = self.url + '/api/v1.0/query'
        # Specify latest OR limit and offset
        if latest:
            url += '/latest'
        elif limit is not None:
            url += '/{}/{}'.format(limit, offset if offset else 0)
        # Execute GET request
        return self.session.get(url,
                                data=self.encode(data),
                                headers=self.headers)

    ## Generate a list of query criteria from a dictionary
    #
    # Starting from a dict with {key[__op]: val, ...}, this function generates
    # a list query criteria dicts. Each criterion consists of a field, a value
    # and an operator. The latter can be specified by attaching its name to the
    # key after two underscores. For example {'deprecated__is': None}. If not
    # specified, the operator is inferred from the value and value type. The
    # final filter criteria list has the following shape
    # [{'field': key, # 'value': val, 'operator', 'eq'}, ...].
    #
    # @param entry type of object (same as column keys in get_columns)
    # @param criteria dict with one criterion per field key
    def format_criteria(self, entry, criteria):
        # Regular expression to extract the operator with
        op_regex = re.compile('__(\w*)$')

        ## Format key, value, operator dictionary
        def format_criterion(key, val, columns):
            # Determine if the operator is specified explicitly
            op = op_regex.search(key)
            # Trim operator from key
            if op:
                key = key[:op.start()]
                op = op.group(1)

            # Check for key validity
            if key not in columns:
                raise ValueError('{} is not a valid column'.format(key))

            # Infer operator if not yet set
            if not op:
                op = infer_operator(val, columns[key])
            # Return properly formatted filter dictionary
            return {'field': key, 'value': val, 'operator': op}

        ## Infer the operator depending on the value and value type
        def infer_operator(val, typestr):
            if typestr == 'list':
                # None or empty list
                if not val:
                    return 'eq'
                # All values in list
                else:
                    return 'all'
            # Wildcard matching
            elif typestr == 'str' and '%' in val:
                return 'like'
            # Simple comparison
            else:
                return 'eq'

        return [format_criterion(k, v, self.get_columns()[entry])
                for k, v in criteria.items()]

    ## Function to receive a prefilled query from criteria dicts
    #
    # Get a list of samples based on flat {field:value,..} criteria dicts.
    # All criteria are evaluated with the equal operator when no '%' wildcard
    # is used. The 'like' operator is used in this case.
    #
    # @param self Object pointer
    # @param sample_critiria Dictionary containing the criteria for samples
    # @param skim_criteria Dictionary containing the criteria for skims
    # @param data If True, sample and skims are assumed to be data
    def get_criteria_query(self,
                           sample_criteria={},
                           skim_criteria={},
                           data=False):
        # Choose type of query objects
        if data:
            sample_name = 'DataSample'
            skim_name = 'DataSkim'
        else:
            sample_name = 'MCSample'
            skim_name = 'MCSkim'
        # Construct query
        query = {
            'models': [sample_name, skim_name],
            'filters': {
                sample_name: self.format_criteria(sample_name, sample_criteria),
                skim_name: self.format_criteria(skim_name, skim_criteria)
            }
        }
        return query

    ## POST request / Dataset submission
    #
    # Note that there are two options for posting. If name_or_id and child are
    # None, the payload is inserted as a standalone entry. If both of them are
    # set, then the payload is inserted as a child of the entry denoted by
    # name_or_id.
    #
    # @param self Object pointer
    # @param payload Dictionary containing the content of the entry/child
    # @param entry Classname of the entry (in) which to insert
    # @param name_or_id If inserting as child, name or ID of the parent
    # @param child If inserting as child, classname of the child
    @retry(3)
    @validate_response
    @ensure_auth
    def post(self, payload, entry, name_or_id=None, child=None):
        # Ensure that not just one value is set
        if (name_or_id and not child) or (not name_or_id and child):
            raise DatabaseException('Parent ID AND child need to be defined.')

        # Append to existing entry as child or insert indepdent entry
        if name_or_id and child:
            url = self.url + '/api/v1.0/{0}/{1}/{2}'.format(entry,
                                                            name_or_id,
                                                            child)
        else:
            url = self.url + '/api/v1.0/{0}'.format(entry)
        # Execute POST request
        return self.session.post(url,
                                 data=self.encode(payload),
                                 headers=self.headers)

    ## PUT request / Dataset modification
    #
    # @param self Object pointer
    # @param entry Classname of the entry
    # @param name_or_id Name or ID of the entry; Name only valid for samples
    @retry(3)
    @validate_response
    @ensure_auth
    def put(self, payload, entry, name_or_id):
        url = self.url + '/api/v1.0/{0}/{1}'.format(entry, name_or_id)
        # Execute PUT request
        return self.session.put(url,
                                data=self.encode(payload),
                                headers=self.headers)

    ## DELETE request / Dataset deletion
    #
    # @param self Object pointer
    # @param entry Classname of the entry
    # @param name_or_id Name or ID of the entry; Name only valid for samples
    @retry(3)
    @validate_response
    @ensure_auth
    def delete(self, entry, name_or_id):
        url = self.url + '/api/v1.0/{0}/{1}'.format(entry, name_or_id)
        # Execute DELETE request
        return self.session.delete(url)


class ObjectDatabaseConnection(DatabaseConnection):

    ## Wrapper function to receive sample skim groups as objects
    #
    # Get a list of samples based on flat {field:value,..} criteria dicts.
    # All criteria are evaluated with the equal operator when no '%' wildcard
    # is used. The 'like' operator is used in this case.
    def get_objects(self, sample_criteria={}, skim_criteria={},
                    data=False, **kwargs):
        query = self.get_criteria_query(sample_criteria, skim_criteria, data)
        return self.query_objects(query, **kwargs)

    ## Wraper function to receive sample skim groups as objects
    #
    # This function returns a list of sample objects based on a given query
    # all matching skims are saved in the samples skims attribute.
    # Additional parameters for the get_query function can be added as kwargs
    def query_objects(self, query, **kwargs):
        # Check whether querying for MC or Data
        data = False if 'MCSample' in query['models'] else True
        # Execute the GET request
        res = self.get_query(query, **kwargs)
        # Reformat the results into class instances
        objects = []
        for sample_dict in res:
            if data:
                sample = DataSample(sample_dict,dbcon=self)
                sample.skims = [DataSkim(field_dict=skim,dbcon=self) for skim in sample.skims]
            else:
                sample = MCSample(sample_dict,dbcon=self)
                sample.skims = [MCSkim(field_dict=skim, dbcon=self) for skim in sample.skims]
            objects.append(sample)
        # Return the result's class instance objects
        return objects

class DatabaseEntryList(collections.MutableSet):
    def __init__(self, iterable=None):
        iterable = iterable if iterable is not None else []
        self._entries = []
        for entry in iterable:
            self.add(entry)


    def __repr__(self):
        return '{}([{}])'.format(self.__class__.__name__,
                                 ', '.join(e.__str__() for e in self._entries))

    def __contains__(self, entry):
        return any(entry.similar(e) for e in self._entries)

    def __iter__(self):
        return iter(self._entries)

    def add(self, entry):
        for i,existing in enumerate(self._entries):
            # Update item if a item is found with this id
            if existing.id and existing.id == entry.id:
                self._entries[i] = entry
                return

            # If it is a similar item exists, assume we update it
            if existing.similar(entry):
                entry.id = existing.id
                self._entries[i] = entry
                return
        self._entries.append(entry)

    def discard(self, entry):
        self._entries = [f for f in self._entries if entry.similar(f)]

    def __len__(self):
        return len(self._entries)


## Base class for database entry conversion
class DatabaseEntry(object):
    ## The object constructor
    #
    # Either create an object from a query result or initialize an empty one
    # with the field structure obtained from db
    # @param field_dict A object dictionary as returned by DatabseConnection queries
    # @param A existing instance of a DatabaseConnection object
    def __init__(self,
                 field_dict={},
                 dbid=None,
                 dbcon=None,
                 init_update=False,
                 **dbkwargs):
        # Set cache with passed db or keep it None (default)
        self._dbcon = dbcon
        self._dbkwargs = dbkwargs
        self._field_cache = None
        self._init_fields(dbid=dbid,
                          field_dict=field_dict,
                          init_update=init_update)
        # Define field names which can be used for similarity comparisons
        self.SIMILARITY_FIELDS = []

    def _init_fields(self,
                     dbid=None,
                     field_dict={},
                     init_update=False):
        # Always create the ID field
        self.id = dbid
        # Create the object from a field dict
        if field_dict:
            self._field_cache = field_dict
            for key in field_dict:
                setattr(self, key, field_dict[key])
        # Try to get object from db if only id passed
        elif dbid and init_update:
            fields_init = self.get()
        # Alternatively, Get field infos from db and init fields
        for field, ftype in self.fields.items():
            default = None if ftype != 'list' else []
            if not hasattr(self, field):
                setattr(self, field, default)

    @property
    def fields(self):
        fields = self.dbcon.get_columns()[self.entry_type]
        return fields

    @property
    def dbcon(self):
        if not self._dbcon:
            self._dbcon = DatabaseConnection(**self._dbkwargs)
        return self._dbcon

    @property
    def entry_type(self):
        return self.__class__.__name__

    ## Type of object either MC or Data
    @property
    def db_type(self):
        if 'MC' in self.entry_type:
            return 'MC'
        return 'Data'

    @property
    def is_updated(self):
        for field in self.fields:
            # Ensure field is not vetoed
            if self._is_vetoed(field):
                return False
            if not self._field_cache:
                return True
            # previously unset field is filled now
            if not field in self._field_cache and hasattr(self, field):
                return True
            # check if set field was updated
            if getattr(self, field) != self._field_cache[field]:
                return True
        return False

    def __repr__(self):
        return '<{} {}>'.format(self.__class__, self.__dict__)

    def __hash__(self):
        return hash(self.id)

    def __eq__(self, other):
        return self.id == other.id

    def update(self, update_dict):
        self.__dict__.update(update_dict)

    ## Check if this object was updated from databse
    #
    def get(self):
        if not self.id:
            return False
        try:
            payload = self.dbcon.get(self.entry_type, self.id)
            self._init_fields(field_dict=payload)
            return True
        except EntryNotFoundException:
            return False

    ## Check if this object was updated from databse
    #
    def exists(self):
        if not self.id:
            return False
        else:
            return True

    ## Check if all similarity defining fields are equal for another dbentry
    #
    #
    def similar(self, dbentry):
        def comp(d1, d2, f):
            return getattr(d1, f) == getattr(d2, f)
        return all([comp(self, dbentry, f) for f in self.SIMILARITY_FIELDS])

    def _is_vetoed(self, field, value=None):
        return False

    def _alter_field(self, field, value):
        return value

    ## Return a cleaned dictionary representation of the object
    #
    # This function removes meta information fields like the dbconnection or
    # other possibly user added attributes, which may interfere with the json
    # serialization
    def to_dict(self, keep_none=False):
        d = {}
        for field in self.fields:
            value = getattr(self, field)
            if self._is_vetoed(field, value):
                continue
            value = self._alter_field(field, value)
            if not field in self.fields:
                continue
            if value is None and not keep_none:
                continue
            d[field] = value
        return d

    ## Save the object to the database
    #
    def save(self, **kwargs):
        # Skip saving object if nothing was updated
        if not self.is_updated:
            return self.id
        # Save skims first and drop the if they exist
        if self.exists():
            self.dbcon.put(self.to_dict(), self.entry_type, self.id)
        else:
             res = self.dbcon.post(self.to_dict(), **kwargs)
             self.id = res['id']
        return self.id

    ## Add an item to a list field which represents a set
    #
    def add_set_item(self, field, value):
        if not isinstance(getattr(self,field), list):
            raise TypeError('Field %s is not a list' % (field))
        if not value in getattr(self,field):
            getattr(self,field).append(value)

    ## Remove an item from a list field which represents a set
    #
    def remove_set_item(self, field, value):
        if not isinstance(getattr(self,field), list):
            raise TypeError('Field %s is not a list' % field)
        if value in getattr(self,field):
            getattr(self,field).remove(value)


## Generic implementation of a DBSample
#
class DBSample(DatabaseEntry):
    def __init__(self, *args, **kwargs):
        super(DBSample, self).__init__(*args, **kwargs)
        self.SIMILARITY_FIELDS = ['name']

    def _is_vetoed(self, field, value=None):
        if field == 'skims':
            return True

    def save(self):
        self.id = super(DBSample,self).save(entry=self.entry_type)
        for skim in self.skims:
            if not skim:
                continue
            if not isinstance(skim, int):
                skim.sample_id = self.id
                skim.save()


## Generic implementation of a DBSkim
#
class DBSkim(DatabaseEntry):
    def __init__(self, *args, **kwargs):
        self._files = DatabaseEntryList()
        super(DBSkim, self).__init__(*args, **kwargs)
        self.SIMILARITY_FIELDS = ['datasetpath', 'version', 'globaltag']

    @property
    def files(self):
        return self._files

    @files.setter
    def files(self, value):
        # make sure objects in files are DBFile objects
        file_list = []
        for val in value:
            if not issubclass(DBFile, type(val)):
                kwargs = {'dbcon': self.dbcon}
                if type(val) is int:
                    kwargs['dbid'] = val
                if type(val) is dict:
                    kwargs['field_dict'] = val
                val = getattr(sys.modules[__name__],
                              self.db_type + 'File')(**kwargs)
            file_list.append(val)
        self._files = DatabaseEntryList(file_list)

    @files.deleter
    def files(self):
        self._files = DatabaseEntryList()

    def _alter_field(self, field, value):
        if field == 'files':
            return [f.to_dict() for f in self.files]
        return value

    def save(self):
        super(DBSkim, self).save(entry=self.db_type + 'Sample',
                                 name_or_id=self.sample_id,
                                 child=self.entry_type)

    @property
    def isdeprecated(self):
        return self.deprecated is not None

## Generic implementation of a DBFile
#
class DBFile(DatabaseEntry):
    def __init__(self, *args, **kwargs):
        super(DBFile, self).__init__(*args, **kwargs)
        self.SIMILARITY_FIELDS = ['path']

## A MCSample DB object
#
class MCSample(DBSample):
    pass

## A DataSample DB object
#
class DataSample(DBSample):
    pass

## A MCSkim DB object
#
class MCSkim(DBSkim):
    pass

## A DataSkim DB object
#
class DataSkim(DBSkim):
    def __init__(self, *args, **kwargs):
        super(DataSkim, self).__init__(*args, **kwargs)
        self.SIMILARITY_FIELDS += ['run_json']

## A MCFile DB object
#
class MCFile(DBFile):
    pass

## A MCFile DB object
#
class DataFile(DBFile):
    pass

class ObjectDatabaseConnection(DatabaseConnection):

    ## Wrapper function to receive sample skim groups as objects
    #
    # Get a list of samples based on flat {field:value,..} criteria dicts.
    # All criteria are evaluated with the equal operator when no '%' wildcard
    # is used. The 'like' operator is used in this case.
    def get_objects(self, sample_criteria={}, skim_criteria={},
                    data=False, **kwargs):
        query = self.get_criteria_query(sample_criteria, skim_criteria, data)
        return self.query_objects(query, **kwargs)

    ## Wrapper function to receive sample skim groups as objects
    #
    # This function returns a list of sample objects based on a given query
    # all matching skims are saved in the samples skims attribute.
    # Additional parameters for the get_query function can be added as kwargs
    def query_objects(self, query, **kwargs):
        # Check whether querying for MC or Data
        data = False if 'MCSample' in query['models'] else True
        # Execute the GET request
        res = self.get_query(query, **kwargs)
        # Reformat the results into class instances
        objects = []
        for sample_dict in res:
            if data:
                sample = DataSample(sample_dict,dbcon=self)
                sample.skims = [DataSkim(field_dict=skim,dbcon=self) for skim in sample.skims]
            else:
                sample = MCSample(sample_dict,dbcon=self)
                sample.skims = [MCSkim(field_dict=skim, dbcon=self) for skim in sample.skims]
            objects.append(sample)
        # Return the result's class instance objects
        return objects


if __name__ == '__main__':
    ## INITIALIZE ##
    logging.basicConfig(level=logging.DEBUG)
    dbcon = DatabaseConnection(
        # # Local server testing
        # url='http://lx3a48.physik.rwth-aachen.de:8080',
    )
    # dbcon.authorize()
    print dbcon.get_columns()
    # print dbcon.get('MCSkim', 10)

    ## FULL QUERY ##
    # data = {
    #     # Determine the order; Should start with Sample
    #     'models': ['MCSample', 'MCSkim'],
    #     # Unordered sets of filters
    #     'filters': {
    #         'MCSample': [
    #             {'field': 'name',
    #              'value': 'ZToMuMu_13TeV_PH_',
    #              'operator': 'startswith'},
    #             {'field': 'id',
    #              'value': 9,
    #              'operator': 'eq'},
    #             {'field': 'generator',
    #              'value': 'powheg',
    #              'operator': 'eq'}
    #         ],
    #         'MCSkim': [
    #             {'field': 'version',
    #              'value': 'CMSSW_8_0_v2.1',
    #              'operator': 'eq'},
    #             {'field': 'sites',
    #              'value': 'T2_DE_RWTH',
    #              'operator': 'any'}
    #         ]
    #     }
    # }
    # print dbcon.get_query(data, True)

    sys.exit(0)
