# Work tools
This repo contains tools that don't really belong anywhere. And some of mine
config files.

Some tools are mentioned here, others don't deserve it...

## Tools
These are helpers programs, to make your life easier.
Some of them are:

- tools/sqlite_parser - C# utility able to dump structure of sqlite file.
  Currently has only CLI, but in future I plan to add GUI
- tools/hex2bin - C utility to convert raw hex data to binary. This is the one
  tool, that is missing in all distros (actually this can be achieved with xxd
  I just didn't figure it out from its documentation)
- scratch/rasbi - this is work in progress on my own lisp like programming
  language



## Containers

- containers/debug-docker - Dockerfile to build image that only sleeps forever.
  Very useful for attaching to it and curling away
- containers/hello_world - Dockerfile + image that can be pulled from ghcr.
  Responds on port 80 with hello world
