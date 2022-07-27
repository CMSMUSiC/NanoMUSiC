#!/bin/bash

cd -P "$( dirname "${BASH_SOURCE[0]}" )"

LUIGI_CONFIG_PATH="$PWD/luigi.cfg"

LUIGI_PID_PATH="$PWD/work/luigid.pid"
LUIGI_LOG_PATH="$PWD/work/logs"

pid() {
    cat $LUIGI_PID_PATH
}

run() {
    if [ -z $CMSSW_BASE ]; then
        echo "Please source CMSSW before starting luigid." 1>&2
        return 1
    fi
    luigid --background --pidfile $LUIGI_PID_PATH --logdir $LUIGI_LOG_PATH
}

is_running() {
    if [ -f $LUIGI_PID_PATH ] && kill -0 $(pid) 2>/dev/null; then
        # running = success
        return 0
    else
        # not running = fail
        return 1
    fi
}

start() {
    if is_running; then
        echo "Luigid already running" >&2
    else
        run
    fi
}

stop() {
    if is_running; then
        PID=$(pid)
        kill -SIGTERM $PID
        while kill -0 $PID 2>/dev/null; do
            sleep 0.5
        done
    else
        echo "Luigid not running." >&2
    fi
}

status() {
    if is_running; then
        PID=$(pid)
        echo "Luigid is running at PID $PID."
        CMD=$(ps -o args= --pid $PID)
        echo "Command: $CMD"
        return 0
    else
        echo "Luigid is not running."
        return 1
    fi
}

case "$1" in
    start)
        start
        ;;
    stop)
        stop
        ;;
    restart)
        stop
        start
        ;;
    status)
        status
        ;;
    *)
        echo "Usage: $0 {start|stop|restart|status}"
esac

