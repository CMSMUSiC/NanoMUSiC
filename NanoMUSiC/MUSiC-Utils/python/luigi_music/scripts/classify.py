import luigi

from luigi_music.tasks import SampleClassificationTask

if __name__=="__main__":
    luigi.run( main_task_cls=SampleClassificationTask )
