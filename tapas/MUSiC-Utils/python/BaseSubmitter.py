import gridlib

## Simplified base class which may be used for custom submitters
#
# This base class wraps several gridlib funtionalities and also reduces
# some of the possible options for task creation in favor of simplicity.
# The class is only intended for simple use cases and custom submitters
# for special usecases may use the task, storage_element, proxy_delegator
# classes directly
class BaseSubmitter( gridlib.se.GridpackManager ):

    ## The Object constructor
    #
    # @param site Name of distination site
    # @param mode Job/Task creation mode
    # @param scram_arch scram archetecture (uses currently used $SCRAM_ARCH if not set)
    # @param cmssw_version CMSSW version (uses currently sourced CMSSW version if not set)
    # @param delegation_id Proxy delegation id (auto created if not set)
    # @param gridpacks A list of gridpacks which should be added to all jobs / tasks
    def __init__( self,
                  site,
                  mode ='RECREATE',
                  scram_arch = None,
                  cmssw_version = None,
                  #delegation_id = None, #Yannik commented it out
                  executable = None,
                  executable_upload = True,
                  gridpacks = []
                ):
        storage_element = gridlib.se.get_se_instance( site )
        super(BaseSubmitter, self).__init__( storage_element )

        #if not delegation_id:
         #   proxy_delegator = gridlib.ce.ProxyDelegator( self.storage_element )#Yannik commented it out
          #  self.delegation_id = proxy_delegator.get_delegation_id()
        #else:
         #   self.delegation_id = delegation_id
        self.site = site
        self.mode = mode
        self.scram_arch = scram_arch
        self.cmssw_version = cmssw_version
        self.executable = executable
        self.executable_upload = executable_upload
        self.gridpacks = gridpacks

        # Container for classes
        self.tasks = []

    ## Create a new Job for with submitter settings
    #
    # The Jobs created by this function are automatically added to the list of tasks
    # @param self Object pointer
    # @param name The task name
    def create_task( self, name ):
        #task = gridlib.ce.Task(name,
          #                     mode = self.mode,
           #                    scram_arch = self.scram_arch,
            #                   cmssw_version=self.cmssw_version,       #Yannik commented it out
             #                  storage_element = self.storage_element,
              #                 delegation_id = self.delegation_id
               #                )
        task = gridlib.ce.Task(name,
                               mode = self.mode,
                               scram_arch = self.scram_arch,
                               cmssw_version=self.cmssw_version,
                               storage_element = self.storage_element
                               )       
        task.executable = self.executable
        task.executable_upload = self.executable_upload
        if self.gridpacks:
            for gp in self.gridpacks:
                task.add_gridpack( gp )
        self.tasks.append( task )
        return self.tasks[-1]
