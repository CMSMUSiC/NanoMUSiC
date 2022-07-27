import luigi

from luigi_music.tasks import dump_snapshot

if __name__ == "__main__":
    dump_snapshot("snapshot.cfg")

