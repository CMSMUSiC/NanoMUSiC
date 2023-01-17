#!/usr/bin/env bash
##
## setup.sh
## 
## This script sets up hooks and performs additional setup tasks for
## a repository in the TAPAS framework
##
## LICENSE
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
## 
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
## MA 02110-1301, USA.

# first get th dir where the install script is placed
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

## setup hooks

#################
## setup hooks ##
#################
if [ -d "$DIR/hooks/" ]; then
  # Control will enter here if $DIRECTORY exists.
  echo "dirctory hooks exists"
  if [ ! -f "$DIR/hooks/githookcontroller.py" ]; then
        $pwd = `pwd`
        cd "$DIR/hooks/"
        git pull
        cd $pwd
        setlinks=0
    else
        echo "githookcontroller not in hooks"
        setlinks=1
    fi
else
    # clone hookcontrollers
    git clone git@github.com:Aachen-3A/githookcontroller.git $DIR/hooks
    setlinks=0
fi

#check if path for doc is set
echo ${TAPASDOC:?"Need to set TAPASDOC environment variable !"}

if [ $setlinks -eq 0 ];then    
    # make sure all hooks can be executed
    chmod u+x $DIR/hooks/post-commit.py
    chmod u+x $DIR/hooks/pre-commit.py
    chmod u+x $DIR/hooks/pre-push.py
    
    # create symlinks for hooks
    ln -fs $DIR/hooks/githookcontroller.py $DIR/.git/hooks/githookcontroller.py
    
    ln -fs $DIR/hooks/pre-commit.py $DIR/.git/hooks/pre-commit
    ln -fs $DIR/hooks/post-commit.py $DIR/.git/hooks/post-commit
    ln -fs $DIR/hooks/pre-push.py $DIR/.git/hooks/pre-push
    
fi    
