#!/bin/sh

if [ $# != 2 ]; then
  echo "usage: $0 PORT_NUMBER" 1>&2
  exit 0
fi

./net_phone_rsa3 $1 $2 | play -t raw -b 16 -c 1 -e s -r 44100 -
