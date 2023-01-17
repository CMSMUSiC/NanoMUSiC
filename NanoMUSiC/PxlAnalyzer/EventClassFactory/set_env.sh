SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

FOLDER="$(echo $DIR | rev | cut -d'/' -f 1 | rev)"

export MYPXLANA="$FOLDER"
export LD_LIBRARY_PATH=$PXLANA/lib/:$LD_LIBRARY_PATH
export PATH=$PXLANA/EventClassFactory/scripts/:$PATH
