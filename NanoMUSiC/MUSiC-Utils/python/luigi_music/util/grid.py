import re
import urlparse


def job_dir(job):
    
    job_node_id = job.node_id
    dir_name = 'grid-'+ str(job_node_id) 
    #Yannik changes MAIN    
    return dir_name
    ### END Yannik changes
    '''
    job_name = job.job_id
    if not job_name:
        raise ValueError("Job name not known (yet)")
    url_parts = urlparse.urlsplit( job_name, allow_fragments=False )
    assert url_parts.port is not None
    job_name_sanitized = "%s_%d_%s" % ( url_parts.hostname, url_parts.port, url_parts.path )
    job_name_sanitized = re.sub( r"[^a-zA-Z0-9_\.-]", "", job_name_sanitized )
    return  job_name_sanitized
	'''

def exit_code_from_info(info_dict):
    exit_code_str = info_dict.get( 'ExitCode', -1 )
    try:
        # sometimes exit_code_str is 'N/A'
        exit_code = int( exit_code_str )
    except ValueError:
        exit_code = -1
    return exit_code
