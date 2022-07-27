## @package gridmon
# This module is an api to the gridmon web service of rwth
#
import os
import pycurl
from StringIO import StringIO

class Gridmon:
    def __init__(self, jobid, passphrase, baseurl="https://grid-mon.physik.rwth-aachen.de/cgi-bin/jobmon/rwth-aachen/monitor.cgi", cacert="/etc/grid-security/certificates/GermanGrid.pem"):
        self.baseurl = baseurl
        self.cacert = cacert
        self.passphrase = passphrase
        self.jid = self._getLocalId(jobid)
    def _getLocalId(self, jobid):
        x = self._request("localid",jobid)
        return x.splitlines()[-1]
    def _request(self, action, jid):
        home = os.environ['HOME']
        buffer = StringIO()
        c = pycurl.Curl()
        c.setopt(c.URL, '{0}?command={1}&jid={2}'.format(self.baseurl, action, jid))
        c.setopt(c.WRITEFUNCTION, buffer.write)
        c.setopt(c.SSLCERT, home+'/.globus/usercert.pem')
        c.setopt(c.SSLCERTPASSWD, self.passphrase)
        c.setopt(c.SSLKEY, home+'/.globus/userkey.pem')
        if self.cacert is False:
            c.setopt(c.SSL_VERIFYPEER, 0)
        else:
            c.setopt(c.SSL_VERIFYPEER, 1)
            c.setopt(c.SSL_VERIFYHOST, 2)
            c.setopt(c.CAINFO, self.cacert)
        c.perform()
        c.close()
        return buffer.getvalue()
    def ps(self):
        if not self.jid.isdigit(): return "Job id not found in gridmon. Maybe the job is already finished."
        return self._request("ps",self.jid)
    def workdir(self):
        if not self.jid.isdigit(): return "Job id not found in gridmon. Maybe the job is already finished."
        return self._request("workdir",self.jid)
    def jobdir(self):
        if not self.jid.isdigit(): return "Job id not found in gridmon. Maybe the job is already finished."
        return self._request("jobdir",self.jid)
    def stdout(self):
        if not self.jid.isdigit(): return "Job id not found in gridmon. Maybe the job is already finished."
        return self._request("log",self.jid)
    def stderr(self):
        if not self.jid.isdigit(): return "Job id not found in gridmon. Maybe the job is already finished."
        return self._request("error",self.jid)
    def top(self):
        if not self.jid.isdigit(): return "Job id not found in gridmon. Maybe the job is already finished."
        return self._request("top",self.jid)

