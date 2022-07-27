import luigi

from luigi_music.tasks import EventClassPlotWrapperTask

if __name__=="__main__":
    luigi.run( main_task_cls=EventClassPlotWrapperTask )
