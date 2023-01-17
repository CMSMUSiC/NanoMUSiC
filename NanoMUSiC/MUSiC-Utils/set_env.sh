SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
    DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
    SOURCE="$(readlink "$SOURCE")"
    [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

export MUSIC_UTILS=$DIR
export LD_LIBRARY_PATH=$MUSIC_UTILS/lib/:$LD_LIBRARY_PATH
export PATH=$MUSIC_UTILS/scripts/:$PATH
export PYTHONPATH=$MUSIC_UTILS/python:$PYTHONPATH
export LUIGI_CONFIG_PATH=$MUSIC_UTILS/python/luigi_music/luigi.cfg
